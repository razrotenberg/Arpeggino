#include "internal.h"

#include <assert.h>

namespace arpeggino
{

checker::Action checker::Octave()
{
    if (io::Octave.check() == controlino::Key::Event::Down)
    {
        return Action::Summary;
    }

    return Action::None;
}

void changer::Octave()
{
    state::config->octave((state::config->octave() % 7) + 1);
}

void viewer::Octave(What what, How how)
{
    assert(how == How::Summary);

    if (what == What::Title)
    {
        io::lcd.print(3, 0, 'O');
    }
    else if (what == What::Data)
    {
        io::lcd.print(4, 0, state::config->octave());
    }
}

} // arpeggino
