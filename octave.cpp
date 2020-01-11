#include "internal.h"

#include <assert.h>

namespace arpegguino
{

namespace configurer
{

Action Octave::check()
{
    static auto __key = controlino::Key(__multiplexer, pin::configure::Octave);

    if (__key.check() == controlino::Key::Event::Down)
    {
        return Action::Summary;
    }

    return Action::None;
}

void Octave::update()
{
    __config.octave = (__config.octave % 7) + 1;
}

INIT_CONFIGURER(Octave);

} // configurer

namespace viewer
{

void Octave::print(What what, How how)
{
    assert(how == How::Summary);

    if (what == What::Title)
    {
        _print(3, 0, 'O');
    }
    else if (what == What::Data)
    {
        _print(4, 0, __config.octave);
    }
}

INIT_VIEWER(Octave);

} // viewer

} // arpegguino
