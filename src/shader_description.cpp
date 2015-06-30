//
//  shader_description.cpp
//  OpenTomb
//
//  Created by Torsten Kammer on 17.05.15.
//  Copyright (c) 2015 Torsten Kammer. All rights reserved.
//

#include "shader_description.h"

#include <cstdlib>

ShaderStage::ShaderStage(GLenum type, const char *filename, const char *additionalDefines)
{
    shader = glCreateShaderObjectARB(type);
    if (!loadShaderFromFile(shader, filename, additionalDefines))
        abort();
}

ShaderStage::~ShaderStage()
{
    glDeleteObjectARB(shader);
}

ShaderDescription::ShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
{
    program = glCreateProgramObjectARB();
    glAttachObjectARB(program, vertex.shader);
    glAttachObjectARB(program, fragment.shader);
    glLinkProgramARB(program);
    printInfoLog(program);
    
    sampler = glGetUniformLocationARB(program, "color_map");
}

ShaderDescription::~ShaderDescription()
{
    glDeleteObjectARB(program);
}

GuiShaderDescription::GuiShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
: ShaderDescription(vertex, fragment)
{
    offset = glGetUniformLocationARB(program, "offset");
    factor = glGetUniformLocationARB(program, "factor");
    glBindAttribLocationARB(program, vertex_attribs::position, "position");
    glBindAttribLocationARB(program, vertex_attribs::color, "color");
    glLinkProgramARB(program);
    printInfoLog(program);
}

TextShaderDescription::TextShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
: ShaderDescription(vertex, fragment)
{
    glBindAttribLocationARB(program, vertex_attribs::position, "position");
    glBindAttribLocationARB(program, vertex_attribs::color, "color");
    glBindAttribLocationARB(program, vertex_attribs::tex_coord, "texCoord");
    glLinkProgramARB(program);
    
    printInfoLog(program);

    screenSize = glGetUniformLocationARB(program, "screenSize");
}

SpriteShaderDescription::SpriteShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
: ShaderDescription(vertex, fragment)
{
    model_view = glGetUniformLocationARB(program, "modelView");
    projection = glGetUniformLocationARB(program, "projection");
    glBindAttribLocationARB(program, vertex_attribs::position, "position");
    glBindAttribLocationARB(program, vertex_attribs::corner_offset, "cornerOffset");
    glBindAttribLocationARB(program, vertex_attribs::tex_coord, "texCoord");
    glLinkProgramARB(program);
    printInfoLog(program);
}

UnlitShaderDescription::UnlitShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
: ShaderDescription(vertex, fragment)
{
    glBindAttribLocationARB(program, VertexAttribs::Position, "position");
    glBindAttribLocationARB(program, VertexAttribs::Color, "color");
    glBindAttribLocationARB(program, VertexAttribs::TexCoord, "texCoord");
    glBindAttribLocationARB(program, VertexAttribs::Normal, "normal");
    glBindAttribLocationARB(program, VertexAttribs::MatrixIndex, "matrixIndex");
    glLinkProgramARB(program);
    
    printInfoLog(program);
    
    model_view_projection = glGetUniformLocationARB(program, "modelViewProjection");
}

LitShaderDescription::LitShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
: UnlitShaderDescription(vertex, fragment)
{
    model_view = glGetUniformLocationARB(program, "modelView");
    projection = glGetUniformLocationARB(program, "projection");
    number_of_lights = glGetUniformLocationARB(program, "number_of_lights");
    light_position = glGetUniformLocationARB(program, "light_position");
    light_color = glGetUniformLocationARB(program, "light_color");
    light_inner_radius = glGetUniformLocationARB(program, "light_innerRadius");
    light_outer_radius = glGetUniformLocationARB(program, "light_outerRadius");
    light_ambient = glGetUniformLocationARB(program, "light_ambient");
}

UnlitTintedShaderDescription::UnlitTintedShaderDescription(const ShaderStage &vertex, const ShaderStage &fragment)
: UnlitShaderDescription(vertex, fragment)
{
    current_tick = glGetUniformLocationARB(program, "fCurrentTick");
    tint_mult = glGetUniformLocationARB(program, "tintMult");
}
