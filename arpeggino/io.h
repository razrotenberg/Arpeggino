#pragma once

#include <Controlino.h>

#include "lcd.h"

namespace arpeggino
{
namespace io
{

extern utils::LCD lcd;
extern controlino::Multiplexer multiplexer;
extern controlino::Potentiometer BPM;
extern controlino::Key Mode;
extern controlino::Key Note;
extern controlino::Key Octave;
extern controlino::Key Perm;
extern controlino::Key Rhythm;
extern controlino::Key Steps;
extern controlino::Button Layer;
extern controlino::Button Record;

} // io
} // arpeggino
