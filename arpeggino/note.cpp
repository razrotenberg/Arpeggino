#include "internal.h"

#include <assert.h>

namespace arpeggino
{

checker::Action checker::Note()
{
    if (io::Note.check() == controlino::Key::Event::Down)
    {
        return Action::Summary;
    }

    return Action::None;
}

void changer::Note()
{
    if (state::config->accidental() == midier::Accidental::Flat)
    {
        state::config->accidental(midier::Accidental::Natural);
    }
    else if (state::config->accidental() == midier::Accidental::Natural)
    {
        state::config->accidental(midier::Accidental::Sharp);
    }
    else if (state::config->accidental() == midier::Accidental::Sharp)
    {
        state::config->accidental(midier::Accidental::Flat);

        if      (state::config->note() == midier::Note::C) { state::config->note(midier::Note::D); }
        else if (state::config->note() == midier::Note::D) { state::config->note(midier::Note::E); }
        else if (state::config->note() == midier::Note::E) { state::config->note(midier::Note::F); }
        else if (state::config->note() == midier::Note::F) { state::config->note(midier::Note::G); }
        else if (state::config->note() == midier::Note::G) { state::config->note(midier::Note::A); }
        else if (state::config->note() == midier::Note::A) { state::config->note(midier::Note::B); }
        else if (state::config->note() == midier::Note::B) { state::config->note(midier::Note::C); }
    }
}

void viewer::Note(What what, How how)
{
    assert(how == How::Summary);

    if (what == What::Data)
    {
        io::lcd.setCursor(0, 0);

        if      (state::config->note() == midier::Note::A) { io::lcd.print('A'); }
        else if (state::config->note() == midier::Note::B) { io::lcd.print('B'); }
        else if (state::config->note() == midier::Note::C) { io::lcd.print('C'); }
        else if (state::config->note() == midier::Note::D) { io::lcd.print('D'); }
        else if (state::config->note() == midier::Note::E) { io::lcd.print('E'); }
        else if (state::config->note() == midier::Note::F) { io::lcd.print('F'); }
        else if (state::config->note() == midier::Note::G) { io::lcd.print('G'); }

        if      (state::config->accidental() == midier::Accidental::Flat)    { io::lcd.print('b'); }
        else if (state::config->accidental() == midier::Accidental::Natural) { io::lcd.print(' '); }
        else if (state::config->accidental() == midier::Accidental::Sharp)   { io::lcd.print('#'); }
    }
}

} // arpeggino
