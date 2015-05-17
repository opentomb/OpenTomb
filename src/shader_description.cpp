//
//  shader_description.cpp
//  OpenTomb
//
//  Created by Torsten Kammer on 17.05.15.
//  Copyright (c) 2015 Torsten Kammer. All rights reserved.
//

#include "shader_description.h"

shader_description::shader_description(const char *vertexFilename, const char *fragmentFilename)
{
    GLhandleARB vertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    loadShaderFromFile(vertexShader, vertexFilename);
    
    GLhandleARB fragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    loadShaderFromFile(fragmentShader, fragmentFilename);
    
    load(vertexShader, fragmentShader);
    
    glDeleteObjectARB(vertexShader);
    glDeleteObjectARB(fragmentShader);
}

shader_description::shader_description(const char *filename, GLhandleARB fragmentShader)
{
    GLhandleARB vertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    loadShaderFromFile(vertexShader, filename);
    
    load(vertexShader, fragmentShader);
    
    glDeleteObjectARB(vertexShader);
}

void shader_description::load(GLhandleARB vertexShader, GLhandleARB fragmentShader)
{
    program = glCreateProgramObjectARB();
    glAttachObjectARB(program, vertexShader);
    glAttachObjectARB(program, fragmentShader);
    glLinkProgramARB(program);
    printInfoLog(program);
    
    glDeleteObjectARB(vertexShader);
    
    sampler = glGetUniformLocationARB(program, "color_map");
}

shader_description::~shader_description()
{
    glDeleteObjectARB(program);
}

unlit_shader_description::unlit_shader_description(const char *vertexFilename, const char *fragmentFilename)
: shader_description(vertexFilename, fragmentFilename)
{
    model_view_projection = glGetUniformLocationARB(program, "modelViewProjection");
}

unlit_shader_description::unlit_shader_description(const char *vertexFilename, GLhandleARB fragmentShader)
: shader_description(vertexFilename, fragmentShader)
{
    model_view_projection = glGetUniformLocationARB(program, "modelViewProjection");
}

lit_shader_description::lit_shader_description(const char *vertexFilename, const char *fragmentFilename)
: unlit_shader_description(vertexFilename, fragmentFilename)
{
    model_view = glGetUniformLocationARB(program, "modelView");
    number_of_lights = glGetUniformLocationARB(program, "number_of_lights");
    light_position = glGetUniformLocationARB(program, "light_position");
    light_color = glGetUniformLocationARB(program, "light_color");
    light_falloff = glGetUniformLocationARB(program, "light_falloff");
    light_ambient = glGetUniformLocationARB(program, "light_ambient");
}

unlit_tinted_shader_description::unlit_tinted_shader_description(const char *vertexFilename, const char *fragmentFilename)
: unlit_shader_description(vertexFilename, fragmentFilename)
{
    current_tick = glGetUniformLocationARB(program, "fCurrentTick");
    tint_mult = glGetUniformLocationARB(program, "tintMult");
}

unlit_tinted_shader_description::unlit_tinted_shader_description(const char *vertexFilename, GLhandleARB fragmentShader)
: unlit_shader_description(vertexFilename, fragmentShader)
{
    current_tick = glGetUniformLocationARB(program, "fCurrentTick");
    tint_mult = glGetUniformLocationARB(program, "tintMult");
}
