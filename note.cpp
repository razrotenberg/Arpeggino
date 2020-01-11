#include "internal.h"

#include <assert.h>

namespace arpegguino
{

namespace configurer
{

Action Note::check()
{
    static auto __key = controlino::Key(__multiplexer, pin::configure::Note);

    if (__key.check() == controlino::Key::Event::Down)
    {
        return Action::Summary;
    }

    return Action::None;
}

void Note::update()
{
    if (__config.accidental == midiate::Accidental::Flat)
    {
        __config.accidental = midiate::Accidental::Natural;
    }
    else if (__config.accidental == midiate::Accidental::Natural)
    {
        __config.accidental = midiate::Accidental::Sharp;
    }
    else if (__config.accidental == midiate::Accidental::Sharp)
    {
        __config.accidental = midiate::Accidental::Flat;

        if      (__config.note == midiate::Note::C) { __config.note = midiate::Note::D; }
        else if (__config.note == midiate::Note::D) { __config.note = midiate::Note::E; }
        else if (__config.note == midiate::Note::E) { __config.note = midiate::Note::F; }
        else if (__config.note == midiate::Note::F) { __config.note = midiate::Note::G; }
        else if (__config.note == midiate::Note::G) { __config.note = midiate::Note::A; }
        else if (__config.note == midiate::Note::A) { __config.note = midiate::Note::B; }
        else if (__config.note == midiate::Note::B) { __config.note = midiate::Note::C; }
    }
}

INIT_CONFIGURER(Note);

} // configurer

namespace viewer
{

void Note::print(What what, How how)
{
    assert(how == How::Summary);

    if (what == What::Data)
    {
        __lcd.setCursor(0, 0);

        if      (__config.note == midiate::Note::A) { _print('A'); }
        else if (__config.note == midiate::Note::B) { _print('B'); }
        else if (__config.note == midiate::Note::C) { _print('C'); }
        else if (__config.note == midiate::Note::D) { _print('D'); }
        else if (__config.note == midiate::Note::E) { _print('E'); }
        else if (__config.note == midiate::Note::F) { _print('F'); }
        else if (__config.note == midiate::Note::G) { _print('G'); }

        if      (__config.accidental == midiate::Accidental::Flat)    { _print('b'); }
        else if (__config.accidental == midiate::Accidental::Natural) { _print(' '); }
        else if (__config.accidental == midiate::Accidental::Sharp)   { _print('#'); }
    }
}

INIT_VIEWER(Note);

} // viewer

} // arpegguino
