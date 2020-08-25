#include "internal.h"
#include "lcd.h"
#include "pin.h"
#include "timer.h"

namespace arpeggino
{

namespace state
{

struct : utils::Timer
{
    midier::Layer * layer = nullptr;
    unsigned char id;
} layer;

} // state

namespace io
{

utils::Timer flashing;

} // io

namespace viewer
{

struct : utils::Timer
{
    // query
    bool operator==(Viewer other) const { return _viewer == other; }
    bool operator!=(Viewer other) const { return _viewer != other; }

    // assignment
    void operator=(Viewer other) { _viewer = other; }

    // access
    void print(What what, How how) { _viewer(what, how); }

private:
    Viewer _viewer = nullptr;
} focused;

} // viewer

namespace component
{

struct Component
{
    checker::Checker checker;
    changer::Changer changer;
    viewer::Viewer viewer;
};

Component All[] =
    {
        { checker::BPM,     changer::BPM,       viewer::BPM     },
        { checker::Note,    changer::Note,      viewer::Note    },
        { checker::Mode,    changer::Mode,      viewer::Mode    },
        { checker::Octave,  changer::Octave,    viewer::Octave  },
        { checker::Perm,    changer::Perm,      viewer::Style   },
        { checker::Steps,   changer::Steps,     viewer::Style   },
        { checker::Rhythm,  changer::Rhythm,    viewer::Rhythm  },
    };

} // component

namespace control
{

void flash()
{
    if (io::flashing.ticking())
    {
        return; // already flashing
    }

    digitalWrite(pin::LED::Flash, HIGH);
    io::flashing.start();
}

namespace view
{

void summary(viewer::Viewer viewer = nullptr) // 'nullptr' means all components
{
    if (viewer::focused != nullptr) // some viewer is currently in focus
    {
        viewer::focused.stop(); // stop the timer
        viewer::focused = nullptr; // mark as there's no viewer currently in focus
        io::lcd.clear(); // clear the screen entirely
        viewer = nullptr; // mark to print all titles and values
    }

    if (viewer == nullptr)
    {
        for (const auto & component : component::All)
        {
            component.viewer(viewer::What::Title, viewer::How::Summary);
            component.viewer(viewer::What::Data, viewer::How::Summary);
        }

        // layers and bars

        io::lcd.setCursor(13, 0);

        char written = 0;

        if (state::layer.layer != nullptr)
        {
            written += io::lcd.print('L');
            written += io::lcd.print(state::layer.id);
        }

        while (written++ < 3)
        {
            io::lcd.write(' ');
        }
    }
    else
    {
        viewer(viewer::What::Data, viewer::How::Summary);
    }
}

void focus(viewer::Viewer viewer)
{
    if (viewer::focused != viewer) // either in summary mode or another viewer is currently in focus
    {
        io::lcd.clear(); // clear the screen entirely
        viewer::focused = viewer; // mark this viewer as the one being in focus
        viewer::focused.print(viewer::What::Title, viewer::How::Focus); // print the title (only if just became the one in focus)
    }

    viewer::focused.print(viewer::What::Data, viewer::How::Focus); // print the data anyways
    viewer::focused.start(); // start the timer or restart it if ticking already
}

void bar(midier::Sequencer::Bar bar)
{
    io::lcd.setCursor(14, 0);

    char written = 0;

    if (bar != midier::Sequencer::Bar::None)
    {
        written = io::lcd.print((unsigned)bar);
    }

    while (written++ < 2)
    {
        io::lcd.write(' ');
    }
}

} // view

namespace config
{

void layer(midier::Layer * layer, unsigned char id) // `nullptr` means go back to global
{
    if (state::layer.layer == nullptr && layer == nullptr)
    {
        return; // nothing to do
    }

    // we allow setting the same layer for updating its config and the timer

    state::layer.layer = layer;
    state::layer.id = id;

    if (layer == nullptr)
    {
        // increase the volume of all layers
        state::sequencer.layers.eval([](midier::Layer & layer)
            {
                layer.velocity = midier::midi::Velocity::High;
            });

        state::layer.stop(); // stop the timer
        state::config = &state::sequencer.config; // point to global configuration
    }
    else
    {
        // lower the volume of all layers
        state::sequencer.layers.eval([](midier::Layer & layer)
            {
                layer.velocity = midier::midi::Velocity::Low;
            });

        // increase the volume of the selected layer
        state::layer.layer->velocity = midier::midi::Velocity::High;

        state::layer.start(); // start ticking
        state::config = layer->config.view(); // point to this layer's configuration
    }

    control::view::summary();
}

void global()
{
    layer(nullptr, 0);
}

} // config

} // control

namespace handle
{

void flashing()
{
    if (io::flashing.elapsed(70))
    {
        digitalWrite(pin::LED::Flash, LOW);
        io::flashing.stop();
    }
}

void recording()
{
    static bool __recording = false;

    const auto recording = state::sequencer.recording(); // is recording at the moment?

    if (__recording != recording)
    {
        digitalWrite(pin::LED::Record, recording ? HIGH : LOW);
        __recording = recording;
    }
}

void focus()
{
    if (viewer::focused.elapsed(3200))
    {
        state::layer.reset(); // restart the layer timer

        control::view::summary(); // go back to summary view
    }
}

void components()
{
    // components will update the configuration on I/O events

    for (const auto & component : component::All)
    {
        const auto action = component.checker();

        if (action == checker::Action::None)
        {
            continue; // nothing to do
        }

        const auto layered = (state::layer.layer != nullptr) && (component.checker != checker::BPM); // all components are per layer but BPM

        if (layered)
        {
            state::layer.start(); // start ticking
        }

        // update the configuration only if in summary mode or if this configurer is in focus

        if ((action == checker::Action::Summary && viewer::focused == nullptr) ||
            (action == checker::Action::Focus && viewer::focused == component.viewer))
        {
            if (layered && state::layer.layer->config.outer())
            {
                // the selected layer should now detach from the global configuration as
                // it is being configured specifically.
                state::layer.layer->config = state::sequencer.config; // deep copy the global configuration

                // we also need to point to the configuration of this layer
                state::config = state::layer.layer->config.view();
            }

            component.changer();
        }

        if (action == checker::Action::Summary)
        {
            control::view::summary(component.viewer);
        }
        else if (action == checker::Action::Focus)
        {
            control::view::focus(component.viewer);
        }
    }
}

void keys()
{
    // we extend `controlino::Key` so we could hold a Midier handle with every key
    struct Key : controlino::Key
    {
        Key(char pin) : controlino::Key(io::multiplexer, pin) // keys are behind the multiplexer
        {}

        midier::Sequencer::Handle h;
    };

    static Key __keys[] = { 15, 14, 13, 12, 11, 10, 9, 8 }; // channel numbers of the multiplexer

    for (auto i = 0; i < sizeof(__keys) / sizeof(Key); ++i)
    {
        auto & key = __keys[i];

        const auto event = key.check();

        if (event == Key::Event::None)
        {
            continue; // nothing has changed
        }

        if (event == Key::Event::Down) // a key was pressed
        {
            control::config::global(); // go back to global configutarion when playing new layers

            key.h = state::sequencer.start(i + 1); // start playing an arpeggio of the respective scale degree
        }
        else if (event == Key::Event::Up) // a key was released
        {
            state::sequencer.stop(key.h); // stop playing the arpeggio
        }
    }
}

void record()
{
    const auto event = io::Record.check();

    if (event == controlino::Button::Event::Click)
    {
        state::sequencer.record();
    }
    else if (event == controlino::Button::Event::Press)
    {
        if (state::layer.layer == nullptr)
        {
            state::sequencer.revoke(); // revoke the last recorded layer as no layer is selected
        }
        else
        {
            state::layer.layer->revoke(); // revoke the selected layer
        }
    }
    else if (event == controlino::Button::Event::ClickPress)
    {
        state::sequencer.wander();
    }
    else
    {
        return;
    }

    control::config::global(); // go back to global configuration
}

void layer()
{
    if (state::layer.elapsed(6000))
    {
        control::config::global(); // go back to global configuration after 6 seconds
    }
    else
    {
        const auto event = io::Layer.check();

        if (event == controlino::Button::Event::Click) // iterate layers
        {
            if (viewer::focused != nullptr)
            {
                state::layer.reset(); // reset the layer timer only if there's one selected currently

                control::view::summary(); // go back to summary view
            }
            else
            {
                static const auto __count = state::sequencer.layers.count();

                static unsigned char __index = 0;

                if (state::layer.layer == nullptr || __index >= __count)
                {
                    __index = 0; // search from the start again
                }

                midier::Layer * layer = nullptr;

                while (__index < __count)
                {
                    midier::Layer & prospect = state::sequencer.layers[__index++];

                    if (prospect.running())
                    {
                        layer = &prospect;
                        break;
                    }
                }

                if (layer == nullptr)
                {
                    control::config::global();
                }
                else
                {
                    control::config::layer(layer, __index);
                }
            }
        }
        else if (event == controlino::Button::Event::Press)
        {
            if (state::layer.layer != nullptr) // a layer is selected
            {
                if (state::layer.layer->config.inner())
                {
                    // we make it point to the global configuration
                    state::layer.layer->config = state::config = &state::sequencer.config;

                    // reset the timer
                    state::layer.reset();

                    // print the new (global) configuration
                    control::view::summary();
                }
            }
            else // no layer is selected
            {
                // making all previous dynamic layers static

                state::sequencer.layers.eval([](midier::Layer & layer)
                    {
                        if (layer.config.outer())
                        {
                            layer.config = state::sequencer.config; // make it static and copy the current global configuration
                        }
                    });
            }
        }
        else if (event == controlino::Button::Event::ClickPress)
        {
            // set all layers to be dynamically configured

            state::sequencer.layers.eval([](midier::Layer & layer)
                {
                    layer.config = &state::sequencer.config;
                });

            control::config::global();
        }
    }
}

void click()
{
    const auto bar = state::sequencer.click(midier::Sequencer::Run::Async);

    if (bar != midier::Sequencer::Bar::Same)
    {
        control::flash();

        if (viewer::focused == nullptr && state::layer.layer == nullptr)
        {
            control::view::bar(bar);
        }
    }
}

} // handle

extern "C" void setup()
{
    // initialize the Arduino "Serial" module and set the baud rate
    // to the same value you are using in your software.
    // if connected physically using a MIDI 5-DIN connection, use 31250.
    Serial.begin(9600);

    // initialize the LEDs
    pinMode(pin::LED::Flash, OUTPUT);
    pinMode(pin::LED::Record, OUTPUT);

    // initialize the LCD
    io::lcd.begin(16, 2);

    // print the initial configuration
    control::view::summary();
}

extern "C" void loop()
{
    handle::flashing();
    handle::recording();
    handle::focus();
    handle::components();
    handle::keys();
    handle::record();
    handle::layer();
    handle::click();
}

} // arpeggino
