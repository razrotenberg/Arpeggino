#pragma once

#include <Arduino.h>

#define MUXED(x) x // just for clearity

namespace pin
{

constexpr auto Record = MUXED(8);

namespace selector
{

constexpr auto S0 = 2;
constexpr auto S1 = 3;
constexpr auto S2 = 4;
constexpr auto S3 = 5;

} // selector

namespace multiplexer
{

constexpr auto SIG = 6;

} // multiplexer

namespace LCD
{

constexpr auto RS = 8;
constexpr auto E  = 9;
constexpr auto D4 = 10;
constexpr auto D5 = 11;
constexpr auto D6 = 12;
constexpr auto D7 = A5;

} // LCD

namespace LED
{

constexpr auto Flash  = 13;
constexpr auto Record = A1;

} // LED

namespace configure
{

constexpr auto BPM    = A0;
constexpr auto Mode   = MUXED(14);
constexpr auto Note   = MUXED(10);
constexpr auto Octave = MUXED(11);
constexpr auto Rhythm = MUXED(15);
constexpr auto Steps  = MUXED(12);
constexpr auto Perm   = MUXED(13);

} // configure

} // pin
