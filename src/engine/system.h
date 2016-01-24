#pragma once

#include <glm/glm.hpp>

#include <cstdint>

namespace engine
{
#define LOG_FILENAME     "debug.log"
#define LUA_LOG_FILENAME "lua.log"
#define GL_LOG_FILENAME  "gl.log"

struct SystemSettings
{
    bool logging = false;
};

struct ScreenInfo
{
    int16_t     x = 50;
    int16_t     y = 20;
    int16_t     w = 800;
    glm::float_t w_unit;   // Metering unit.
    int16_t     h = 600;
    glm::float_t h_unit;   // Metering unit.

    float       fps;
    float       fov = 75.0f;
    float       scale_factor;
    bool        FS_flag = false;
    bool        show_debuginfo = false;
    bool        vsync;
};

extern ScreenInfo screen_info;
extern SystemSettings system_settings;

void Sys_InitGlobals();
} // namespace engine
