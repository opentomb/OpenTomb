
#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#define SYS_LOG_FILENAME            "d_log.txt"

// Screen metering resolution specifies user-friendly relative dimensions of screen,
// which are not dependent on screen resolution. They're primarily used to parse
// bar and string dimensions.

#define SYS_SCREEN_METERING_RESOLUTION (1000.0f)

typedef struct system_settings_s
{
    uint32_t    logging : 1;
} system_settings_t, *system_settings_p;

typedef struct screen_info_s
{
    int16_t     x;
    int16_t     y;
    int16_t     w;
    int16_t     h;

    float       fps;
    float       fov;
    float       scale_factor;
    uint32_t    debug_view_state : 8;
    uint32_t    fullscreen : 1;
    uint32_t    crosshair : 1;
} screen_info_t, *screen_info_p;

extern system_settings_t system_settings;
extern screen_info_t screen_info;

void Sys_Init();
void Sys_InitGlobals();
void Sys_Destroy();

void *Sys_GetTempMem(size_t size);
void Sys_ReturnTempMem(size_t size);
void Sys_ResetTempMem();

float Sys_FloatTime(void);
void Sys_Strtime(char *buf, size_t buf_size);

void Sys_Init(void);
void Sys_Error(const char *error, ...);
void Sys_Warn(const char *warning, ...);
void Sys_DebugLog(const char *file, const char *fmt, ...);

void Sys_TakeScreenShot();

int Sys_FileFound(const char *name, int checkWrite);

#define Sys_LogCurrPlace Sys_DebugLog(SYS_LOG_FILENAME, "\"%s\" str = %d\n", __FILE__, __LINE__);
#define Sys_extError(...) {Sys_LogCurrPlace Sys_Error(__VA_ARGS__);}
#define Sys_extWarn(...) {Sys_LogCurrPlace Sys_Warn(__VA_ARGS__);}

#ifdef	__cplusplus
}
#endif

#endif
