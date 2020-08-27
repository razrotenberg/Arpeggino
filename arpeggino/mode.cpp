#include "internal.h"

namespace arpeggino
{

checker::Action checker::Mode()
{
    if (io::Mode.check() == controlino::Key::Event::Down)
    {
        return Action::Focus;
    }

    return Action::None;
}

void changer::Mode()
{
    const auto current = state::config->mode();
    const auto next = (midier::Mode)(((unsigned)current + 1) % (unsigned)midier::Mode::Count);

    state::config->mode(next);
}

void viewer::Mode(What what, How how)
{
    if (what == What::Data)
    {
        midier::mode::Name name;
        midier::mode::name(state::config->mode(), /* out */ name);

        if (how == How::Summary)
        {
            name[3] = '\0'; // trim the full name into a 3-letter shortcut
            io::lcd.print(0, 1, name);
        }
        else if (how == How::Focus)
        {
            io::lcd.print(0, 1, sizeof(name), name);
        }
    }
    else if (what == What::Title && how == How::Focus)
    {
        io::lcd.print(0, 0, "Mode: ");
    }
}

} // arpeggino
