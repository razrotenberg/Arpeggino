#pragma once

namespace arpegguino
{
namespace viewer
{

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

struct Base
{
    virtual void print(What what, How how) = 0;

protected:
    template <typename T>
    char _print(const T & arg)
    {
        return __lcd.print(arg);
    }

    template <typename T>
    char _print(char col, char row, const T & arg)
    {
        __lcd.setCursor(col, row);
        return _print(arg);
    }

    template <typename T>
    void _print(char col, char row, char max, const T & arg)
    {
        const auto written = _print(col, row, arg);

        for (unsigned i = 0; i < max - written; ++i)
        {
            __lcd.write(' '); // make sure the non-used characters are clear
        }
    }
};

#define VIEWER(name)                                \
    struct name : public Base                       \
    {                                               \
        void print(What what, How how) override;    \
    };                                              \
                                                    \
    extern name __ ## name

VIEWER(BPM);
VIEWER(Mode);
VIEWER(Note);
VIEWER(Octave);
VIEWER(Style);
VIEWER(Rhythm);

#undef VIEWER

#define INIT_VIEWER(name) name __ ## name

} // viewer
} // arpegguino
