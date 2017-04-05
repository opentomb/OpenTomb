#ifdef _MSC_VER///@GH0ST
#include <SDL.h>
#undef main
#else
#include <SDL2/SDL.h>
#endif

#include <stdlib.h>

#include "engine.h"

/*
 * see ENGINE.md
 * see TODO.md
 */

int main(int argc, char **argv)
{
    Engine_Start(argc, argv);
    Engine_MainLoop();
    Engine_Shutdown(EXIT_SUCCESS);

    return(EXIT_SUCCESS);
}
