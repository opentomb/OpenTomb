#include "gl_util.h"

#include "engine/system.h"

#include <cstdio>
#include <cstring>
#include <vector>

#include <boost/log/trivial.hpp>

namespace render
{

/*
 * Shaders generation section
 */
bool checkOpenGLErrorDetailed(const char *file, int line)
{
    while(true)
    {
        GLenum  glErr = glGetError();
        if(glErr == GL_NO_ERROR)
        {
            return false;
        }

        switch(glErr)
        {
            case GL_INVALID_VALUE:
                BOOST_LOG_TRIVIAL(error) << "glError: GL_INVALID_VALUE in " << file << ":" << line;
                return true;

            case GL_INVALID_ENUM:
                BOOST_LOG_TRIVIAL(error) << "glError: GL_INVALID_ENUM in " << file << ":" << line;
                return true;

            case GL_INVALID_OPERATION:
                BOOST_LOG_TRIVIAL(error) << "glError: GL_INVALID_OPERATION in " << file << ":" << line;
                return true;

            case GL_STACK_OVERFLOW:
                BOOST_LOG_TRIVIAL(error) << "glError: GL_STACK_OVERFLOW in " << file << ":" << line;
                return true;

            case GL_STACK_UNDERFLOW:
                BOOST_LOG_TRIVIAL(error) << "glError: GL_STACK_UNDERFLOW in " << file << ":" << line;
                return true;

            case GL_OUT_OF_MEMORY:
                BOOST_LOG_TRIVIAL(error) << "glError: GL_OUT_OF_MEMORY in " << file << ":" << line;
                return true;

                /* GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB
                   GL_LOSE_CONTEXT_ON_RESET_ARB
                   GL_GUILTY_CONTEXT_RESET_ARB
                   GL_INNOCENT_CONTEXT_RESET_ARB
                   GL_UNKNOWN_CONTEXT_RESET_ARB
                   GL_RESET_NOTIFICATION_STRATEGY_ARB
                   GL_NO_RESET_NOTIFICATION_ARB*/

            default:
                BOOST_LOG_TRIVIAL(error) << "glError: unknown error 0x" << std::hex << glErr << std::dec << " in " << file << ":" << line;
                return true;
        };
    }
}

void printShaderInfoLog(GLuint object)
{
    const auto isProgram = glIsProgram(object);
    const auto isShader = glIsShader(object);

    if(!(isProgram^isShader))
    {
        BOOST_LOG_TRIVIAL(error) << "Object " << object << " is neither a shader nor a program";
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
        BOOST_LOG_TRIVIAL(info) << "GL_InfoLog[" << charsWritten << "]:" << static_cast<const char*>(infoLog.data());
    }
}

int loadShaderFromBuff(GLuint ShaderObj, char * source)
{
    GLint compileStatus = 0;
    GLint size = strlen(source);
    glShaderSource(ShaderObj, 1, const_cast<const char **>(&source), &size);
    BOOST_LOG_TRIVIAL(debug) << "source loaded";                   // compile the particle vertex shader, and print out
    glCompileShader(ShaderObj);
    BOOST_LOG_TRIVIAL(debug) << "trying to compile";
    if(CHECK_OPENGL_ERROR())                          // check for OpenGL errors
    {
        BOOST_LOG_TRIVIAL(error) << "compilation failed";
        return 0;
    }
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &compileStatus);
    printShaderInfoLog(ShaderObj);

    if(compileStatus != GL_TRUE)
        BOOST_LOG_TRIVIAL(error) << "compilation failed";

    return compileStatus == GL_TRUE;
}

int loadShaderFromFile(GLuint ShaderObj, const char * fileName, const char *additionalDefines)
{
    GLint   compileStatus;
    int size;
    FILE * file;
    BOOST_LOG_TRIVIAL(debug) << "GL_Loading " << fileName;
    file = fopen(fileName, "rb");
    if(file == nullptr)
    {
        BOOST_LOG_TRIVIAL(error) << "Error opening " << fileName;
        return 0;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);

    if(size < 1)
    {
        fclose(file);
        BOOST_LOG_TRIVIAL(error) << "Error loading file " << fileName << ": size < 1";
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
    BOOST_LOG_TRIVIAL(debug) << "source loaded";
    buf.clear();                                   // compile the particle vertex shader, and print out
    glCompileShader(ShaderObj);
    BOOST_LOG_TRIVIAL(debug) << "trying to compile";
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &compileStatus);
    printShaderInfoLog(ShaderObj);

    if(compileStatus != GL_TRUE)
        BOOST_LOG_TRIVIAL(error) << "compilation failed";
    else
        BOOST_LOG_TRIVIAL(debug) << "compilation succeeded";

    return compileStatus == GL_TRUE;
}

} // namespace render
