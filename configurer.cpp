#include "configurer.h"

#include <Arduino.h>

namespace configurer
{

Base::Base(midiate::Looper::Config & config, LiquidCrystal & lcd, int col, int row, int size) :
    _config(config),
    _lcd(lcd),
    _col(col),
    _row(row),
    _size(size)
{}

void Base::print()
{
    const auto written = _print();

    for (auto i = 0; i < _size - written; ++i)
    {
        _lcd.write(' '); // make sure the non-used characters are clear
    }
}

Note::Note(midiate::Looper::Config & config, LiquidCrystal & lcd) :
    Base(config, lcd, 0, 0, 2)
{}

bool Note::set(short pot)
{
    // we have 7 notes with 3 options (flat, natural, sharp) each

    const auto number = constrain(
        map(pot, 0, 1020, 0, 21),
        0, 20
    );

    const auto div = number / 3;

    midiate::Note note;

    if      (div == 0) { note = midiate::Note::C; }
    else if (div == 1) { note = midiate::Note::D; }
    else if (div == 2) { note = midiate::Note::E; }
    else if (div == 3) { note = midiate::Note::F; }
    else if (div == 4) { note = midiate::Note::G; }
    else if (div == 5) { note = midiate::Note::A; }
    else if (div == 6) { note = midiate::Note::B; }

    const auto mod = number % 3;

    midiate::Accidental accidental;

    if      (mod == 0) { accidental = midiate::Accidental::Flat;    }
    else if (mod == 1) { accidental = midiate::Accidental::Natural; }
    else if (mod == 2) { accidental = midiate::Accidental::Sharp;   }
    
    if (note != _config.note || accidental != _config.accidental)
    {
        /* out */ _config.note = note;
        /* out */ _config.accidental = accidental;
        return true;
    }

    return false;
}

int Note::_print()
{
    if      (_config.note == midiate::Note::A) { _lcd.print('A'); }
    else if (_config.note == midiate::Note::B) { _lcd.print('B'); }
    else if (_config.note == midiate::Note::C) { _lcd.print('C'); }
    else if (_config.note == midiate::Note::D) { _lcd.print('D'); }
    else if (_config.note == midiate::Note::E) { _lcd.print('E'); }
    else if (_config.note == midiate::Note::F) { _lcd.print('F'); }
    else if (_config.note == midiate::Note::G) { _lcd.print('G'); }

    if      (_config.accidental == midiate::Accidental::Flat)   { _lcd.print('b'); }
    else if (_config.accidental == midiate::Accidental::Sharp)  { _lcd.print('#'); }

    return _config.accidental == midiate::Accidental::Natural ? 1 : 2;
}

Mode::Mode(midiate::Looper::Config & config, LiquidCrystal & lcd) :
    Base(config, lcd, 4, 0, 10)
{}

bool Mode::set(short pot)
{
    const auto number = constrain(
        map(pot, 0, 1020, 0, 7),
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

int Mode::_print()
{
    if      (_config.mode == midiate::Mode::Ionian)       { return _lcd.print("Ionian");     }
    else if (_config.mode == midiate::Mode::Dorian)       { return _lcd.print("Dorian");     }
    else if (_config.mode == midiate::Mode::Phrygian)     { return _lcd.print("Phrygian");   }
    else if (_config.mode == midiate::Mode::Lydian)       { return _lcd.print("Lydian");     }
    else if (_config.mode == midiate::Mode::Mixolydian)   { return _lcd.print("Mixolydian"); }
    else if (_config.mode == midiate::Mode::Aeolian)      { return _lcd.print("Aeolian");    }
    else if (_config.mode == midiate::Mode::Locrian)      { return _lcd.print("Locrian");    }
}

Octave::Octave(midiate::Looper::Config & config, LiquidCrystal & lcd) :
    Base(config, lcd, 4, 1, 1)
{}

void Octave::init()
{
    _lcd.setCursor(0, 1);
    _lcd.print("Oct");
}

bool Octave::set(short pot)
{
    const auto octave = constrain(
        map(pot, 0, 1020, 1, 9),
        1, 9
    );

    if (octave != _config.octave)
    {
        /* out */ _config.octave = octave;
        return true;
    }

    return false;
}

int Octave::_print()
{
    return _lcd.print(_config.octave, DEC);
}

BPM::BPM(midiate::Looper::Config & config, LiquidCrystal & lcd) :
    Base(config, lcd, 9, 1, 3)
{}

void BPM::init()
{
    _lcd.setCursor(13, 1);
    _lcd.print("bpm");
}

bool BPM::set(short pot)
{
    const auto bpm = constrain(
        map(pot, 0, 1020, 35, 240),
        40, 230
    );

    if (bpm != _config.bpm)
    {
        /* out */ _config.bpm = bpm;
        return true;
    }

    return false;
}

int BPM::_print()
{
    return _lcd.print(_config.bpm, DEC);
}

} // configurer
