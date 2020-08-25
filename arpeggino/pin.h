#pragma once

#include <Arduino.h>

#define MUXED(x) x // just for clearity

namespace arpeggino
{
namespace pin
{

namespace control
{

constexpr auto Layer  = MUXED(1);
constexpr auto Record = MUXED(0);

} // control

namespace selector
{

constexpr auto S0 = 6;
constexpr auto S1 = 5;
constexpr auto S2 = 4;
constexpr auto S3 = 3;

} // selector

namespace multiplexer
{

constexpr auto SIG = 2;

} // multiplexer

namespace LCD
{

constexpr auto RS = 7;
constexpr auto E  = 8;
constexpr auto D4 = 9;
constexpr auto D5 = 10;
constexpr auto D6 = 11;
constexpr auto D7 = 12;

} // LCD

namespace LED
{

constexpr auto Flash  = 13;
constexpr auto Record = A1;

} // LED

namespace configure
{

constexpr auto BPM    = A0;
constexpr auto Note   = MUXED(7);
constexpr auto Mode   = MUXED(6);
constexpr auto Octave = MUXED(5);
constexpr auto Perm   = MUXED(4);
constexpr auto Steps  = MUXED(3);
constexpr auto Rhythm = MUXED(2);

} // configure

} // pin
} // arpeggino
