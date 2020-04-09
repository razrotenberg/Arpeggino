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
    if (__config->accidental == midier::Accidental::Flat)
    {
        __config->accidental = midier::Accidental::Natural;
    }
    else if (__config->accidental == midier::Accidental::Natural)
    {
        __config->accidental = midier::Accidental::Sharp;
    }
    else if (__config->accidental == midier::Accidental::Sharp)
    {
        __config->accidental = midier::Accidental::Flat;

        if      (__config->note == midier::Note::C) { __config->note = midier::Note::D; }
        else if (__config->note == midier::Note::D) { __config->note = midier::Note::E; }
        else if (__config->note == midier::Note::E) { __config->note = midier::Note::F; }
        else if (__config->note == midier::Note::F) { __config->note = midier::Note::G; }
        else if (__config->note == midier::Note::G) { __config->note = midier::Note::A; }
        else if (__config->note == midier::Note::A) { __config->note = midier::Note::B; }
        else if (__config->note == midier::Note::B) { __config->note = midier::Note::C; }
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

        if      (__config->note == midier::Note::A) { _print('A'); }
        else if (__config->note == midier::Note::B) { _print('B'); }
        else if (__config->note == midier::Note::C) { _print('C'); }
        else if (__config->note == midier::Note::D) { _print('D'); }
        else if (__config->note == midier::Note::E) { _print('E'); }
        else if (__config->note == midier::Note::F) { _print('F'); }
        else if (__config->note == midier::Note::G) { _print('G'); }

        if      (__config->accidental == midier::Accidental::Flat)    { _print('b'); }
        else if (__config->accidental == midier::Accidental::Natural) { _print(' '); }
        else if (__config->accidental == midier::Accidental::Sharp)   { _print('#'); }
    }
}

INIT_VIEWER(Note);

} // viewer

} // arpegguino
