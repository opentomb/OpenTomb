#include "gl_util.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "engine/system.h"

#define SAFE_GET_PROC(func, type, name) func = (type)SDL_GL_GetProcAddress(name)

namespace render
{

/*
 * Shaders generation section
 */
int checkOpenGLErrorDetailed(const char *file, int line)
{
    for(; ; )
    {
        GLenum  glErr = glGetError();
        if(glErr == GL_NO_ERROR)
        {
            return 0;
        }

        switch(glErr)
        {
            case GL_INVALID_VALUE:
                engine::Sys_DebugLog(GL_LOG_FILENAME, "glError: GL_INVALID_VALUE in %s:%d", file, line);
                return 1;

            case GL_INVALID_ENUM:
                engine::Sys_DebugLog(GL_LOG_FILENAME, "glError: GL_INVALID_ENUM in %s:%d", file, line);
                return 1;

            case GL_INVALID_OPERATION:
                engine::Sys_DebugLog(GL_LOG_FILENAME, "glError: GL_INVALID_OPERATION in %s:%d", file, line);
                return 1;

            case GL_STACK_OVERFLOW:
                engine::Sys_DebugLog(GL_LOG_FILENAME, "glError: GL_STACK_OVERFLOW in %s:%d", file, line);
                return 1;

            case GL_STACK_UNDERFLOW:
                engine::Sys_DebugLog(GL_LOG_FILENAME, "glError: GL_STACK_UNDERFLOW in %s:%d", file, line);
                return 1;

            case GL_OUT_OF_MEMORY:
                engine::Sys_DebugLog(GL_LOG_FILENAME, "glError: GL_OUT_OF_MEMORY in %s:%d", file, line);
                return 1;

                /* GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB
                   GL_LOSE_CONTEXT_ON_RESET_ARB
                   GL_GUILTY_CONTEXT_RESET_ARB
                   GL_INNOCENT_CONTEXT_RESET_ARB
                   GL_UNKNOWN_CONTEXT_RESET_ARB
                   GL_RESET_NOTIFICATION_STRATEGY_ARB
                   GL_NO_RESET_NOTIFICATION_ARB*/

            default:
                engine::Sys_DebugLog(GL_LOG_FILENAME, "glError: uncnown error = 0x%X in %s:%d", file, line, glErr);
                return 1;
        };
    }
}

void printShaderInfoLog(GLuint object)
{
    const auto isProgram = glIsProgram(object);
    const auto isShader = glIsShader(object);

    if(!(isProgram^isShader))
    {
        engine::Sys_DebugLog(GL_LOG_FILENAME, "Object %d is neither a shader nor a program", object);
        return;
    }

    GLint       logLength = 0;
    GLint       charsWritten = 0;

    CHECK_OPENGL_ERROR();                         // check for OpenGL errors
    if(isProgram)
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &logLength);
    else
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &logLength);

    if(logLength > 0)
    {
        std::vector<GLchar> infoLog(logLength);
        
        if(isProgram)
            glGetProgramInfoLog(object, logLength, &charsWritten, infoLog.data());
        else
            glGetShaderInfoLog(object, logLength, &charsWritten, infoLog.data());
        engine::Sys_DebugLog(GL_LOG_FILENAME, "GL_InfoLog[%d]:", charsWritten);
        engine::Sys_DebugLog(GL_LOG_FILENAME, "%s", static_cast<const char*>(infoLog.data()));
    }
}

int loadShaderFromBuff(GLuint ShaderObj, char * source)
{
    int size;
    GLint compileStatus = 0;
    size = strlen(source);
    glShaderSource(ShaderObj, 1, const_cast<const char **>(&source), &size);
    engine::Sys_DebugLog(GL_LOG_FILENAME, "source loaded");                   // compile the particle vertex shader, and print out
    glCompileShader(ShaderObj);
    engine::Sys_DebugLog(GL_LOG_FILENAME, "trying to compile");
    if(CHECK_OPENGL_ERROR())                          // check for OpenGL errors
    {
        engine::Sys_DebugLog(GL_LOG_FILENAME, "compilation failed");
        return 0;
    }
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &compileStatus);
    printShaderInfoLog(ShaderObj);

    if(!compileStatus)
        engine::Sys_DebugLog(GL_LOG_FILENAME, "compilation failed");

    return compileStatus != 0;
}

int loadShaderFromFile(GLuint ShaderObj, const char * fileName, const char *additionalDefines)
{
    GLint   compileStatus;
    int size;
    FILE * file;
    engine::Sys_DebugLog(GL_LOG_FILENAME, "GL_Loading %s", fileName);
    file = fopen(fileName, "rb");
    if(file == nullptr)
    {
        engine::Sys_DebugLog(GL_LOG_FILENAME, "Error opening %s", fileName);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);

    if(size < 1)
    {
        fclose(file);
        engine::Sys_DebugLog(GL_LOG_FILENAME, "Error loading file %s: size < 1", fileName);
        return 0;
    }

    std::vector<char> buf(size);
    fseek(file, 0, SEEK_SET);
    fread(buf.data(), 1, size, file);
    fclose(file);

    //printf ( "source = %s\n", buf );
    static const char* version = "#version 150\n";
    static const GLint versionLen = strlen(version);
    if(additionalDefines)
    {
        const char *bufs[3] = { version, additionalDefines, buf.data() };
        const GLint lengths[3] = { versionLen, static_cast<GLint>(strlen(additionalDefines)), size };
        glShaderSource(ShaderObj, 3, bufs, lengths);
    }
    else
    {
        const char *bufs[2] = { version, buf.data() };
        const GLint lengths[2] = { versionLen, size };
        glShaderSource(ShaderObj, 2, bufs, lengths);
    }
    engine::Sys_DebugLog(GL_LOG_FILENAME, "source loaded");
    buf.clear();                                   // compile the particle vertex shader, and print out
    glCompileShader(ShaderObj);
    engine::Sys_DebugLog(GL_LOG_FILENAME, "trying to compile");
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &compileStatus);
    printShaderInfoLog(ShaderObj);

    if(compileStatus != GL_TRUE)
        engine::Sys_DebugLog(GL_LOG_FILENAME, "compilation failed");
    else
        engine::Sys_DebugLog(GL_LOG_FILENAME, "compilation succeeded");

    return compileStatus != 0;
}

} // namespace render
