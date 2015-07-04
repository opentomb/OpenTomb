
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_keycode.h>

#include "gl_font.h"
#include "console.h"
#include "system.h"

console_info_t          con_base;
gl_font_manager_p       con_font_manager = NULL;

int  Engine_ExecCmd(char *ch);

void Con_Init()
{
    uint16_t i;
            
    con_base.inited = 0;
    con_base.log_pos = 0;

    con_base.font = NULL;

    // lines count check
    if(con_base.line_count < CON_MIN_LOG)
    {
        con_base.line_count = CON_MIN_LOG;
    }
    if(con_base.line_count > CON_MAX_LOG)
    {
        con_base.line_count = CON_MAX_LOG;
    }

    // log size check
    if(con_base.log_lines_count < CON_MIN_LOG)
    {
        con_base.log_lines_count = CON_MIN_LOG;
    }
    if(con_base.log_lines_count > CON_MAX_LOG)
    {
        con_base.log_lines_count = CON_MAX_LOG;
    }

    // showing lines count check
    if(con_base.showing_lines < CON_MIN_LINES)
    {
        con_base.showing_lines = CON_MIN_LINES;
    }
    if(con_base.showing_lines > CON_MAX_LINES)
    {
        con_base.showing_lines = CON_MAX_LINES;
    }

    // spacing check
    if(con_base.spacing < CON_MIN_LINE_INTERVAL)
    {
        con_base.spacing = CON_MIN_LINE_INTERVAL;
    }
    if(con_base.spacing > CON_MAX_LINE_INTERVAL)
    {
        con_base.spacing = CON_MAX_LINE_INTERVAL;
    }

    // linesize check
    if(con_base.line_size < CON_MIN_LINE_SIZE)
    {
        con_base.line_size = CON_MIN_LINE_SIZE;
    }
    if(con_base.line_size > CON_MAX_LINE_SIZE)
    {
        con_base.line_size = CON_MAX_LINE_SIZE;
    }

    con_base.line_text  = (char**)malloc(con_base.line_count*sizeof(char*));
    con_base.line_style_id = (uint16_t*)malloc(con_base.line_count * sizeof(uint16_t));

    for(i=0;i<con_base.line_count;i++)
    {
        con_base.line_text[i]  = (char*) calloc(con_base.line_size*sizeof(char), 1);
        con_base.line_style_id[i] = FONTSTYLE_GENERIC;
    }

    con_base.log_lines = (char**) realloc(con_base.log_lines, con_base.log_lines_count * sizeof(char*));
    for(i=0;i<con_base.log_lines_count;i++)
    {
        con_base.log_lines[i] = (char*) calloc(con_base.line_size * sizeof(char), 1);
    }
    
    con_font_manager = glf_create_manager();
    
    con_base.inited = 1;
}

void Con_InitFont(struct gl_tex_font_s *font)
{
    con_base.font = font;
    Con_SetLineInterval(con_base.spacing);
}

void Con_InitGlobals()
{
    con_base.background_color[0] = 1.0;
    con_base.background_color[1] = 0.9;
    con_base.background_color[2] = 0.7;
    con_base.background_color[3] = 0.4;

    con_base.log_lines_count = CON_MIN_LOG;
    con_base.log_lines       = NULL;

    con_base.spacing         = CON_MIN_LINE_INTERVAL;
    con_base.line_count      = CON_MIN_LINES;
    con_base.line_size       = CON_MIN_LINE_SIZE;
    con_base.line_text       = NULL;
    con_base.line_style_id   = NULL;

    con_base.showing_lines = con_base.line_count;
    con_base.show_cursor_period = 0.5;
}

void Con_Destroy()
{
    if(con_base.inited)
    {
        uint16_t i;
                
        for(i=0;i<con_base.line_count;i++)
        {
            free(con_base.line_text[i]);
        }
        free(con_base.line_text);
        free(con_base.line_style_id);

        for(i=0;i<con_base.log_lines_count;i++)
        {
            free(con_base.log_lines[i]);
        }
        free(con_base.log_lines);
        con_base.log_lines = NULL;

        con_base.inited = 0;
    }
    
    if(con_font_manager)
    {
        glf_free_manager(con_font_manager);
        con_font_manager = NULL;
    }
}

void Con_SetLineInterval(float interval)
{
    if((con_base.inited == 0) || (con_base.font == NULL) ||
       (interval < CON_MIN_LINE_INTERVAL) || (interval > CON_MAX_LINE_INTERVAL))
    {
        return; // nothing to do
    }

    con_base.inited = 0;
    con_base.spacing = interval;
    // con_base.font->font_size has absolute size (after scaling)
    con_base.line_height = (1.0 + con_base.spacing) * con_base.font->font_size;
    con_base.cursor_x = 8 + 1;
    con_base.cursor_y = screen_info.h - con_base.line_height * con_base.showing_lines;
    if(con_base.cursor_y < 8)
    {
        con_base.cursor_y = 8;
    }
    con_base.inited = 1;
}


void Con_Filter(char *text)
{
    if(text != NULL)
    {
        while(*text != '\0')
        {
            Con_Edit(*text);
            text++;
        }
    }
}


void Con_Edit(int key)
{
    if(key == SDLK_UNKNOWN || key == SDLK_BACKQUOTE || key == SDLK_BACKSLASH || !con_base.inited)
    {
        return;
    }

    if((key == SDLK_RETURN) && con_base.line_text[0])
    {
        Con_AddLog(con_base.line_text[0]);
        if(!Engine_ExecCmd(con_base.line_text[0]))
        {
            //Con_AddLine(con_base.text[0]);
        }
        con_base.line_text[0][0] = 0;
        con_base.cursor_pos = 0;
        con_base.cursor_x = 8 + 1;
        return;
    }

    con_base.cursor_time = 0.0;
    con_base.show_cursor = 1;

    int16_t oldLength = utf8_strlen(con_base.line_text[0]);    // int16_t is absolutly enough

    switch(key)
    {
        case SDLK_UP:
            //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUPAGE));
            if(*con_base.log_lines[con_base.log_pos])
            {
                strncpy(con_base.line_text[0], con_base.log_lines[con_base.log_pos], con_base.line_size);
                con_base.log_pos++;
                if((!*con_base.log_lines[con_base.log_pos]) || (con_base.log_pos >= con_base.log_lines_count))
                {
                    con_base.log_pos = 0;
                }
                con_base.cursor_pos = utf8_strlen(con_base.line_text[0]);
            }
            break;

        case SDLK_DOWN:
            //Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUPAGE));
            if(con_base.log_pos == 0)
            {
                for(;(con_base.log_pos < con_base.log_lines_count) && *con_base.log_lines[con_base.log_pos];con_base.log_pos++);
            }
            if((con_base.log_pos > 0) && *con_base.log_lines[con_base.log_pos-1])
            {
                strncpy(con_base.line_text[0], con_base.log_lines[con_base.log_pos-1], con_base.line_size);
                con_base.log_pos--;
                con_base.cursor_pos = utf8_strlen(con_base.line_text[0]);
            }
            break;

        case SDLK_LEFT:
            if(con_base.cursor_pos > 0)
            {
                con_base.cursor_pos--;
            }
            break;

        case SDLK_RIGHT:
            if(con_base.cursor_pos < oldLength)
            {
                con_base.cursor_pos++;
            }
            break;

        case SDLK_HOME:
            con_base.cursor_pos = 0;
            break;

        case SDLK_END:
            con_base.cursor_pos = oldLength;
            break;

        case SDLK_BACKSPACE:
            if(con_base.cursor_pos > 0)
            {
                int16_t i;
                for(i = con_base.cursor_pos; i < oldLength ;i++)
                {
                    con_base.line_text[0][i-1] = con_base.line_text[0][i];
                }
                con_base.line_text[0][oldLength-1] = 0;
                con_base.line_text[0][oldLength] = 0;
                con_base.cursor_pos--;
            }
            break;

        case SDLK_DELETE:
            if(/*(con_base.cursor_pos > 0) && */(con_base.cursor_pos < oldLength))
            {
                int16_t i;
                for(i=con_base.cursor_pos;i<oldLength-1;i++)
                {
                    con_base.line_text[0][i] = con_base.line_text[0][i+1];
                }
                con_base.line_text[0][oldLength-1] = 0;
            }
            break;

        default:
            if((oldLength < con_base.line_size-1) && (key >= SDLK_SPACE))
            {
                int16_t i;
                for(i=oldLength;i>con_base.cursor_pos;i--)
                {
                    con_base.line_text[0][i] = con_base.line_text[0][i-1];
                }
                con_base.line_text[0][con_base.cursor_pos] = key;
                con_base.line_text[0][++oldLength] = 0;
                con_base.cursor_pos++;
            }
            break;
    }

    Con_CalcCursorPosition();
}


void Con_CalcCursorPosition()
{
    if(con_base.font)
    {
        con_base.cursor_x = 8 + 1 + glf_get_string_len(con_base.font, con_base.line_text[0], con_base.cursor_pos);
    }
}


void Con_AddLog(const char *text)
{
    if(con_base.inited && (text != NULL) && (text[0] != 0))
    {
        if(con_base.log_lines_count > 1)
        {
            char *last = con_base.log_lines[con_base.log_lines_count-1];        // save pointer to the last log string
            uint16_t i;
            for(i=con_base.log_lines_count-1;i>0;i--)                           // shift log
            {
                con_base.log_lines[i] = con_base.log_lines[i-1];                // shift is round
            }
            con_base.log_lines[0] = last;                                       // cycle the shift
            strncpy(con_base.log_lines[0], text, con_base.line_size);
            con_base.log_lines[0][con_base.line_size-1] = 0;                    // paranoid end of string
        }
        else
        {
            strncpy(con_base.log_lines[0], text, con_base.line_size);
            con_base.log_lines[0][con_base.line_size-1] = 0;                    // paranoid end of string
        }

        con_base.log_pos = 0;
    }
}


void Con_AddLine(const char *text, uint16_t font_style)
{
    if(con_base.inited && (text != NULL))
    {
        size_t len = 0;
        do
        {
            char *last = con_base.line_text[con_base.line_count-1];             // save pointer to the last log string
            uint16_t i;
            
            len = strlen(text);
            for(i=con_base.line_count-1;i>1;i--)                                // shift log
            {
                con_base.line_style_id[i] = con_base.line_style_id[i-1];
                con_base.line_text[i]  = con_base.line_text[i-1];               // shift is round
            }

            con_base.line_text[1] = last;                                       // cycle the shift
            con_base.line_style_id[1] = font_style;
            strncpy(con_base.line_text[1], text, con_base.line_size);
            con_base.line_text[1][con_base.line_size-1] = 0;                    // paranoid end of string
            text += con_base.line_size-1;
        }
        while(len >= con_base.line_size);
    }
}


void Con_AddText(const char *text, uint16_t font_style)
{
    char buf[4096], ch;
    size_t i, j, text_size = strlen(text);

    buf[0] = 0;
    for(i=0,j=0;i<text_size;i++)
    {
        ch = text[i];
        if((ch == 10) || (ch == 13))
        {
            j = (j < 4096)?(j):(4095);
            buf[j] = 0;
            buf[4095] = 0;
            if((j > 0) && ((buf[0] != 10) && (buf[0] != 13) && ((buf[0] > 31) || (buf[1] > 32))))
            {
                Con_AddLine(buf, font_style);
            }
            j=0;
        }
        else if(j < 4096)
        {
           buf[j++] = ch;
        }
    }

    buf[4095] = 0;
    if(j < 4096)
    {
        buf[j] = 0;
    }
    if((j > 0) && ((buf[0] != 10) && (buf[0] != 13) && ((buf[0] > 31) || (buf[1] > 32))))
    {
        Con_AddLine(buf, font_style);
    }
}


void Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];

    va_start(argptr, fmt);
    vsnprintf(buf, 4096, fmt, argptr);
    buf[4096-1] = 0;
    va_end(argptr);
    Con_AddLine(buf, FONTSTYLE_CONSOLE_NOTIFY);
}


void Con_Warning(const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];

    va_start(argptr, fmt);
    vsnprintf(buf, 4096, fmt, argptr);
    buf[4096-1] = 0;
    va_end(argptr);
    Con_AddLine(buf, FONTSTYLE_CONSOLE_WARNING);
}


void Con_Notify(const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];

    va_start(argptr, fmt);
    vsnprintf(buf, 4096, fmt, argptr);
    buf[4096-1] = 0;
    va_end(argptr);
    Con_AddLine(buf, FONTSTYLE_CONSOLE_NOTIFY);
}


void Con_Clean()
{
    uint16_t i;
    for(i=0;i<con_base.line_count;i++)
    {
        con_base.line_text[i][0]  = 0;
        con_base.line_style_id[i] = FONTSTYLE_GENERIC;
    }
}


