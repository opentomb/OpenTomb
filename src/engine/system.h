#pragma once

#include <cstdint>

#include <GL/glew.h>

namespace engine
{

#define LOG_FILENAME     "debug.log"
#define LUA_LOG_FILENAME "lua.log"
#define GL_LOG_FILENAME  "gl.log"

struct SystemSettings
{
    bool        logging = false;
};

struct ScreenInfo
{
    int16_t     x;
    int16_t     y;
    int16_t     w;  GLfloat w_unit;   // Metering unit.
    int16_t     h;  GLfloat h_unit;   // Metering unit.

    float       fps;
    float       fov;
    float       scale_factor;
    bool        FS_flag;
    bool        show_debuginfo;
    bool        vsync;
};

extern ScreenInfo screen_info;
extern SystemSettings system_settings;

void Sys_Init();
void Sys_InitGlobals();
void Sys_Destroy();

void Sys_Init(void);

} // namespace engine
