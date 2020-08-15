#pragma once

namespace arpeggino
{

struct Timer
{
    // control
    void start(); // start (or restart)
    void reset(); // restart only if ticking
    void stop();

    // query
    bool elapsed(unsigned ms) const; // only if ticking
    bool ticking() const;

private:
    unsigned long _millis = -1; // make `short` and document
};

} // arpeggino
