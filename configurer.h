#pragma once

namespace arpegguino
{
namespace configurer
{

enum class Action
{
    None,

    Summary,
    Focus,
};

struct Base
{
    virtual Action check() = 0;
    virtual void update() = 0;
};

#define CONFIGURER(name)            \
    struct name : public Base       \
    {                               \
        Action check() override;    \
        void update() override;     \
    };                              \
                                    \
    extern name __ ## name

CONFIGURER(BPM);
CONFIGURER(Mode);
CONFIGURER(Note);
CONFIGURER(Octave);
CONFIGURER(Perm);
CONFIGURER(Steps);
CONFIGURER(Rhythm);

#undef CONFIGURER

#define INIT_CONFIGURER(name) name __ ## name

} // configurer
} // arpegguino
