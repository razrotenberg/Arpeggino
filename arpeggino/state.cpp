#include "state.h"

namespace arpeggino
{
namespace state
{

midier::Layers<48> layers; // the number of layers chosen will affect the global variable size
midier::Sequencer sequencer(layers);
midier::Config * config = &sequencer.config;

} // state
} // arpeggino
