#pragma once

#include "view.h"

#include <Midiate.h> 

namespace configurer
{

struct Base : public View
{
    Base(midiate::Looper::Config & config, LiquidCrystal & lcd, char col, char row) :
        View(lcd),
        _config(config),
        _col(col),
        _row(row)
    {}

    // maybe update the configuration according the value of the knob
    virtual bool set(short pot) = 0;

    char col() const { return _col; };
    char row() const { return _row; };

protected:
    midiate::Looper::Config & _config;

private:
    char _col;
    char _row;
};

#define CONFIGURER(name, col, row)                                      \
    struct name : public Base                                           \
    {                                                                   \
        name(midiate::Looper::Config & config, LiquidCrystal & lcd) :   \
            Base(config, lcd, col, row) {}                              \
                                                                        \
        bool set(short pot) override;                                   \
        void print(What what) override;                                 \
    }

CONFIGURER(Note,    0, 0);
CONFIGURER(Mode,    6, 0);
CONFIGURER(Octave,  4, 0);
CONFIGURER(BPM,     9, 1);
CONFIGURER(Style,   1, 1);
CONFIGURER(Rhythm,  5, 1);

#undef CONFIGURER

} // configurer
