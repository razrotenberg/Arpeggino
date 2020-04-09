#pragma once

#include <Controlino.h>
#include <LiquidCrystal.h>
#include <Midier.h>

namespace arpegguino
{

extern LiquidCrystal            __lcd;
extern controlino::Selector     __selector;
extern controlino::Multiplexer  __multiplexer;
extern midier::Looper           __looper;
extern midier::Layer::Config *  __config;

} // arpegguino

#include "configurer.h"
#include "pin.h"
#include "viewer.h"
