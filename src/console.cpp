
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_keycode.h>

#include "gl_font.h"
#include "console.h"
#include "system.h"
#include "engine.h"
#include "script.h"
#include "gui.h"

console_info_t con_base;

void Con_Init()
{
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
    con_base.line_style = (gui_fontstyle_p*)malloc(con_base.line_count*sizeof(gui_fontstyle_p));

    for(uint16_t i=0;i<con_base.line_count;i++)
    {
        con_base.line_text[i]  = (char*) calloc(con_base.line_size*sizeof(char), 1);
        con_base.line_style[i] = NULL;
    }

    con_base.log_lines = (char**) realloc(con_base.log_lines, con_base.log_lines_count * sizeof(char*));
    for(uint16_t i=0;i<con_base.log_lines_count;i++)
    {
        con_base.log_lines[i] = (char*) calloc(con_base.line_size * sizeof(char), 1);
    }
    con_base.inited = 1;
}

void Con_InitFonts()
{
    for(uint16_t i=0;i<con_base.line_count;i++)
    {
        con_base.line_style[i] = FontManager->GetFontStyle(FONTSTYLE_GENERIC);
    }
    con_base.font = FontManager->GetFont(FONT_CONSOLE);
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

    con_base.spacing    = CON_MIN_LINE_INTERVAL;
    con_base.line_count = CON_MIN_LINES;
    con_base.line_size  = CON_MIN_LINE_SIZE;
    con_base.line_text  = NULL;
    con_base.line_style = NULL;

    con_base.showing_lines = con_base.line_count;
    con_base.show_cursor_period = 0.5;
}

void Con_Destroy()
{
    if(con_base.inited)
    {
        for(uint16_t i=0;i<con_base.line_count;i++)
        {
            free(con_base.line_text[i]);
        }
        free(con_base.line_text);
        free(con_base.line_style);

        for(uint16_t i=0;i<con_base.log_lines_count;i++)
        {
            free(con_base.log_lines[i]);
        }
        free(con_base.log_lines);

        con_base.inited = 0;
    }
}

void Con_SetLineInterval(float interval)
{
    if((con_base.inited == 0) ||
       (interval < CON_MIN_LINE_INTERVAL) || (interval > CON_MAX_LINE_INTERVAL))
    {
        return; // nothing to do
    }

    float scale_factor = (screen_info.w >= screen_info.h)?(screen_info.w_unit):(screen_info.h_unit);

    con_base.inited = 0;
    con_base.spacing = interval;
    con_base.line_height = con_base.spacing * glf_get_ascender(con_base.font) * scale_factor;
    con_base.cursor_x = 8 + 1;
    con_base.cursor_y = screen_info.h - con_base.line_height * con_base.showing_lines;
    if(con_base.cursor_y < 8)
    {
        con_base.cursor_y = 8;
    }
    con_base.inited = 1;
}

void Con_Draw()
{
    if(con_base.inited && con_base.show)
    {
        int x, y;
        glBindTexture(GL_TEXTURE_2D, 0);                                        // drop current texture
        Con_DrawBackground();
        x = 8;
        y = con_base.cursor_y;

        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        for(uint16_t i=0;i<con_base.showing_lines;i++)
        {
            glColor4fv(con_base.line_style[i]->real_color);

            y += con_base.line_height;
            glf_render_str(con_base.font, (GLfloat)x, (GLfloat)y, con_base.line_text[i]);
        }

        glPopClientAttrib();
        Con_DrawCursor();
    }
}

void Con_DrawBackground()
{
    /*
     * Draw console background to see the text
     */
    GLfloat rectCoords[8];
    glColor4fv(con_base.background_color);
    rectCoords[0] = 0.0;                        rectCoords[1] = (GLfloat)screen_info.h;
    rectCoords[2] = 0.0;                        rectCoords[3] = (GLfloat)(con_base.cursor_y + con_base.line_height - 8);
    rectCoords[4] = (GLfloat)screen_info.w;     rectCoords[5] = (GLfloat)(con_base.cursor_y + con_base.line_height - 8);
    rectCoords[6] = (GLfloat)screen_info.w;     rectCoords[7] = (GLfloat)screen_info.h;

    glVertexPointer(2, GL_FLOAT, 0, rectCoords);
    glDrawArrays(GL_POLYGON, 0, 4);

    /*
     * Draw finalise line
     */
    GLfloat white_color[4] = {1.0,1.0,1.0,0.7};
    glColor4fv(white_color);
    glVertexPointer(2, GL_FLOAT, 0, rectCoords + 2);
    glDrawArrays(GL_LINES, 0, 2);
}

void Con_DrawCursor()
{
    GLint y = con_base.cursor_y + con_base.line_height;

    if(con_base.show_cursor_period)
    {
        con_base.cursor_time += engine_frame_time;
        if(con_base.cursor_time > con_base.show_cursor_period)
        {
            con_base.cursor_time = 0.0;
            con_base.show_cursor = !con_base.show_cursor;
        }
    }

    if(con_base.show_cursor)
    {
        GLfloat coords[4];
        GLfloat white_color[4] = {1.0,1.0,1.0,0.7};
        glColor4fv(white_color);
        coords[0] = (GLfloat)con_base.cursor_x;     coords[1] = (GLfloat)y - 0.1 * (GLfloat)con_base.line_height;
        coords[2] = (GLfloat)con_base.cursor_x;     coords[3] = (GLfloat)y + 0.7 * (GLfloat)con_base.line_height;
        glBindTexture(GL_TEXTURE_2D, 0);   // otherways cursor does not swown in smooth font case
        glVertexPointer(2, GL_FLOAT, 0, coords);
        glDrawArrays(GL_LINES, 0, 2);
    }
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

    if(key == SDLK_RETURN)
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
            Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUPAGE));
            strncpy(con_base.line_text[0], con_base.log_lines[con_base.log_pos], con_base.line_size);
            con_base.log_pos++;
            if(con_base.log_pos >= con_base.log_lines_count)
            {
                con_base.log_pos = 0;
            }
            con_base.cursor_pos = utf8_strlen(con_base.line_text[0]);
            break;

        case SDLK_DOWN:
            Audio_Send(lua_GetGlobalSound(engine_lua, TR_AUDIO_SOUND_GLOBALID_MENUPAGE));
            strncpy(con_base.line_text[0], con_base.log_lines[con_base.log_pos], con_base.line_size);
            if(con_base.log_pos == 0)
            {
                con_base.log_pos = con_base.log_lines_count - 1;
            }
            else
            {
                con_base.log_pos--;
            }
            con_base.cursor_pos = utf8_strlen(con_base.line_text[0]);
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
                for(int16_t i = con_base.cursor_pos; i < oldLength ;i++)
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
                for(int16_t i=con_base.cursor_pos;i<oldLength-1;i++)
                {
                    con_base.line_text[0][i] = con_base.line_text[0][i+1];
                }
                con_base.line_text[0][oldLength-1] = 0;
            }
            break;

        default:
            if((oldLength < con_base.line_size-1) && (key >= SDLK_SPACE))
            {
                for(int16_t i=oldLength;i>con_base.cursor_pos;i--)
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
            for(uint16_t i=con_base.log_lines_count-1;i>0;i--)                  // shift log
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

void Con_AddLine(const char *text, font_Style style)
{
    if(con_base.inited && (text != NULL))
    {
        size_t len = 0;
        do
        {
            len = strlen(text);
            char *last = con_base.line_text[con_base.line_count-1];             // save pointer to the last log string
            for(uint16_t i=con_base.line_count-1;i>1;i--)                       // shift log
            {
                con_base.line_style[i] = con_base.line_style[i-1];
                con_base.line_text[i]  = con_base.line_text[i-1];            // shift is round
            }

            con_base.line_text[1] = last;                                     // cycle the shift
            con_base.line_style[1] = FontManager->GetFontStyle(style);
            strncpy(con_base.line_text[1], text, con_base.line_size);
            con_base.line_text[1][con_base.line_size-1] = 0;                  // paranoid end of string
            text += con_base.line_size-1;
        }
        while(len >= con_base.line_size);
    }
}

void Con_AddText(const char *text, font_Style style)
{
    char buf[4096], ch;
    size_t j = 0, text_size = strlen(text);

    buf[0] = 0;
    for(size_t i=0,j=0;i<text_size;i++)
    {
        ch = text[i];
        if((ch == 10) || (ch == 13))
        {
            j = (j < 4096)?(j):(4095);
            buf[j] = 0;
            buf[4095] = 0;
            if((j > 0) && ((buf[0] != 10) && (buf[0] != 13) && ((buf[0] > 31) || (buf[1] > 32))))
            {
                Con_AddLine(buf, style);
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
        Con_AddLine(buf, style);
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

void Con_Clean()
{
    for(uint16_t i=0;i<con_base.line_count;i++)
    {
        con_base.line_text[i][0] = 0;
        con_base.line_style[i]   = FontManager->GetFontStyle(FONTSTYLE_GENERIC);
    }
}


