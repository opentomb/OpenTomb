
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_keycode.h>
#include "ftgl/FTGLBitmapFont.h"
#include "ftgl/FTGLTextureFont.h"
#include "console.h"
#include "system.h"
#include "engine.h"
#include "script.h"

console_info_t con_base;

void Con_InitGlobals()
{

}

void Con_Init()
{
    long int i;
    FILE *f;
    
    con_base.inited = 0;
    con_base.log_pos = 0;
    con_base.log_lines_count = 0;

    con_base.font_color[0] = 0.0;
    con_base.font_color[1] = 0.0;
    con_base.font_color[2] = 0.0;
    con_base.font_color[3] = 1.0;
    
    con_base.background_color[0] = 1.0;
    con_base.background_color[1] = 0.9;
    con_base.background_color[2] = 0.7;
    con_base.background_color[3] = 0.4;
    
    strncpy(con_base.font_patch, "VeraMono.ttf", 255);
    
    con_base.font_size = 12;
    con_base.log_lines_count = CON_MIN_LOG;
    con_base.shown_lines_count = CON_MIN_LINES;
    con_base.line_size = CON_MIN_LINE_SIZE;
    con_base.spacing = CON_MIN_LINE_INTERVAL;
    con_base.showing_lines = con_base.shown_lines_count;
    con_base.show_cursor_period = 0.5;
    
    f = fopen(con_base.font_patch, "rb");
    if(!f)
    {
        Sys_Error("Console: could not find font = \"%s\"", con_base.font_patch);
    }
    fclose(f);
    
    con_base.font_bitmap = new FTGLBitmapFont(con_base.font_patch);
    con_base.font_bitmap->FaceSize(con_base.font_size);
    con_base.font_texture = new FTGLTextureFont(con_base.font_patch);
    con_base.font_texture->FaceSize(con_base.font_size);
    con_base.smooth = 0;
    
    con_base.shown_lines = (char**) malloc(con_base.shown_lines_count*sizeof(char*));
    for(i=0;i<con_base.shown_lines_count;i++)
    {
        con_base.shown_lines[i] = (char*) calloc(con_base.line_size*sizeof(char), 1);
    }

    con_base.log_lines = (char**) malloc(con_base.log_lines_count*sizeof(char*));
    for(i=0;i<con_base.log_lines_count;i++)
    {
        con_base.log_lines[i] = (char*) calloc(con_base.line_size*sizeof(char), 1);
    }

    con_base.line_height = con_base.spacing * ((con_base.smooth)?(con_base.font_texture->Ascender()):(con_base.font_bitmap->Ascender()));
    con_base.cursor_x = 8 + 1;
    con_base.cursor_y = screen_info.h - con_base.line_height * con_base.showing_lines;
    if(con_base.cursor_y < 8)
    {
        con_base.cursor_y = 8;
    }

    con_base.inited = 1;
}

void Con_Destroy()
{
    int i;
    if(con_base.inited)
    {
        if(con_base.font_texture)
        {
            delete con_base.font_texture;
        }

        if(con_base.font_bitmap)
        {
            delete con_base.font_bitmap;
        }

        for(i=0;i<con_base.shown_lines_count;i++)
        {
            free(con_base.shown_lines[i]);
        }
        free(con_base.shown_lines);

        for(i=0;i<con_base.log_lines_count;i++)
        {
            free(con_base.log_lines[i]);
        }
        free(con_base.log_lines);

        con_base.inited = 0;
    }
}

void Con_SetFontSize(int size)
{
    if((con_base.inited == 0) || (size < 1) || (size > 72))
    {
        return;                                                                 // nothing to do here
    }

    con_base.inited = 0;
    con_base.font_size = size;
    con_base.font_bitmap->FaceSize(con_base.font_size);
    con_base.font_texture->FaceSize(con_base.font_size);
    con_base.line_height = con_base.spacing * ((con_base.smooth)?(con_base.font_texture->Ascender()):(con_base.font_bitmap->Ascender()));
    con_base.cursor_x = 8 + 1;
    con_base.cursor_y = screen_info.h - con_base.line_height * con_base.showing_lines;
    if(con_base.cursor_y < 8)
    {
        con_base.cursor_y = 8;
    }
    con_base.inited = 1;
    Con_Printf("New font size = %d", size);
}

void Con_SetLineInterval(float interval)
{
    if((con_base.inited == 0) ||
       (interval < CON_MIN_LINE_INTERVAL) || (interval > CON_MAX_LINE_INTERVAL))
    {
        return;                                                                 // nothing to do
    }

    con_base.inited = 0;
    con_base.spacing = interval;
    con_base.line_height = con_base.spacing * ((con_base.smooth)?(con_base.font_texture->Ascender()):(con_base.font_bitmap->Ascender()));
    con_base.cursor_x = 8 + 1;
    con_base.cursor_y = screen_info.h - con_base.line_height * con_base.showing_lines;
    if(con_base.cursor_y < 8)
    {
        con_base.cursor_y = 8;
    }
    con_base.inited = 1;
    Con_Printf("New line interval = %f", interval);
}

void Con_Draw()
{
    int x, y, i;
    
    if(con_base.inited && con_base.show)
    {
        glBindTexture(GL_TEXTURE_2D, 0);                                        // drop current texture
        Con_DrawBackground();
        x = 8;
        y = con_base.cursor_y;
        glColor4fv(con_base.font_color);
        if(con_base.smooth)
        {
            for(i=0;i<con_base.showing_lines;i++)
            {
                y += con_base.line_height;
                glPushMatrix();
                glTranslatef((GLfloat)x, (GLfloat)y, 0.0);
                con_base.font_texture->RenderRaw(con_base.shown_lines[i]);
                glPopMatrix();
            }
        }
        else    
        {
            for(i=0;i<con_base.showing_lines;i++)
            {
                y += con_base.line_height;
                glRasterPos2i(x, y);
                con_base.font_bitmap->RenderRaw(con_base.shown_lines[i]);
            }
        }
        Con_DrawCursor();
    }
}

void Con_DrawBackground()
{
    /*
     * Отрисуем фон консоли, чтобы было видно текст на фоне рендерера
     */
    GLfloat rectCoords[8];
    glColor4fv(con_base.background_color);
    rectCoords[0] = 0.0;                        rectCoords[1] = (GLfloat)screen_info.h;
    rectCoords[2] = (GLfloat)screen_info.w;     rectCoords[3] = (GLfloat)screen_info.h;
    rectCoords[4] = (GLfloat)screen_info.w;     rectCoords[5] = (GLfloat)(con_base.cursor_y + con_base.line_height - 8);
    rectCoords[6] = 0.0;                        rectCoords[7] = (GLfloat)(con_base.cursor_y + con_base.line_height - 8);
    glVertexPointer(2, GL_FLOAT, 0, rectCoords);
    glDrawArrays(GL_POLYGON, 0, 4);

    /*
     * Отрисуем заключительную линию
     */
    glColor4fv(con_base.font_color);
    
    rectCoords[0] = 0.0;                        rectCoords[1] = (GLfloat)(con_base.cursor_y + con_base.line_height - 8);
    rectCoords[2] = (GLfloat)screen_info.w;     rectCoords[3] = rectCoords[1];
    glVertexPointer(2, GL_FLOAT, 0, rectCoords);
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
        glColor4fv(con_base.font_color);
        coords[0] = (GLfloat)con_base.cursor_x;     coords[1] = (GLfloat)y - 0.1 * (GLfloat)con_base.line_height;
        coords[2] = (GLfloat)con_base.cursor_x;     coords[3] = (GLfloat)y + 0.7 * (GLfloat)con_base.line_height;
        glBindTexture(GL_TEXTURE_2D, 0);                                        // otherways cursor does not swown in smooth font case
        glVertexPointer(2, GL_FLOAT, 0, coords);
        glDrawArrays(GL_LINES, 0, 2); 
    }
}

void Con_Edit(int key)
{
    int i, len;

    if(key == SDLK_UNKNOWN || !con_base.inited)
    {
        return;
    }

    if(key == SDLK_RETURN)
    {
        Con_AddLog(con_base.shown_lines[0]);
        if(!Engine_ExecCmd(con_base.shown_lines[0]))
        {
            //Con_AddLine(con_base.text[0]);
        }
        con_base.shown_lines[0][0] = 0;
        con_base.cursor_pos = 0;
        con_base.cursor_x = 8 + 1;
        return;
    }

    con_base.cursor_time = 0.0;
    con_base.show_cursor = 1;

    len = strlen(con_base.shown_lines[0]);

    switch(key)
    {
        case SDLK_UP:
            strncpy(con_base.shown_lines[0], con_base.log_lines[con_base.log_pos], con_base.line_size);
            con_base.log_pos++;
            if(con_base.log_pos >= con_base.log_lines_count)
            {
                con_base.log_pos = 0;
            }
            con_base.cursor_pos = strlen(con_base.shown_lines[0]);
            break;

        case SDLK_DOWN:
            strncpy(con_base.shown_lines[0], con_base.log_lines[con_base.log_pos], con_base.line_size);
            con_base.log_pos--;
            if(con_base.log_pos < 0)
            {
                con_base.log_pos = con_base.log_lines_count - 1;
            }
            con_base.cursor_pos = strlen(con_base.shown_lines[0]);
            break;

        case SDLK_LEFT:
            if(con_base.cursor_pos > 0)
            {
                con_base.cursor_pos--;
            }
            break;

        case SDLK_RIGHT:
            if(con_base.cursor_pos < len)
            {
                con_base.cursor_pos++;
            }
            break;

        case SDLK_HOME:
            con_base.cursor_pos = 0;
            break;

        case SDLK_END:
            con_base.cursor_pos = len;
            break;

        case SDLK_BACKSPACE:
            if(con_base.cursor_pos > 0)
            {
                for(i = con_base.cursor_pos; i < len ;i++)
                {
                    con_base.shown_lines[0][i-1] = con_base.shown_lines[0][i];
                }
                con_base.shown_lines[0][i-1] = 0;
                con_base.shown_lines[0][i] = 0;
                con_base.cursor_pos--;
            }
            break;

        case SDLK_DELETE:
            if(/*(con_base.cursor_pos > 0) && */(con_base.cursor_pos < len))
            {
                for(i=con_base.cursor_pos;i<len-1;i++)
                {
                    con_base.shown_lines[0][i] = con_base.shown_lines[0][i+1];
                }
                con_base.shown_lines[0][len-1] = 0;
            }
            break;

        default:
            if((len < con_base.line_size-1) && (key >= SDLK_SPACE))
            {
                for(i=len;i>con_base.cursor_pos;i--)
                {
                    con_base.shown_lines[0][i] = con_base.shown_lines[0][i-1];
                }
                con_base.shown_lines[0][con_base.cursor_pos++] = key;
                con_base.shown_lines[0][len+1] = 0;
            }
            break;
    }

    Con_CalcCursorPosition();
}

void Con_CalcCursorPosition()
{
    char ch = con_base.shown_lines[0][con_base.cursor_pos];
    con_base.shown_lines[0][con_base.cursor_pos] = 0;
    if(con_base.smooth)
    {
        con_base.cursor_x = 8 + 1 + con_base.font_texture->Advance(con_base.shown_lines[0]);
    }
    else
    {
        con_base.cursor_x = 8 + 1 + con_base.font_bitmap->Advance(con_base.shown_lines[0]);
    }
    con_base.shown_lines[0][con_base.cursor_pos] = ch;
}

void Con_AddLog(const char *text)
{
    int i;
    char *last;

    if(con_base.inited && (text != NULL) && (text[0] != 0))
    {
        if(con_base.log_lines_count > 1)
        {
            last = con_base.log_lines[con_base.log_lines_count-1];              // сохраняем указатель на последнюю строку лога
            for(i=con_base.log_lines_count-1;i>0;i--)                           // Сдвигаем лог
            {
                con_base.log_lines[i] = con_base.log_lines[i-1];                // лог идет по кругу
            }
            con_base.log_lines[0] = last;                                       // зацикливаем сдвиг. Фактически поворот.
            strncpy(con_base.log_lines[0], text, con_base.line_size);
            con_base.log_lines[0][con_base.line_size-1] = 0;                    // подстраховка от случая переполнения строки
        }
        else
        {
            strncpy(con_base.log_lines[0], text, con_base.line_size);
            con_base.log_lines[0][con_base.line_size-1] = 0;                          // подстраховка от случая переполнения строки
        }

        con_base.log_pos = 0;
    }
}

void Con_AddLine(const char *text)
{
    int i;
    char *last;
    uint32_t len;
    
    if(con_base.inited && (text != NULL))
    {
        do
        {
            len = strlen(text);
            last = con_base.shown_lines[con_base.shown_lines_count-1];          // сохраняем указатель на последнюю строку лога
            for(i=con_base.shown_lines_count-1;i>1;i--)                         // Сдвигаем лог
            {
                con_base.shown_lines[i] = con_base.shown_lines[i-1];            // лог идет по кругу
            }
            con_base.shown_lines[1] = last;                                     // зацикливаем сдвиг. Фактически поворот.
            strncpy(con_base.shown_lines[1], text, con_base.line_size);
            con_base.shown_lines[1][con_base.line_size-1] = 0;                  // подстраховка от случая переполнения строки
            text += con_base.line_size-1;
        }
        while(len >= con_base.line_size);
    }
}

void Con_AddText(const char *text)
{
    char buf[4096], ch;
    int text_size = strlen(text);
    int i, j;
    
    buf[0] = 0;
    buf[4095] = 0;
    for(i=0,j=0;i<text_size;i++)
    {
        ch = text[i];
        if((ch == 10) || (ch == 13))
        {
            j = (j < 4096)?(j):(4095);
            buf[j] = 0;
            if((j > 0) && ((buf[0] != 10) && (buf[0] != 13) && ((buf[0] > 31) || (buf[1] > 32))))
            {
                Con_AddLine(buf);
            }
            j=0;
            continue;
        }
        if(j < 4096)
        {
           buf[j++] = ch;
        }
    }

    if(j < 4096)
    {
        buf[j] = 0;
    }
    if((j > 0) && ((buf[0] != 10) && (buf[0] != 13) && ((buf[0] > 31) || (buf[1] > 32))))
    {
        Con_AddLine(buf);
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
    Con_AddLine(buf);
}

void Con_Clean()
{
    int i;
    for(i=0;i<con_base.shown_lines_count;i++)
    {
        con_base.shown_lines[i][0] = 0;
    }
}


