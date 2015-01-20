
#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include "gl_util.h"
#include "bullet/LinearMath/btScalar.h"

#define LOG_FILENAME "d_log.txt"

typedef struct screen_info_s
{
    int16_t     x;
    int16_t     y;
    int16_t     w;  GLfloat w_unit;   // Metering unit.
    int16_t     h;  GLfloat h_unit;   // Metering unit.
    
    float       fps;
    float       fov;
    int8_t      FS_flag;
    int8_t      show_debuginfo;
} screen_info_t, *screen_info_p;

extern screen_info_t screen_info;

void Sys_Init();
void Sys_Destroy();

void Sys_Strtime(char *buf, size_t buf_size);
void Sys_StrRunSec(char *buf, size_t buf_size);
btScalar Sys_FloatTime(void);
void Sys_Printf(char *fmt, ...);
void Sys_Init(void);
void Sys_Error(const char *error, ...);
void Sys_Warn(const char *warning, ...);
void Sys_DebugLog(const char *file, const char *fmt, ...);

#define Sys_LogCurrPlace Sys_DebugLog(LOG_FILENAME, "\"%s\" str = %d\n", __FILE__, __LINE__);
#define Sys_extError(...) {Sys_LogCurrPlace Sys_Error(__VA_ARGS__);}
#define Sys_extWarn(...) {Sys_LogCurrPlace Sys_Warn(__VA_ARGS__);}

#endif
