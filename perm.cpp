#include "internal.h"

namespace arpegguino
{

namespace configurer
{

Action Perm::check()
{
    static auto __key = controlino::Key(__multiplexer, pin::configure::Perm);

    if (__key.check() == controlino::Key::Event::Down)
    {
        return Action::Focus;
    }

    return Action::None;
}

void Perm::update()
{
    __config->style.perm = (__config->style.perm + 1) % midiate::style::count(__config->style.steps);
}

INIT_CONFIGURER(Perm);

} // configurer

} // arpegguino
