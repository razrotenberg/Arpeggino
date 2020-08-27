#include <Controlino.h>
#include <Midier.h>

namespace arpeggino
{

namespace state
{

midier::Layers<8> layers; // the number of layers chosen will affect the global variable size
midier::Sequencer sequencer(layers);

} // state

namespace handle
{

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
    handle::keys();
    handle::click();
}

} // arpeggino
