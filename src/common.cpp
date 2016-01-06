#include "common.h"

#include <cstdio>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <CImg.h>

#include "loader/level.h"

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
    glGetIntegerv(GL_VIEWPORT, ViewPort);

    const auto str_size = ViewPort[2] * 4;

    std::vector<GLubyte> pixels(str_size * ViewPort[3]);
    glReadPixels(0, 0, ViewPort[2], ViewPort[3], GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    std::vector<GLubyte> buf(str_size);
    for(int h = 0; h < ViewPort[3] / 2; h++)
    {
        memcpy(buf.data(), &pixels[h * str_size], str_size);
        memcpy(&pixels[h * str_size], &pixels[(ViewPort[3] - h - 1) * str_size], str_size);
        memcpy(&pixels[(ViewPort[3] - h - 1) * str_size], buf.data(), str_size);
    }

    cimg_library::CImg<uint8_t> img(pixels.data(), ViewPort[2], ViewPort[3], 1, 4, true);
    img.save("screen.png", screenshot_cnt, 5);
    screenshot_cnt++;
}
