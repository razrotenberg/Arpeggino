#include "configurer.h"

#include <Arduino.h>

namespace configurer
{

bool Octave::set(short pot)
{
    const auto octave = constrain(
        map(pot, 10, 1020, 0, 8),
        1, 7
    );

    if (octave != _config.octave)
    {
        /* out */ _config.octave = octave;
        return true;
    }

    return false;
}

void Octave::print(What what)
{
    if (what == What::Title)
    {
        _print(3, 0, 'O');
    }
    else if (what == What::Data)
    {
        _print(_config.octave);
    }
}

} // configurer
