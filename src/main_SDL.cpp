#include <chrono>
#include <cstdlib>

#include "engine/engine.h"
#include "engine/system.h"

#define NO_AUDIO  0

bool done = false;
btScalar time_scale = 1.0;

int main(int /*argc*/, char** /*argv*/)
{
    engine::Engine_Start();

    // Entering main loop.

    std::chrono::high_resolution_clock::time_point prev_time = std::chrono::high_resolution_clock::now();

    while(!done)
    {
        std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::microseconds>(now - prev_time).count() / 1.0e6;
        prev_time = now;

        engine::Engine_Frame(delta * time_scale);
        engine::Engine_Display();
    }

    // Main loop interrupted; shutting down.

    engine::Engine_Shutdown(EXIT_SUCCESS);
    return(EXIT_SUCCESS);
}
