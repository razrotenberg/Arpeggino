#pragma once

#include <LiquidCrystal.h>

namespace arpeggino
{
namespace utils
{

struct LCD : LiquidCrystal
{
    LCD(uint8_t rs, uint8_t e, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) : LiquidCrystal(rs, e, d4, d5, d6, d7)
    {}

    template <typename T>
    char print(const T & arg)
    {
        return LiquidCrystal::print(arg);
    }

    template <typename T>
    char print(char col, char row, const T & arg)
    {
        setCursor(col, row);
        return print(arg);
    }

    template <typename T>
    char print(char col, char row, char max, const T & arg)
    {
        const auto written = print(col, row, arg);

        for (unsigned i = 0; i < max - written; ++i)
        {
            write(' '); // make sure the non-used characters are clear
        }

        return written;
    }
};

} // utils
} // arpeggino
