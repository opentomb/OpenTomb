//
//  shader_description.cpp
//  OpenTomb
//
//  Created by Torsten Kammer on 17.05.15.
//  Copyright (c) 2015 Torsten Kammer. All rights reserved.
//

#include "shader_description.h"

#include <stdlib.h>

shader_stage::shader_stage(GLenum type, const char *filename, const char *additionalDefines)
{
    shader = glCreateShaderObjectARB(type);
    if (!loadShaderFromFile(shader, filename, additionalDefines))
        abort();
}

shader_stage::~shader_stage()
{
    glDeleteObjectARB(shader);
}

shader_description::shader_description(const shader_stage &vertex, const shader_stage &fragment)
{
    program = glCreateProgramObjectARB();
    glAttachObjectARB(program, vertex.shader);
    glAttachObjectARB(program, fragment.shader);
    glLinkProgramARB(program);
    printInfoLog(program);
    
    sampler = glGetUniformLocationARB(program, "color_map");
}

shader_description::~shader_description()
{
    glDeleteObjectARB(program);
}

gui_shader_description::gui_shader_description(const shader_stage &vertex, const shader_stage &fragment)
: shader_description(vertex, fragment)
{
    offset = glGetUniformLocationARB(program, "offset");
    factor = glGetUniformLocationARB(program, "factor");
    glBindAttribLocationARB(program, vertex_attribs::position, "position");
    glBindAttribLocationARB(program, vertex_attribs::color, "color");
    glLinkProgramARB(program);
    printInfoLog(program);
}

text_shader_description::text_shader_description(const shader_stage &vertex, const shader_stage &fragment)
: shader_description(vertex, fragment)
{
    glBindAttribLocationARB(program, vertex_attribs::position, "position");
    glBindAttribLocationARB(program, vertex_attribs::color, "color");
    glBindAttribLocationARB(program, vertex_attribs::tex_coord, "texCoord");
    glLinkProgramARB(program);
    
    printInfoLog(program);

    screenSize = glGetUniformLocationARB(program, "screenSize");
}

sprite_shader_description::sprite_shader_description(const shader_stage &vertex, const shader_stage &fragment)
: shader_description(vertex, fragment)
{
    model_view = glGetUniformLocationARB(program, "modelView");
    projection = glGetUniformLocationARB(program, "projection");
    glBindAttribLocationARB(program, vertex_attribs::position, "position");
    glBindAttribLocationARB(program, vertex_attribs::corner_offset, "cornerOffset");
    glBindAttribLocationARB(program, vertex_attribs::tex_coord, "texCoord");
    glLinkProgramARB(program);
    printInfoLog(program);
}

unlit_shader_description::unlit_shader_description(const shader_stage &vertex, const shader_stage &fragment)
: shader_description(vertex, fragment)
{
    glBindAttribLocationARB(program, vertex_attribs::position, "position");
    glBindAttribLocationARB(program, vertex_attribs::color, "color");
    glBindAttribLocationARB(program, vertex_attribs::tex_coord, "texCoord");
    glBindAttribLocationARB(program, vertex_attribs::normal, "normal");
    glBindAttribLocationARB(program, vertex_attribs::matrix_index, "matrixIndex");
    glLinkProgramARB(program);
    
    printInfoLog(program);
    
    model_view_projection = glGetUniformLocationARB(program, "modelViewProjection");
}

lit_shader_description::lit_shader_description(const shader_stage &vertex, const shader_stage &fragment)
: unlit_shader_description(vertex, fragment)
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

unlit_tinted_shader_description::unlit_tinted_shader_description(const shader_stage &vertex, const shader_stage &fragment)
: unlit_shader_description(vertex, fragment)
{
    current_tick = glGetUniformLocationARB(program, "fCurrentTick");
    tint_mult = glGetUniformLocationARB(program, "tintMult");
}
