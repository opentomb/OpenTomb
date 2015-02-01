/*
 * File:   gl_font.c
 * Author: TeslaRus
 *
 * Created on January 16, 2015, 10:46 PM
 */

#include <stdint.h>
#include <SDL2/SDL_opengl.h>
#include <math.h>

#include "freetype2/ft2build.h"
#include "freetype2/freetype/freetype.h"
#include "freetype2/freetype/ftglyph.h"
#include "freetype2/freetype/ftmodapi.h"

#include "gl_font.h"


gl_tex_font_p glf_create_font(FT_Library ft_library, const char *file_name, uint16_t font_size)
{
    if(ft_library != NULL)
    {
        gl_tex_font_p glf = (gl_tex_font_p)malloc(sizeof(gl_tex_font_t));
        glf->ft_face = (FT_Face)calloc(sizeof(struct FT_FaceRec_), 1);

        if(FT_New_Face(ft_library, file_name, 0, &glf->ft_face))
        {
            free(glf->ft_face);
            free(glf);
            return NULL;
        }

        glf->glyphs_count = glf->ft_face->num_glyphs;
        glf->glyphs = (char_info_p)malloc(glf->glyphs_count * sizeof(char_info_t));

        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glf->gl_max_tex_width);
        glf->gl_tex_width = glf->gl_max_tex_width;
        glf->gl_tex_indexes = NULL;
        glf->gl_tex_indexes_count = 0;
        glf->gl_real_tex_indexes_count = 0;
        glf_resize(glf, font_size);
        FT_Select_Charmap(glf->ft_face, FT_ENCODING_UNICODE);

        return glf;
    }

    return NULL;
}


void glf_free_font(gl_tex_font_p glf)
{
    if(glf != NULL)
    {
        if(glf->ft_face != NULL)
        {
            FT_Done_Face(glf->ft_face);
            free(glf->ft_face);
            glf->ft_face = NULL;
        }

        glf->ft_face = NULL;
        if(glf->glyphs != NULL)
        {
            free(glf->glyphs);
            glf->glyphs = NULL;
        }
        glf->glyphs_count = 0;

        glf->gl_real_tex_indexes_count = 0;
        if(glf->gl_tex_indexes != NULL)
        {
            if(glf->gl_tex_indexes_count > 0)
            {
                glDeleteTextures(glf->gl_tex_indexes_count, glf->gl_tex_indexes);
            }
            free(glf->gl_tex_indexes);
        }
        glf->gl_tex_indexes_count = 0;
        glf->gl_tex_indexes = NULL;

        free(glf);
    }
}


static __inline GLuint NextPowerOf2(GLuint in)
{
     in -= 1;

     in |= in >> 16;
     in |= in >> 8;
     in |= in >> 4;
     in |= in >> 2;
     in |= in >> 1;

     return in + 1;
}


static __inline void bbox_add(float *x0, float *x1, float *y0, float *y1,
                              float *x_min, float *x_max, float *y_min, float *y_max)
{
    float min, max;

    if(*x0 > *x1)
    {
        min = *x1;
        max = *x0;
    }
    else
    {
        min = *x0;
        max = *x1;
    }

    if(*x_min > min)
    {
        *x_min = min;
    }
    if(*x_max < max)
    {
        *x_max = max;
    }

    if(*y0 > *y1)
    {
        min = *y1;
        max = *y0;
    }
    else
    {
        min = *y0;
        max = *y1;
    }

    if(*y_min > min)
    {
        *y_min = min;
    }
    if(*y_max < max)
    {
        *y_max = max;
    }
}


void glf_resize(gl_tex_font_p glf, uint16_t font_size)
{
    if((glf != NULL) && (glf->ft_face != NULL))
    {
        const GLint padding = 2;
        GLubyte *buffer;
        GLint chars_in_row, chars_in_column;
        size_t buffer_size;
        int x, y, xx, yy;
        int i, ii, i0 = 0;

        // clear old atlas, if exists
        if(glf->gl_tex_indexes != NULL)
        {
            if(glf->gl_tex_indexes_count > 0)
            {
                glDeleteTextures(glf->gl_tex_indexes_count, glf->gl_tex_indexes);
            }
            free(glf->gl_tex_indexes);
        }
        glf->gl_tex_indexes = NULL;
        glf->gl_real_tex_indexes_count = 0;

        // resize base font
        glf->font_size = font_size;
        FT_Set_Char_Size(glf->ft_face, font_size << 6, font_size << 6, 0, 0);

        // calculate texture atlas size
        chars_in_row = 1 + sqrt(glf->glyphs_count);
        glf->gl_tex_width = (font_size + padding) * chars_in_row;
        glf->gl_tex_width = NextPowerOf2(glf->gl_tex_width);
        if(glf->gl_tex_width > glf->gl_max_tex_width)
        {
            glf->gl_tex_width = glf->gl_max_tex_width;
        }

        // create new atlas
        chars_in_row = glf->gl_tex_width / (font_size + padding);
        chars_in_column = glf->glyphs_count / chars_in_row + 1;
        glf->gl_tex_indexes_count = (chars_in_column * (font_size + padding)) / glf->gl_tex_width + 1;
        glf->gl_tex_indexes = (GLuint*)malloc(glf->gl_tex_indexes_count * sizeof(GLuint));
        glGenTextures(glf->gl_tex_indexes_count, glf->gl_tex_indexes);

        buffer_size = glf->gl_tex_width * glf->gl_tex_width * sizeof(GLubyte);
        buffer = (GLubyte*)malloc(buffer_size);
        memset(buffer, 0x00, buffer_size);

        for(i=0,x=0,y=0;i<glf->glyphs_count;i++)
        {
            FT_GlyphSlot g;
            glf->glyphs[i].tex_index = 0;

            /* load glyph image into the slot (erase previous one) */
            if(FT_Load_Glyph(glf->ft_face, i, FT_LOAD_RENDER))
            {
                continue;
            }
            /* convert to an anti-aliased bitmap */
            if(FT_Render_Glyph(glf->ft_face->glyph, FT_RENDER_MODE_NORMAL))
            {
                continue;
            }

            g = glf->ft_face->glyph;
            glf->glyphs[i].width = g->bitmap.width;
            glf->glyphs[i].height = g->bitmap.rows;
            glf->glyphs[i].advance_x = g->advance.x;
            glf->glyphs[i].advance_y = g->advance.y;
            glf->glyphs[i].left = g->bitmap_left;
            glf->glyphs[i].top = g->bitmap_top;

            if((g->bitmap.width == 0) || (g->bitmap.rows == 0))
            {
                continue;
            }

            if(x + g->bitmap.width > glf->gl_tex_width)
            {
                x = 0;
                y += glf->font_size + padding;
                if(y + glf->font_size > glf->gl_tex_width)
                {
                    int ii;
                    glBindTexture(GL_TEXTURE_2D, glf->gl_tex_indexes[glf->gl_real_tex_indexes_count]);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, glf->gl_tex_width, glf->gl_tex_width, 0, GL_ALPHA, GL_UNSIGNED_BYTE, buffer);
                    for(ii=i0;ii<i;ii++)
                    {
                        glf->glyphs[ii].tex_x0 /= (GLfloat)glf->gl_tex_width;
                        glf->glyphs[ii].tex_x1 /= (GLfloat)glf->gl_tex_width;
                        glf->glyphs[ii].tex_y0 /= (GLfloat)glf->gl_tex_width;
                        glf->glyphs[ii].tex_y1 /= (GLfloat)glf->gl_tex_width;
                    }
                    memset(buffer, 0x00, buffer_size);
                    y = 0;
                    i0 = i;
                    glf->gl_real_tex_indexes_count++;
                }
            }

            glf->glyphs[i].tex_x0 = (GLfloat)x;
            glf->glyphs[i].tex_y0 = (GLfloat)y;
            glf->glyphs[i].tex_x1 = (GLfloat)(x + g->bitmap.width);
            glf->glyphs[i].tex_y1 = (GLfloat)(y + g->bitmap.rows);

            glf->glyphs[i].tex_index = glf->gl_tex_indexes[glf->gl_real_tex_indexes_count];
            for(xx=0;xx<g->bitmap.width;xx++)
            {
                for(yy=0;yy<g->bitmap.rows;yy++)
                {
                    buffer[(y+yy)*glf->gl_tex_width + (x+xx)] = g->bitmap.buffer[yy * g->bitmap.width + xx];
                }
            }

            x += (g->bitmap.width + padding);
        }

        glBindTexture(GL_TEXTURE_2D, glf->gl_tex_indexes[glf->gl_real_tex_indexes_count]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        chars_in_column = NextPowerOf2(y + font_size + padding);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, glf->gl_tex_width, chars_in_column, 0, GL_ALPHA, GL_UNSIGNED_BYTE, buffer);
        for(ii=i0;ii<glf->glyphs_count;ii++)
        {
            glf->glyphs[ii].tex_x0 /= (GLfloat)glf->gl_tex_width;
            glf->glyphs[ii].tex_x1 /= (GLfloat)glf->gl_tex_width;
            glf->glyphs[ii].tex_y0 /= (GLfloat)chars_in_column;
            glf->glyphs[ii].tex_y1 /= (GLfloat)chars_in_column;
        }
        free(buffer);
        glf->gl_real_tex_indexes_count++;
    }
}


void glf_reface(gl_tex_font_p glf, const char *file_name, uint16_t font_size)
{
    if(FT_New_Face(glf->ft_library, file_name, 0, &glf->ft_face))
    {
        return;
    }
    glf_resize(glf, font_size);
}


float glf_get_ascender(gl_tex_font_p glf)
{
    if((glf->font_size == 0) || (glf->ft_face == NULL))
    {
        return 0.0;
    }

    return (float)(glf->ft_face->ascender) / 64.0f;
}


uint16_t glf_get_font_size(gl_tex_font_p glf)
{
    if((glf != NULL) && (glf->ft_face != NULL))
    {
        return glf->font_size;
    }
    else
    {
        return 0;
    }
}


float glf_get_string_len(gl_tex_font_p glf, const char *text, int n)
{
    float x = 0.0;

    if((glf != NULL) && (glf->ft_face != NULL))
    {
        uint8_t *nch, *nch2, *ch = (uint8_t*)text;
        uint32_t curr_utf32, next_utf32;
        int i;

        nch = utf8_to_utf32(ch, &curr_utf32);
        curr_utf32 = FT_Get_Char_Index(glf->ft_face, curr_utf32);

        for(i=0;(*ch!=0) && !((n>=0)&&(i>=n));i++)
        {
            FT_Vector kern;

            nch2 = utf8_to_utf32(nch, &next_utf32);
            next_utf32 = FT_Get_Char_Index(glf->ft_face, next_utf32);
            ch = nch;
            nch = nch2;

            FT_Get_Kerning(glf->ft_face, curr_utf32, next_utf32, FT_KERNING_UNSCALED, &kern);   // kern in 1/64 pixel
            curr_utf32 = next_utf32;
            x += (GLfloat)(kern.x + glf->glyphs[curr_utf32].advance_x) / 64.0;
        }
    }

    return x;
}


void glf_get_string_bb(gl_tex_font_p glf, const char *text, int n, GLfloat *x0, GLfloat *y0, GLfloat *x1, GLfloat *y1)
{
    *x0 = 0.0;
    *x1 = 0.0;
    *y0 = 0.0;
    *y1 = 0.0;

    if((glf != NULL) && (glf->ft_face != NULL))
    {

        uint8_t *nch, *nch2, *ch = (uint8_t*)text;
        float x = 0.0;
        float y = 0.0;
        float xx0, xx1, yy0, yy1;
        int i;
        uint32_t curr_utf32, next_utf32;

        nch = utf8_to_utf32(ch, &curr_utf32);
        curr_utf32 = FT_Get_Char_Index(glf->ft_face, curr_utf32);

        for(i=0;(*ch!=0) && !((n>=0)&&(i>=n));i++)
        {
            FT_Vector kern;
            char_info_p g = glf->glyphs + curr_utf32;

            nch2 = utf8_to_utf32(nch, &next_utf32);

            next_utf32 = FT_Get_Char_Index(glf->ft_face, next_utf32);
            ch = nch;
            nch = nch2;

            FT_Get_Kerning(glf->ft_face, curr_utf32, next_utf32, FT_KERNING_UNSCALED, &kern);   // kern in 1/64 pixel
            curr_utf32 = next_utf32;

            xx0 = x  + g->left;
            xx1 = xx0 + g->width;
            yy0 = y  + g->top;
            yy1 = yy0 - g->height;
            bbox_add(&xx0, &xx1, &yy0, &yy1, x0, x1, y0, y1);

            x += (GLfloat)(kern.x + g->advance_x) / 64.0;
            y += (GLfloat)(kern.y + g->advance_y) / 64.0;
        }
    }
}


void glf_render_str(gl_tex_font_p glf, GLfloat x, GLfloat y, const char *text)
{
    uint8_t *nch, *ch = (uint8_t*)text;
    FT_Vector kern;

    if((glf == NULL) || (glf->ft_face == NULL) || (text == NULL))
    {
        return;
    }

    if(glf->gl_real_tex_indexes_count == 1)
    {
        GLfloat *p, buffer[24 * strlen(text)];
        GLuint elements_count = 0;
        uint32_t curr_utf32, next_utf32;
        nch = utf8_to_utf32(ch, &curr_utf32);
        curr_utf32 = FT_Get_Char_Index(glf->ft_face, curr_utf32);

        for(p=buffer;*ch;)
        {
            char_info_p g;
            uint8_t *nch2 = utf8_to_utf32(nch, &next_utf32);

            next_utf32 = FT_Get_Char_Index(glf->ft_face, next_utf32);
            ch = nch;
            nch = nch2;

            g = glf->glyphs + curr_utf32;
            FT_Get_Kerning(glf->ft_face, curr_utf32, next_utf32, FT_KERNING_UNSCALED, &kern);   // kern in 1/64 pixel
            curr_utf32 = next_utf32;

            if(g->tex_index != 0)
            {
                GLfloat x0 = x  + g->left;
                GLfloat x1 = x0 + g->width;
                GLfloat y0 = y  + g->top;
                GLfloat y1 = y0 - g->height;

                *p = x0;            p++;
                *p = y0;            p++;
                *p = g->tex_x0;     p++;
                *p = g->tex_y0;     p++;

                *p = x1;            p++;
                *p = y0;            p++;
                *p = g->tex_x1;     p++;
                *p = g->tex_y0;     p++;

                *p = x1;            p++;
                *p = y1;            p++;
                *p = g->tex_x1;     p++;
                *p = g->tex_y1;     p++;
                elements_count++;

                *p = x0;            p++;
                *p = y0;            p++;
                *p = g->tex_x0;     p++;
                *p = g->tex_y0;     p++;

                *p = x1;            p++;
                *p = y1;            p++;
                *p = g->tex_x1;     p++;
                *p = g->tex_y1;     p++;

                *p = x0;            p++;
                *p = y1;            p++;
                *p = g->tex_x0;     p++;
                *p = g->tex_y1;     p++;
                elements_count++;
            }
            x += (GLfloat)(kern.x + g->advance_x) / 64.0;
            y += (GLfloat)(kern.y + g->advance_y) / 64.0;
        }
        ///RENDER
        if(elements_count != 0)
        {
            glBindTexture(GL_TEXTURE_2D, glf->gl_tex_indexes[0]);
            glVertexPointer(2, GL_FLOAT, 4 * sizeof(GLfloat), buffer);
            glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(GLfloat), buffer + 2);
            glDrawArrays(GL_TRIANGLES, 0, elements_count * 3);
        }
    }
    else
    {
        GLuint active_texture = 0;
        uint32_t curr_utf32, next_utf32;
        nch = utf8_to_utf32(ch, &curr_utf32);
        curr_utf32 = FT_Get_Char_Index(glf->ft_face, curr_utf32);
        for(;*ch;)
        {
            char_info_p g;
            uint8_t *nch2 = utf8_to_utf32(nch, &next_utf32);

            next_utf32 = FT_Get_Char_Index(glf->ft_face, next_utf32);
            ch = nch;
            nch = nch2;

            g = glf->glyphs + curr_utf32;
            FT_Get_Kerning(glf->ft_face, curr_utf32, next_utf32, FT_KERNING_UNSCALED, &kern);   // kern in 1/64 pixel
            curr_utf32 = next_utf32;

            if(g->tex_index != 0)
            {
                if(active_texture != g->tex_index)
                {
                    glBindTexture(GL_TEXTURE_2D, g->tex_index);
                    active_texture = g->tex_index;
                }
                ///RENDER
                GLfloat x0 = x  + g->left;
                GLfloat x1 = x0 + g->width;
                GLfloat y0 = y  + g->top;
                GLfloat y1 = y0 - g->height;

                GLfloat box[] = {
                x0, y0, g->tex_x0, g->tex_y0,
                x1, y0, g->tex_x1, g->tex_y0,
                x1, y1, g->tex_x1, g->tex_y1,
                x0, y1, g->tex_x0, g->tex_y1};

                glVertexPointer(2, GL_FLOAT, 4 * sizeof(GLfloat), box);
                glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(GLfloat), box + 2);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            }
            x += (GLfloat)(kern.x + g->advance_x) / 64.0;
            y += (GLfloat)(kern.y + g->advance_y) / 64.0;
        }
    }
}

/** Additional functions */

static uint8_t* utf8_next_symbol(uint8_t *utf8)
{
    uint8_t b = *utf8;

    // save ASC symbol as is
    if(!(b & 0x80))
    {
        return utf8 + 1;
    }

    // calculate lenght
    while(b & 0x80)
    {
        b <<= 1;
        utf8++;
    }

    return utf8;
}


uint32_t utf8_strlen(const char *str)
{
    uint32_t i = 0;
    uint8_t *ch = (uint8_t*)str;

    for(;*ch;i++)
    {
        ch = utf8_next_symbol(ch);
    }

    return i;
}


uint8_t* utf8_to_utf32(uint8_t *utf8, uint32_t *utf32)
{
    uint8_t *u_utf8 = utf8;
    uint8_t b = *u_utf8++;
    uint32_t c, shift;
    int len = 0;

    // save ASC symbol as is
    if(!(b & 0x80))
    {
       *utf32 = b;
        return utf8 + 1;
    }

    // calculate lenght
    while(b & 0x80)
    {
        b <<= 1;
        ++len;
    }

    c = b;
    shift = 6 - len;

    while(--len)
    {
        c <<= shift;
        c |= (*u_utf8++) & 0x3f;
        shift = 6;
    }

   *utf32 = c;
    return u_utf8;
}
