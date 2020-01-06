#include "configurer.h"

#include <Controlino.h>
#include <Midiate.h>
#include <TimerOne.h>

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

midiate::Looper __looper(__config);

configurer::BPM     __bpm   (__config, __lcd);
configurer::Note    __note  (__config, __lcd);
configurer::Octave  __octave(__config, __lcd);
configurer::Mode    __mode  (__config, __lcd);
configurer::Style   __style (__config, __lcd);
configurer::Rhythm  __rhythm(__config, __lcd);

configurer::Base * const __configurers[] = {
    &__bpm,
    &__note,
    &__octave,
    &__mode,
    &__style,
    &__rhythm,
};

struct
{
    configurer::Base * configurer = nullptr;
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

void summary(configurer::Base * configurer = nullptr) // 'nullptr' means all configurers
{
    if (__focused.configurer != nullptr) // some configurer is in focus currently
    {
        __focused.millis = -1;
        __focused.configurer = nullptr;
        __lcd.clear();

        configurer = nullptr; // print all titles and values
    }

    if (configurer == nullptr)
    {
        for (auto configurer : __configurers)
        {
            configurer->print(View::What::Title, View::How::Summary);
            configurer->print(View::What::Data, View::How::Summary);
        }
    }
    else
    {
        configurer->print(View::What::Data, View::How::Summary);
    }
}

void focus(configurer::Base * configurer)
{
    __focused.millis = millis();

    if (__focused.configurer != configurer)
    {
        __lcd.clear();
        __focused.configurer = configurer;
        __focused.configurer->print(View::What::Title, View::How::Focus);
    }

    __focused.configurer->print(View::What::Data, View::How::Focus);
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

void configurers()
{
    for (auto configurer : __configurers)
    {
        const auto action = configurer->check();

        if (action == configurer::Action::None)
        {
            continue;
        }

        if (action == configurer::Action::Summary)
        {
            control::summary(configurer);
        }
        else if (action == configurer::Action::Focus)
        {
            control::focus(configurer);
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
    handle::configurers();
    handle::keys();
    handle::record();
}

void setup()
{
    Serial.begin(9600);
    __lcd.begin(16, 2);

    Timer1.initialize(10000); // 10000us = 10ms
    Timer1.attachInterrupt(interrupt);

    pinMode(pin::LED::Flash, OUTPUT);
    pinMode(pin::LED::Record, OUTPUT);

    control::summary();
}

void loop()
{
    __looper.run([](int bar)
        {
            if (bar != -1)
            {
                control::flash();
            }

            control::bar(bar);
        });
}
