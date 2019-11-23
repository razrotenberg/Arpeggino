#include "configurer.h"

#include <Controlino.h>
#include <Midiate.h>
#include <TimerOne.h>

namespace pin
{

constexpr auto S0 = 2;
constexpr auto S1 = 3;
constexpr auto S2 = 4;
constexpr auto S3 = 5;
constexpr auto SIG = 6;
constexpr auto Configure = 8; // muxed
constexpr auto Record = 9; // muxed
constexpr auto Knob = A0;

namespace LCD
{

constexpr auto RS = 8;
constexpr auto E  = 9;
constexpr auto D4 = 10;
constexpr auto D5 = 11;
constexpr auto D6 = 12;
constexpr auto D7 = A5;

} // LCD

namespace LED
{

constexpr auto Flash = 13;
constexpr auto Record = A1;

} // LED

} // pin

//  LCD |   Arduino
//  ---------------
//  VSS |   GND
//  VDD |   5V
//  V0  |   Potentiometer
//  RS  |   8
//  RW  |   GND
//  E   |   9
//  D4  |   10
//  D5  |   11
//  D6  |   12
//  D7  |   A5
//  A   |   5V
//  K   |   GND
// 
LiquidCrystal __lcd(pin::LCD::RS, pin::LCD::E, pin::LCD::D4, pin::LCD::D5, pin::LCD::D6, pin::LCD::D7);

controlino::Selector __selector(pin::S0, pin::S1, pin::S2, pin::S3);
controlino::Multiplexer __multiplexer(pin::SIG, __selector);

midiate::Looper::Config __config = {
    .note       = midiate::Note::C,
    .accidental = midiate::Accidental::Natural,
    .octave     = 3,
    .mode       = midiate::Mode::Ionian,
    .bpm        = 60,
    .style      = midiate::Style::Up,
    .rhythm     = midiate::Rhythm::F,
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

int __configurer = 0;
unsigned long __blinking = -1;
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

void blink()
{
    if (__blinking == -1)
    {
        __lcd.blink();
    }

    __blinking = millis();
}

void cursor(configurer::Base * const configurer = __configurers[__configurer])
{
    __lcd.setCursor(
        configurer->col(),
        configurer->row()
    );
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

void blinking()
{
    if (__blinking == -1)
    {
        return;
    }

    if (millis() - __blinking < 5000)
    {
        return;
    }

    __lcd.noBlink();
    __blinking = -1;
}

void knob()
{
    static controlino::Potentiometer __potentiometer(pin::Knob);

    int pot;
    if (__potentiometer.check(/* out */ pot))
    {
        if (__configurers[__configurer]->set(pot))
        {
            __configurers[__configurer]->print(View::What::Data);

            control::cursor(); // reset the cursor after printing
            control::blink();
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

void configurer()
{
    static controlino::Key __key(__multiplexer, pin::Configure);

    if (__key.check() == controlino::Key::Event::Down)
    {
        if (__blinking != -1)
        {
            // go to the next configurer only if already blinking
            __configurer = (__configurer + 1) % (sizeof(__configurers) / sizeof(configurer::Base *));

            control::cursor();
        }

        control::blink(); // blink and restart the timer anyway
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
    handle::blinking();
    handle::knob();
    handle::keys();
    handle::configurer();
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

    for (auto configurer : __configurers)
    {
        configurer->print(View::What::Title);
        control::cursor(configurer);
        configurer->print(View::What::Data);
    }

    control::cursor();
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
            control::cursor(); // return the cursor
        });
}
