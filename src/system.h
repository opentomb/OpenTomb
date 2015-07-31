#pragma once

#include <cstdint>
#include <LinearMath/btScalar.h>

#include "gl_util.h"

#define LOG_FILENAME     "debug.log"
#define LUA_LOG_FILENAME "lua.log"
#define GL_LOG_FILENAME  "gl.log"

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
};

extern ScreenInfo screen_info;

void Sys_Init();
void Sys_InitGlobals();
void Sys_Destroy();

void Sys_Strtime(char *buf, size_t buf_size);
void Sys_StrRunSec(char *buf, size_t buf_size);
btScalar Sys_FloatTime(void);
void Sys_Printf(char *fmt, ...);
void Sys_Init(void);
#ifdef __GNUC__
void Sys_Error(const char *error, ...) __attribute__((noreturn));
#else
void Sys_Error(const char *error, ...);
#endif
void Sys_Warn(const char *warning, ...);
void Sys_DebugLog(const char *file, const char *fmt, ...);

#define Sys_LogCurrPlace Sys_DebugLog(LOG_FILENAME, "\"%s\" str = %d\n", __FILE__, __LINE__);
#define Sys_extError(...) {Sys_LogCurrPlace Sys_Error(__VA_ARGS__);}
#define Sys_extWarn(...) {Sys_LogCurrPlace Sys_Warn(__VA_ARGS__);}
