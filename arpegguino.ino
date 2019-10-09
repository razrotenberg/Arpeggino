#include "configurer.h"

#include <LiquidCrystal.h> 
#include <TimerOne.h>
#include <Midiate.h>

//  LCD |   Arduino
//  ---------------
//  VSS |   GND
//  VDD |   5V
//  V0  |   Potentiometer
//  RS  |   D12
//  RW  |   GND
//  E   |   D11
//  D4  |   D5
//  D5  |   D4
//  D6  |   D3
//  D7  |   D2
//  A   |   5V
//  K   |   GND
// 
LiquidCrystal __lcd(12, 11, 5, 4, 3, 2);

using namespace midiate;

Looper::Config __config = {
    .note       = Note::C,
    .accidental = Accidental::Natural,
    .octave     = 3,
    .mode       = Mode::Ionian,
    .bpm        = 60,
    .style      = Style::Up,
    .rhythm     = Rhythm::F,
};

Looper __looper(__config);

short __pot = -1; // the value of the configuration potentiometer

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

void flash()
{
    if (__flashing != -1)
    {
        return;
    }

    digitalWrite(LED_BUILTIN, HIGH);
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

void bar(int i)
{
    __lcd.setCursor(14, 0);
    if (__lcd.print(i + 1, DEC) == 1)
    {
        __lcd.write(' '); // make sure the second digit is erased
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

    if (millis() - __flashing < 50)
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
    const short pot = analogRead(A5);

    if (abs(pot - __pot) > 3) // ignore noise
    {
        if (__configurers[__configurer]->set(pot))
        {
            __configurers[__configurer]->print();

            control::cursor(); // reset the cursor after printing
            control::blink();
            
            __looper.config(__config);
        }

        __pot = pot;
    }
}

void keys()
{
    static char tags[] = { -1, -1 }; // tags of the pressed buttons or (-1) for non-pressed ones

    for (auto i = 0; i < sizeof(tags) / sizeof(char); ++i)
    {
        const auto degree = i + 1;
        const auto pin    = i + 14; // buttons are connected to pins [A0,A1] which are [14,15]

        const auto pressed = digitalRead(pin) == HIGH;
    
        if (pressed == true && tags[i] == -1)
        {
            tags[i] = __looper.start(degree);
        }
        else if (pressed == false && tags[i] != -1)
        {
            __looper.stop(tags[i]);
            tags[i] = -1;
        }
    }
}

void configurer()
{
    static bool previous = false;

    const auto pressed = digitalRead(8) == HIGH;

    if (pressed == true && previous == false)
    {
        if (__blinking != -1)
        {
            // go to the next configurer only if already blinking
            __configurer = (__configurer + 1) % (sizeof(__configurers) / sizeof(configurer::Base *));

            control::cursor();
        }
    
        control::blink(); // blink and restart the timer anyway
    }

    previous = pressed;
}

void record()
{
    static bool previous = false;

    const auto pressed = digitalRead(A2) == HIGH;

    if (pressed == true && previous == false)
    {
        const auto state = __looper.state();

        if (state == Looper::State::Wander)
        {
            __looper.state(Looper::State::Record);
        }
        else if (state == Looper::State::Record)
        {
            __looper.state(Looper::State::Playback);
        }
    }

    previous = pressed;
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

    __pot = analogRead(A5); // save the current potentiometer value to calibrate handle::knob()

    Timer1.initialize(10000); // 10000us = 10ms
    Timer1.attachInterrupt(interrupt);

    pinMode(8, INPUT);

    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(14, INPUT);
    pinMode(15, INPUT);
    pinMode(16, INPUT);

    for (auto configurer : __configurers)
    {
        configurer->init();
        control::cursor(configurer);
        configurer->print();
    }

    control::cursor();
}

void loop()
{
    __looper.run([](int bar)
        {
            control::flash();
            control::bar(bar);
            control::cursor(); // return the cursor
        });
}
