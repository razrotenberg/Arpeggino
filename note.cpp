#include "configurer.h"

#include <Arduino.h>

namespace configurer
{

Action Note::check()
{
    static auto __key = controlino::Key(__multiplexer, pin::configure::Note);

    if (__key.check() == controlino::Key::Event::Down)
    {
        if (_config.accidental == midiate::Accidental::Flat)
        {
            _config.accidental = midiate::Accidental::Natural;
        }
        else if (_config.accidental == midiate::Accidental::Natural)
        {
            _config.accidental = midiate::Accidental::Sharp;
        }
        else if (_config.accidental == midiate::Accidental::Sharp)
        {
            _config.accidental = midiate::Accidental::Flat;

            if      (_config.note == midiate::Note::C) { _config.note = midiate::Note::D; }
            else if (_config.note == midiate::Note::D) { _config.note = midiate::Note::E; }
            else if (_config.note == midiate::Note::E) { _config.note = midiate::Note::F; }
            else if (_config.note == midiate::Note::F) { _config.note = midiate::Note::G; }
            else if (_config.note == midiate::Note::G) { _config.note = midiate::Note::A; }
            else if (_config.note == midiate::Note::A) { _config.note = midiate::Note::B; }
            else if (_config.note == midiate::Note::B) { _config.note = midiate::Note::C; }
        }

        return Action::Summary;
    }

    return Action::None;
}

void Note::print(What what, How)
{
    if (what == What::Data)
    {
        _lcd.setCursor(0, 0);

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
