#pragma once

#include "gl_util.h"

namespace render
{

// A shader stage is a simple wrapper to load an OpenGL shader
// object more easily.

// Note about ownership: The destructor calls glDeleteShader,
// which marks the shader object for deletion. OpenGL only
// deletes it when the last program (shader description) using
// it is deleted.

struct ShaderStage
{
    GLuint shader;

    ShaderStage(GLenum type, const char *filename, const char *additionalDefines = nullptr);
    ~ShaderStage();
};

// A shader description consists of a program, code to load the
// program, and the indices of the various uniform values. Each
// shader or set of related shaders will have its own subclass
// of shader_description. We assume (for now) that every shader
// has a texture.

struct ShaderDescription
{
    GLuint program;
    GLint sampler;

    ShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment);
    ~ShaderDescription();
};

// A shader description specifically for use in GUI situations.

struct GuiShaderDescription : public ShaderDescription
{
    GLint offset;
    GLint factor;

    enum vertex_attribs
    {
        position = 0,
        color
    };

    GuiShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment);
};

// A shader description specifically for use in GUI situations.

struct SpriteShaderDescription : public ShaderDescription
{
    GLint model_view;
    GLint projection;

    enum vertex_attribs
    {
        position = 0,
        corner_offset,
        tex_coord
    };

    SpriteShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment);
};


// A shader description for text

struct TextShaderDescription : public ShaderDescription
{
    GLint screenSize;

    enum vertex_attribs
    {
        position = 0,
        color,
        tex_coord
    };

    TextShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment);
};

// A shader description type that contains transform information.
// This comes in the form of a model view projection matrix.

struct UnlitShaderDescription : public ShaderDescription
{
    GLint model_view_projection;

    enum VertexAttribs
    {
        Position = 0,
        Color,
        TexCoord,
        Normal,
        MatrixIndex
    };

    UnlitShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment);
};

// A shader description type that is suitable for lit objects. Also
// contains a model view matrix and information about the current
// light situation

struct LitShaderDescription : public UnlitShaderDescription
{
    GLint model_view;
    GLint projection;
    GLint number_of_lights;
    GLint light_position;
    GLint light_color;
    GLint light_inner_radius;
    GLint light_outer_radius;
    GLint light_ambient;

    LitShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment);
};

struct UnlitTintedShaderDescription : public UnlitShaderDescription
{
    GLint current_tick;
    GLint tint_mult;

    UnlitTintedShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment);
};

} // namespace render
