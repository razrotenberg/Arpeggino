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
    if (__config->style.looped() == false)
    {
        __config->style.looped(true);
    }
    else
    {
        unsigned steps = __config->style.steps() + 1;

        if (steps > 6)
        {
            steps = 3;
        }

        __config->style.steps(steps);
        __config->style.perm(0);
        __config->style.looped(false);
    }
}

INIT_CONFIGURER(Steps);

} // configurer

} // arpegguino
