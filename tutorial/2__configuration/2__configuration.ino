#include <Controlino.h>
#include <Midier.h>

namespace arpeggino
{

namespace state
{

midier::Layers<8> layers; // the number of layers chosen will affect the global variable size
midier::Sequencer sequencer(layers);

} // state

namespace io
{

// here we declare all I/O controls with their corresponding pin numbers

controlino::Potentiometer BPM(A0, /* min = */ 20, /* max = */ 230); // we limit the value of BPM to [20,230]
controlino::Key Note(10);
controlino::Key Mode(11);
controlino::Key Octave(12);
controlino::Key Perm(A5);
controlino::Key Steps(A4);
controlino::Key Rhythm(A3);

} // io

namespace configurer
{

// a configurer is a method that is responsible for updating a single
// configuration parameter according to changes of an I/O control
using Configurer = void(*)();

void BPM()
{
    if (io::BPM.check() == controlino::Potentiometer::Event::Changed)
    {
        state::sequencer.bpm = io::BPM.read();
    }
}

void Note()
{
    if (io::Note.check() != controlino::Key::Event::Down)
    {
        return; // nothing to do
    }

    // the key was just pressed

    auto & config = state::sequencer.config; // a shortcut

    if (config.accidental() == midier::Accidental::Flat)
    {
        config.accidental(midier::Accidental::Natural);
    }
    else if (config.accidental() == midier::Accidental::Natural)
    {
        config.accidental(midier::Accidental::Sharp);
    }
    else if (config.accidental() == midier::Accidental::Sharp)
    {
        config.accidental(midier::Accidental::Flat);

        if      (config.note() == midier::Note::C) { config.note(midier::Note::D); }
        else if (config.note() == midier::Note::D) { config.note(midier::Note::E); }
        else if (config.note() == midier::Note::E) { config.note(midier::Note::F); }
        else if (config.note() == midier::Note::F) { config.note(midier::Note::G); }
        else if (config.note() == midier::Note::G) { config.note(midier::Note::A); }
        else if (config.note() == midier::Note::A) { config.note(midier::Note::B); }
        else if (config.note() == midier::Note::B) { config.note(midier::Note::C); }
    }
}

void Mode()
{
    if (io::Mode.check() == controlino::Key::Event::Down)
    {
        const auto current = state::sequencer.config.mode();
        const auto next = (midier::Mode)(((unsigned)current + 1) % (unsigned)midier::Mode::Count);

        state::sequencer.config.mode(next);
    }
}

void Octave()
{
    if (io::Octave.check() == controlino::Key::Event::Down)
    {
        const auto current = state::sequencer.config.octave();
        const auto next = (current % 7) + 1;

        state::sequencer.config.octave(next);
    }
}

void Perm()
{
    if (io::Perm.check() == controlino::Key::Event::Down)
    {
        const auto current = state::sequencer.config.perm();
        const auto next = (current + 1) % midier::style::count(state::sequencer.config.steps());

        state::sequencer.config.perm(next);
    }
}

void Steps()
{
    if (io::Steps.check() == controlino::Key::Event::Down)
    {
        auto & config = state::sequencer.config; // a shortcut

        if (config.looped() == false) // we set to loop if currently not looping
        {
            config.looped(true);
        }
        else
        {
            unsigned steps = config.steps() + 1;

            if (steps > 6)
            {
                steps = 3;
            }

            config.steps(steps);
            config.perm(0); // reset the permutation
            config.looped(false); // set as non looping
        }
    }
}

void Rhythm()
{
    if (io::Rhythm.check() == controlino::Key::Event::Down)
    {
        const auto current = state::sequencer.config.rhythm();
        const auto next = (midier::Rhythm)(((unsigned)current + 1) % (unsigned)midier::Rhythm::Count);

        state::sequencer.config.rhythm(next);
    }
}

Configurer All[] =
    {
        BPM,
        Note,
        Mode,
        Octave,
        Perm,
        Steps,
        Rhythm,
    };

} // configurer

namespace handle
{

void configurers()
{
    // configurers will update the configuration on I/O events

    for (const auto & configurer : configurer::All)
    {
        configurer();
    }
}

void keys()
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
            key.h = state::sequencer.start(i + 1); // start playing an arpeggio of the respective scale degree
        }
        else if (event == Key::Event::Up) // a key was released
        {
            state::sequencer.stop(key.h); // stop playing the arpeggio
        }
    }
}

void click()
{
    // actually click Midier for it to play the MIDI notes
    state::sequencer.click(midier::Sequencer::Run::Async);
}

} // handle

extern "C" void setup()
{
    // initialize the Arduino "Serial" module and set the baud rate
    // to the same value you are using in your software.
    // if connected physically using a MIDI 5-DIN connection, use 31250.
    Serial.begin(9600);
}

extern "C" void loop()
{
    handle::configurers();
    handle::keys();
    handle::click();
}

} // arpeggino
