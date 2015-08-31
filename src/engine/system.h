#pragma once

#include <cstdint>

#include <LinearMath/btScalar.h>

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

void Sys_Printf(char *fmt, ...);
void Sys_Init(void);
#ifdef __GNUC__
void Sys_Error(const char *error, ...) __attribute__((noreturn));
#else
void Sys_Error(const char *error, ...);
#endif
void Sys_Warn(const char *warning, ...);
void Sys_DebugLog(const char *file, const char *fmt, ...);

#define SYS_LOG_CURR_PLACE Sys_DebugLog(LOG_FILENAME, "\"%s\" str = %d\n", __FILE__, __LINE__);
#define SYS_EXT_ERROR(...) {SYS_LOG_CURR_PLACE Sys_Error(__VA_ARGS__);}
#define SYS_EXT_WARN(...) {SYS_LOG_CURR_PLACE Sys_Warn(__VA_ARGS__);}

} // namespace engine
