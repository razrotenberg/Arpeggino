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

namespace handler
{

void knob()
{
    const short pot = analogRead(A5);

    if (abs(pot - __pot) > 3) // ignore noise
    {
        auto configurer = __configurers[__configurer];

        if (configurer->set(pot))
        {
            configurer->print();

            __lcd.setCursor( // reset the cursor after printing
                configurer->col(),
                configurer->row()
            );
            
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
        __configurer = (__configurer + 1) % (sizeof(__configurers) / sizeof(configurer::Base *));

        __lcd.setCursor(
            __configurers[__configurer]->col(),
            __configurers[__configurer]->row()
        );
    }

    previous = pressed;
}

} // handler

void interrupt()
{
    handler::knob();
    handler::keys();
    handler::configurer();
}

void setup()
{
    Serial.begin(9600);
    __lcd.begin(16, 2);

    __pot = analogRead(A5); // save the current potentiometer value

    Timer1.initialize(10000); // 10000us = 10ms
    Timer1.attachInterrupt(interrupt);

    pinMode(8, INPUT);

    pinMode(14, INPUT);
    pinMode(15, INPUT);
    pinMode(16, INPUT);

    for (auto configurer : __configurers)
    {
        configurer->init();

        __lcd.setCursor(
            configurer->col(),
            configurer->row()
        );

        configurer->print();
    }

    __lcd.setCursor(
        __configurers[__configurer]->col(),
        __configurers[__configurer]->row()
    );

    __lcd.blink();
}

void loop()
{
   __looper.run();
}
