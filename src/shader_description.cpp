//
//  shader_description.cpp
//  OpenTomb
//
//  Created by Torsten Kammer on 17.05.15.
//  Copyright (c) 2015 Torsten Kammer. All rights reserved.
//

#include <cstdlib>
#include <cassert>

#include "shader_description.h"

ShaderStage::ShaderStage(GLenum type, const char *filename, const char *additionalDefines)
{
    shader = glCreateShader(type);
    if (!loadShaderFromFile(shader, filename, additionalDefines))
        abort();
}

ShaderStage::~ShaderStage()
{
    glDeleteShader(shader);
}

ShaderDescription::ShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
{
    program = glCreateProgram();
    glAttachShader(program, vertex.shader);
    glAttachShader(program, fragment.shader);
    glLinkProgram(program);
    GLint isLinked;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    assert(isLinked == GL_TRUE);

    checkOpenGLError();
    printShaderInfoLog(program);

    sampler = glGetUniformLocation(program, "color_map");
}

ShaderDescription::~ShaderDescription()
{
    glDeleteProgram(program);
}

GuiShaderDescription::GuiShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
: ShaderDescription(vertex, fragment)
{
    offset = glGetUniformLocation(program, "offset");
    factor = glGetUniformLocation(program, "factor");
    glBindAttribLocation(program, vertex_attribs::position, "position");
    glBindAttribLocation(program, vertex_attribs::color, "color");
    glLinkProgram(program);
    GLint isLinked;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    assert(isLinked == GL_TRUE);

    checkOpenGLError();
    printShaderInfoLog(program);
}

TextShaderDescription::TextShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
: ShaderDescription(vertex, fragment)
{
    glBindAttribLocation(program, vertex_attribs::position, "position");
    glBindAttribLocation(program, vertex_attribs::color, "color");
    glBindAttribLocation(program, vertex_attribs::tex_coord, "texCoord");
    glLinkProgram(program);
    GLint isLinked;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    assert(isLinked == GL_TRUE);

    checkOpenGLError();

    printShaderInfoLog(program);

    screenSize = glGetUniformLocation(program, "screenSize");
}

SpriteShaderDescription::SpriteShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
: ShaderDescription(vertex, fragment)
{
    model_view = glGetUniformLocation(program, "modelView");
    projection = glGetUniformLocation(program, "projection");
    glBindAttribLocation(program, vertex_attribs::position, "position");
    glBindAttribLocation(program, vertex_attribs::corner_offset, "cornerOffset");
    glBindAttribLocation(program, vertex_attribs::tex_coord, "texCoord");
    glLinkProgram(program);
    GLint isLinked;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    assert(isLinked == GL_TRUE);

    checkOpenGLError();
    printShaderInfoLog(program);
}

UnlitShaderDescription::UnlitShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
: ShaderDescription(vertex, fragment)
{
    glBindAttribLocation(program, VertexAttribs::Position, "position");
    glBindAttribLocation(program, VertexAttribs::Color, "color");
    glBindAttribLocation(program, VertexAttribs::TexCoord, "texCoord");
    glBindAttribLocation(program, VertexAttribs::Normal, "normal");
    glBindAttribLocation(program, VertexAttribs::MatrixIndex, "matrixIndex");
    glLinkProgram(program);
    GLint isLinked;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    assert(isLinked == GL_TRUE);

    checkOpenGLError();

    printShaderInfoLog(program);

    model_view_projection = glGetUniformLocation(program, "modelViewProjection");
}

LitShaderDescription::LitShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
: UnlitShaderDescription(vertex, fragment)
{
    model_view = glGetUniformLocation(program, "modelView");
    projection = glGetUniformLocation(program, "projection");
    number_of_lights = glGetUniformLocation(program, "number_of_lights");
    light_position = glGetUniformLocation(program, "light_position");
    light_color = glGetUniformLocation(program, "light_color");
    light_inner_radius = glGetUniformLocation(program, "light_innerRadius");
    light_outer_radius = glGetUniformLocation(program, "light_outerRadius");
    light_ambient = glGetUniformLocation(program, "light_ambient");
}

UnlitTintedShaderDescription::UnlitTintedShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
: UnlitShaderDescription(vertex, fragment)
{
    current_tick = glGetUniformLocation(program, "fCurrentTick");
    tint_mult = glGetUniformLocation(program, "tintMult");
}
