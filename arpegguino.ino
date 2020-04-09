#include "internal.h"
#include "timer.h"

#include <TimerOne.h>

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

void bar(int i) // i of (-1) clears the bar from the screen
{
    __lcd.setCursor(14, 0);

    char written = 0;

    if (i != -1)
    {
        written = __lcd.print(i + 1, DEC);
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
        __config = &__looper.config; // go back global configuration
    }
    else
    {
        __layer.start();

        if (layer->configured == midier::Layer::Configured::Dynamic)
        {
            layer->config = __looper.config; // set the layer's configuration to the global one
        }

        __config = &layer->config;
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

        if ((action == configurer::Action::Summary && __focused.viewer == nullptr) ||
            (action == configurer::Action::Focus && __focused.viewer == &component.viewer))
        {
            if (layered && __layer.layer->configured == midier::Layer::Configured::Dynamic)
            {
                __layer.layer->configured = midier::Layer::Configured::Static;
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
    struct Key : public controlino::Key
    {
        Key(char pin) : controlino::Key(__multiplexer, pin)
        {}

        char tag = -1;
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
            __looper.state = Looper::State::Prerecord;

            for (auto & layer : __looper.layers)
            {
                if (layer.tag != -1)
                {
                    // start recording immediately if there's a layer playing at the moment
                    __looper.state = Looper::State::Record;
                    break;
                }
            }
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
        char tag = -1; // last recorded layer

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
                constexpr static unsigned __count = sizeof(midier::Looper::layers) / sizeof(midier::Layer);

                static char __index = 0;

                if (__layer.layer == nullptr || __index >= __count)
                {
                    __index = 0; // search from the start again
                }

                midier::Layer * layer = nullptr;

                while (__index < __count)
                {
                    midier::Layer * const prospect = __looper.layers + __index++;

                    if (prospect->tag != -1)
                    {
                        layer = prospect;
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
            if (__layer.layer != nullptr && __layer.layer->configured == midier::Layer::Configured::Static)
            {
                __layer.layer->configured = midier::Layer::Configured::Dynamic;

                // reprint the new (global) configuration
                control::config::layer(__layer.layer);
            }
        }
        else if (event == controlino::Button::Event::ClickPress)
        {
            for (auto & layer : __looper.layers)
            {
                if (layer.tag == -1)
                {
                    continue; // unused layer
                }

                layer.configured = midier::Layer::Configured::Dynamic;
            }

            control::config::global();
        }
    }
}

} // handle

void interrupt()
{
    handle::flashing();
    handle::focus();
    handle::components();
    handle::keys();
    handle::record();
    handle::layer();
}

} //

extern "C" void setup()
{
    Serial.begin(9600);
    __lcd.begin(16, 2);

    Timer1.initialize(10000); // 10000us = 10ms
    Timer1.attachInterrupt(interrupt);

    pinMode(pin::LED::Flash, OUTPUT);
    pinMode(pin::LED::Record, OUTPUT);

    control::view::summary();
}

extern "C" void loop()
{
    __looper.run([](int bar)
        {
            if (bar != -1)
            {
                control::ui::flash();
            }

            if (__focused.viewer == nullptr && __layer.layer == nullptr)
            {
                control::view::bar(bar);
            }
        });
}

} // arpegguino
