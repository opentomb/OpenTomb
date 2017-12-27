//
//  shader_description.cpp
//  OpenTomb
//
//  Created by Torsten Kammer on 17.05.15.
//  Copyright (c) 2015 Torsten Kammer. All rights reserved.
//

#include "shader_description.h"
#include "../engine.h"

#include <stdlib.h>

shader_stage::shader_stage(GLenum type, const char *filename, const char *additionalDefines)
{
    char shader_path[1024];
    size_t shader_path_base_len = sizeof(shader_path) - 1;
    strncpy(shader_path, Engine_GetBasePath(), shader_path_base_len);
    shader_path[shader_path_base_len] = 0;
    strncat(shader_path, filename, shader_path_base_len - strlen(shader_path));
    shader = qglCreateShaderObjectARB(type);
    if (!loadShaderFromFile(shader, shader_path, additionalDefines))
        abort();
}

shader_stage::~shader_stage()
{
    qglDeleteObjectARB(shader);
}

shader_description::shader_description(const shader_stage &vertex, const shader_stage &fragment)
{
    program = qglCreateProgramObjectARB();
    qglAttachObjectARB(program, vertex.shader);
    qglAttachObjectARB(program, fragment.shader);
    qglLinkProgramARB(program);
    //printInfoLog(program);

    sampler = qglGetUniformLocationARB(program, "color_map");
}

shader_description::~shader_description()
{
    qglDeleteObjectARB(program);
}

text_shader_description::text_shader_description(const shader_stage &vertex, const shader_stage &fragment)
: shader_description(vertex, fragment)
{
    screenSize = qglGetUniformLocationARB(program, "screenSize");
    colorReplace = qglGetUniformLocationARB(program, "colorReplace");
}

unlit_shader_description::unlit_shader_description(const shader_stage &vertex, const shader_stage &fragment)
: shader_description(vertex, fragment)
{
    model_view_projection = qglGetUniformLocationARB(program, "modelViewProjection");
    dist_fog = qglGetUniformLocationARB(program, "distFog");
}

lit_shader_description::lit_shader_description(const shader_stage &vertex, const shader_stage &fragment)
: unlit_shader_description(vertex, fragment)
{
    model_view = qglGetUniformLocationARB(program, "modelView");
    number_of_lights = qglGetUniformLocationARB(program, "number_of_lights");
    light_position = qglGetUniformLocationARB(program, "light_position");
    light_color = qglGetUniformLocationARB(program, "light_color");
    light_inner_radius = qglGetUniformLocationARB(program, "light_innerRadius");
    light_outer_radius = qglGetUniformLocationARB(program, "light_outerRadius");
    light_ambient = qglGetUniformLocationARB(program, "light_ambient");
}

unlit_tinted_shader_description::unlit_tinted_shader_description(const shader_stage &vertex, const shader_stage &fragment)
: unlit_shader_description(vertex, fragment)
{
    current_tick = qglGetUniformLocationARB(program, "fCurrentTick");
    tint_mult = qglGetUniformLocationARB(program, "tintMult");
}
