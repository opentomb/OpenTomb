
#include <SDL2/SDL_opengl.h>
#include <stdio.h>

#include "system.h"
#include "console.h"
#include "script.h"
#include "vt/vt_level.h"

static int screenshot_cnt = 0;

extern screen_info_t screen_info;

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
    GLubyte *rect;

    glGetIntegerv(GL_VIEWPORT, ViewPort);

    snprintf(fname, 128, "screen_%0.5d.tga", screenshot_cnt);

    rect = (GLubyte*)malloc(ViewPort[2] * ViewPort[3] * 4);
    glReadPixels(0, 0, ViewPort[2], ViewPort[3], GL_RGBA, GL_UNSIGNED_BYTE, rect);
    WriteTGAfile(fname, (uint8_t*)rect, ViewPort[2], ViewPort[3], 1);
    free(rect);
    screenshot_cnt++;
}
