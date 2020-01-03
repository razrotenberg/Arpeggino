#include "configurer.h"

namespace configurer
{

Action Mode::check()
{
    static auto __key = controlino::Key(__multiplexer, pin::configure::Mode);

    if (__key.check() == controlino::Key::Event::Down)
    {
        /* out */ _config.mode = (midiate::Mode)(((unsigned)_config.mode + 1) % (unsigned)midiate::Mode::Count);
        return Action::Summary;
    }

    return Action::None;
}

void Mode::print(What what, How)
{
    if (what == What::Data)
    {
        midiate::mode::Name name;
        midiate::mode::name(_config.mode, /* out */ name);
        name[3] = '\0';
        _print(6, 0, name);
    }
}

} // configurer
