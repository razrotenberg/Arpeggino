#pragma once

#include "pin.h"
#include "view.h"

#include <Controlino.h>
#include <Midiate.h>

namespace configurer
{

enum class Action
{
    None,

    Summary,
    Focus,
};

struct Base : public View
{
    Base(midiate::Looper::Config & config, LiquidCrystal & lcd) :
        View(lcd),
        _config(config)
    {}

    virtual Action check() = 0;

protected:
    midiate::Looper::Config & _config;
};

#define CONFIGURER(name)                                                \
    struct name : public Base                                           \
    {                                                                   \
        name(midiate::Looper::Config & config, LiquidCrystal & lcd) :   \
            Base(config, lcd) {}                                        \
                                                                        \
        Action check() override;                                        \
        void print(What what, How how) override;                        \
    }

CONFIGURER(Note);
CONFIGURER(Mode);
CONFIGURER(Octave);
CONFIGURER(BPM);
CONFIGURER(Style);
CONFIGURER(Rhythm);

#undef CONFIGURER

} // configurer

extern controlino::Multiplexer __multiplexer;
