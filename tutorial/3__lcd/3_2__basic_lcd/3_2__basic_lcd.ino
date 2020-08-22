#include <Controlino.h>
#include <LiquidCrystal.h>
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

controlino::Selector Selector(/* s0 = */ 6, /* s1 = */ 5, /* s2 = */ 4, /* s3 = */ 3);
controlino::Multiplexer Multiplexer(/* sig = */ 2, Selector);

controlino::Potentiometer BPM(A0, /* min = */ 20, /* max = */ 230); // we limit the value of BPM to [20,230]

// all configuration keys are behind the multiplexer
controlino::Key Note(Multiplexer, 7);
controlino::Key Mode(Multiplexer, 6);
controlino::Key Octave(Multiplexer, 5);
controlino::Key Perm(Multiplexer, 4);
controlino::Key Steps(Multiplexer, 3);
controlino::Key Rhythm(Multiplexer, 2);

struct LCD : LiquidCrystal
{
    LCD(uint8_t rs, uint8_t e, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) : LiquidCrystal(rs, e, d4, d5, d6, d7)
    {}

    template <typename T>
    char print(const T & arg)
    {
        return LiquidCrystal::print(arg);
    }

    template <typename T>
    char print(char col, char row, const T & arg)
    {
        setCursor(col, row);
        return print(arg);
    }

    template <typename T>
    char print(char col, char row, char max, const T & arg)
    {
        const auto written = print(col, row, arg);

        for (unsigned i = 0; i < max - written; ++i)
        {
            write(' '); // make sure the non-used characters are clear
        }

        return written;
    }
};

LCD lcd(/* rs = */ 7, /* e = */ 8,  /* d4 = */ 9, /* d5 = */ 10, /* d6 = */ 11, /* d7 = */ 12);

} // io

namespace configurer
{

// a configurer is a method that is responsible for updating a single
// configuration parameter according to changes of an I/O control
using Configurer = bool(*)();

bool BPM()
{
    if (io::BPM.check() == controlino::Potentiometer::Event::Changed)
    {
        state::sequencer.bpm = io::BPM.read();
        return true;
    }

    return false;
}

bool Note()
{
    if (io::Note.check() != controlino::Key::Event::Down)
    {
        return false; // nothing to do
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

    return true;
}

bool Mode()
{
    if (io::Mode.check() == controlino::Key::Event::Down)
    {
        const auto current = state::sequencer.config.mode();
        const auto next = (midier::Mode)(((unsigned)current + 1) % (unsigned)midier::Mode::Count);

        state::sequencer.config.mode(next);
        return true;
    }

    return false;
}

bool Octave()
{
    if (io::Octave.check() == controlino::Key::Event::Down)
    {
        const auto current = state::sequencer.config.octave();
        const auto next = (current % 7) + 1;

        state::sequencer.config.octave(next);
        return true;
    }

    return false;
}

bool Perm()
{
    if (io::Perm.check() == controlino::Key::Event::Down)
    {
        const auto current = state::sequencer.config.perm();
        const auto next = (current + 1) % midier::style::count(state::sequencer.config.steps());

        state::sequencer.config.perm(next);
        return true;
    }

    return false;
}

bool Steps()
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

        return true;
    }

    return false;
}

bool Rhythm()
{
    if (io::Rhythm.check() == controlino::Key::Event::Down)
    {
        const auto current = state::sequencer.config.rhythm();
        const auto next = (midier::Rhythm)(((unsigned)current + 1) % (unsigned)midier::Rhythm::Count);

        state::sequencer.config.rhythm(next);
        return true;
    }

    return false;
}

} // configurer

namespace viewer
{

enum class What
{
    Title,
    Data,
};

using Viewer = void(*)(What);

void BPM(What what)
{
    if (what == What::Title)
    {
        io::lcd.print(13, 1, "bpm");
    }

    if (what == What::Data)
    {
        io::lcd.print(9, 1, 3, state::sequencer.bpm);
    }
}

void Note(What what)
{
    if (what == What::Data)
    {
        io::lcd.setCursor(0, 0);

        const auto & config = state::sequencer.config; // a shortcut

        if      (config.note() == midier::Note::A) { io::lcd.print('A'); }
        else if (config.note() == midier::Note::B) { io::lcd.print('B'); }
        else if (config.note() == midier::Note::C) { io::lcd.print('C'); }
        else if (config.note() == midier::Note::D) { io::lcd.print('D'); }
        else if (config.note() == midier::Note::E) { io::lcd.print('E'); }
        else if (config.note() == midier::Note::F) { io::lcd.print('F'); }
        else if (config.note() == midier::Note::G) { io::lcd.print('G'); }

        if      (config.accidental() == midier::Accidental::Flat)    { io::lcd.print('b'); }
        else if (config.accidental() == midier::Accidental::Natural) { io::lcd.print(' '); }
        else if (config.accidental() == midier::Accidental::Sharp)   { io::lcd.print('#'); }
    }
}

void Mode(What what)
{
    if (what == What::Data)
    {
        midier::mode::Name name;
        midier::mode::name(state::sequencer.config.mode(), /* out */ name);
        name[3] = '\0'; // trim the full name into a 3-letter shortcut
        io::lcd.print(0, 1, name);
    }
}

void Octave(What what)
{
    if (what == What::Title)
    {
        io::lcd.print(3, 0, 'O');
    }
    else if (what == What::Data)
    {
        io::lcd.print(4, 0, state::sequencer.config.octave());
    }
}

void Style(What what)
{
    if (what == What::Title)
    {
        io::lcd.print(6, 0, 'S');
    }
    else if (what == What::Data)
    {
        const auto & config = state::sequencer.config; // a shortcut

        io::lcd.print(7, 0, config.steps());
        io::lcd.print(8, 0, config.looped() ? '+' : '-');
        io::lcd.print(9, 0, 3, config.perm() + 1);
    }
}

void Rhythm(What what)
{
    if (what == What::Title)
    {
        io::lcd.print(4, 1, 'R');
    }
    else if (what == What::Data)
    {
        io::lcd.print(5, 1, 2, (unsigned)state::sequencer.config.rhythm() + 1);
    }
}

} // viewer

namespace component
{

struct Component
{
    configurer::Configurer configurer;
    viewer::Viewer viewer;
};

Component All[] =
    {
        { configurer::BPM, viewer::BPM },
        { configurer::Note, viewer::Note },
        { configurer::Mode, viewer::Mode },
        { configurer::Octave, viewer::Octave },
        { configurer::Perm, viewer::Style },
        { configurer::Steps, viewer::Style },
        { configurer::Rhythm, viewer::Rhythm },
    };

} // component

namespace handle
{

void components()
{
    // components will update the configuration on I/O events

    for (const auto & component : component::All)
    {
        if (component.configurer())
        {
            component.viewer(viewer::What::Data); // reprint the value on the LCD if changed
        }
    }
}

void keys()
{
    // we extend `controlino::Key` so we could hold a Midier handle with every key
    struct Key : controlino::Key
    {
        Key(char pin) : controlino::Key(io::Multiplexer, pin) // keys are behind the multiplexer
        {}

        midier::Sequencer::Handle h;
    };

    static Key __keys[] = { 15, 14, 13, 12, 11, 10, 9, 8 }; // channel numbers of the multiplexer

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

    // initialize the LCD
    io::lcd.begin(16, 2);

    // print the initial configuration
    for (const auto & component : component::All)
    {
        component.viewer(viewer::What::Title);
        component.viewer(viewer::What::Data);
    }
}

extern "C" void loop()
{
    handle::components();
    handle::keys();
    handle::click();
}

} // arpeggino
