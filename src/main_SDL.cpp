
#include <SDL2/SDL.h>

#include "engine.h"

// BULLET IS PERFECT PHYSICS LIBRARY!!!
/*
 * 1) console
 *      - add notify functions
 * 2) LUA enngine global script:
 *      - reorganize script system... too heavy!!!
 * 3) Skeletal models functionality:
 *      - add multianimation system: weapon animations (not a car / byke case: it ia two entities
 *        with the same coordinates / orientation);
 *      - add head, torso rotation for actor->look_at (additional and final multianimation);
 *      - add mesh replace system (I.E. Lara's head wit emotion / speek);
 *      - fix animation interpolation in animations switch case;
 * 6) Menu (create own menu)
 *      - settings
 *      - map loading list
 *      - saves loading list
 * 10) OpenGL
 *      - shaders - refactoring shader manager
 *      - add shared universal VAO + VBO by gl_util
 *      - reflections
 *      - shadows
 *      - particles
 *      - GL and renderer optimisations
 * 40) Physics / gameplay
 *      - add ray backface filtering
 *      - update character controller physics interface
 *      - weapons
 * 41) scripts module
 *      - cutscenes playing
 *      - enemies AI
 * 42) sound
 *      - AL thread bug fix!
 *      - click removal;
 *      - add ADPCM and CDAUDIO.WAD soundtrack support;
 */

int main(int argc, char **argv)
{
    ///@TODO: add alternate config filename from argv
    Engine_Start("config.lua");

    // Entering main loop.
    Engine_MainLoop();

    // Main loop interrupted; shutting down.
    Engine_Shutdown(EXIT_SUCCESS);
    return(EXIT_SUCCESS);
}
