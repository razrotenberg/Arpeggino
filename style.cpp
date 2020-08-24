#include "internal.h"

namespace arpeggino
{

void viewer::Style(What what, How how)
{
    if (how == How::Summary)
    {
        if (what == What::Title)
        {
            io::lcd.print(6, 0, 'S');
        }
        else if (what == What::Data)
        {
            io::lcd.print(7, 0, state::config->steps());
            io::lcd.print(8, 0, state::config->looped() ? '+' : '-');
            io::lcd.print(9, 0, 3, state::config->perm() + 1);
        }
    }
    else if (how == How::Focus)
    {
        if (what == What::Title)
        {
            io::lcd.print(0, 0, "Style: ");
        }
        else if (what == What::Data)
        {
            io::lcd.print(7, 0, state::config->steps());
            io::lcd.print(8, 0, state::config->looped() ? '+' : '-');
            io::lcd.print(9, 0, 3, state::config->perm() + 1);

            midier::style::Description desc;
            midier::style::description(state::config->steps(), state::config->perm(), /* out */ desc);
            io::lcd.print(0, 1, 16, desc); // all columns in the LCD

            if (state::config->looped())
            {
                io::lcd.setCursor(strlen(desc) + 1, 1);

                for (unsigned i = 0; i < 3; ++i)
                {
                    io::lcd.print('.');
                }
            }
        }
    }
}

} // arpeggino
