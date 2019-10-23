#include "configurer.h"

#include <Arduino.h>

namespace configurer
{

bool BPM::set(short pot)
{
    const auto bpm = constrain(
        map(pot, 10, 1020, 20, 240),
        40, 230
    );

    if (bpm != _config.looper.bpm)
    {
        /* out */ _config.looper.bpm = bpm;
        return true;
    }

    return false;
}

void BPM::print(What what)
{
    if (what == What::Title)
    {
        _print(13, 1, "bpm");
    }

    if (what == What::Data)
    {
        _print(col(), row(), 3, _config.looper.bpm);
    }
}

} // configurer
