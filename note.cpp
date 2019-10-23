#include "configurer.h"

#include <Arduino.h>

namespace configurer
{

bool Note::set(short pot)
{
    // we have 7 notes with 3 options (flat, natural, sharp) each

    const auto number = constrain(
        map(pot, 10, 1020, -1, 21),
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
        /* out */ _config.looper.scale = midiate::Scale(
            midiate::Pitch(_config.note, _config.accidental, _config.octave),
            _config.mode
        );
        return true;
    }

    return false;
}

void Note::print(What what)
{
    if (what == What::Data)
    {
        if      (_config.note == midiate::Note::A) { _print('A'); }
        else if (_config.note == midiate::Note::B) { _print('B'); }
        else if (_config.note == midiate::Note::C) { _print('C'); }
        else if (_config.note == midiate::Note::D) { _print('D'); }
        else if (_config.note == midiate::Note::E) { _print('E'); }
        else if (_config.note == midiate::Note::F) { _print('F'); }
        else if (_config.note == midiate::Note::G) { _print('G'); }

        if      (_config.accidental == midiate::Accidental::Flat)    { _print('b'); }
        else if (_config.accidental == midiate::Accidental::Natural) { _print(' '); }
        else if (_config.accidental == midiate::Accidental::Sharp)   { _print('#'); }
    }
}

} // configurer
