#include "configurer.h"

#include <Arduino.h>

namespace configurer
{

bool Mode::set(short pot)
{
    const auto number = constrain(
        map(pot, 10, 1020, -1, 7),
        0, 6
    );

    const auto mode = static_cast<midiate::Mode>(number);

    if (mode != _config.mode)
    {
        /* out */ _config.mode = mode;
        return true;
    }

    return false;
}

void Mode::print(What what)
{
    if (what == What::Data)
    {
        static const char * __names[] = {
            "Ion",
            "Dor",
            "Phr",
            "Lyd",
            "Mix",
            "Aeo",
            "Loc",
        };
        
        _print(__names[static_cast<int>(_config.mode)]);
    }
}

} // configurer
