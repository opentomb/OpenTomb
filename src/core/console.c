
#include <stdio.h>
#include <stdlib.h>

#include <memory.h>

#include "utf8_32.h"
#include "gl_font.h"
#include "gl_util.h"
#include "console.h"
#include "system.h"
#include "vmath.h"
#include "gl_text.h"


static struct
{
    uint8_t                     background_color[4];
    uint16_t                    lines_count;
    uint16_t                    commands_count;
    uint16_t                    lines_buff_size;
    uint16_t                    commands_buff_size;
    uint16_t                   *lines_styles;
    char                      **lines_buff;
    char                      **commands_buff;
    
    int                       (*exec_cmd)(char *ch);        // Exec function pointer

    uint16_t                    height;                     // Console height in pixels
    int16_t                     command_pos;
    
    float                       spacing;                    // Line spacing
} con_base;


static void Con_AddCommandToHistory(const char *text);

void Con_Init()
{
    con_base.lines_buff_size = 128;
    con_base.lines_buff = (char**)calloc(con_base.lines_buff_size, sizeof(char*));
    con_base.lines_styles = (uint16_t*)calloc(con_base.lines_buff_size, sizeof(uint16_t));
    con_base.commands_buff_size = 128;   
    con_base.commands_buff = (char**)calloc(con_base.commands_buff_size, sizeof(char*));
    
    con_base.lines_count = 0;
    con_base.commands_count = 0;
    con_base.command_pos = 0;
    con_base.height = 240;

    // spacing check
    if(con_base.spacing < CON_MIN_LINE_INTERVAL)
    {
        con_base.spacing = CON_MIN_LINE_INTERVAL;
    }
    if(con_base.spacing > CON_MAX_LINE_INTERVAL)
    {
        con_base.spacing = CON_MAX_LINE_INTERVAL;
    }
}


void Con_SetExecFunction(int(*exec_cmd)(char *ch))
{
    con_base.exec_cmd = exec_cmd;
}


void Con_InitGlobals()
{
    con_base.background_color[0] = 1.0f;
    con_base.background_color[1] = 0.9f;
    con_base.background_color[2] = 0.7f;
    con_base.background_color[3] = 0.4f;

    con_base.exec_cmd        = NULL;
    con_base.spacing         = CON_MIN_LINE_INTERVAL;
    con_base.height          = 240;
}


void Con_Destroy()
{
    con_base.lines_count = 0;
    if(con_base.lines_buff)
    {
        for(uint16_t i = 0; i < con_base.lines_buff_size; ++i)
        {
            if(con_base.lines_buff[i])
            {
                free(con_base.lines_buff[i]);
            }
        }
        free(con_base.lines_buff);
        con_base.lines_buff = NULL;
        con_base.lines_buff_size = 0;
    }
    
    con_base.commands_count = 0;
    if(con_base.commands_buff)
    {
        for(uint16_t i = 0; i < con_base.commands_buff_size; ++i)
        {
            if(con_base.commands_buff[i])
            {
                free(con_base.commands_buff[i]);
            }
        }
        free(con_base.commands_buff);
        con_base.commands_buff = NULL;
        con_base.commands_buff_size = 0;
    }
}


void Con_SetParams(console_params_p cp)
{
    con_base.background_color[0] = cp->background_color[0];
    con_base.background_color[1] = cp->background_color[1];
    con_base.background_color[2] = cp->background_color[2];
    con_base.background_color[3] = cp->background_color[3];
    
    Con_SetCommandsHistorySize(cp->commands_count);
    Con_SetLinesHistorySize(cp->lines_count);
    con_base.height = cp->height;
    if((cp->spacing >= CON_MIN_LINE_INTERVAL) && (cp->spacing <= CON_MAX_LINE_INTERVAL))
    {
        con_base.spacing = cp->spacing;
    }
}


void Con_GetParams(console_params_p cp)
{
    cp->background_color[0] = con_base.background_color[0];
    cp->background_color[1] = con_base.background_color[1];
    cp->background_color[2] = con_base.background_color[2];
    cp->background_color[3] = con_base.background_color[3];
    
    cp->commands_count = con_base.commands_buff_size;
    cp->lines_count = con_base.lines_buff_size;
    cp->height = con_base.height;
    cp->spacing = con_base.spacing;
}


void Con_GetLines(uint16_t *lines_count, char ***lines_buff, uint16_t **lines_styles)
{
    *lines_count = con_base.lines_count;
    *lines_buff = con_base.lines_buff;
    *lines_styles = con_base.lines_styles;
}


char *Con_ListExecHistory(int direction)
{
    char *ret = NULL;
    if(direction && (con_base.commands_count > 0))
    {
        ret = con_base.commands_buff[con_base.command_pos];
        con_base.command_pos++;
        con_base.command_pos = (con_base.command_pos >= con_base.commands_count) ? (0) : (con_base.command_pos);
    }
    if(!direction && (con_base.commands_count > 0))
    {
        ret = con_base.commands_buff[con_base.command_pos];
        con_base.command_pos--;
        con_base.command_pos = (con_base.command_pos < 0) ? (con_base.commands_count - 1) : (con_base.command_pos);
    }
    return ret;
}


void Con_SetLinesHistorySize(uint16_t count)
{
    if((count >= 16) && (count <= 32767) && (count != con_base.lines_buff_size))
    {
        char **new_buff = (char**)calloc(count, sizeof(char*));
        for(uint16_t i = 0; i < con_base.lines_count; ++i)
        {
            if(i < count)
            {
                new_buff[i] = con_base.lines_buff[i];
            }
            else
            {
                free(con_base.lines_buff[i]);
            }
        }
        free(con_base.lines_buff);
        con_base.lines_buff = new_buff;
        con_base.lines_buff_size = count;
        con_base.lines_count = (con_base.lines_count < count) ? (con_base.lines_count) : (count - 1);
    }
}


void Con_SetCommandsHistorySize(uint16_t count)
{
    if((count >= 16) && (count <= 32767) && (count != con_base.commands_buff_size))
    {
        char **new_buff = (char**)calloc(count, sizeof(char*));
        for(uint16_t i = 0; i < con_base.commands_count; ++i)
        {
            if(i < count)
            {
                new_buff[i] = con_base.commands_buff[i];
            }
            else
            {
                free(con_base.commands_buff[i]);
            }
        }
        free(con_base.commands_buff);
        con_base.commands_buff = new_buff;
        con_base.commands_buff_size = count;
        con_base.commands_count = (con_base.commands_count < count) ? (con_base.commands_count) : (count - 1);
    }
}


void Con_Exec(char *text)
{
    Con_AddCommandToHistory(text);
    if(con_base.exec_cmd && !con_base.exec_cmd(text))
    {
        Con_AddLine(text, 0);
    }
    con_base.command_pos = (con_base.commands_count > 0) ? (con_base.commands_count - 1) : (0);
}


void Con_AddCommandToHistory(const char *text)
{
    if(text && *text)
    {
        uint32_t len = strlen(text);
        char *str = (char*)malloc((len + 1) * sizeof(char));
        memcpy(str, text, len);
        str[len] = 0;
        if(con_base.commands_count + 1 < con_base.commands_buff_size)
        {
            con_base.commands_buff[con_base.commands_count++] = str;
        }
        else
        {
            free(con_base.commands_buff[0]);
            for(uint16_t i = 1; i < con_base.commands_buff_size; ++i)
            {
                con_base.commands_buff[i - 1] = con_base.commands_buff[i];
            }
            con_base.commands_buff[con_base.commands_buff_size - 1] = str;
        }
    }
}


void Con_AddLine(const char *text, uint16_t font_style)
{
    if(text && *text)
    {
        uint32_t len = strlen(text);
        char *str = (char*)malloc((len + 1) * sizeof(char));
        memcpy(str, text, len);
        str[len] = 0;
        if(con_base.lines_count < con_base.lines_buff_size)
        {
            con_base.lines_styles[con_base.lines_count] = font_style;
            con_base.lines_buff[con_base.lines_count++] = str;
        }
        else
        {
            free(con_base.lines_buff[0]);
            for(uint16_t i = 1; i < con_base.lines_buff_size; ++i)
            {
                con_base.lines_buff[i - 1] = con_base.lines_buff[i];
                con_base.lines_styles[i - 1] = con_base.lines_styles[i];
            }
            con_base.lines_buff[con_base.lines_buff_size - 1] = str;
            con_base.lines_styles[con_base.lines_buff_size - 1] = font_style;
        }
    }
}


void Con_AddText(const char *text, uint16_t font_style)
{
    if(text)
    {
        char buf[4096], ch;
        size_t i, j, text_size = strlen(text);

        buf[0] = 0;
        for(i = 0, j = 0; i < text_size; i++)
        {
            ch = text[i];
            if((ch == 10) || (ch == 13))
            {
                j = (j < sizeof(buf)) ? (j) : (sizeof(buf) - 1);
                buf[j] = 0;
                buf[sizeof(buf) - 1] = 0;
                if((j > 0) && ((buf[0] != 10) && (buf[0] != 13) && ((buf[0] > 31) || (buf[1] > 32))))
                {
                    Con_AddLine(buf, font_style);
                }
                j = 0;
            }
            else if(j < sizeof(buf) - 1)
            {
               buf[j++] = ch;
            }
        }

        buf[sizeof(buf) - 1] = 0;
        if(j < sizeof(buf))
        {
            buf[j] = 0;
        }
        if((j > 0) && ((buf[0] != 10) && (buf[0] != 13) && ((buf[0] > 31) || (buf[1] > 32))))
        {
            Con_AddLine(buf, font_style);
        }
    }
}


void Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];

    va_start(argptr, fmt);
    vsnprintf(buf, sizeof(buf), fmt, argptr);
    buf[sizeof(buf) - 1] = 0;
    va_end(argptr);
    Con_AddLine(buf, FONTSTYLE_CONSOLE_NOTIFY);
}


void Con_Warning(const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];

    va_start(argptr, fmt);
    vsnprintf(buf, sizeof(buf), fmt, argptr);
    buf[sizeof(buf) - 1] = 0;
    va_end(argptr);
    Con_AddLine(buf, FONTSTYLE_CONSOLE_WARNING);
}


void Con_Notify(const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];

    va_start(argptr, fmt);
    vsnprintf(buf, sizeof(buf), fmt, argptr);
    buf[sizeof(buf) - 1] = 0;
    va_end(argptr);
    Con_AddLine(buf, FONTSTYLE_CONSOLE_NOTIFY);
}


void Con_Clean()
{
    for(uint16_t i = 0; i < con_base.lines_count; i++)
    {
        free(con_base.lines_buff[i]);
        con_base.lines_buff[i] = NULL;
    }
    con_base.lines_count = 0;
}
