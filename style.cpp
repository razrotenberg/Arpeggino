#include "internal.h"

namespace arpeggino
{

namespace viewer
{

void Style::print(What what, How how)
{
    if (how == How::Summary)
    {
        if (what == What::Title)
        {
            _print(6, 0, 'S');
        }
        else if (what == What::Data)
        {
            _print(7, 0, __config->steps());
            _print(8, 0, __config->looped() ? '+' : '-');
            _print(9, 0, 3, __config->perm() + 1);
        }
    }
    else if (how == How::Focus)
    {
        if (what == What::Title)
        {
            _print(0, 0, "Style: ");
        }
        else if (what == What::Data)
        {
            _print(7, 0, __config->steps());
            _print(8, 0, __config->looped() ? '+' : '-');
            _print(9, 0, 3, __config->perm() + 1);

            midier::style::Description desc;
            midier::style::description(__config->steps(), __config->perm(), /* out */ desc);
            _print(0, 1, 16, desc); // all columns in the LCD

            if (__config->looped())
            {
                __lcd.setCursor(strlen(desc) + 1, 1);

                for (unsigned i = 0; i < 3; ++i)
                {
                    _print('.');
                }
            }
        }
    }
}

INIT_VIEWER(Style);

} // viewer

} // arpeggino
