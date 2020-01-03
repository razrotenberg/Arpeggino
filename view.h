#pragma once

#include <LiquidCrystal.h>

struct View
{
    View(LiquidCrystal & lcd) : _lcd(lcd) {}

    enum class What
    {
        Title,
        Data,
    };

    enum class How
    {
        Summary,
        Focus,
    };

    virtual void print(What what, How how) = 0;

protected:
    template <typename T>
    char _print(const T & arg)
    {
        return _lcd.print(arg);
    }

    template <typename T>
    char _print(char col, char row, const T & arg)
    {
        _lcd.setCursor(col, row);
        return _print(arg);
    }

    template <typename T>
    void _print(char col, char row, char max, const T & arg)
    {
        const auto written = _print(col, row, arg);

        for (unsigned i = 0; i < max - written; ++i)
        {
            _lcd.write(' '); // make sure the non-used characters are clear
        }
    }

    LiquidCrystal & _lcd;
};
