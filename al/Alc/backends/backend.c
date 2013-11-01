
#include <SDL2/SDL_platform.h>

/**@FIXME: add platform defines to include other *.hpp
 */

#include "../../config.h"

#ifdef __WIN32__
#include "winmm.hpp"
#else
#error "SEE AL/Alc/backend.c and insert here you platform specific include *.hpp"
#endif