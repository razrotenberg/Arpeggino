#include "configurer.h"

#include <Arduino.h>

namespace configurer
{

bool Rhythm::set(short pot)
{
    constexpr auto Count = (unsigned)midiate::Rhythm::Count;

    const auto number = constrain(
        map(pot, 10, 1020, -1, Count),
        0, Count - 1
    );

    const auto rhythm = static_cast<midiate::Rhythm>(number);

    if (rhythm != _config.rhythm)
    {
        /* out */ _config.rhythm = rhythm;
        return true;
    }

    return false;
}

void Rhythm::print(What what)
{
    if (what == What::Title)
    {
        _print(4, 1, 'R');
    }
    else if (what == What::Data)
    {
        _print(col(), row(), 2, static_cast<int>(_config.rhythm) + 1);
    }
}

} // configurer
