#pragma once

#include <Midiate.h>

struct Config
{
    midiate::Note       note       = midiate::Note::C;
    midiate::Accidental accidental = midiate::Accidental::Natural;
    midiate::Octave     octave     = 3;
    midiate::Mode       mode       = midiate::Mode::Ionian;

    midiate::Looper::Config looper = {
        .scale  = midiate::Scale(midiate::Pitch(note, accidental, octave), mode),
        .bpm    = 60,
        .style  = midiate::Style::Up,
        .rhythm = midiate::Rhythm::F,
    };
};
