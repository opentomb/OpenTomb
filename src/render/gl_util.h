#pragma once

#include <GL/glew.h>

#define CHECK_OPENGL_ERROR() checkOpenGLErrorDetailed(__FILE__, __LINE__)

namespace render
{

int  checkOpenGLErrorDetailed(const char *file, int line);
void printShaderInfoLog(GLuint object);
int  loadShaderFromBuff(GLuint ShaderObj, char * source);
int  loadShaderFromFile(GLuint ShaderObj, const char * fileName, const char *additionalDefines);

} // namespace render
