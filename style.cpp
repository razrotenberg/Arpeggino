#include "configurer.h"

namespace configurer
{

Action Style::check()
{
    static auto __key = controlino::Key(__multiplexer, pin::configure::Style);

    if (__key.check() == controlino::Key::Event::Down)
    {
        /* out */ _config.style = (midiate::Style)(((unsigned)_config.style + 1) % (unsigned)midiate::Style::Count);
        return Action::Focus;
    }

    return Action::None;
}

void Style::print(What what, How how)
{
    if (how == How::Summary)
    {
        if (what == What::Title)
        {
            _print(0, 1, 'S');
        }
        else if (what == What::Data)
        {
            _print(1, 1, 2, (unsigned)_config.style + 1);
        }
    }
    else if (how == How::Focus)
    {
        if (what == What::Title)
        {
            _print(0, 0, "Style #");
        }
        else if (what == What::Data)
        {
            _print(7, 0, 2, (unsigned)_config.style + 1);

            midiate::style::Name name;
            midiate::style::name(_config.style, /* out */ name);
            _print(0, 1, sizeof(midiate::style::Name), name);
        }
    }
}

} // configurer
