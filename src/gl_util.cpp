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
 *-- History -------------------------------------------------
 *
 * 2001.12.31:
 * Mongoose - Created
 * TeslaRus - modyfied
 =================================================================*/

#include "gl_util.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "system.h"
#include "console.h"

#define GL_LOG_FILENAME "gl_log.txt"
#define SAFE_GET_PROC(func, type, name) func = (type)SDL_GL_GetProcAddress(name)

/*
 * Shaders generation section
 */
int checkOpenGLError()
{
    for( ; ; )
    {
        GLenum  glErr = glGetError();
        if(glErr == GL_NO_ERROR)
        {
            return 0;
        }

        switch(glErr)
        {
            case GL_INVALID_VALUE:
                Sys_DebugLog(GL_LOG_FILENAME, "glError: GL_INVALID_VALUE");
                break;

            case GL_INVALID_ENUM:
                Sys_DebugLog(GL_LOG_FILENAME, "glError: GL_INVALID_ENUM");
                break;

            case GL_INVALID_OPERATION:
                Sys_DebugLog(GL_LOG_FILENAME, "glError: GL_INVALID_OPERATION");
                break;

            case GL_STACK_OVERFLOW:
                Sys_DebugLog(GL_LOG_FILENAME, "glError: GL_STACK_OVERFLOW");
                break;

            case GL_STACK_UNDERFLOW:
                Sys_DebugLog(GL_LOG_FILENAME, "glError: GL_STACK_UNDERFLOW");
                break;

            case GL_OUT_OF_MEMORY:
                Sys_DebugLog(GL_LOG_FILENAME, "glError: GL_OUT_OF_MEMORY");
                break;

               /* GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB
                  GL_LOSE_CONTEXT_ON_RESET_ARB
                  GL_GUILTY_CONTEXT_RESET_ARB
                  GL_INNOCENT_CONTEXT_RESET_ARB
                  GL_UNKNOWN_CONTEXT_RESET_ARB
                  GL_RESET_NOTIFICATION_STRATEGY_ARB
                  GL_NO_RESET_NOTIFICATION_ARB*/

            default:
                Sys_DebugLog(GL_LOG_FILENAME, "glError: uncnown error = 0x%X", glErr);
                break;
        };
    }
    return 1;
}

void printShaderInfoLog (GLuint object)
{
    const auto isProgram = glIsProgram(object);
    const auto isShader = glIsShader(object);
    
    if(!(isProgram^isShader)) {
        Sys_DebugLog(GL_LOG_FILENAME, "Object %d is neither a shader nor a program", object);
        return;
    }
    
    GLint       logLength     = 0;
    GLint       charsWritten  = 0;
    GLchar * infoLog;

    checkOpenGLError();                         // check for OpenGL errors
    if(isProgram)
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &logLength);
    else
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0)
    {
        infoLog = (GLchar*)malloc(logLength);
        if(isProgram)
            glGetProgramInfoLog(object, logLength, &charsWritten, infoLog);
        else
            glGetShaderInfoLog(object, logLength, &charsWritten, infoLog);
        Sys_DebugLog(GL_LOG_FILENAME, "GL_InfoLog[%d]:", charsWritten);
        Sys_DebugLog(GL_LOG_FILENAME, "%s", (const char*)infoLog);
        free(infoLog);
    }
}

int loadShaderFromBuff(GLuint ShaderObj, char * source)
{
    int size;
    GLint compileStatus = 0;
    size = strlen(source);
    glShaderSource(ShaderObj, 1, (const char **) &source, &size);
    Sys_DebugLog(GL_LOG_FILENAME, "source loaded");                   // compile the particle vertex shader, and print out
    glCompileShader(ShaderObj);
    Sys_DebugLog(GL_LOG_FILENAME, "trying to compile");
    if(checkOpenGLError())                          // check for OpenGL errors
    {
        Sys_DebugLog(GL_LOG_FILENAME, "compilation failed");
        return 0;
    }
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &compileStatus);
    printShaderInfoLog(ShaderObj);

    if(!compileStatus)
        Sys_DebugLog(GL_LOG_FILENAME, "compilation failed");

    return compileStatus != 0;
}

int loadShaderFromFile(GLuint ShaderObj, const char * fileName, const char *additionalDefines)
{
    GLint   compileStatus;
    int size;
    FILE * file;
    Sys_DebugLog(GL_LOG_FILENAME, "GL_Loading %s", fileName);
    file = fopen (fileName, "rb");
    if (file == NULL)
    {
        Sys_DebugLog(GL_LOG_FILENAME, "Error opening %s", fileName);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);

    if(size < 1)
    {
        fclose(file);
        Sys_DebugLog(GL_LOG_FILENAME, "Error loading file %s: size < 1", fileName);
        return 0;
    }

    char *buf = (char*)malloc(size);
    fseek(file, 0, SEEK_SET);
    fread(buf, 1, size, file);
    fclose(file);

    //printf ( "source = %s\n", buf );
    static const char* version = "#version 150\n";
    static const GLint versionLen = strlen(version);
    if (additionalDefines)
    {
        const char *bufs[3] = { version, additionalDefines, buf };
        const GLint lengths[3] = { versionLen, (GLint) strlen(additionalDefines), size };
        glShaderSource(ShaderObj, 3, bufs, lengths);
    }
    else
    {
        
        const char *bufs[2] = { version, buf };
        const GLint lengths[2] = { versionLen, size };
        glShaderSource(ShaderObj, 2, bufs, lengths);
    }
    Sys_DebugLog(GL_LOG_FILENAME, "source loaded");
    free(buf);                                   // compile the particle vertex shader, and print out
    glCompileShader(ShaderObj);
    Sys_DebugLog(GL_LOG_FILENAME, "trying to compile");
    if(checkOpenGLError())                       // check for OpenGL errors
    {
        Sys_DebugLog(GL_LOG_FILENAME, "compilation failed");
        return 0;
    }
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &compileStatus);
    printShaderInfoLog(ShaderObj);

    if(compileStatus != GL_TRUE)
        Sys_DebugLog(GL_LOG_FILENAME, "compilation failed");
    else
        Sys_DebugLog(GL_LOG_FILENAME, "compilation succeeded");

    return compileStatus != 0;
}
