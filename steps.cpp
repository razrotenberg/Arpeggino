#include "internal.h"

namespace arpegguino
{

namespace configurer
{

Action Steps::check()
{
    static auto __key = controlino::Key(__multiplexer, pin::configure::Steps);

    if (__key.check() == controlino::Key::Event::Down)
    {
        return Action::Focus;
    }

    return Action::None;
}

void Steps::update()
{
    if (__config.looped == false)
    {
        __config.looped = true;
    }
    else
    {
        if (++__config.steps == 7)
        {
            __config.steps = 3;
        }

        __config.perm = 0;
        __config.looped = false;
    }
}

INIT_CONFIGURER(Steps);

} // configurer

} // arpegguino
