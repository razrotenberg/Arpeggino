#include "internal.h"

namespace arpegguino
{

namespace configurer
{

Action Rhythm::check()
{
    static auto __key = controlino::Key(__multiplexer, pin::configure::Rhythm);

    if (__key.check() == controlino::Key::Event::Down)
    {
        return Action::Focus;
    }

    return Action::None;
}

void Rhythm::update()
{
    __config->rhythm = (midier::Rhythm)(((unsigned)__config->rhythm + 1) % (unsigned)midier::Rhythm::Count);
}

INIT_CONFIGURER(Rhythm);

} // configurer

namespace viewer
{

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
            _print(5, 1, 2, (unsigned)__config->rhythm + 1);
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
            _print(8, 0, 2, (unsigned)__config->rhythm + 1);

            midier::rhythm::Description desc;
            midier::rhythm::description(__config->rhythm, /* out */ desc);
            _print(0, 1, desc);
        }
    }
}

INIT_VIEWER(Rhythm);

} // viewer

} // arpegguino
