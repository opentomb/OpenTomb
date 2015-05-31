//
//  shader_description.cpp
//  OpenTomb
//
//  Created by Torsten Kammer on 17.05.15.
//  Copyright (c) 2015 Torsten Kammer. All rights reserved.
//

#include "shader_description.h"

shader_description::shader_description(const char *vertexFilename, const char *fragmentFilename, const char *additionalDefines)
{
    GLhandleARB vertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    loadShaderFromFile(vertexShader, vertexFilename, additionalDefines);
    
    GLhandleARB fragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    loadShaderFromFile(fragmentShader, fragmentFilename, additionalDefines);
    
    load(vertexShader, fragmentShader);
    
    glDeleteObjectARB(vertexShader);
    glDeleteObjectARB(fragmentShader);
}

shader_description::shader_description(const char *filename, GLhandleARB fragmentShader, const char *additionalDefines)
{
    GLhandleARB vertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    loadShaderFromFile(vertexShader, filename, additionalDefines);
    
    load(vertexShader, fragmentShader);
    
    glDeleteObjectARB(vertexShader);
}

shader_description::shader_description(GLhandleARB vertexShader, const char *fragmentFilename, const char *additionalDefines)
{
    GLhandleARB fragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    loadShaderFromFile(fragmentShader, fragmentFilename, additionalDefines);
    
    load(vertexShader, fragmentShader);
    
    glDeleteObjectARB(fragmentShader);
}

void shader_description::load(GLhandleARB vertexShader, GLhandleARB fragmentShader)
{
    program = glCreateProgramObjectARB();
    glAttachObjectARB(program, vertexShader);
    glAttachObjectARB(program, fragmentShader);
    glLinkProgramARB(program);
    printInfoLog(program);
    
    sampler = glGetUniformLocationARB(program, "color_map");
}

shader_description::~shader_description()
{
    glDeleteObjectARB(program);
}

gui_shader_description::gui_shader_description(GLhandleARB vertexShader, const char *fragmentFilename)
: shader_description(vertexShader, fragmentFilename, 0)
{
    offset = glGetUniformLocationARB(program, "offset");
    factor = glGetUniformLocationARB(program, "factor");
}

text_shader_description::text_shader_description(const char *vertexFilename, const char *fragmentFilename)
: shader_description(vertexFilename, fragmentFilename, 0)
{
    screenSize = glGetUniformLocationARB(program, "screenSize");
}

unlit_shader_description::unlit_shader_description(const char *vertexFilename, const char *fragmentFilename, const char *additionalDefines)
: shader_description(vertexFilename, fragmentFilename, additionalDefines)
{
    model_view_projection = glGetUniformLocationARB(program, "modelViewProjection");
}

unlit_shader_description::unlit_shader_description(const char *vertexFilename, GLhandleARB fragmentShader, const char *additionalDefines)
: shader_description(vertexFilename, fragmentShader, additionalDefines)
{
    model_view_projection = glGetUniformLocationARB(program, "modelViewProjection");
}

unlit_shader_description::unlit_shader_description(GLhandleARB vertexShader, const char *fragmentFilename, const char *additionalDefines)
: shader_description(vertexShader, fragmentFilename, additionalDefines)
{
    model_view_projection = glGetUniformLocationARB(program, "modelViewProjection");
}

lit_shader_description::lit_shader_description(GLhandleARB vertexShader, const char *fragmentFilename, const char *additionalDefines)
: unlit_shader_description(vertexShader, fragmentFilename, additionalDefines)
{
    model_view = glGetUniformLocationARB(program, "modelView");
    number_of_lights = glGetUniformLocationARB(program, "number_of_lights");
    light_position = glGetUniformLocationARB(program, "light_position");
    light_color = glGetUniformLocationARB(program, "light_color");
    light_falloff = glGetUniformLocationARB(program, "light_falloff");
    light_ambient = glGetUniformLocationARB(program, "light_ambient");
}

unlit_tinted_shader_description::unlit_tinted_shader_description(const char *vertexFilename, const char *fragmentFilename, const char *additionalDefines)
: unlit_shader_description(vertexFilename, fragmentFilename, additionalDefines)
{
    current_tick = glGetUniformLocationARB(program, "fCurrentTick");
    tint_mult = glGetUniformLocationARB(program, "tintMult");
}

unlit_tinted_shader_description::unlit_tinted_shader_description(const char *vertexFilename, GLhandleARB fragmentShader, const char *additionalDefines)
: unlit_shader_description(vertexFilename, fragmentShader, additionalDefines)
{
    current_tick = glGetUniformLocationARB(program, "fCurrentTick");
    tint_mult = glGetUniformLocationARB(program, "tintMult");
}
