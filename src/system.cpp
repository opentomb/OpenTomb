#include "system.h"

#include <cstdio>
#include <cstdlib>

#include "console.h"
#include "engine.h"
#include "gui.h"

ScreenInfo     screen_info;
SystemSettings system_settings;


TextLine system_fps;

// =======================================================================
// General routines
// =======================================================================

void Sys_Printf(char *fmt, ...)
{
    va_list     argptr;
    char        text[4096];

    va_start(argptr, fmt);
    vsnprintf(text, 4096, fmt, argptr);
    va_end(argptr);
    fprintf(stderr, "%s", text);

    //Con_Print (text);
}

void Sys_Init()
{
    system_fps.text.clear();

    system_fps.X = (10.0);
    system_fps.Y = (10.0);
    system_fps.Xanchor = HorizontalAnchor::Right;
    system_fps.Yanchor = VerticalAnchor::Bottom;

    system_fps.font_id = FontType::Primary;
    system_fps.style_id = FontStyle::MenuTitle;

    system_fps.show = true;

    Gui_AddLine(&system_fps);
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

    system_settings.logging = false;
}

void Sys_Destroy()
{
    system_fps.show = false;
    system_fps.text.clear();
}

void Sys_Error(const char *error, ...)
{
    va_list     argptr;
    char        string[4096];

    va_start(argptr, error);
    vsnprintf(string, 4096, error, argptr);
    va_end(argptr);

    Sys_DebugLog(LOG_FILENAME, "System error: %s", string);
    Engine_Shutdown(1);
}

void Sys_Warn(const char *warning, ...)
{
    va_list     argptr;
    char        string[4096];

    va_start(argptr, warning);
    vsnprintf(string, 4096, warning, argptr);
    va_end(argptr);
    Sys_DebugLog(LOG_FILENAME, "Warning: %s", string);
}

void Sys_DebugLog(const char *file, const char *fmt, ...)
{
    if(!system_settings.logging) return;

    va_list argptr;
    static char data[4096];
    FILE *fp;

    va_start(argptr, fmt);
    sprintf(data, "\n");
    vsnprintf(&data[1], 4095, fmt, argptr);
    va_end(argptr);
    fp = fopen(file, "a");
    if(fp == nullptr)
    {
        fp = fopen(file, "w");
    }
    if(fp != nullptr)
    {
        fwrite(data, strlen(data), 1, fp);
        fclose(fp);
    }
    fwrite(data, strlen(data), 1, stderr);
}
