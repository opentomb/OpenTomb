#pragma once

/*
 * File:   gl_font.h
 * Author: TeslaRus
 *
 * Created on January 16, 2015, 10:46 PM
 */

#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <GL/glew.h>

#include <cstdint>
#include <memory>
#include <vector>


struct CharInfo
{
    GLuint          tex_index;
    GLint           width;
    GLint           height;
    GLint           left;
    GLint           top;

    GLfloat         tex_x0;
    GLfloat         tex_y0;
    GLfloat         tex_x1;
    GLfloat         tex_y1;

    GLfloat         advance_x;
    GLfloat         advance_y;
};

struct FontTexture
{
    FT_Library               ft_library;
    std::shared_ptr<std::remove_pointer<FT_Face>::type> ft_face{nullptr, &FT_Done_Face};
    uint16_t                 font_size;

    std::vector<CharInfo> glyphs;

    uint16_t                 gl_real_tex_indexes_count;
    std::vector<GLuint>      gl_tex_indexes;
    GLint                    gl_max_tex_width;
    GLint                    gl_tex_width;
    glm::vec4                gl_font_color;

    ~FontTexture()
    {
        if(!gl_tex_indexes.empty())
        {
            glDeleteTextures(static_cast<GLsizei>(gl_tex_indexes.size()), gl_tex_indexes.data());
        }
    }
};

std::shared_ptr<FontTexture> glf_create_font(FT_Library ft_library, const char *file_name, uint16_t font_size);
FontTexture* glf_create_font_mem(FT_Library ft_library, void *face_data, size_t face_data_size, uint16_t font_size);
void glf_resize(FontTexture* glf, uint16_t font_size);
void glf_reface(FontTexture* glf, const char *file_name, uint16_t font_size);

float    glf_get_string_len(FontTexture* glf, const char *text, int n);
float    glf_get_ascender(FontTexture* glf);
uint16_t glf_get_font_size(FontTexture* glf);
void     glf_get_string_bb(FontTexture* glf, const char *text, int n, GLfloat *x0, GLfloat *y0, GLfloat *x1, GLfloat *y1);

void     glf_render_str(FontTexture* glf, GLfloat x, GLfloat y, const char *text);     // UTF-8

uint32_t utf8_strlen(const char *str);
const uint8_t* utf8_to_utf32(const uint8_t *utf8, uint32_t *utf32);
