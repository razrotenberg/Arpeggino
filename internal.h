#pragma once

#include <Controlino.h>
#include <LiquidCrystal.h>
#include <Midiate.h>

namespace arpegguino
{

extern LiquidCrystal            __lcd;
extern controlino::Selector     __selector;
extern controlino::Multiplexer  __multiplexer;
extern midiate::Looper          __looper;
extern midiate::Layer::Config * __config;

} // arpegguino

#include "configurer.h"
#include "pin.h"
#include "viewer.h"
