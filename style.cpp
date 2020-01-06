#include "configurer.h"

namespace configurer
{

Action Style::check()
{
    static auto __steps = controlino::Key(__multiplexer, pin::configure::Steps);

    if (__steps.check() == controlino::Key::Event::Down)
    {
        if (_config.looped == false)
        {
            /* out */ _config.looped = true;
        }
        else
        {
            if (/* out */ ++_config.steps == 7)
            {
                /* out */ _config.steps = 3;
            }

            /* out */ _config.perm = 0;
            /* out */ _config.looped = false;
        }

        return Action::Focus;
    }

    static auto __key = controlino::Key(__multiplexer, pin::configure::Perm);

    if (__key.check() == controlino::Key::Event::Down)
    {
        /* out */ _config.perm = (_config.perm + 1) % midiate::style::count(_config.steps);
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
            _print(6, 0, 'S');
        }
        else if (what == What::Data)
        {
            _print(7, 0, (unsigned)_config.steps);
            _print(8, 0, _config.looped ? '+' : '-');
            _print(9, 0, 4, (unsigned)_config.perm + 1);
        }
    }
    else if (how == How::Focus)
    {
        if (what == What::Title)
        {
            _print(0, 0, "Style: ");
        }
        else if (what == What::Data)
        {
            _print(7, 0, (unsigned)_config.steps);
            _print(8, 0, _config.looped ? '+' : '-');
            _print(9, 0, 4, (unsigned)_config.perm + 1);

            midiate::style::Description desc;
            midiate::style::description(_config.steps, _config.perm, /* out */ desc);
            _print(0, 1, 16, desc); // all columns in the LCD

            if (_config.looped)
            {
                _lcd.setCursor(strlen(desc) + 1, 1);

                for (unsigned i = 0; i < 3; ++i)
                {
                    _print('.');
                }
            }
        }
    }
}

} // configurer
