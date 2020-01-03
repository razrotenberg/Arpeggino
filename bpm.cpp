#include "configurer.h"

namespace configurer
{

Action BPM::check()
{
    static auto __potentiometer = controlino::Potentiometer(pin::configure::BPM);

    int bpm;
    if (__potentiometer.check<40, 230>(/* out */ bpm))
    {
        /* out */ _config.bpm = bpm;
        return Action::Summary;
    }

    return Action::None;
}

void BPM::print(What what, How)
{
    if (what == What::Title)
    {
        _print(13, 1, "bpm");
    }

    if (what == What::Data)
    {
        _print(9, 1, 3, _config.bpm);
    }
}

} // configurer
