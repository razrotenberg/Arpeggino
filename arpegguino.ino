#include "internal.h"

#include <TimerOne.h>

namespace arpegguino
{

LiquidCrystal __lcd(pin::LCD::RS, pin::LCD::E, pin::LCD::D4, pin::LCD::D5, pin::LCD::D6, pin::LCD::D7);

controlino::Selector __selector(pin::selector::S0, pin::selector::S1, pin::selector::S2, pin::selector::S3);
controlino::Multiplexer __multiplexer(pin::multiplexer::SIG, __selector);

midiate::Looper::Config __config = {
    .note       = midiate::Note::C,
    .accidental = midiate::Accidental::Natural,
    .octave     = 3,
    .mode       = midiate::Mode::Ionian,
    .bpm        = 60,
    .rhythm     = midiate::Rhythm::_7,
    .steps      = 3,
    .perm       = 0,
    .looped     = false,
};

namespace
{

midiate::Looper __looper(__config);

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

        if ((action == configurer::Action::Summary && __focused.viewer == nullptr) ||
            (action == configurer::Action::Focus && __focused.viewer == &component.viewer))
        {
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
        Key(char pin) :
            controlino::Key(__multiplexer, pin),
            tag(-1)
        {}

        char tag;
    };

    static Key __keys[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    for (auto i = 0; i < sizeof(__keys) / sizeof(Key); ++i)
    {
        auto & key = __keys[i];

        const auto event = key.check();

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

    if (event == Button::Event::None)
    {
        return;
    }

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

    control::record(__looper.state == Looper::State::Record || __looper.state == Looper::State::Overlay);
}

} // handle

void interrupt()
{
    handle::flashing();
    handle::focus();
    handle::components();
    handle::keys();
    handle::record();
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

            arpegguino::control::bar(bar);
        });
}
