#include <Arduino.h>

#include "timer.h"

namespace arpeggino
{
namespace utils
{

void Timer::start()
{
    _millis = millis();
}

void Timer::reset()
{
    if (ticking())
    {
        start();
    }
};

void Timer::stop()
{
    _millis = -1;
}

bool Timer::elapsed(unsigned ms) const
{
    return ticking() && millis() - _millis >= ms;
}

bool Timer::ticking() const
{
    return _millis != -1;
}

} // utils
} // arpeggino
