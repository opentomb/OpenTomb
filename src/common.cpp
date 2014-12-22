
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>

#include "system.h"
#include "console.h"
#include "script.h"
#include "vt/vt_level.h"
#include "vmath.h"

static int screenshot_cnt = 0;

void Com_Init()
{

}

void Com_Destroy()
{

}

void Com_TakeScreenShot()
{
    GLint ViewPort[4];
    char fname[128];
    GLubyte *pixels;
    SDL_Surface *surface;
    uint32_t str_size;

    glGetIntegerv(GL_VIEWPORT, ViewPort);
    snprintf(fname, 128, "screen_%0.5d.png", screenshot_cnt);
    str_size = ViewPort[2] * 4;
    pixels = (GLubyte*)malloc(str_size * ViewPort[3]);
    glReadPixels(0, 0, ViewPort[2], ViewPort[3], GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    GLubyte buf[str_size];
    for(int h=0;h<ViewPort[3]/2;h++)
    {
        memcpy(buf, pixels + h * str_size, str_size);
        memcpy(pixels + h * str_size, pixels + (ViewPort[3] - h - 1) * str_size, str_size);
        memcpy(pixels + (ViewPort[3] - h - 1) * str_size, buf, str_size);
    }
    ///@TODO: use surface creation without pixels dublication
    surface = SDL_CreateRGBSurfaceFrom(pixels, ViewPort[2], ViewPort[3], 32, str_size, 0x000000FF, 0x00000FF00, 0x00FF0000, 0xFF000000);
    surface->format->format = SDL_PIXELFORMAT_RGBA8888;

    IMG_SavePNG(surface, fname);
    SDL_FreeSurface(surface);
    free(pixels);
    screenshot_cnt++;
}
