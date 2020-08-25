#pragma once

namespace arpeggino
{
namespace checker
{

enum class Action
{
    None, // no changes on this I/O control

    // when I/O control changed, every component
    // chooses which view it should be printed in

    Summary,
    Focus,
};

Action BPM    ();
Action Mode   ();
Action Note   ();
Action Octave ();
Action Perm   ();
Action Rhythm ();
Action Steps  ();

using Checker = Action(*)();

} // checker
} // arpeggino
