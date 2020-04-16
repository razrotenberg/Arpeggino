#include "internal.h"

#include <assert.h>

namespace arpeggino
{

namespace configurer
{

static auto __potentiometer = controlino::Potentiometer<20, 230>(pin::configure::BPM);

Action BPM::check()
{
    if (__potentiometer.check() == controlino::Potentiometer<20, 230>::Event::Changed)
    {
        return Action::Summary;
    }

    return Action::None;
}

void BPM::update()
{
    __looper.bpm = __potentiometer.read();
}

INIT_CONFIGURER(BPM);

} // configurer

namespace viewer
{

void BPM::print(What what, How how)
{
    assert(how == How::Summary);

    if (what == What::Title)
    {
        _print(13, 1, "bpm");
    }

    if (what == What::Data)
    {
        _print(9, 1, 3, __looper.bpm);
    }
}

INIT_VIEWER(BPM);

} // viewer

} // arpeggino
