/*
 * File:   gl_font.c
 * Author: TeslaRus
 *
 * Created on January 16, 2015, 10:46 PM
 */

#include "gl_font.h"

#include <cmath>
#include <cstdint>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "gl_font_buffer.h"

#define vec4_copy(x, y) {(x)[0] = (y)[0]; (x)[1] = (y)[1]; (x)[2] = (y)[2]; (x)[3] = (y)[3];}

std::shared_ptr<FontTexture> glf_create_font(FT_Library ft_library, const char *file_name, uint16_t font_size)
{
    if(ft_library != nullptr)
    {
        std::shared_ptr<FontTexture> glf = std::make_shared<FontTexture>();

        FT_Face face = nullptr;
        if(FT_New_Face(ft_library, file_name, 0, &face))                //T4Larson <t4larson@gmail.com>: fixed font construction and destruction!
        {
            return nullptr;
        }
        glf->ft_face.reset(face, &FT_Done_Face);

        glf->glyphs.resize(glf->ft_face->num_glyphs);

        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glf->gl_max_tex_width);
        glf->gl_tex_width = glf->gl_max_tex_width;
        glf->gl_tex_indexes.clear();
        glf->gl_real_tex_indexes_count = 0;
        glf->gl_font_color[0] = 0.0;
        glf->gl_font_color[1] = 0.0;
        glf->gl_font_color[2] = 0.0;
        glf->gl_font_color[3] = 1.0;

        glf_resize(glf.get(), font_size);
        FT_Select_Charmap(glf->ft_face.get(), FT_ENCODING_UNICODE);

        return glf;
    }

    return nullptr;
}

/**
 * Creates gl texture font from true type font;
 * @param ft_library: base font library;
 * @param face_data: pointer to the buffer with font file content; DO NOT FREE that pointer otherway using FT_Face prevets to crash;
 * @param face_data_size: size of buffer with font file content;
 * @param font_size: size of font glyph?
 * @return pointer to the gl_tex_font_s structure;
 */
FontTexture *glf_create_font_mem(FT_Library ft_library, void *face_data, size_t face_data_size, uint16_t font_size)
{
    if(ft_library != nullptr)
    {
        FontTexture* glf = new FontTexture();

        FT_Face face;
        if(FT_New_Memory_Face(ft_library, static_cast<const FT_Byte*>(face_data), static_cast<FT_Long>(face_data_size), 0, &face))
        {
            delete glf;
            return nullptr;
        }
        glf->ft_face.reset(face, &FT_Done_Face);

        glf->glyphs.resize(glf->ft_face->num_glyphs);

        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glf->gl_max_tex_width);
        glf->gl_tex_width = glf->gl_max_tex_width;
        glf->gl_tex_indexes.clear();
        glf->gl_real_tex_indexes_count = 0;
        glf_resize(glf, font_size);
        FT_Select_Charmap(glf->ft_face.get(), FT_ENCODING_UNICODE);

        return glf;
    }

    return nullptr;
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

void glf_resize(FontTexture *glf, uint16_t font_size)
{
    if(glf != nullptr && glf->ft_face != nullptr)
    {
        const GLint padding = 2;
        GLint chars_in_row, chars_in_column;
        size_t buffer_size;

        // clear old atlas, if exists
        if(!glf->gl_tex_indexes.empty())
        {
            glDeleteTextures(static_cast<GLsizei>(glf->gl_tex_indexes.size()), glf->gl_tex_indexes.data());
        }
        glf->gl_tex_indexes.clear();
        glf->gl_real_tex_indexes_count = 0;

        // resize base font
        glf->font_size = font_size;
        FT_Set_Char_Size(glf->ft_face.get(), font_size << 6, font_size << 6, 0, 0);

        // calculate texture atlas size
        chars_in_row = static_cast<GLint>( 1 + std::sqrt(glf->glyphs.size()) );
        glf->gl_tex_width = (font_size + padding) * chars_in_row;
        glf->gl_tex_width = NextPowerOf2(glf->gl_tex_width);
        if(glf->gl_tex_width > glf->gl_max_tex_width)
        {
            glf->gl_tex_width = glf->gl_max_tex_width;
        }

        // create new atlas
        chars_in_row = glf->gl_tex_width / (font_size + padding);
        chars_in_column = static_cast<GLint>( glf->glyphs.size() / chars_in_row + 1 );
        glf->gl_tex_indexes.resize(chars_in_column * (font_size + padding) / glf->gl_tex_width + 1);
        glGenTextures(static_cast<GLsizei>(glf->gl_tex_indexes.size()), glf->gl_tex_indexes.data());

        buffer_size = glf->gl_tex_width * glf->gl_tex_width * sizeof(GLubyte);
        std::vector<GLubyte> buffer(buffer_size, 0);

        size_t y = 0;
        size_t i0 = 0;
        for(size_t i = 0, x = 0; i < glf->glyphs.size(); i++)
        {
            glf->glyphs[i].tex_index = 0;

            /* load glyph image into the slot (erase previous one) */
            if(FT_Load_Glyph(glf->ft_face.get(), i, FT_LOAD_RENDER))
            {
                continue;
            }
            /* convert to an anti-aliased bitmap */
            if(FT_Render_Glyph(glf->ft_face->glyph, FT_RENDER_MODE_NORMAL))
            {
                continue;
            }

            FT_GlyphSlot g = glf->ft_face->glyph;
            glf->glyphs[i].width = g->bitmap.width;
            glf->glyphs[i].height = g->bitmap.rows;
            glf->glyphs[i].advance_x = static_cast<GLfloat>(g->advance.x);
            glf->glyphs[i].advance_y = static_cast<GLfloat>(g->advance.y);
            glf->glyphs[i].left = g->bitmap_left;
            glf->glyphs[i].top = g->bitmap_top;

            if(g->bitmap.width == 0 || g->bitmap.rows == 0)
            {
                continue;
            }

            if(static_cast<GLint>(x + g->bitmap.width) > glf->gl_tex_width)
            {
                x = 0;
                y += glf->font_size + padding;
                if(static_cast<GLint>(y + glf->font_size) > glf->gl_tex_width)
                {
                    glBindTexture(GL_TEXTURE_2D, glf->gl_tex_indexes[glf->gl_real_tex_indexes_count]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, glf->gl_tex_width, glf->gl_tex_width, 0, GL_R8, GL_UNSIGNED_BYTE, buffer.data());
                    for(size_t ii2 = i0; ii2 < i; ii2++)
                    {
                        glf->glyphs[ii2].tex_x0 /= static_cast<GLfloat>(glf->gl_tex_width);
                        glf->glyphs[ii2].tex_x1 /= static_cast<GLfloat>(glf->gl_tex_width);
                        glf->glyphs[ii2].tex_y0 /= static_cast<GLfloat>(glf->gl_tex_width);
                        glf->glyphs[ii2].tex_y1 /= static_cast<GLfloat>(glf->gl_tex_width);
                    }
                    std::fill(buffer.begin(), buffer.end(), 0);
                    y = 0;
                    i0 = i;
                    glf->gl_real_tex_indexes_count++;
                }
            }

            glf->glyphs[i].tex_x0 = static_cast<GLfloat>(x);
            glf->glyphs[i].tex_y0 = static_cast<GLfloat>(y);
            glf->glyphs[i].tex_x1 = static_cast<GLfloat>(x + g->bitmap.width);
            glf->glyphs[i].tex_y1 = static_cast<GLfloat>(y + g->bitmap.rows);

            glf->glyphs[i].tex_index = glf->gl_tex_indexes[glf->gl_real_tex_indexes_count];
            for(size_t xx = 0; xx < g->bitmap.width; xx++)
            {
                for(size_t yy = 0; yy < g->bitmap.rows; yy++)
                {
                    buffer[(y + yy)*glf->gl_tex_width + (x + xx)] = g->bitmap.buffer[yy * g->bitmap.width + xx];
                }
            }

            x += g->bitmap.width + padding;
        }

        glBindTexture(GL_TEXTURE_2D, glf->gl_tex_indexes[glf->gl_real_tex_indexes_count]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        chars_in_column = NextPowerOf2(y + font_size + padding);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, glf->gl_tex_width, chars_in_column, 0, GL_RED, GL_UNSIGNED_BYTE, buffer.data());

        for(size_t ii = i0; ii < glf->glyphs.size(); ii++)
        {
            glf->glyphs[ii].tex_x0 /= static_cast<GLfloat>(glf->gl_tex_width);
            glf->glyphs[ii].tex_x1 /= static_cast<GLfloat>(glf->gl_tex_width);
            glf->glyphs[ii].tex_y0 /= static_cast<GLfloat>(chars_in_column);
            glf->glyphs[ii].tex_y1 /= static_cast<GLfloat>(chars_in_column);
        }
        buffer.clear();
        glf->gl_real_tex_indexes_count++;
    }
}

void glf_reface(FontTexture *glf, const char *file_name, uint16_t font_size)
{
    FT_Face face;
    if(FT_New_Face(glf->ft_library, file_name, 0, &face))
    {
        return;
    }
    glf->ft_face.reset(face, &FT_Done_Face);
    glf_resize(glf, font_size);
}

float glf_get_ascender(FontTexture *glf)
{
    if(glf->font_size == 0 || glf->ft_face == nullptr)
    {
        return 0.0;
    }

    return static_cast<float>(glf->ft_face->ascender) / 64.0f;
}

uint16_t glf_get_font_size(FontTexture *glf)
{
    if(glf != nullptr && glf->ft_face != nullptr)
    {
        return glf->font_size;
    }
    else
    {
        return 0;
    }
}

float glf_get_string_len(FontTexture *glf, const char *text, int n)
{
    float x = 0.0;

    if(glf != nullptr && glf->ft_face != nullptr)
    {
        const uint8_t *ch = reinterpret_cast<const uint8_t*>(text);
        uint32_t curr_utf32, next_utf32;
        int i;

        const uint8_t *nch = utf8_to_utf32(ch, &curr_utf32);
        curr_utf32 = FT_Get_Char_Index(glf->ft_face.get(), curr_utf32);

        for(i = 0; *ch != 0 && !(n >= 0 && i >= n); i++)
        {
            FT_Vector kern;

            const uint8_t *nch2 = utf8_to_utf32(nch, &next_utf32);
            next_utf32 = FT_Get_Char_Index(glf->ft_face.get(), next_utf32);
            ch = nch;
            nch = nch2;

            FT_Get_Kerning(glf->ft_face.get(), curr_utf32, next_utf32, FT_KERNING_UNSCALED, &kern);   // kern in 1/64 pixel
            curr_utf32 = next_utf32;
            x += (kern.x + glf->glyphs[curr_utf32].advance_x) / 64.0f;
        }
    }

    return x;
}

void glf_get_string_bb(FontTexture *glf, const char *text, int n, GLfloat *x0, GLfloat *y0, GLfloat *x1, GLfloat *y1)
{
    *x0 = 0.0;
    *x1 = 0.0;
    *y0 = 0.0;
    *y1 = 0.0;

    if(glf != nullptr && glf->ft_face != nullptr)
    {
        const uint8_t *nch2;
        const uint8_t *ch = reinterpret_cast<const uint8_t*>(text);
        float x = 0.0;
        float y = 0.0;
        float xx0, xx1, yy0, yy1;
        int i;
        uint32_t curr_utf32, next_utf32;

        const uint8_t *nch = utf8_to_utf32(ch, &curr_utf32);
        curr_utf32 = FT_Get_Char_Index(glf->ft_face.get(), curr_utf32);

        for(i = 0; *ch != 0 && !(n >= 0 && i >= n); i++)
        {
            FT_Vector kern;
            CharInfo* g = &glf->glyphs[curr_utf32];

            nch2 = utf8_to_utf32(nch, &next_utf32);

            next_utf32 = FT_Get_Char_Index(glf->ft_face.get(), next_utf32);
            ch = nch;
            nch = nch2;

            FT_Get_Kerning(glf->ft_face.get(), curr_utf32, next_utf32, FT_KERNING_UNSCALED, &kern);   // kern in 1/64 pixel
            curr_utf32 = next_utf32;

            xx0 = x + g->left;
            xx1 = xx0 + g->width;
            yy0 = y + g->top;
            yy1 = yy0 - g->height;
            bbox_add(&xx0, &xx1, &yy0, &yy1, x0, x1, y0, y1);

            x += (kern.x + g->advance_x) / 64.0f;
            y += (kern.y + g->advance_y) / 64.0f;
        }
    }
}

void glf_render_str(FontTexture *glf, GLfloat x, GLfloat y, const char *text)
{
    const uint8_t *nch;
    const uint8_t *ch = reinterpret_cast<const uint8_t*>(text);
    FT_Vector kern;

    if(glf == nullptr || glf->ft_face == nullptr || text == nullptr || text[0] == '\0')
    {
        return;
    }

    FontBuffer_Bind();

    if(glf->gl_real_tex_indexes_count == 1)
    {
        GLfloat *p = FontBuffer_ResizeAndMap(48 * utf8_strlen(text) * sizeof(GLfloat));
        GLuint elements_count = 0;
        uint32_t curr_utf32, next_utf32;
        nch = utf8_to_utf32(ch, &curr_utf32);
        curr_utf32 = FT_Get_Char_Index(glf->ft_face.get(), curr_utf32);

        while(*ch)
        {
            CharInfo* g;
            const uint8_t *nch2 = utf8_to_utf32(nch, &next_utf32);

            next_utf32 = FT_Get_Char_Index(glf->ft_face.get(), next_utf32);
            ch = nch;
            nch = nch2;

            g = &glf->glyphs[curr_utf32];
            FT_Get_Kerning(glf->ft_face.get(), curr_utf32, next_utf32, FT_KERNING_UNSCALED, &kern);   // kern in 1/64 pixel
            curr_utf32 = next_utf32;

            if(g->tex_index != 0)
            {
                GLfloat x0 = x + g->left;
                GLfloat x1 = x0 + g->width;
                GLfloat y0 = y + g->top;
                GLfloat y1 = y0 - g->height;

                *p = x0;            p++;
                *p = y0;            p++;
                *p = g->tex_x0;     p++;
                *p = g->tex_y0;     p++;
                vec4_copy(p, glf->gl_font_color);   p += 4;

                *p = x1;            p++;
                *p = y0;            p++;
                *p = g->tex_x1;     p++;
                *p = g->tex_y0;     p++;
                vec4_copy(p, glf->gl_font_color);   p += 4;

                *p = x1;            p++;
                *p = y1;            p++;
                *p = g->tex_x1;     p++;
                *p = g->tex_y1;     p++;
                vec4_copy(p, glf->gl_font_color);   p += 4;
                elements_count++;

                *p = x0;            p++;
                *p = y0;            p++;
                *p = g->tex_x0;     p++;
                *p = g->tex_y0;     p++;
                vec4_copy(p, glf->gl_font_color);   p += 4;

                *p = x1;            p++;
                *p = y1;            p++;
                *p = g->tex_x1;     p++;
                *p = g->tex_y1;     p++;
                vec4_copy(p, glf->gl_font_color);   p += 4;

                *p = x0;            p++;
                *p = y1;            p++;
                *p = g->tex_x0;     p++;
                *p = g->tex_y1;     p++;
                vec4_copy(p, glf->gl_font_color);   p += 4;
                elements_count++;
            }
            x += (kern.x + g->advance_x) / 64.0f;
            y += (kern.y + g->advance_y) / 64.0f;
        }
        FontBuffer_Unmap();
        ///RENDER
        if(elements_count != 0)
        {
            glBindTexture(GL_TEXTURE_2D, glf->gl_tex_indexes[0]);
            glDrawArrays(GL_TRIANGLES, 0, elements_count * 3);
        }
    }
    else
    {
        GLuint active_texture = 0;
        uint32_t curr_utf32, next_utf32;
        nch = utf8_to_utf32(ch, &curr_utf32);
        curr_utf32 = FT_Get_Char_Index(glf->ft_face.get(), curr_utf32);
        for(; *ch;)
        {
            GLfloat *p = FontBuffer_ResizeAndMap(sizeof(GLfloat[32]));
            CharInfo* g;
            const uint8_t *nch2 = utf8_to_utf32(nch, &next_utf32);

            next_utf32 = FT_Get_Char_Index(glf->ft_face.get(), next_utf32);
            ch = nch;
            nch = nch2;

            g = &glf->glyphs[curr_utf32];
            FT_Get_Kerning(glf->ft_face.get(), curr_utf32, next_utf32, FT_KERNING_UNSCALED, &kern);   // kern in 1/64 pixel
            curr_utf32 = next_utf32;

            if(g->tex_index != 0)
            {
                if(active_texture != g->tex_index)
                {
                    glBindTexture(GL_TEXTURE_2D, g->tex_index);
                    active_texture = g->tex_index;
                }
                ///RENDER
                GLfloat x0 = x + g->left;
                GLfloat x1 = x0 + g->width;
                GLfloat y0 = y + g->top;
                GLfloat y1 = y0 - g->height;

                *p = x0;            p++;
                *p = y0;            p++;
                *p = g->tex_x0;     p++;
                *p = g->tex_y0;     p++;
                vec4_copy(p, glf->gl_font_color);   p += 4;

                *p = x1;            p++;
                *p = y0;            p++;
                *p = g->tex_x1;     p++;
                *p = g->tex_y0;     p++;
                vec4_copy(p, glf->gl_font_color);   p += 4;

                *p = x1;            p++;
                *p = y1;            p++;
                *p = g->tex_x1;     p++;
                *p = g->tex_y1;     p++;
                vec4_copy(p, glf->gl_font_color);   p += 4;

                *p = x0;            p++;
                *p = y1;            p++;
                *p = g->tex_x0;     p++;
                *p = g->tex_y1;     p++;
                vec4_copy(p, glf->gl_font_color);

                FontBuffer_Unmap();

                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            }
            x += (kern.x + g->advance_x) / 64.0f;
            y += (kern.y + g->advance_y) / 64.0f;
        }
    }
}

/** Additional functions */

static const uint8_t* utf8_next_symbol(const uint8_t *utf8)
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
    const uint8_t *ch = reinterpret_cast<const uint8_t*>(str);

    for(; *ch; i++)
    {
        ch = utf8_next_symbol(ch);
    }

    return i;
}

const uint8_t* utf8_to_utf32(const uint8_t *utf8, uint32_t *utf32)
{
    const uint8_t *u_utf8 = utf8;
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
        c |= *u_utf8++ & 0x3f;
        shift = 6;
    }

    *utf32 = c;
    return u_utf8;
}
