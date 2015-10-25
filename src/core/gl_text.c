/*
 * File:   gl_text.cpp
 * Author: TeslaRus
 *
 * Created on October 24, 2015, 11:34 AM
 */

#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <math.h>

#include <ft2build.h>
#include <freetype.h>
#include <ftglyph.h>
#include <ftmodapi.h>

#include "system.h"
#include "gl_text.h"
#include "gl_font.h"
#include "gl_util.h"
#include "console.h"


#define vec4_copy(x, y) {(x)[0] = (y)[0]; (x)[1] = (y)[1]; (x)[2] = (y)[2]; (x)[3] = (y)[3];}

static gl_text_line_p      gl_base_lines = NULL;
static gl_text_line_t      gl_temp_lines[GLTEXT_MAX_TEMP_LINES];
static uint16_t            temp_lines_used = 0;


void GLText_InitTempLines()
{
    for(int i = 0; i < GLTEXT_MAX_TEMP_LINES; i++)
    {
        gl_temp_lines[i].text_size = GUI_LINE_DEFAULTSIZE;
        gl_temp_lines[i].text = (char*)malloc(GUI_LINE_DEFAULTSIZE * sizeof(char));
        gl_temp_lines[i].text[0] = 0;
        gl_temp_lines[i].show = 0;

        gl_temp_lines[i].next = NULL;
        gl_temp_lines[i].prev = NULL;

        gl_temp_lines[i].font_id  = FONT_SECONDARY;
        gl_temp_lines[i].style_id = FONTSTYLE_GENERIC;
    }
}


void GLText_DestroyTempLines()
{
    for(int i = 0; i < GLTEXT_MAX_TEMP_LINES ;i++)
    {
        gl_temp_lines[i].show = 0;
        gl_temp_lines[i].text_size = 0;
        free(gl_temp_lines[i].text);
        gl_temp_lines[i].text = NULL;
    }

    temp_lines_used = GLTEXT_MAX_TEMP_LINES;
}


void GLText_UpdateResize()
{
    gl_text_line_p l = gl_base_lines;

    for(; l; l = l->next)
    {
        l->absXoffset = l->x * screen_info.scale_factor;
        l->absYoffset = l->y * screen_info.scale_factor;
    }

    l = gl_temp_lines;
    for(uint16_t i = 0; i < temp_lines_used; i++, l++)
    {
        l->absXoffset = l->x * screen_info.scale_factor;
        l->absYoffset = l->y * screen_info.scale_factor;
    }
}


void GLText_RenderStringLine(gl_text_line_p l)
{
    GLfloat real_x = 0.0, real_y = 0.0;

    gl_tex_font_p gl_font = NULL;
    gl_fontstyle_p style = NULL;

    if(!l->show || ((gl_font = Con_GetFont(l->font_id)) == NULL) || ((style = Con_GetFontStyle(l->style_id)) == NULL))
    {
        return;
    }

    glf_get_string_bb(gl_font, l->text, -1, l->rect+0, l->rect+1, l->rect+2, l->rect+3);

    switch(l->x_align)
    {
        case GLTEXT_ALIGN_LEFT:
            real_x = l->absXoffset;   // Used with center and right alignments.
            break;
        case GLTEXT_ALIGN_RIGHT:
            real_x = (float)screen_info.w - (l->rect[2] - l->rect[0]) - l->absXoffset;
            break;
        case GLTEXT_ALIGN_CENTER:
            real_x = ((float)screen_info.w / 2.0) - ((l->rect[2] - l->rect[0]) / 2.0) + l->absXoffset;  // Absolute center.
            break;
    }

    switch(l->y_align)
    {
        case GLTEXT_ALIGN_BOTTOM:
            real_y += l->absYoffset;
            break;
        case GLTEXT_ALIGN_TOP:
            real_y = (float)screen_info.h - (l->rect[3] - l->rect[1]) - l->absYoffset;
            break;
        case GLTEXT_ALIGN_CENTER:
            real_y = ((float)screen_info.h / 2.0) + (l->rect[3] - l->rect[1]) - l->absYoffset;          // Consider the baseline.
            break;
    }

    if(style->rect)
    {
        BindWhiteTexture();
        GLfloat x0 = l->rect[0] + real_x - style->rect_border * screen_info.w_unit;
        GLfloat y0 = l->rect[1] + real_y - style->rect_border * screen_info.h_unit;
        GLfloat x1 = l->rect[2] + real_x + style->rect_border * screen_info.w_unit;
        GLfloat y1 = l->rect[3] + real_y + style->rect_border * screen_info.h_unit;
        GLfloat *v, backgroundArray[32];

        v = backgroundArray;
       *v++ = x0; *v++ = y0;
        vec4_copy(v, style->rect_color);
        v += 4;
       *v++ = 0.0; *v++ = 0.0;

       *v++ = x1; *v++ = y0;
        vec4_copy(v, style->rect_color);
        v += 4;
       *v++ = 0.0; *v++ = 0.0;

       *v++ = x1; *v++ = y1;
        vec4_copy(v, style->rect_color);
        v += 4;
       *v++ = 0.0; *v++ = 0.0;

       *v++ = x0; *v++ = y1;
        vec4_copy(v, style->rect_color);
        v += 4;
       *v++ = 0.0; *v++ = 0.0;

        qglVertexPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), backgroundArray);
        qglColorPointer(4, GL_FLOAT, 8 * sizeof(GLfloat), backgroundArray + 2);
        qglTexCoordPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), backgroundArray + 6);
        qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    if(style->shadowed)
    {
        gl_font->gl_font_color[0] = 0.0f;
        gl_font->gl_font_color[1] = 0.0f;
        gl_font->gl_font_color[2] = 0.0f;
        gl_font->gl_font_color[3] = (float)style->font_color[3] * GUI_FONT_SHADOW_TRANSPARENCY;// Derive alpha from base color.
        glf_render_str(gl_font,
                       (real_x + GUI_FONT_SHADOW_HORIZONTAL_SHIFT),
                       (real_y + GUI_FONT_SHADOW_VERTICAL_SHIFT  ),
                       l->text);
    }

    vec4_copy(gl_font->gl_font_color, style->font_color);
    glf_render_str(gl_font, real_x, real_y, l->text);
}


void GLText_RenderStrings()
{
    gl_text_line_p l = gl_base_lines;

    qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while(l)
    {
        GLText_RenderStringLine(l);
        l = l->next;
    }

    l = gl_temp_lines;
    for(uint16_t i = 0; i < temp_lines_used; i++, l++)
    {
        if(l->show)
        {
            GLText_RenderStringLine(l);
            l->show = 0;
        }
    }

    temp_lines_used = 0;
}


void GLText_AddLine(gl_text_line_p line)
{
    if(gl_base_lines == NULL)
    {
        gl_base_lines = line;
        line->next = NULL;
        line->prev = NULL;
        return;
    }

    line->prev = NULL;
    line->next = gl_base_lines;
    gl_base_lines->prev = line;
    gl_base_lines = line;
}


// line must be in the list, otherway You crash engine!
void GLText_DeleteLine(gl_text_line_p line)
{
    if(line == gl_base_lines)
    {
        gl_base_lines = line->next;
        if(gl_base_lines != NULL)
        {
            gl_base_lines->prev = NULL;
        }
        return;
    }

    line->prev->next = line->next;
    if(line->next)
    {
        line->next->prev = line->prev;
    }
}

void GLText_MoveLine(gl_text_line_p line)
{
    line->absXoffset = line->x * screen_info.scale_factor;
    line->absYoffset = line->y * screen_info.scale_factor;
}

/**
 * For simple temporary lines rendering.
 * Really all strings will be rendered in Gui_Render() function.
 */
gl_text_line_p GLText_OutTextXY(GLfloat x, GLfloat y, const char *fmt, ...)
{
    if(temp_lines_used < GLTEXT_MAX_TEMP_LINES - 1)
    {
        va_list argptr;
        gl_text_line_p l = gl_temp_lines + temp_lines_used;

        l->font_id = FONT_SECONDARY;
        l->style_id = FONTSTYLE_GENERIC;

        va_start(argptr, fmt);
        vsnprintf(l->text, GUI_LINE_DEFAULTSIZE, fmt, argptr);
        va_end(argptr);

        l->next = NULL;
        l->prev = NULL;

        temp_lines_used++;

        l->x = x;
        l->y = y;
        l->x_align = GLTEXT_ALIGN_LEFT;
        l->y_align = GLTEXT_ALIGN_BOTTOM;

        l->absXoffset = l->x * screen_info.scale_factor;
        l->absYoffset = l->y * screen_info.scale_factor;

        l->show = 1;
        return l;
    }

    return NULL;
}
