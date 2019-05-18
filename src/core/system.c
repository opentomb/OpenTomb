
#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_audio.h>
#include <sys/stat.h>
#ifdef __GNUC__
#   include <sys/time.h>
#else
#   include "timer.h"
#endif

#include <time.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "system.h"
#include "utf8_32.h"
#include "console.h"
#include "gl_util.h"

#define INIT_TEMP_MEM_SIZE          (4096 * 1024)

// stupid broken defines checking in internal stat.h
#ifndef S_IFMT
#define S_IFMT __S_IFMT
#endif

#ifndef S_IFDIR
#define S_IFDIR __S_IFDIR
#endif

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

static int file_info_cmp(file_info_p f1, file_info_p f2)
{
    if(f1->is_dir && !f2->is_dir)
    {
        return -1;
    }
    else if(!f1->is_dir && f2->is_dir)
    {
        return 1;
    }
    else
    {
        uint32_t c1, c2;
        uint8_t *s1 = (uint8_t*)f1->name;
        uint8_t *s2 = (uint8_t*)f2->name;
        while(*s1 && *s2)
        {
            s1 = utf8_to_utf32(s1, &c1);
            s2 = utf8_to_utf32(s2, &c2);
            if(c1 != c2)
            {
                return (c1 < c2) ? (-1) : (1);
            }
        }
    }
    return 0;
}

#define WILD_COMPARE_SYM(strCh, wildCh) ((strCh == wildCh) || ('?' == wildCh))

/*
 * '*' (asterisk):          Matches zero or more characters.
 * '?' (question mark):     Matches a single character.
 */
static int wildcmp(uint8_t *wild, uint8_t *string)
{
    // Written by Jack Handy - <A href="mailto:jakkhandy@hotmail.com">jakkhandy@hotmail.com</A>
    // Taken from http://www.codeproject.com/KB/string/wildcmp.aspx
    uint8_t *cp = NULL, *mp = NULL;
    uint8_t *string_next, *wild_next;
    uint32_t wild_ch, string_ch;
    
    // check pattern until first star or end
    while((*string) && (*wild != '*'))
    {
        wild = utf8_to_utf32(wild, &wild_ch);
        string = utf8_to_utf32(string, &string_ch);
        if(!WILD_COMPARE_SYM(string_ch, wild_ch))
        {
            return 0;
        }
    }
    
    while(*string)
    {
        if(*wild == '*')
        {
            if(!*++wild)
            {
                return 1;
            }
            // save entrance for star
            mp = wild;
            cp = utf8_next_symbol(string);
        }
        else if((string_next = utf8_to_utf32(string, &string_ch)), (wild_next = utf8_to_utf32(wild, &wild_ch)), WILD_COMPARE_SYM(string_ch, wild_ch))
        {
            wild = wild_next;
            string = string_next;
        }
        else
        {
            // increase star pattern part
            wild = mp;
            string = cp;
            cp = utf8_next_symbol(cp);
        }
    }

    while(*wild == '*')
    {
        wild++;
    }

    return !*wild;
}

#undef WILD_COMPARE_SYM

file_info_p Sys_ListDir(const char *path, const char *wild)
{
    file_info_p ret = NULL;
    DIR *dir = opendir(path);
    if(dir)
    {
        struct stat st;
        struct dirent *d;
        file_info_p *ins;
        int base_len = strlen(path);
        char *buff = (char*)malloc(base_len + sizeof(((struct dirent*)0)->d_name) + 1);
        strncpy(buff, path, sizeof(((struct dirent*)0)->d_name));
        buff[base_len++] = '/';
        while((d = readdir(dir)))
        {
            if(!wild || wildcmp((uint8_t*)wild, (uint8_t*)d->d_name))
            {
                strncpy(buff + base_len, d->d_name, sizeof(((struct dirent*)0)->d_name));
                if(d->d_name[0] && (d->d_name[0] != '.') && !stat(buff, &st))
                {
                    int local_len = strnlen(d->d_name, sizeof(((struct dirent*)0)->d_name));
                    file_info_p p = (file_info_p)malloc(sizeof(file_info_t));
                    p->next = NULL;
                    p->full_name = (char*)malloc(base_len + local_len + 1);
                    p->name = p->full_name + base_len;
                    strncpy(p->full_name, buff, base_len + local_len + 1);
                    p->is_dir = ((st.st_mode & S_IFMT) == S_IFDIR) ? (0x01) : (0x00);
                    for(ins = &ret; *ins && (0 < file_info_cmp(p, *ins)); ins = &((*ins)->next));
                    p->next = *ins;
                    *ins = p;
                }
            }
        }
        free(buff);
        closedir(dir);
    }
    return ret;
}


void Sys_ListDirFree(file_info_p list)
{
    while(list)
    {
        file_info_p p = list->next;
        free(list->full_name);
        free(list);
        list = p;
    }
}


/*
===============================================================================
SYS TIME
===============================================================================
*/

int64_t Sys_MicroSecTime(int64_t sec_offset)
{
    int64_t ret = 0;
    struct timeval tp;
#ifdef __GNUC__
    if (0 == gettimeofday(&tp, NULL))
#else
    if (0 == _gettimeofday(&tp, NULL))
#endif
    {
        ret = tp.tv_sec - sec_offset;
        ret *= 1e6;
        ret += tp.tv_usec;
    }
    return ret;
}


void Sys_Strtime(char *buf, size_t buf_size)
{
    struct tm *tm_;
    static time_t t_;

    time(&t_);
    tm_ = gmtime(&t_);

    snprintf(buf, buf_size, "%02d:%02d:%02d", tm_->tm_hour, tm_->tm_min, tm_->tm_sec);
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

    va_start(argptr,error);
    vsnprintf(string, sizeof(string), error, argptr);
    va_end(argptr);

    Sys_DebugLog(SYS_LOG_FILENAME, "System error: %s", string);
    //Engine_Shutdown(1);
    exit(1);
}


void Sys_Warn(const char *warning, ...)
{
    va_list     argptr;
    char        string[4096];

    va_start (argptr, warning);
    vsnprintf (string, sizeof(string), warning, argptr);
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


void Sys_WriteTGAfile(const char *filename, const uint8_t *data, const int width, const int height, int bpp, char invY)
{
    uint8_t c;
    uint16_t s;
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
        c = 0xFF & bpp;
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
    snprintf(fname, sizeof(fname), "screen_%.5d.tga", screenshot_cnt);
    str_size = ViewPort[2] * 4;
    pixels = (GLubyte*)malloc(str_size * ViewPort[3]);
    qglReadPixels(0, 0, ViewPort[2], ViewPort[3], GL_BGRA, GL_UNSIGNED_BYTE, pixels);
    Sys_WriteTGAfile(fname, (const uint8_t*)pixels, ViewPort[2], ViewPort[3], 32, 1);

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
