#pragma once

#include <GL/glew.h>

#define CHECK_OPENGL_ERROR() checkOpenGLErrorDetailed(__FILE__, __LINE__)

namespace render
{
bool checkOpenGLErrorDetailed(const char *file, int line);
void printShaderInfoLog(GLuint object);
bool loadShaderFromBuff(GLuint ShaderObj, char * source);
bool loadShaderFromFile(GLuint ShaderObj, const char * fileName, const char *additionalDefines);
} // namespace render
