#include "configurer.h"

namespace configurer
{

Action Rhythm::check()
{
    static auto __key = controlino::Key(__multiplexer, pin::configure::Rhythm);

    if (__key.check() == controlino::Key::Event::Down)
    {
        /* out */ _config.rhythm = (midiate::Rhythm)(((unsigned)_config.rhythm + 1) % (unsigned)midiate::Rhythm::Count);
        return Action::Focus;
    }

    return Action::None;
}

void Rhythm::print(What what, How how)
{
    if (how == How::Summary)
    {
        if (what == What::Title)
        {
            _print(4, 1, 'R');
        }
        else if (what == What::Data)
        {
            _print(5, 1, 2, (unsigned)_config.rhythm + 1);
        }
    }
    else if (how == How::Focus)
    {
        if (what == What::Title)
        {
            _print(0, 0, "Rhythm #");
        }
        else if (what == What::Data)
        {
            _print(8, 0, 2, (unsigned)_config.rhythm + 1);

            midiate::rhythm::Description desc;
            midiate::rhythm::description(_config.rhythm, /* out */ desc);
            _print(0, 1, desc);
        }
    }
}

} // configurer
