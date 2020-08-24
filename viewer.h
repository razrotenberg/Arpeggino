#pragma once

namespace arpeggino
{
namespace viewer
{

enum class What
{
    Title,
    Data,
};

enum class How
{
    Summary,
    Focus,
};

void BPM    (What, How);
void Mode   (What, How);
void Note   (What, How);
void Octave (What, How);
void Rhythm (What, How);
void Style  (What, How);

using Viewer = void(*)(What, How);

} // viewer
} // arpeggino
