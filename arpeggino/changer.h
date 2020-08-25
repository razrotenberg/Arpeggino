#pragma once

namespace arpeggino
{
namespace changer
{

void BPM    ();
void Mode   ();
void Note   ();
void Octave ();
void Perm   ();
void Rhythm ();
void Steps  ();

using Changer = void(*)();

} // changer
} // arpeggino
