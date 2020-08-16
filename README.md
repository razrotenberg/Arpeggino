
<div align="center">
    <img src="extras/Arpeggino.png" width="40%">
</div>

# Arpeggino

An Arduino MIDI arpeggiator, sequencer, recorder and looper.

*Arpeggino* lets you play MIDI arpeggios in every scale and pattern,
record and loop your sequences,
and modify your sequences even after recording and be creative.

## Table of Contents

* [What Is Arpeggino?](#what-is-arpeggino)
* [Demo](#demo)
* [Project Code](#project-code)
* [Project Setup](#project-setup)
* [Tutorial](#tutorial)
    * [Step Zero - Prerequisites](#tutorial-step-zero---prerequisites)
    * [Step One - Playing Arpeggios](#tutorial-step-one---playing-arpeggios)

## What Is Arpeggino?

*Arpeggino* is an Arduino based MIDI arpeggiator, sequencer, recorder and looper.
It supports playing MIDI arpeggios in every scale and pattern, and in many rhythms.

When pressing one of the keys, an arpeggio of the respective scale degree will be playing.
If not in recording mode, the arpeggio will stop playing once the key is released.

Every aspect of the arpeggios can be configured, and the configuration is shown on the LCD screen:
* Root note of the scale (e.g. C, D#, E, Fb)
* Scale mode: Ionian, Dorian, Phrygian, Lydian, Mixolydian, Aeolian, Locrian
* Octave (e.g. 2, 3, 4)
* Permutation (i.e. the order in which the arpeggio notes are being played)
* Number of Steps (i.e. the number of notes of the arpeggio)
* Rhythm (i.e. the musical rhythm in which the arpeggio is being played)
* BPM

*Arpeggino* also lets you to record your sequences, for any number of bars you want.
You can delete recorded layers, record new ones on top of the sequence, and even reconfigure recorded layers.
This lets you focus on your musical creativity and experiment with different scales, styles, and rhythms.

## Demo

A short demo video is available on [YouTube](https://youtu.be/HbMf0oO-zfE).

## Project Code

Arpeggino consists of three sub-projects:

- [Controlino](https://github.com/levosos/Controlino) -
An Arduino library for using complex gestures of input controls behind a multiplexer.
It is the library used in Arpeggino to easily control buttons and potentiometers behind a multiplexer.
It allows the detection of complex click gestures such as click/double-click/long click (press)/etc.
- [Midier](https://github.com/levosos/Midier) -
An Arduino library for playing, recording, looping and programming MIDI notes, arpeggios and sequences.
This is the engine behind Arpggino, that lets it do all the musical and rhythmical stuff.
- *Arpeggino* (this very repository) -
The Arduino sketch project.
This project is responsible for handling all I/O controls (keys, buttons, potentiometers, LCD), configuring Midier and using it for recording and playing the sequences.

Both [Controlino](https://github.com/levosos/Controlino) and [Midier](https://github.com/levosos/Midier) are comprehensively documented and feature plenty of [examples](https://github.com/levosos/Midier#listen-to-examples) you can check out and listen to right from your browser.

Both libraries are officially published and can be installed directly from the Arduino IDE Library Manager ([here's](https://www.arduino.cc/en/guide/libraries) a short guide).

## Project Setup

*Arpeggino* sends MIDI commands over the Arduino serial connection.
These MIDI commands can be the input to any device that supports MIDI as input.
A computer software can accept MIDI commands as well.
Therefore, the computer can be the device we connect the Arduino to.

There are two main ways you can connect Arpeggino to your computer:
1. Over the USB connection using the help of some software (I use [Hairless MIDI-Serial Bridge](https://projectgus.github.io/hairless-midiserial/))
2. Using a MIDI-to-USB cable

The two ways are fully documented [here](https://github.com/levosos/Midier#setup) in Midier GitHub repository.

Note that you need to set the baud rate properly in the Arduino sketch (the parameter passed to the `Serial.begin()`) according to the way you are connecting your Arduino to the MIDI device. This is also fully described [here](https://github.com/levosos/Midier#baud-rate) in Midier GitHub repository.

# Tutorial

This is a tutorial on how to build Arpeggino by yourself.
Code (sketches) and schemas of all steps are in the subdirectories under [tutorial](tutorial).

## Tutorial: Step Zero - Prerequisites

Install Midier and Controlino using the Arduino Library Manager ([here's](https://www.arduino.cc/en/guide/libraries) a short guide)

Pick your MIDI-to-Serial bridge software. I personally use [Hairless MIDI-Serial Bridge](https://projectgus.github.io/hairless-midiserial/) for this. Make sure you choose the same baud rate as you will be using in the Arduino sketch (I use 9600)

Pick your favorite software for playing MIDI notes. A few that I know of: GarageBand (my choice), Logic Pro X, Ableton, Cubase, LMMS, etc.

Optionallly, check out some of [Midier examples](https://github.com/levosos/Midier#listen-to-examples) on GitHub and verify your setup works

## Tutorial: Step One - Playing Arpeggios

First we need to connect a few buttons to the Arduino. Having eight buttons is optimal as it is the number of notes in diatonic scales (including one for the root at an octave higher).

To simplify the development, it's better to connect all buttons to sequential pins on the Arduino (e.g. pins 2-9).

We use `INPUT_PULLUP` for buttons ([here's](https://www.arduino.cc/en/Tutorial/InputPullupSerial) an explanation) one leg of the button should be plugged to GND and the other leg to the Arduino.

Here's a [schema](tutorial/1__playing_arpeggios/1__playing_arpeggios.fzz) you can use:

<div align="center">
    <img src="tutorial/1__playing_arpeggios/1__playing_arpeggios.png" width="60%">
</div>

Now the setup is ready, and we can start coding.

You can use the [existing Arduino sketch](tutorial/1__playing_arpeggios/1__playing_arpeggios.ino), or you can code it yourself to see how easy it is.
For the sake of the tutorial, I'll assume you'll be coding yourself.

Open up a new Arduino sketch and add the following imports:
```
#include <Controlino.h>
#include <Midier.h>
```

We now need to create a Midier sequencer to play arpeggios, as Midier is the engine behind Arpeggino. A sequencer should be initialized with some layers (which dictate the maximum number of concurrent layers that could be played) upon creation.

The sequencer should be declared in a global scope for its state to be preserved all the time, and should not be declared inside a method.
```
midier::Layers<8> layers;
midier::Sequencer sequencer(layers);
```

We also need to initialize the Arduino Serial module with the correct baud rate (the one you are using in the software as well). I'm using 9600.
```
void setup()
{
    Serial.begin(9600);
}
```

Now the real action comes. Every iteration, we want to check if any of the keys was pressed or released, and start or stop playing the arpeggio respectively. After checking for I/O, we have to "click" Midier.

The following code might seem complicated at first sight but it is really not.
We define a structure that extends Controlino's `Key` that can hold the handler of an arpeggio that is being played by Midier.
We use this structure and initialize an array of keys using with the pin numbers to which the buttons are connected (here the buttons are connected to pins 2-9).
We then check if the key was pressed (`Down` event) or released (`Up` event) and start or stop the arpeggio of the respective scale degree.
Eventually we "click" the Midier sequencer for it to play and stop the MIDI notes.

```
void loop()
{
    // we extend `controlino::Key` so we could hold a Midier handle with every key
    struct Key : controlino::Key
    {
        Key(char pin) : controlino::Key(pin)
        {}

        midier::Sequencer::Handle h;
    };

    static Key __keys[] = { 2, 3, 4, 5, 6, 7, 8, 9 }; // initialize with pin numbers


    for (auto i = 0; i < sizeof(__keys) / sizeof(Key); ++i)
    {
        auto & key = __keys[i];

        const auto event = key.check();

        if (event == Key::Event::None)
        {
            continue; // nothing has changed
        }

        if (event == Key::Event::Down) // a key was pressed
        {
            key.h = sequencer.start(i + 1); // start playing an arpeggio of the respective scale degree
        }
        else if (event == Key::Event::Up) // a key was released
        {
            sequencer.stop(key.h); // stop playing the arpeggio
        }
    }

    // actually click Midier for it to play the MIDI notes
    sequencer.click(midier::Sequencer::Run::Async);
}
```

And that's it! Upload it onto your Arduino, start the MIDI-to-Serial bridge software, open up your DAW (or any software that plays MIDI notes) and press the keys!
You should hear arpeggios from the C major scale as you press the keys.
