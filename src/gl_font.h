/* 
 * File:   gl_font.h
 * Author: nTesla
 *
 * Created on January 16, 2015, 10:46 PM
 */

#ifndef GL_FONT_H
#define	GL_FONT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <SDL2/SDL_opengl.h>
#include "freetype2/ft2build.h"
#include "freetype2/freetype/freetype.h"
    
    
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
}gl_tex_font_t, *gl_tex_font_p;


gl_tex_font_p glf_create_font(FT_Library ft_library, const char *file_name, uint16_t font_size);
void glf_free_font(gl_tex_font_p glf);
void glf_resize(gl_tex_font_p glf, uint16_t font_size);
void glf_reface(gl_tex_font_p glf, const char *file_name, uint16_t font_size);

float glf_get_string_len(gl_tex_font_p glf, const char *text, int n);
void glf_get_string_bb(gl_tex_font_p glf, const char *text, int n, GLfloat *x0, GLfloat *y0, GLfloat *x1, GLfloat *y1);
void glf_render_str(gl_tex_font_p glf, GLfloat x, GLfloat y, const char *text);

#ifdef	__cplusplus
}
#endif

#endif	/* GL_FONT_H */

