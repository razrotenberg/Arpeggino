#include "internal.h"

namespace arpeggino
{

checker::Action checker::Steps()
{
    if (io::Steps.check() == controlino::Key::Event::Down)
    {
        return Action::Focus;
    }

    return Action::None;
}

void changer::Steps()
{
    if (state::config->looped() == false)
    {
        state::config->looped(true);
    }
    else
    {
        unsigned steps = state::config->steps() + 1;

        if (steps > 6)
        {
            steps = 3;
        }

        state::config->steps(steps);
        state::config->perm(0);
        state::config->looped(false);
    }
}

} // arpeggino
