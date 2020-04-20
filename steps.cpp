#include "internal.h"

namespace arpeggino
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
    if (__config->looped() == false)
    {
        __config->looped(true);
    }
    else
    {
        unsigned steps = __config->steps() + 1;

        if (steps > 6)
        {
            steps = 3;
        }

        __config->steps(steps);
        __config->perm(0);
        __config->looped(false);
    }
}

INIT_CONFIGURER(Steps);

} // configurer

} // arpeggino
