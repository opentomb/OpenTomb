#include <chrono>

#include "engine/engine.h"
#include "util/helpers.h"

#define NO_AUDIO  0

bool done = false;
float time_scale = 1.0;

int main(int /*argc*/, char** /*argv*/)
{
    engine::start();

    // Entering main loop.

    util::TimePoint prev_time = util::now();

    while(!done)
    {
        util::TimePoint now = util::now();
        util::Duration delta = now - prev_time;
        delta *= time_scale;
        prev_time = now;

        engine::frame(delta);
        engine::display();
    }

    // Main loop interrupted; shutting down.

    engine::shutdown(EXIT_SUCCESS);
    return(EXIT_SUCCESS);
}
