#include "internal.h"

namespace arpegguino
{

LiquidCrystal __lcd(
    pin::LCD::RS, pin::LCD::E, pin::LCD::D4, pin::LCD::D5, pin::LCD::D6, pin::LCD::D7);

controlino::Selector __selector(
    pin::selector::S0, pin::selector::S1, pin::selector::S2, pin::selector::S3);

controlino::Multiplexer __multiplexer(
    pin::multiplexer::SIG, __selector);

midier::Looper __looper;

midier::Layer::Config * __config = &__looper.config;

} // arpegguino
