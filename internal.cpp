#include "internal.h"

namespace arpeggino
{

namespace
{

// the # of layers affect the global variable size
// this amount of layers requires ~75% of the maximum allowed
midier::Layers<35> __layers(&__config.data());

} //

LiquidCrystal __lcd(
    pin::LCD::RS, pin::LCD::E, pin::LCD::D4, pin::LCD::D5, pin::LCD::D6, pin::LCD::D7);

controlino::Selector __selector(
    pin::selector::S0, pin::selector::S1, pin::selector::S2, pin::selector::S3);

controlino::Multiplexer __multiplexer(
    pin::multiplexer::SIG, __selector);

midier::Sequencer __sequencer(__layers);

midier::Config::Viewed __config;

} // arpeggino
