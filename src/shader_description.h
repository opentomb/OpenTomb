#ifndef __OpenTomb__shader_description__
#define __OpenTomb__shader_description__

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include "gl_util.h"

struct shader_stage
{
    GLhandleARB shader;
    
    shader_stage(GLenum type, const char *filename, const char *additionalDefines = 0);
    ~shader_stage();
};

/*!
 * A shader description consists of a program, code to load the
 * program, and the indices of the various uniform values. Each
 * shader or set of related shaders will have its own subclass
 * of shader_description. We assume (for now) that every shader
 * has a texture.
 */
struct shader_description
{
    GLhandleARB program;
    GLint sampler;
    
    shader_description(const shader_stage &vertex, const shader_stage &fragment);
    ~shader_description();
};

/*!
 * A shader description specifically for use in GUI situations.
 */
struct gui_shader_description : public shader_description
{
    GLint offset;
    GLint factor;
    
    enum vertex_attribs {
        position = 0,
        color
    };
    
    gui_shader_description(const shader_stage &vertex, const shader_stage &fragment);
};

/*!
 * A shader description specifically for use in GUI situations.
 */
struct sprite_shader_description : public shader_description
{
    GLint model_view;
    GLint projection;
    
    enum vertex_attribs {
        position = 0,
        corner_offset,
        tex_coord
    };
    
    sprite_shader_description(const shader_stage &vertex, const shader_stage &fragment);
};

/*!
 * A shader description for text
 */
struct text_shader_description : public shader_description
{
    GLint screenSize;
    
    enum vertex_attribs {
        position = 0,
        color,
        tex_coord
    };
    
    text_shader_description(const shader_stage &vertex, const shader_stage &fragment);
};

/*!
 * A shader description type that contains transform information. This comes in the form of a model view projection matrix.
 */
struct unlit_shader_description : public shader_description
{
    GLint model_view_projection;
    
    enum vertex_attribs {
        position = 0,
        color,
        tex_coord,
        normal,
        matrix_index
    };
    
    unlit_shader_description(const shader_stage &vertex, const shader_stage &fragment);
};

/*!
 * A shader description type that is suitable for lit objects. Also
 * contains a model view matrix and information about the current
 * light situation
 */
struct lit_shader_description : public unlit_shader_description
{
    GLint model_view;
    GLint projection;
    GLint number_of_lights;
    GLint Lightosition;
    GLint light_color;
    GLint light_inner_radius;
    GLint light_outer_radius;
    GLint light_ambient;
    
    lit_shader_description(const shader_stage &vertex, const shader_stage &fragment);
};

struct unlit_tinted_shader_description : public unlit_shader_description
{
    GLint current_tick;
    GLint tint_mult;
    
    unlit_tinted_shader_description(const shader_stage &vertex, const shader_stage &fragment);
};

#endif /* defined(__OpenTomb__shader_description__) */
