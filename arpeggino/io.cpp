#include "io.h"
#include "pin.h"

namespace arpeggino
{
namespace io
{

utils::LCD lcd(pin::LCD::RS, pin::LCD::E, pin::LCD::D4, pin::LCD::D5, pin::LCD::D6, pin::LCD::D7);

controlino::Selector selector(pin::selector::S0, pin::selector::S1, pin::selector::S2, pin::selector::S3);
controlino::Multiplexer multiplexer(pin::multiplexer::SIG, selector);

controlino::Potentiometer BPM(pin::configure::BPM, /* min = */ 20, /* max = */ 230);
controlino::Key Mode(multiplexer, pin::configure::Mode);
controlino::Key Note(multiplexer, pin::configure::Note);
controlino::Key Octave(multiplexer, pin::configure::Octave);
controlino::Key Perm(multiplexer, pin::configure::Perm);
controlino::Key Rhythm(multiplexer, pin::configure::Rhythm);
controlino::Key Steps(multiplexer, pin::configure::Steps);

controlino::Button Layer(multiplexer, pin::control::Layer);
controlino::Button Record(multiplexer, pin::control::Record);

} // io
} // arpeggino
