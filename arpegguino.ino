#include "internal.h"
#include "timer.h"

namespace arpegguino
{

namespace
{

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
} __layer;

namespace control
{

namespace ui
{

void record()
{
    const auto recording = \
        __looper.state == midier::Looper::State::Prerecord  ||
        __looper.state == midier::Looper::State::Record     ||
        __looper.state == midier::Looper::State::Overlay;

    digitalWrite(pin::LED::Record, recording ? HIGH : LOW);
}

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
            written += __lcd.print((unsigned)__layer.layer->tag);
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

void bar(midier::Looper::Bar bar)
{
    __lcd.setCursor(14, 0);

    char written = 0;

    if (bar != midier::Looper::Bar::None)
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

void layer(midier::Layer * layer) // nullptr means go back to global
{
    if (__layer.layer == nullptr && layer == nullptr)
    {
        return; // nothing to do
    }

    // we allow setting the same layer for updating its config and the timer

    __layer.layer = layer;

    if (layer == nullptr)
    {
        __layer.stop();
        __config = &__config.data(); // go back global configuration
    }
    else
    {
        __layer.start();
        __config = layer->config.view();
    }

    control::view::summary();
}

void global()
{
    layer(nullptr);
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

    digitalWrite(LED_BUILTIN, LOW);
    __flashing.stop();
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
            if (layered && __layer.layer->config.view() == &__config.data())
            {
                // the selected layer should now detach from the global configuration as
                // it is being configured specifically.
                __layer.layer->config = __config.data();

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

        midier::Layer::Tag tag = -1;
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

        control::config::global(); // go back to global configutarion when playing new layers

        if (event == Key::Event::Down)
        {
            key.tag = __looper.start(i + 1);
        }
        else if (event == Key::Event::Up && key.tag != -1) // the tag could be (-1) if there was no place for the layer when the key was pressed
        {
            __looper.stop(key.tag);
            key.tag = -1;
        }
    }
}

void record()
{
    using controlino::Button;
    using midier::Looper;

    static auto __button = Button(__multiplexer, pin::Record);

    const auto event = __button.check();
    const auto state = __looper.state;

    if (event == Button::Event::Click)
    {
        if (state == Looper::State::Wander)
        {
            // we want to set the state to `Prerecord` if there are no layers at the moment,
            // so `__looper` will start recording when the first layer will be played
            // we want to start recording immediately if there's a layer playing at the moment
            __looper.state = __looper.layers.idle() ? Looper::State::Prerecord : Looper::State::Record;
        }
        else if (state == Looper::State::Record || state == Looper::State::Overlay)
        {
            __looper.state = Looper::State::Playback;
        }
        else if (state == Looper::State::Playback)
        {
            __looper.state = Looper::State::Overlay;
        }
    }
    else if (event == Button::Event::Press)
    {
        midier::Layer::Tag tag = -1; // last recorded layer

        if (__layer.layer != nullptr)
        {
            tag = __layer.layer->tag;
        }

        __looper.revoke(tag);
    }
    else if (event == Button::Event::ClickPress)
    {
        __looper.state = Looper::State::Wander;
    }
    else
    {
        return;
    }

    control::config::global();
    control::ui::record();
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
                // go back tp summary view while keeping the same layer and resetting the timer
                control::config::layer(__layer.layer);
            }
            else
            {
                static const auto __count = __looper.layers.count();

                static char __index = 0;

                if (__layer.layer == nullptr || __index >= __count)
                {
                    __index = 0; // search from the start again
                }

                midier::Layer * layer = nullptr;

                while (__index < __count)
                {
                    midier::Layer & prospect = __looper.layers[__index++];

                    if (prospect.tag != -1)
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
                    control::config::layer(layer);
                }
            }
        }
        else if (event == controlino::Button::Event::Press)
        {
            if (__layer.layer != nullptr) // a layer is selected
            {
                if (__layer.layer->config.view() != &__config.data())
                {
                    __layer.layer->config = &__config.data();

                    // reprint the new (global) configuration
                    control::config::layer(__layer.layer);
                }
            }
            else // no layer is selected
            {
                // making all previous dynamic layers static

                __looper.layers.eval([](midier::Layer & layer)
                    {
                        if (layer.config.view() == &__config.data()) // if the layer is dynamically configured
                        {
                            layer.config = __config.data(); // make it static and copy the current global configuration
                        }
                    });
            }
        }
        else if (event == controlino::Button::Event::ClickPress)
        {
            // set all layers to dynamically configured

            __looper.layers.eval([](midier::Layer & layer)
                {
                    layer.config = &__config.data();
                });

            control::config::global();
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
    handle::focus();
    handle::components();
    handle::keys();
    handle::record();
    handle::layer();

    static Timer __timer;

    const auto bps = (float)__bpm / 60.f;
    const auto mspb = 1000.f / bps;
    const auto msps = mspb / (float)midier::Time::Subdivisions;

    if (!__timer.ticking() || __timer.elapsed(msps))
    {
        const auto bar = __looper.click();

        if (bar != midier::Looper::Bar::Same)
        {
            control::ui::flash();

            if (__focused.viewer == nullptr && __layer.layer == nullptr)
            {
                control::view::bar(bar);
            }
        }

        __timer.start();
    }
}

} // arpegguino
