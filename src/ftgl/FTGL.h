#ifndef     __FTGL__
#define     __FTGL__

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#if defined(__MACOSX__)
#include <OpenGL/gl.h>	/* Header File For The OpenGL Library */
#include <OpenGL/glu.h>	/* Header File For The GLU Library */
#elif defined(__MACOS__)
#include <gl.h>		/* Header File For The OpenGL Library */
#include <glu.h>	/* Header File For The GLU Library */
#else
#include <GL/gl.h>	/* Header File For The OpenGL Library */
#include <GL/glu.h>	/* Header File For The GLU Library */
#endif
typedef double   FTGL_DOUBLE;
typedef float    FTGL_FLOAT;


#define FTGL_EXPORT


// Fixes for deprecated identifiers in 2.1.5
#ifndef FT_OPEN_MEMORY
    #define FT_OPEN_MEMORY (FT_Open_Flags)1
#endif

#ifndef FT_RENDER_MODE_MONO
    #define FT_RENDER_MODE_MONO ft_render_mode_mono
#endif

#ifndef FT_RENDER_MODE_NORMAL
    #define FT_RENDER_MODE_NORMAL ft_render_mode_normal
#endif


#endif  //  __FTGL__
