#include "internal.h"

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

struct
{
    viewer::Base * viewer = nullptr;
    unsigned long millis = -1;
} __focused;

unsigned long __flashing = -1;

struct
{
    midiate::Layer * layer = nullptr;
    unsigned long millis = -1;
} __layer;

namespace control
{

void record(bool recording)
{
    digitalWrite(pin::LED::Record, recording ? HIGH : LOW);
}

void flash()
{
    if (__flashing != -1)
    {
        return;
    }

    digitalWrite(pin::LED::Flash, HIGH);
    __flashing = millis();
}

void summary(viewer::Base * viewer = nullptr) // 'nullptr' means all configurers
{
    if (__focused.viewer != nullptr) // some viewer is in focus currently
    {
        __focused.millis = -1;
        __focused.viewer = nullptr;
        __lcd.clear();

        viewer = nullptr; // print all titles and values
    }

    if (viewer == nullptr)
    {
        for (auto & component : __components)
        {
            component.viewer.print(viewer::What::Title, viewer::How::Summary);
            component.viewer.print(viewer::What::Data, viewer::How::Summary);
        }

        if (__layer.layer != nullptr)
        {
            __lcd.setCursor(11, 0);
            __lcd.print('L');
            __lcd.print((unsigned)__layer.layer->tag);
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
    __focused.millis = millis();
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

void layer(midiate::Layer * layer) // nullptr means go back to global
{
    if (__layer.layer == layer)
    {
        return; // nothing to do
    }

    __layer.layer = layer;

    if (layer == nullptr)
    {
        __layer.millis = -1;
        __config = &__looper.config; // go back global configuration
    }
    else
    {
        __layer.millis = millis();

        if (layer->configured == midiate::Layer::Configured::Dynamic)
        {
            layer->config = __looper.config; // set the layer's configuration to the global one
        }

        __config = &layer->config;
    }

    control::summary();
}

void global()
{
    layer(nullptr);
}

} // control

namespace handle
{

void flashing()
{
    if (__flashing == -1)
    {
        return;
    }

    if (millis() - __flashing < 70)
    {
        return;
    }

    digitalWrite(LED_BUILTIN, LOW);
    __flashing = -1;
}

void focus()
{
    if (__focused.millis == -1)
    {
        return;
    }

    if (millis() - __focused.millis < 3200)
    {
        return;
    }

    if (__layer.layer != nullptr)
    {
        __layer.millis = millis();
    }

    control::summary();
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
            __layer.millis = millis();
        }

        if ((action == configurer::Action::Summary && __focused.viewer == nullptr) ||
            (action == configurer::Action::Focus && __focused.viewer == &component.viewer))
        {
            if (layered && __layer.layer->configured == midiate::Layer::Configured::Dynamic)
            {
                __layer.layer->configured = midiate::Layer::Configured::Static;
            }

            component.configurer.update();
        }

        if (action == configurer::Action::Summary)
        {
            control::summary(&component.viewer);
        }
        else if (action == configurer::Action::Focus)
        {
            control::focus(component.viewer);
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

        control::global(); // go back to global configutarion when playing new layers

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
    using midiate::Looper;

    static auto __button = Button(__multiplexer, pin::Record);

    const auto event = __button.check();
    const auto state = __looper.state;

    if (event == Button::Event::Click)
    {
        if (state == Looper::State::Wander)
        {
            __looper.state = Looper::State::Record;
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
        __looper.undo();
    }
    else if (event == Button::Event::ClickPress)
    {
        __looper.state = Looper::State::Wander;
    }
    else
    {
        return;
    }

    control::global();
    control::record(__looper.state == Looper::State::Record || __looper.state == Looper::State::Overlay);
}

void layer()
{
    if (__layer.layer != nullptr && millis() - __layer.millis >= 6000)
    {
        control::global();
    }
    else
    {
        static auto __key = controlino::Key(pin::Layer);

        const auto event = __key.check();

        if (event == controlino::Key::Event::Down)
        {
            if (__focused.viewer != nullptr)
            {
                control::summary();
            }
            else
            {
                constexpr static unsigned __count = sizeof(midiate::Looper::layers) / sizeof(midiate::Layer);

                static char __index = 0;

                if (__layer.layer == nullptr || __index >= __count)
                {
                    __index = 0; // search from the start again
                }

                midiate::Layer * layer = nullptr;

                while (__index < __count)
                {
                    midiate::Layer * const prospect = __looper.layers + __index++;

                    if (prospect->tag != -1)
                    {
                        layer = prospect;
                        break;
                    }
                }

                if (layer == nullptr)
                {
                    control::global();
                }
                else
                {
                    control::layer(layer);
                }
            }
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

} // arpegguino

void setup()
{
    Serial.begin(9600);
    arpegguino::__lcd.begin(16, 2);

    Timer1.initialize(10000); // 10000us = 10ms
    Timer1.attachInterrupt(arpegguino::interrupt);

    pinMode(pin::LED::Flash, OUTPUT);
    pinMode(pin::LED::Record, OUTPUT);

    arpegguino::control::summary();
}

void loop()
{
    arpegguino::__looper.run([](int bar)
        {
            if (bar != -1)
            {
                arpegguino::control::flash();
            }

            if (arpegguino::__focused.viewer == nullptr)
            {
                arpegguino::control::bar(bar);
            }
        });
}
