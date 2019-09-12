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
configurer::Mode    __mode  (__config, __lcd);
configurer::Note    __note  (__config, __lcd);
configurer::Octave  __octave(__config, __lcd);

configurer::Base * const __configurers[] = {
    &__bpm,
    &__mode,
    &__note,
    &__octave,
};

int __configurer = 0;
unsigned long __blinking = -1;

namespace control
{

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

} // control

namespace handle
{

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
    static bool previous[] = { false, false, false };

    for (int i = 0; i < sizeof(previous) / sizeof(bool); ++i)
    {
        const auto degree = i + 1;
        const auto pin    = i + 14; // buttons are connected to pins [A0..A2] which are [14..16]

        const auto pressed = digitalRead(pin) == HIGH;
    
        if (pressed == true && previous[i] == false)
        {
            __looper.play(degree, i);
        }
        else if (pressed == false && previous[i] == true)
        {
            __looper.stop(i);
        }

        previous[i] = pressed;
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

} // handle

void interrupt()
{
    handle::blinking();
    handle::knob();
    handle::keys();
    handle::configurer();
}

void setup()
{
    Serial.begin(9600);
    __lcd.begin(16, 2);

    __pot = analogRead(A5); // save the current potentiometer value to calibrate handle::knob()

    Timer1.initialize(10000); // 10000us = 10ms
    Timer1.attachInterrupt(interrupt);

    pinMode(8, INPUT);

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
   __looper.run();
}
