#include "internal.h"

#include <assert.h>

namespace arpeggino
{

checker::Action checker::BPM()
{
    if (io::BPM.check() == controlino::Potentiometer::Event::Changed)
    {
        return Action::Summary;
    }

    return Action::None;
}

void changer::BPM()
{
    state::sequencer.bpm = io::BPM.read();
}

void viewer::BPM(What what, How how)
{
    assert(how == How::Summary);

    if (what == What::Title)
    {
        io::lcd.print(13, 1, "bpm");
    }

    if (what == What::Data)
    {
        io::lcd.print(9, 1, 3, state::sequencer.bpm);
    }
}

} // arpeggino
