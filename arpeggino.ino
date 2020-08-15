#include "internal.h"
#include "timer.h"

namespace arpeggino
{

namespace
{

// // the # of layers affect the global variable size
// // this amount of layers requires ~75% of the maximum allowed
// midier::Layers<35> __layers(config::global);
// midier::Sequencer __sequencer(__layers);

struct Component
{
    configurer::Base & configurer;
    viewer::Base & viewer;
};

Component __components[] = {
    { configurer::__BPM,    viewer::__BPM },
    { configurer::__Mode,   viewer::__Mode },
    { configurer::__Note,   viewer::__Note },
    { configurer::__Octave, viewer::__Octave },
    { configurer::__Perm,   viewer::__Style },
    { configurer::__Steps,  viewer::__Style },
    { configurer::__Rhythm, viewer::__Rhythm },
};

struct : Timer
{
    viewer::Base * viewer = nullptr;
} __focused;

Timer __flashing;

struct : Timer
{
    midier::Layer * layer = nullptr;
    unsigned char id;
} __layer;

namespace control
{

namespace ui
{

void flash()
{
    if (__flashing.ticking())
    {
        return; // already flashing
    }

    digitalWrite(pin::LED::Flash, HIGH);
    __flashing.start();
}

} // ui

namespace view
{

void summary(viewer::Base * viewer = nullptr) // 'nullptr' means all components
{
    if (__focused.viewer != nullptr) // some viewer is in focus currently
    {
        __focused.stop();
        __focused.viewer = nullptr;
        __lcd.clear();

        viewer = nullptr; // mark to print all titles and values
    }

    if (viewer == nullptr)
    {
        for (auto & component : __components)
        {
            component.viewer.print(viewer::What::Title, viewer::How::Summary);
            component.viewer.print(viewer::What::Data, viewer::How::Summary);
        }

        // layers and bars

        __lcd.setCursor(13, 0);

        char written = 0;

        if (__layer.layer != nullptr)
        {
            written += __lcd.print('L');
            written += __lcd.print(__layer.id);
        }

        while (written++ < 3)
        {
            __lcd.write(' ');
        }
    }
    else
    {
        viewer->print(viewer::What::Data, viewer::How::Summary);
    }
}

void focus(viewer::Base & viewer)
{
    if (__focused.viewer != &viewer) // either in summary mode or another viewer is currently in focus
    {
        __lcd.clear();
        __focused.viewer = &viewer;
        __focused.viewer->print(viewer::What::Title, viewer::How::Focus);
    }

    __focused.viewer->print(viewer::What::Data, viewer::How::Focus);
    __focused.start();
}

void bar(midier::Sequencer::Bar bar)
{
    __lcd.setCursor(14, 0);

    char written = 0;

    if (bar != midier::Sequencer::Bar::None)
    {
        written = __lcd.print((unsigned)bar, DEC);
    }

    while (written++ < 2)
    {
        __lcd.write(' ');
    }
}

} // view

namespace config
{

void layer(midier::Layer * layer, unsigned char id) // `nullptr` means go back to global
{
    if (__layer.layer == nullptr && layer == nullptr)
    {
        return; // nothing to do
    }

    // we allow setting the same layer for updating its config and the timer

    __layer.layer = layer;
    __layer.id = id;

    if (layer == nullptr)
    {
        __sequencer.layers.eval([](midier::Layer & layer)
            {
                layer.velocity = midier::midi::Velocity::High;
            });

        __layer.stop();
        __config = &__sequencer.config; // go back global configuration
    }
    else
    {
        __sequencer.layers.eval([](midier::Layer & layer)
            {
                layer.velocity = midier::midi::Velocity::Low;
            });

        __layer.layer->velocity = midier::midi::Velocity::High;

        __layer.start();
        __config = layer->config.view();
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
    if (!__flashing.elapsed(70))
    {
        return;
    }

    digitalWrite(pin::LED::Flash, LOW);
    __flashing.stop();
}

void recording()
{
    static bool __recording = false;

    const auto recording = __sequencer.recording();

    if (__recording != recording)
    {
        digitalWrite(pin::LED::Record, recording ? HIGH : LOW);
        __recording = recording;
    }
}

void focus()
{
    if (!__focused.elapsed(3200))
    {
        return;
    }

    __layer.reset();

    control::view::summary();
}

void components()
{
    for (auto & component : __components)
    {
        const auto action = component.configurer.check();

        if (action == configurer::Action::None)
        {
            continue;
        }

        const auto layered = (__layer.layer != nullptr) && (&component.configurer != &configurer::__BPM); // all configurers but BPM are per layer

        if (layered)
        {
            __layer.start();
        }

        // actually update the configuration only if in summary mode or if this configurer is in focus

        if ((action == configurer::Action::Summary && __focused.viewer == nullptr) ||
            (action == configurer::Action::Focus && __focused.viewer == &component.viewer))
        {
            if (layered && __layer.layer->config.outer())
            {
                // the selected layer should now detach from the global configuration as
                // it is being configured specifically.
                __layer.layer->config = __sequencer.config;

                // we also need to point to the configuration of this layer
                __config = __layer.layer->config.view();
            }

            component.configurer.update();
        }

        if (action == configurer::Action::Summary)
        {
            control::view::summary(&component.viewer);
        }
        else if (action == configurer::Action::Focus)
        {
            control::view::focus(component.viewer);
        }
    }
}

void keys()
{
    struct Key : controlino::Key
    {
        Key(char pin) : controlino::Key(__multiplexer, pin)
        {}

        midier::Sequencer::Handle h;
    };

    static Key __keys[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    for (auto i = 0; i < sizeof(__keys) / sizeof(Key); ++i)
    {
        auto & key = __keys[i];

        const auto event = key.check();

        if (event == Key::Event::None)
        {
            continue;
        }

        if (event == Key::Event::Down)
        {
            control::config::global(); // go back to global configutarion when playing new layers

            key.h = __sequencer.start(i + 1);
        }
        else if (event == Key::Event::Up)
        {
            __sequencer.stop(key.h);
        }
    }
}

void record()
{
    static auto __button = controlino::Button(__multiplexer, pin::Record);

    const auto event = __button.check();

    if (event == controlino::Button::Event::Click)
    {
        __sequencer.record();
    }
    else if (event == controlino::Button::Event::Press)
    {
        if (__layer.layer == nullptr)
        {
            __sequencer.revoke(); // revoke the last recorded layer as no layer is selected
        }
        else
        {
            __layer.layer->revoke(); // revoke the selected layer
        }
    }
    else if (event == controlino::Button::Event::ClickPress)
    {
        __sequencer.wander();
    }
    else
    {
        return;
    }

    control::config::global();
}

void layer()
{
    if (__layer.elapsed(6000))
    {
        control::config::global();
    }
    else
    {
        static auto __button = controlino::Button(pin::Layer);

        const auto event = __button.check();

        if (event == controlino::Button::Event::Click)
        {
            if (__focused.viewer != nullptr)
            {
                __layer.reset(); // reset the layer timer only if there's one selected currently

                control::view::summary(); // go back to summary view
            }
            else
            {
                static const auto __count = __sequencer.layers.count();

                static unsigned char __index = 0;

                if (__layer.layer == nullptr || __index >= __count)
                {
                    __index = 0; // search from the start again
                }

                midier::Layer * layer = nullptr;

                while (__index < __count)
                {
                    midier::Layer & prospect = __sequencer.layers[__index++];

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
            if (__layer.layer != nullptr) // a layer is selected
            {
                if (__layer.layer->config.inner())
                {
                    __layer.layer->config = __config = &__sequencer.config;
                    __layer.reset();

                    // reprint the new (global) configuration
                    control::view::summary();
                }
            }
            else // no layer is selected
            {
                // making all previous dynamic layers static

                __sequencer.layers.eval([](midier::Layer & layer)
                    {
                        if (layer.config.outer())
                        {
                            layer.config = __sequencer.config; // make it static and copy the current global configuration
                        }
                    });
            }
        }
        else if (event == controlino::Button::Event::ClickPress)
        {
            // set all layers to dynamically configured

            __sequencer.layers.eval([](midier::Layer & layer)
                {
                    layer.config = &__sequencer.config;
                });

            control::config::global();
        }
    }
}

void click()
{
    const auto bar = __sequencer.click(midier::Sequencer::Run::Async);

    if (bar != midier::Sequencer::Bar::Same)
    {
        control::ui::flash();

        if (__focused.viewer == nullptr && __layer.layer == nullptr)
        {
            control::view::bar(bar);
        }
    }
}

} // handle

} //

extern "C" void setup()
{
    Serial.begin(9600);
    __lcd.begin(16, 2);

    pinMode(pin::LED::Flash, OUTPUT);
    pinMode(pin::LED::Record, OUTPUT);

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
