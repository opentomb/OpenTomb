#include <chrono>

#include "engine/engine.h"
#include "util/helpers.h"

#define NO_AUDIO  0

bool done = false;
float time_scale = 1.0;

int main(int /*argc*/, char** /*argv*/)
{
    engine::Engine::instance.start();

    // Entering main loop.

    util::TimePoint prev_time = util::now();

    while(!done)
    {
        util::TimePoint now = util::now();
        util::Duration delta = now - prev_time;
        delta *= time_scale;
        prev_time = now;

        engine::Engine::instance.frame(delta);
        engine::Engine::instance.display();
    }

    // Main loop interrupted; shutting down.

    engine::Engine::instance.shutdown(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}
