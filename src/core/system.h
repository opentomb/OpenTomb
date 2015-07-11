
#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#define SYS_LOG_FILENAME            "d_log.txt"
    
typedef struct screen_info_s
{
    int16_t     x;
    int16_t     y;
    int16_t     w;  
    int16_t     h;  
    GLfloat     w_unit;   // Metering unit.
    GLfloat     h_unit;   // Metering unit.
    
    float       fps;
    float       fov;
    float       scale_factor;
    int8_t      FS_flag;
    int8_t      show_debuginfo;
} screen_info_t, *screen_info_p;

extern screen_info_t screen_info;

void Sys_Init();
void Sys_InitGlobals();
void Sys_Destroy();

void *Sys_GetTempMem(size_t size);
void Sys_ReturnTempMem(size_t size);
void Sys_ResetTempMem();

float Sys_FloatTime(void);
void Sys_Strtime(char *buf, size_t buf_size);
void Sys_StrRunSec(char *buf, size_t buf_size);

void Sys_Printf(char *fmt, ...);
void Sys_Init(void);
void Sys_Error(const char *error, ...);
void Sys_Warn(const char *warning, ...);
void Sys_DebugLog(const char *file, const char *fmt, ...);

void Sys_TakeScreenShot();

#define Sys_LogCurrPlace Sys_DebugLog(SYS_LOG_FILENAME, "\"%s\" str = %d\n", __FILE__, __LINE__);
#define Sys_extError(...) {Sys_LogCurrPlace Sys_Error(__VA_ARGS__);}
#define Sys_extWarn(...) {Sys_LogCurrPlace Sys_Warn(__VA_ARGS__);}

#ifdef	__cplusplus
}
#endif

#endif
