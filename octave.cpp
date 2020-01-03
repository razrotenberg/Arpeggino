#include "configurer.h"

namespace configurer
{

Action Octave::check()
{
    static auto __key = controlino::Key(__multiplexer, pin::configure::Octave);

    if (__key.check() == controlino::Key::Event::Down)
    {
        /* out */ _config.octave = (_config.octave % 7) + 1;
        return Action::Summary;
    }

    return Action::None;
}

void Octave::print(What what, How)
{
    if (what == What::Title)
    {
        _print(3, 0, 'O');
    }
    else if (what == What::Data)
    {
        _print(4, 0, _config.octave);
    }
}

} // configurer
