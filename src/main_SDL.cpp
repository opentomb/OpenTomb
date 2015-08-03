#include <cstdlib>

#include "engine.h"
#include "system.h"

#define NO_AUDIO  0
#include <chrono>

bool done = false;
btScalar time_scale = 1.0;

/*
 * CURRENT TODO LIST:
 *
 * 1) console
 *      - add notify functions
 * 2) LUA engine global script:
 *      - inventory manipulation.
 * 3) Skeletal models functionality:
 *      - add multianimation system: weapon animations (not a car / byke case: it ia two entities
 *        with the same coordinates / orientation);
 *      - add head, torso rotation for actor->look_at (additional and final multianimation);
 *      - add mesh replace system (I.E. Lara's head wit emotion / speek);
 *      - fix animation interpolation in animations switch case;
 * 4) Menu (create own menu)
 *      - title menu
 *      - settings
 *      - map loading list
 *      - saves loading list
 * 5) OpenGL
 *      - shaders
 *      - reflections
 *      - shadows
 *      - particles
 *      - GL and renderer optimisations
 * 6) Physics / gameplay
 *      - optimize and fix character controller, bug fixes: permanent task
 *      - weapons
 * 7) Scripts module
 *      - cutscenes playing
 *      - enemies AI
 */

int main(int /*argc*/, char** /*argv*/)
{
    Engine_Start();

    // Entering main loop.

    std::chrono::high_resolution_clock::time_point prev_time = std::chrono::high_resolution_clock::now();

    while(!done)
    {
        std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::microseconds>(now - prev_time).count() / 1.0e6;
        prev_time = now;

        Engine_Frame(delta * time_scale);
        Engine_Display();
    }

    // Main loop interrupted; shutting down.

    Engine_Shutdown(EXIT_SUCCESS);
    return(EXIT_SUCCESS);
}