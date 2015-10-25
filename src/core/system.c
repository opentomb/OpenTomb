
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_audio.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "system.h"
#include "console.h"
#include "gl_util.h"

#define INIT_TEMP_MEM_SIZE          (4096 * 1024)

screen_info_t           screen_info;

extern lua_State       *engine_lua;

static uint8_t         *engine_mem_buffer             = NULL;
static size_t           engine_mem_buffer_size        = 0;
static size_t           engine_mem_buffer_size_left   = 0;
static int              screenshot_cnt                = 0;

// =======================================================================
// General routines
// =======================================================================

void Sys_Init()
{
    engine_mem_buffer               = (uint8_t*)malloc(INIT_TEMP_MEM_SIZE);
    engine_mem_buffer_size          = INIT_TEMP_MEM_SIZE;
    engine_mem_buffer_size_left     = INIT_TEMP_MEM_SIZE;
}


void Sys_InitGlobals()
{
    screen_info.x = 50;
    screen_info.y = 20;
    screen_info.w = 800;
    screen_info.h = 600;
    screen_info.FS_flag = 0;
    screen_info.show_debuginfo = 0;
    screen_info.fov = 75.0;
}


void Sys_Destroy()
{
    if(engine_mem_buffer)
    {
        free(engine_mem_buffer);
    }
    engine_mem_buffer               = NULL;
    engine_mem_buffer_size          = 0;
    engine_mem_buffer_size_left     = 0;
}


void *Sys_GetTempMem(size_t size)
{
    void *ret = NULL;

    if(engine_mem_buffer_size_left >= size)
    {
        ret = engine_mem_buffer + engine_mem_buffer_size - engine_mem_buffer_size_left;
        engine_mem_buffer_size_left -= size;
    }
    else
    {
        engine_mem_buffer_size_left = engine_mem_buffer_size;                   // glitch generator, but not crash
        ret = engine_mem_buffer;
    }

    return ret;
}


void Sys_ReturnTempMem(size_t size)
{
    if(engine_mem_buffer_size_left + size <= engine_mem_buffer_size)
    {
        engine_mem_buffer_size_left += size;
    }
}


void Sys_ResetTempMem()
{
    engine_mem_buffer_size_left = engine_mem_buffer_size;
}


/*
===============================================================================
SYS TIME
===============================================================================
*/
float Sys_FloatTime (void)
{
    struct              timeval tp;
    static long int     secbase = 0;

    gettimeofday(&tp, NULL);

    if (!secbase)
    {
        secbase = tp.tv_sec;
        return tp.tv_usec * 1.0e-6;
    }

    return (float)(tp.tv_sec - secbase) + (float)tp.tv_usec * 1.0e-6;
}


void Sys_Strtime(char *buf, size_t buf_size)
{
    struct tm *tm_;
    static time_t t_;

    time(&t_);
    tm_=gmtime(&t_);

    snprintf(buf, buf_size, "%02d:%02d:%02d",tm_->tm_hour,tm_->tm_min,tm_->tm_sec);
}

/*
===============================================================================
SYS PRINT FUNCTIONS
===============================================================================
*/
void Sys_Printf(char *fmt, ...)
{
    va_list     argptr;
    char        text[4096];

    va_start (argptr,fmt);
    vsnprintf (text, 4096, fmt,argptr);
    va_end (argptr);
    fprintf(stderr, "%s", text);

    //Con_Print (text);
}


void Sys_Error(const char *error, ...)
{
    va_list     argptr;
    char        string[4096];

    va_start (argptr,error);
    vsnprintf (string, 4096, error, argptr);
    va_end (argptr);

    Sys_DebugLog(SYS_LOG_FILENAME, "System error: %s", string);
    //Engine_Shutdown(1);
    exit(1);
}


void Sys_Warn(const char *warning, ...)
{
    va_list     argptr;
    char        string[4096];

    va_start (argptr, warning);
    vsnprintf (string, 4096, warning, argptr);
    va_end (argptr);
    Sys_DebugLog(SYS_LOG_FILENAME, "Warning: %s", string);
    Con_Warning("Warning: %s", string);
}


void Sys_DebugLog(const char *file, const char *fmt, ...)
{
    va_list argptr;
    static char data[4096];
    FILE *fp;

    va_start(argptr, fmt);
    data[0] = '\n';
    vsnprintf(&data[1], 4095, fmt, argptr);
    va_end(argptr);
    fp = fopen(file, "a");
    if(fp == NULL)
    {
        fp = fopen(file, "w");
    }
    if(fp != NULL)
    {
        fwrite(data, strlen(data), 1, fp);
        fclose(fp);
    }
    fwrite(data, strlen(data), 1, stderr);
}


void Sys_TakeScreenShot()
{
    GLint ViewPort[4], h, h2;
    char fname[128];
    GLubyte *pixels;
    SDL_Surface *surface;
    uint32_t str_size;

    qglGetIntegerv(GL_VIEWPORT, ViewPort);
    snprintf(fname, 128, "screen_%.5d.png", screenshot_cnt);
    str_size = ViewPort[2] * 4;
    pixels = (GLubyte*)malloc(str_size * ViewPort[3]);
    qglReadPixels(0, 0, ViewPort[2], ViewPort[3], GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    GLubyte buf[str_size];
    h2 = ViewPort[3] / 2;
    for(h = 0; h < h2; h++)
    {
        memcpy(buf, pixels + h * str_size, str_size);
        memcpy(pixels + h * str_size, pixels + (ViewPort[3] - h - 1) * str_size, str_size);
        memcpy(pixels + (ViewPort[3] - h - 1) * str_size, buf, str_size);
    }
    surface = SDL_CreateRGBSurfaceFrom(NULL, ViewPort[2], ViewPort[3], 32, str_size, 0x000000FF, 0x00000FF00, 0x00FF0000, 0xFF000000);
    surface->format->format = SDL_PIXELFORMAT_RGBA8888;
    surface->pixels = pixels;
    IMG_SavePNG(surface, fname);

    surface->pixels = NULL;
    SDL_FreeSurface(surface);
    free(pixels);
    screenshot_cnt++;
}


int Sys_FileFound(const char *name, int checkWrite)
{
    SDL_RWops *ff;

    if(checkWrite)
    {
        ff = SDL_RWFromFile(name, "ab");
    }
    else
    {
        ff = SDL_RWFromFile(name, "rb");
    }

    if(!ff)
    {
        return 0;
    }
    else
    {
        SDL_RWclose(ff);
        return 1;
    }
}