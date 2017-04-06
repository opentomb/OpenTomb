
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#ifndef _MSC_VER
#include <sys/time.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
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
    screen_info.debug_view_state = 0;
    screen_info.fullscreen = 0;
    screen_info.crosshair = 0;
    screen_info.fov = 75.0;
    screen_info.scale_factor = 1.0f;
    screen_info.fps = 0.0f;
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

#if _MSC_VER
typedef struct timeval {
	long tv_sec;
	long tv_usec;
} timeval;

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}
#endif

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
    int32_t written;

    va_start(argptr, fmt);
    written = vsnprintf(data, sizeof(data), fmt, argptr);
    va_end(argptr);

    if(written > 0)
    {
        SDL_RWops *fp;
        // Add newline at end (if possible)
        if((written + 1) < sizeof(data))
        {
            data[written + 0] = '\n';
            data[written + 1] = 0;
            written += 1;
        }

        fp = SDL_RWFromFile(file, "a");
        if(fp == NULL)
        {
            fp = SDL_RWFromFile(file, "w");
        }
        if(fp != NULL)
        {
            SDL_RWwrite(fp, data, written, 1);
            SDL_RWclose(fp);
        }
        fwrite(data, written, 1, stderr);
    }
}


void WriteTGAfile(const char *filename, const uint8_t *data, const int width, const int height, char invY)
{
    unsigned char c;
    unsigned short s;
    SDL_RWops *st;

    st = SDL_RWFromFile(filename, "wb");
    if(st)
    {
        // write the header
        // id_length
        c = 0;
        SDL_RWwrite(st, &c, sizeof(c), 1);
        // colormap_type
        c = 0;
        SDL_RWwrite(st, &c, sizeof(c), 1);
        // image_type
        c = 2;
        SDL_RWwrite(st, &c, sizeof(c), 1);
        // colormap_index
        s = 0;
        SDL_RWwrite(st, &s, sizeof(s), 1);
        // colormap_length
        s = 0;
        SDL_RWwrite(st, &s, sizeof(s), 1);
        // colormap_size
        c = 0;
        SDL_RWwrite(st, &c, sizeof(c), 1);
        // x_origin
        s = 0;
        SDL_RWwrite(st, &s, sizeof(s), 1);
        // y_origin
        s = 0;
        SDL_RWwrite(st, &s, sizeof(s), 1);
        // width
        s = SDL_SwapLE16(width);
        SDL_RWwrite(st, &s, sizeof(s), 1);
        // height
        s = SDL_SwapLE16(height);
        SDL_RWwrite(st, &s, sizeof(s), 1);
        // bits_per_pixel
        c = 32;
        SDL_RWwrite(st, &c, sizeof(c), 1);
        // attributes
        c = 0;
        SDL_RWwrite(st, &c, sizeof(c), 1);

        if(invY)
        {
            int y;
            for (y = 0; y < height; y++)
            {
                SDL_RWwrite(st, &data[y * 4 * width], width * 4, 1);
            }
        }
        else
        {
            int y;
            for (y = height-1; y >= 0; y--)
            {
                SDL_RWwrite(st, &data[y * 4 * width], width * 4, 1);
            }
        }
        SDL_RWclose(st);
    }
}


void Sys_TakeScreenShot()
{
    static int screenshot_cnt = 0;
    GLint ViewPort[4];
    char fname[128];
    GLubyte *pixels;
    uint32_t str_size;

    qglGetIntegerv(GL_VIEWPORT, ViewPort);
    snprintf(fname, 128, "screen_%.5d.tga", screenshot_cnt);
    str_size = ViewPort[2] * 4;
    pixels = (GLubyte*)malloc(str_size * ViewPort[3]);
    qglReadPixels(0, 0, ViewPort[2], ViewPort[3], GL_BGRA, GL_UNSIGNED_BYTE, pixels);
    WriteTGAfile(fname, (const uint8_t*)pixels, ViewPort[2], ViewPort[3], 1);

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

    if(ff)
    {
        SDL_RWclose(ff);
        return 1;
    }
    return 0;
}
