#include "internal.h"
#include "timer.h"

namespace arpegguino
{

namespace configurer
{

Action Perm::check()
{
    auto action = Action::None;

    static auto __key = controlino::Key(__multiplexer, pin::configure::Perm);
    static auto __start = Timer();
    static auto __hold = Timer();

    // we define different sleep duration for different number of stpes for it to be
    // both easy to iterate through all options while not iterating too fast
    static const short __durations[] = // in ms
        {
            /* 3 steps: */ 200,
            /* 4 steps: */ 150,
            /* 5 steps: */ 50,
            /* 6 steps: */ 10,
        };

    const auto event = __key.check();

    if (event == controlino::Key::Event::Down)
    {
        action = Action::Focus; // enter focus mode
        __start.start(); // start timer for fast iterating
    }
    else if (event == controlino::Key::Event::Hold)
    {
        const auto sleep = __durations[__config->style.steps() - 3]; // ms to sleep between every fast iteration

        if (__hold.elapsed(sleep)) // already fast iterating and enough time has passed
        {
            action = Action::Focus;
            __hold.reset();
        }
        else if (__start.elapsed(700)) // should start fast iteration
        {
            action = Action::Focus;
            __hold.start();
            __start.stop();
        }
    }
    else if (event == controlino::Key::Event::Up)
    {
        __hold.stop(); // for the next press to start counting from the start
    }

    return action;
}

void Perm::update()
{
    __config->style.perm((__config->style.perm() + 1) % midier::style::count(__config->style.steps()));
}

INIT_CONFIGURER(Perm);

} // configurer

} // arpegguino
