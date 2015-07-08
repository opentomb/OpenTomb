#pragma once

/* -*- Mode: C++; tab-width: 3; indent-tabs-mode: t; c-basic-offset: 3 -*- */
/*================================================================
 * 
 * Project : OpenRaider
 * Author  : Terry 'Mongoose' Hendrix II
 * Website : http://www.westga.edu/~stu7440/
 * Email   : stu7440@westga.edu
 * Object  : gl_util
 * License : No use w/o permission (C) 2001 Mongoose
 * Comments: 
 *
 *
 *           This file was generated using Mongoose's C++ 
 *           template generator script.  <stu7440@westga.edu>
 * 
 *-- History ------------------------------------------------ 
 *
 * 2001.12.31:
 * Mongoose - Created
 * TeslaRus - modyfied
 ================================================================*/

#include <GL/glew.h>

int checkOpenGLError();
void printShaderInfoLog (GLuint object);
int loadShaderFromBuff(GLuint ShaderObj, char * source);
int loadShaderFromFile(GLuint ShaderObj, const char * fileName, const char *additionalDefines);
