#include "internal.h"

namespace arpegguino
{

namespace configurer
{

Action Mode::check()
{
    static auto __key = controlino::Key(__multiplexer, pin::configure::Mode);

    if (__key.check() == controlino::Key::Event::Down)
    {
        return Action::Focus;
    }

    return Action::None;
}

void Mode::update()
{
    __config.mode = (midiate::Mode)(((unsigned)__config.mode + 1) % (unsigned)midiate::Mode::Count);
}

INIT_CONFIGURER(Mode);

} // configurer

namespace viewer
{

void Mode::print(What what, How how)
{
    if (what == What::Data)
    {
        midiate::mode::Name name;
        midiate::mode::name(__config.mode, /* out */ name);

        if (how == How::Summary)
        {
            name[3] = '\0';
            _print(0, 1, name);
        }
        else if (how == How::Focus)
        {
            _print(0, 1, sizeof(name), name);
        }
    }
    else if (what == What::Title && how == How::Focus)
    {
        _print(0, 0, "Mode: ");
    }
}

INIT_VIEWER(Mode);

} // viewer

} // arpegguino
