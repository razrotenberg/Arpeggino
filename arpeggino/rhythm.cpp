#include "internal.h"

namespace arpeggino
{

checker::Action checker::Rhythm()
{
    if (io::Rhythm.check() == controlino::Key::Event::Down)
    {
        return Action::Focus;
    }

    return Action::None;
}

void changer::Rhythm()
{
    const auto current = state::config->rhythm();
    const auto next = (midier::Rhythm)(((unsigned)current + 1) % (unsigned)midier::Rhythm::Count);

    state::config->rhythm(next);
}

void viewer::Rhythm(What what, How how)
{
    if (how == How::Summary)
    {
        if (what == What::Title)
        {
            io::lcd.print(4, 1, 'R');
        }
        else if (what == What::Data)
        {
            io::lcd.print(5, 1, 2, (unsigned)state::config->rhythm() + 1);
        }
    }
    else if (how == How::Focus)
    {
        if (what == What::Title)
        {
            io::lcd.print(0, 0, "Rhythm #");
        }
        else if (what == What::Data)
        {
            io::lcd.print(8, 0, 2, (unsigned)state::config->rhythm() + 1);

            midier::rhythm::Description desc;
            midier::rhythm::description(state::config->rhythm(), /* out */ desc);
            io::lcd.print(0, 1, desc);
        }
    }
}

} // arpeggino
