/*
 * File:   gl_text.cpp
 * Author: TeslaRus
 *
 * Created on October 24, 2015, 11:34 AM
 */

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "gl_text.h"
#include "gl_font.h"
#include "gl_util.h"


#define vec4_copy(x, y) {(x)[0] = (y)[0]; (x)[1] = (y)[1]; (x)[2] = (y)[2]; (x)[3] = (y)[3];}

static struct
{
    gl_text_line_p           gl_base_lines;
    gl_text_line_t           gl_temp_lines[GLTEXT_MAX_TEMP_LINES];
    uint16_t                 temp_lines_used;
    uint16_t                 max_styles;
    struct gl_fontstyle_s   *styles;

    uint16_t                 max_fonts;
    struct gl_font_cont_s   *fonts;
} font_data;


void GLText_Init()
{
    int i;

    font_data.gl_base_lines = NULL;
    font_data.max_styles = GLTEXT_MAX_FONTSTYLES;
    font_data.styles     = (gl_fontstyle_p)malloc(font_data.max_styles * sizeof(gl_fontstyle_t));
    for(i = 0; i < font_data.max_styles; i++)
    {
        font_data.styles[i].font_color[0] = 0.0;
        font_data.styles[i].font_color[1] = 0.0;
        font_data.styles[i].font_color[2] = 0.0;
        font_data.styles[i].font_color[3] = 1.0;
        font_data.styles[i].shadowed = 0x00;
    }

    font_data.max_fonts = GLTEXT_MAX_FONTS;
    font_data.fonts     = (gl_font_cont_p)malloc(font_data.max_fonts * sizeof(gl_font_cont_t));
    for(i = 0; i < font_data.max_fonts; i++)
    {
        font_data.fonts[i].font_size = 0;
        font_data.fonts[i].gl_font   = NULL;
    }

    for(int i = 0; i < GLTEXT_MAX_TEMP_LINES; i++)
    {
        font_data.gl_temp_lines[i].text_size = GUI_LINE_DEFAULTSIZE;
        font_data.gl_temp_lines[i].text = (char*)malloc(GUI_LINE_DEFAULTSIZE * sizeof(char));
        font_data.gl_temp_lines[i].text[0] = 0;
        font_data.gl_temp_lines[i].show = 0;

        font_data.gl_temp_lines[i].next = NULL;
        font_data.gl_temp_lines[i].prev = NULL;

        font_data.gl_temp_lines[i].font_id  = FONT_SECONDARY;
        font_data.gl_temp_lines[i].style_id = FONTSTYLE_GENERIC;
    }

    font_data.temp_lines_used = 0;
}


void GLText_Destroy()
{
    int i;
    
    font_data.gl_base_lines = NULL;
    for(i = 0; i < GLTEXT_MAX_TEMP_LINES ; i++)
    {
        font_data.gl_temp_lines[i].show = 0;
        font_data.gl_temp_lines[i].text_size = 0;
        free(font_data.gl_temp_lines[i].text);
        font_data.gl_temp_lines[i].text = NULL;
    }

    font_data.temp_lines_used = GLTEXT_MAX_TEMP_LINES;

    for(i = 0; i < font_data.max_fonts; i++)
    {
        glf_free_font(font_data.fonts[i].gl_font);
        font_data.fonts[i].font_size = 0;
        font_data.fonts[i].gl_font   = NULL;
    }
    free(font_data.fonts);
    font_data.fonts = NULL;

    free(font_data.styles);
    font_data.styles = NULL;

    font_data.max_fonts = 0;
    font_data.max_styles = 0;
}


void GLText_UpdateResize(int w, int h, float scale)
{
    if(font_data.max_fonts > 0)
    {
        for(uint16_t i = 0; i < font_data.max_fonts; i++)
        {
            if(font_data.fonts[i].gl_font)
            {
                glf_resize(font_data.fonts[i].gl_font, (uint16_t)(((float)font_data.fonts[i].font_size) * scale));
            }
        }
    }
}


void GLText_RenderStringLine(gl_text_line_p l)
{
    gl_tex_font_p gl_font = NULL;
    gl_fontstyle_p style = NULL;
    
    if(l->show && (gl_font = GLText_GetFont(l->font_id)) && (style = GLText_GetFontStyle(l->style_id)))
    {
        GLfloat real_x = 0.0f, real_y = 0.0f;
        int32_t x0, y0, x1, y1;
        GLfloat shadow_color[4];
        int32_t w_pt = (l->line_width * 64.0f + 0.5f);
        GLfloat ascender = glf_get_ascender(gl_font) / 64.0f;
        GLfloat descender = glf_get_descender(gl_font) / 64.0f;
        GLfloat dy = l->line_height * (ascender - descender);
        int n_lines = 1;
        char *begin = l->text;
        char *end = begin;
        
        shadow_color[0] = 0.0f;
        shadow_color[1] = 0.0f;
        shadow_color[2] = 0.0f;
        shadow_color[3] = (float)style->font_color[3] * GUI_FONT_SHADOW_TRANSPARENCY;

        if(l->line_width > 0.0f)
        {
            int n_sym = 0;
            n_lines = 0;
            for(char *ch = glf_get_string_for_width(gl_font, l->text, w_pt, &n_sym); *begin; ch = glf_get_string_for_width(gl_font, ch, w_pt, &n_sym))
            {
                if(!n_lines)
                {
                    glf_get_string_bb(gl_font, l->text, n_sym, &x0, &y0, &x1, &y1);
                }
                ++n_lines;
                begin = ch;
            }
            begin = l->text;
            x1 = x0 + w_pt;
            y1 = y0 + n_lines * dy * 64.0f;
        }
        else
        {
            glf_get_string_bb(gl_font, l->text, -1, &x0, &y0, &x1, &y1);
        }

        l->rect[0] = (GLfloat)x0 / 64.0f;
        l->rect[1] = (GLfloat)y0 / 64.0f;
        l->rect[2] = (GLfloat)x1 / 64.0f;
        l->rect[3] = (GLfloat)y1 / 64.0f;
        
        real_y = l->y - descender;
        switch(l->y_align)
        {
            case GLTEXT_ALIGN_TOP:
                real_y = l->y - ascender - dy * (n_lines - 1);
                break;
            case GLTEXT_ALIGN_CENTER:
                real_y = l->y - descender - dy * n_lines / 2;
                break;
        }

        for(int line = n_lines - 1; line >= 0; --line)
        {
            int n_sym = -1;
            if(n_lines > 1)
            {
                end = glf_get_string_for_width(gl_font, begin, w_pt, &n_sym);
            }
            
            glf_get_string_bb(gl_font, begin, n_sym, &x0, &y0, &x1, &y1);
            
            real_x = l->x - x0 / 64.0f;
            switch(l->x_align)
            {
                case GLTEXT_ALIGN_RIGHT:
                    real_x = l->x - x1 / 64.0f;
                    break;
                case GLTEXT_ALIGN_CENTER:
                    real_x = l->x - (x1 + x0) / 128.0f;
                    break;
            }
        
            if(style->shadowed)
            {
                vec4_copy(gl_font->gl_font_color, shadow_color);
                glf_render_str(gl_font,
                               (real_x + GUI_FONT_SHADOW_HORIZONTAL_SHIFT),
                               (real_y + line * dy + GUI_FONT_SHADOW_VERTICAL_SHIFT),
                               begin, n_sym);
            }
            vec4_copy(gl_font->gl_font_color, style->font_color);
            glf_render_str(gl_font, real_x, real_y + line * dy, begin, n_sym);
            begin = end;
        }
    }
}


void GLText_RenderStrings()
{
    gl_text_line_p l = font_data.gl_base_lines;

    qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while(l)
    {
        GLText_RenderStringLine(l);
        l = l->next;
    }

    l = font_data.gl_temp_lines;
    for(uint16_t i = 0; i < font_data.temp_lines_used; i++, l++)
    {
        if(l->show)
        {
            GLText_RenderStringLine(l);
            l->show = 0;
        }
    }

    font_data.temp_lines_used = 0;
}


void GLText_AddLine(gl_text_line_p line)
{
    if(!line->next && !line->prev && (line != font_data.gl_base_lines))
    {
        if(font_data.gl_base_lines == NULL)
        {
            font_data.gl_base_lines = line;
            line->next = NULL;
            line->prev = NULL;
            return;
        }

        line->prev = NULL;
        line->next = font_data.gl_base_lines;
        font_data.gl_base_lines->prev = line;
        font_data.gl_base_lines = line;
    }
}


// line must be in the list, otherway You crash engine!
void GLText_DeleteLine(gl_text_line_p line)
{
    if(font_data.gl_base_lines)
    {
        if(line == font_data.gl_base_lines)
        {
            font_data.gl_base_lines = line->next;
        }
        else if(line->prev)
        {
            line->prev->next = line->next;
        }
        else
        {
            return;
        }

        if(line->next)
        {
            line->next->prev = line->prev;
        }
        line->next = NULL;
        line->prev = NULL;
    }
}

/**
 * For simple temporary lines rendering.
 * Really all strings will be rendered in Gui_Render() function.
 */
gl_text_line_p GLText_OutTextXY(GLfloat x, GLfloat y, const char *fmt, ...)
{
    gl_text_line_p ret = NULL;
    va_list argptr;

    va_start(argptr, fmt);
    ret = GLText_VOutTextXY(x, y, fmt, argptr);
    va_end(argptr);

    return ret;
}


gl_text_line_p GLText_VOutTextXY(GLfloat x, GLfloat y, const char *fmt, va_list argptr)
{
    if(font_data.temp_lines_used < GLTEXT_MAX_TEMP_LINES - 1)
    {
        gl_text_line_p l = font_data.gl_temp_lines + font_data.temp_lines_used;

        l->font_id = FONT_SECONDARY;
        l->style_id = FONTSTYLE_GENERIC;
        l->line_width = -1.0f;
        l->line_height = 1.75f;
        
        vsnprintf(l->text, GUI_LINE_DEFAULTSIZE, fmt, argptr);

        l->next = NULL;
        l->prev = NULL;

        font_data.temp_lines_used++;

        l->x = x;
        l->y = y;
        l->x_align = GLTEXT_ALIGN_LEFT;
        l->y_align = GLTEXT_ALIGN_BOTTOM;

        l->show = 1;
        return l;
    }

    return NULL;
}


int GLText_AddFont(uint16_t index, uint16_t size, const char* path)
{
    if(index < font_data.max_fonts)
    {
        gl_tex_font_p new_font = glf_create_font(path, size);
        if(new_font)
        {
            if(font_data.fonts[index].gl_font)
            {
                glf_free_font(font_data.fonts[index].gl_font);
            }
            font_data.fonts[index].font_size = size;
            font_data.fonts[index].gl_font = new_font;
            return 1;
        }
    }

    return 0;
}


int GLText_RemoveFont(uint16_t index)
{
    if((index < font_data.max_fonts) && (font_data.fonts[index].gl_font))
    {
        glf_free_font(font_data.fonts[index].gl_font);
        font_data.fonts[index].gl_font = NULL;
        return 1;
    }

    return 0;
}


int GLText_AddFontStyle(uint16_t index,
                     GLfloat R, GLfloat G, GLfloat B, GLfloat A,
                     uint8_t shadow)
{
    if(index < font_data.max_styles)
    {
        gl_fontstyle_p desired_style = font_data.styles + index;
        desired_style->font_color[0]  = R;
        desired_style->font_color[1]  = G;
        desired_style->font_color[2]  = B;
        desired_style->font_color[3]  = A;

        desired_style->shadowed  = shadow;
        return 1;
    }

    return 0;
}


int GLText_RemoveFontStyle(uint16_t index)
{
    if(index < font_data.max_styles)
    {
        font_data.styles[index].font_color[0] = 0.0;
        font_data.styles[index].font_color[1] = 0.0;
        font_data.styles[index].font_color[2] = 0.0;
        font_data.styles[index].font_color[3] = 1.0;

        font_data.styles[index].shadowed = 0x00;
        return 1;
    }

    return 0;
}


gl_tex_font_p GLText_GetFont(uint16_t index)
{
    if(index < font_data.max_fonts)
    {
        return font_data.fonts[index].gl_font;
    }

    return NULL;
}


gl_fontstyle_p GLText_GetFontStyle(uint16_t index)
{
    if(index < font_data.max_styles)
    {
        return font_data.styles + index;
    }

    return NULL;
}
