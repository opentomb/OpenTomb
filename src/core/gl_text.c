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


#define vec4_copy(x, y) {(x)[0] = (y)[0]; (x)[1] = (y)[1]; (x)[2] = (y)[2]; (x)[3] = (y)[3];}

static struct
{
    gl_text_line_p           gl_base_lines;
    gl_text_line_t           gl_temp_lines[GLTEXT_MAX_TEMP_LINES];
    uint16_t                 temp_lines_used;
    FT_Library               font_library;               // GLF font library unit.
    uint16_t                 max_styles;
    struct gl_fontstyle_s   *styles;

    uint16_t                 max_fonts;
    struct gl_font_cont_s   *fonts;
} font_data;


void GLText_Init()
{
    int i;
    font_data.font_library   = NULL;
    FT_Init_FreeType(&font_data.font_library);

    font_data.max_styles = GLTEXT_MAX_FONTSTYLES;
    font_data.styles     = (gl_fontstyle_p)malloc(font_data.max_styles * sizeof(gl_fontstyle_t));
    for(i = 0; i < font_data.max_styles; i++)
    {
        font_data.styles[i].rect_color[0] = 1.0;
        font_data.styles[i].rect_color[1] = 1.0;
        font_data.styles[i].rect_color[2] = 1.0;
        font_data.styles[i].rect_color[3] = 0.0;

        font_data.styles[i].font_color[0] = 0.0;
        font_data.styles[i].font_color[1] = 0.0;
        font_data.styles[i].font_color[2] = 0.0;
        font_data.styles[i].font_color[3] = 1.0;

        font_data.styles[i].shadowed = 0x00;
        font_data.styles[i].rect     = 0x00;
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

    FT_Done_FreeType(font_data.font_library);
    font_data.font_library = NULL;
}


void GLText_UpdateResize(float scale)
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
    GLfloat real_x = 0.0, real_y = 0.0;

    gl_tex_font_p gl_font = NULL;
    gl_fontstyle_p style = NULL;

    if(!l->show || ((gl_font = GLText_GetFont(l->font_id)) == NULL) || ((style = GLText_GetFontStyle(l->style_id)) == NULL))
    {
        return;
    }

    glf_get_string_bb(gl_font, l->text, -1, l->rect+0, l->rect+1, l->rect+2, l->rect+3);

    switch(l->x_align)
    {
        case GLTEXT_ALIGN_LEFT:
            real_x = l->x;   // Used with center and right alignments.
            break;
        case GLTEXT_ALIGN_RIGHT:
            real_x = (float)screen_info.w - (l->rect[2] - l->rect[0]) - l->x;
            break;
        case GLTEXT_ALIGN_CENTER:
            real_x = l->x - 0.5f * (l->rect[2] - l->rect[0]);
            break;
    }

    switch(l->y_align)
    {
        case GLTEXT_ALIGN_BOTTOM:
            real_y = l->y;
            break;
        case GLTEXT_ALIGN_TOP:
            real_y = (float)screen_info.h - (l->rect[3] - l->rect[1]) - l->y;
            break;
        case GLTEXT_ALIGN_CENTER:
            real_y = l->y - 0.5f * (l->rect[3] - l->rect[1]);
            break;
    }

    if(style->rect)
    {
        BindWhiteTexture();
        GLfloat x0 = l->rect[0] + real_x - style->rect_border * screen_info.w;
        GLfloat y0 = l->rect[1] + real_y - style->rect_border * screen_info.h;
        GLfloat x1 = l->rect[2] + real_x + style->rect_border * screen_info.w;
        GLfloat y1 = l->rect[3] + real_y + style->rect_border * screen_info.h;
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


// line must be in the list, otherway You crash engine!
void GLText_DeleteLine(gl_text_line_p line)
{
    if(line == font_data.gl_base_lines)
    {
        font_data.gl_base_lines = line->next;
        if(font_data.gl_base_lines != NULL)
        {
            font_data.gl_base_lines->prev = NULL;
        }
        return;
    }

    line->prev->next = line->next;
    if(line->next)
    {
        line->next->prev = line->prev;
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
        gl_tex_font_p new_font = glf_create_font(font_data.font_library, path, size);
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
                     uint8_t shadow, uint8_t rect, uint8_t rect_border,
                     GLfloat rect_R, GLfloat rect_G, GLfloat rect_B, GLfloat rect_A)
{
    if(index < font_data.max_styles)
    {
        gl_fontstyle_p desired_style = font_data.styles + index;
                
        desired_style->rect_border   = rect_border;
        desired_style->rect_color[0] = rect_R;
        desired_style->rect_color[1] = rect_G;
        desired_style->rect_color[2] = rect_B;
        desired_style->rect_color[3] = rect_A;

        desired_style->font_color[0]  = R;
        desired_style->font_color[1]  = G;
        desired_style->font_color[2]  = B;
        desired_style->font_color[3]  = A;

        desired_style->shadowed  = shadow;
        desired_style->rect      = rect;
        return 1;
    }
    
    return 0;
}


int GLText_RemoveFontStyle(uint16_t index)
{
    if(index < font_data.max_styles)
    {
        font_data.styles[index].rect_color[0] = 1.0;
        font_data.styles[index].rect_color[1] = 1.0;
        font_data.styles[index].rect_color[2] = 1.0;
        font_data.styles[index].rect_color[3] = 0.0;

        font_data.styles[index].font_color[0] = 0.0;
        font_data.styles[index].font_color[1] = 0.0;
        font_data.styles[index].font_color[2] = 0.0;
        font_data.styles[index].font_color[3] = 1.0;

        font_data.styles[index].shadowed = 0x00;
        font_data.styles[index].rect     = 0x00;
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
