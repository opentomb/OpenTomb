#include "engine/engine.h"
#include "util/helpers.h"

#include <chrono>
#include <thread>

bool done = false;
float time_scale = 1.0;

int main(int /*argc*/, char** /*argv*/)
{
    engine::Engine::instance.start();

    // Entering main loop.

    util::TimePoint prev_time = util::now();

    while(!done)
    {
        BOOST_ASSERT(time_scale > 0);

        util::TimePoint now = util::now();
        util::Duration delta = now - prev_time;
        delta *= time_scale;

        if(delta.count() <= 0)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            continue;
        }

        prev_time = now;

        engine::Engine::instance.frame(delta);
        engine::Engine::instance.display();
    }

    // Main loop interrupted; shutting down.

    engine::Engine::instance.shutdown(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}
