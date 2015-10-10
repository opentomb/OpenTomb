/*
 * File:   gl_font.h
 * Author: TeslaRus
 *
 * Created on January 16, 2015, 10:46 PM
 */

#ifndef GL_FONT_H
#define	GL_FONT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <ft2build.h>
#include <freetype.h>


typedef struct char_info_s
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
}char_info_t, *char_info_p;

typedef struct gl_tex_font_s
{
    FT_Library               ft_library;
    FT_Face                  ft_face;
    uint16_t                 font_size;

    struct char_info_s      *glyphs;
    uint16_t                 glyphs_count;

    uint16_t                 gl_tex_indexes_count;
    uint16_t                 gl_real_tex_indexes_count;
    GLuint                  *gl_tex_indexes;
    GLint                    gl_max_tex_width;
    GLint                    gl_tex_width;
    GLfloat                  gl_font_color[4];
}gl_tex_font_t, *gl_tex_font_p;


// Font struct contains additional field for font type which is
// used to dynamically create or delete fonts.
typedef struct gl_font_cont_s
{
    uint16_t                    font_size;
    struct gl_tex_font_s       *gl_font;
}gl_font_cont_t, *gl_font_cont_p;


// Font style is different to font itself - whereas engine can have
// only three fonts, there could be unlimited amount of font styles.
// Font style management is done via font manager.
typedef struct gl_fontstyle_s
{
    GLfloat                     font_color[4];
    GLfloat                     rect_color[4];
    GLfloat                     rect_border;

    uint8_t                     shadowed;
    uint8_t                     rect;
} gl_fontstyle_t, *gl_fontstyle_p;

#define GUI_FONT_FADE_SPEED             1.0                 // Global fading style speed.
#define GUI_FONT_FADE_MIN               0.3                 // Minimum fade multiplier.

#define GUI_FONT_SHADOW_TRANSPARENCY     0.7
#define GUI_FONT_SHADOW_VERTICAL_SHIFT  -0.9
#define GUI_FONT_SHADOW_HORIZONTAL_SHIFT 0.7


gl_tex_font_p glf_create_font(FT_Library ft_library, const char *file_name, uint16_t font_size);
gl_tex_font_p glf_create_font_mem(FT_Library ft_library, void *face_data, size_t face_data_size, uint16_t font_size);
void glf_free_font(gl_tex_font_p glf);
void glf_resize(gl_tex_font_p glf, uint16_t font_size);
void glf_reface(gl_tex_font_p glf, const char *file_name, uint16_t font_size);

float    glf_get_string_len(gl_tex_font_p glf, const char *text, int n);
float    glf_get_ascender(gl_tex_font_p glf);
uint16_t glf_get_font_size(gl_tex_font_p glf);
void     glf_get_string_bb(gl_tex_font_p glf, const char *text, int n, GLfloat *x0, GLfloat *y0, GLfloat *x1, GLfloat *y1);

void     glf_render_str(gl_tex_font_p glf, GLfloat x, GLfloat y, const char *text);     // UTF-8

uint32_t utf8_strlen(const char *str);
uint8_t* utf8_to_utf32(uint8_t *utf8, uint32_t *utf32);


#ifdef	__cplusplus
}
#endif

#endif	/* GL_FONT_H */

