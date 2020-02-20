#include "internal.h"

#include <assert.h>

namespace arpegguino
{

namespace configurer
{

static auto __potentiometer = controlino::Potentiometer(pin::configure::BPM);

Action BPM::check()
{
    if (__potentiometer.check<20, 230>() != -1)
    {
        return Action::Summary;
    }

    return Action::None;
}

void BPM::update()
{
    __looper.bpm = __potentiometer.read<20, 230>();
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

} // arpegguino
