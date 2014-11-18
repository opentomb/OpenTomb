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

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_opengl.h>
#include <string.h>
#include <stdlib.h>
#include "gl_util.h"

#ifndef GL_GLEXT_PROTOTYPES
PFNGLDELETEOBJECTARBPROC                glDeleteObjectARB =                     NULL;
PFNGLGETHANDLEARBPROC                   glGetHandleARB =                        NULL;
PFNGLDETACHOBJECTARBPROC                glDetachObjectARB =                     NULL;
PFNGLCREATESHADEROBJECTARBPROC          glCreateShaderObjectARB =               NULL;
PFNGLSHADERSOURCEARBPROC                glShaderSourceARB =                     NULL;
PFNGLCOMPILESHADERARBPROC               glCompileShaderARB =                    NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC         glCreateProgramObjectARB =              NULL;
PFNGLATTACHOBJECTARBPROC                glAttachObjectARB =                     NULL;
PFNGLLINKPROGRAMARBPROC                 glLinkProgramARB =                      NULL;
PFNGLUSEPROGRAMOBJECTARBPROC            glUseProgramObjectARB =                 NULL;
PFNGLVALIDATEPROGRAMARBPROC             glValidateProgramARB =                  NULL;
PFNGLUNIFORM1FARBPROC                   glUniform1fARB =                        NULL;
PFNGLUNIFORM2FARBPROC                   glUniform2fARB =                        NULL;
PFNGLUNIFORM3FARBPROC                   glUniform3fARB =                        NULL;
PFNGLUNIFORM4FARBPROC                   glUniform4fARB =                        NULL;
PFNGLUNIFORM1IARBPROC                   glUniform1iARB =                        NULL;
PFNGLUNIFORM2IARBPROC                   glUniform2iARB =                        NULL;
PFNGLUNIFORM3IARBPROC                   glUniform3iARB =                        NULL;
PFNGLUNIFORM4IARBPROC                   glUniform4iARB =                        NULL;
PFNGLUNIFORM1FVARBPROC                  glUniform1fvARB =                       NULL;
PFNGLUNIFORM2FVARBPROC                  glUniform2fvARB =                       NULL;
PFNGLUNIFORM3FVARBPROC                  glUniform3fvARB =                       NULL;
PFNGLUNIFORM4FVARBPROC                  glUniform4fvARB =                       NULL;
PFNGLUNIFORM1IVARBPROC                  glUniform1ivARB =                       NULL;
PFNGLUNIFORM2IVARBPROC                  glUniform2ivARB =                       NULL;
PFNGLUNIFORM3IVARBPROC                  glUniform3ivARB =                       NULL;
PFNGLUNIFORM4IVARBPROC                  glUniform4ivARB =                       NULL;
PFNGLUNIFORMMATRIX2FVARBPROC            glUniformMatrix2fvARB =                 NULL;
PFNGLUNIFORMMATRIX3FVARBPROC            glUniformMatrix3fvARB =                 NULL;
PFNGLUNIFORMMATRIX4FVARBPROC            glUniformMatrix4fvARB =                 NULL;
PFNGLGETOBJECTPARAMETERFVARBPROC        glGetObjectParameterfvARB =             NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC        glGetObjectParameterivARB =             NULL;
PFNGLGETINFOLOGARBPROC                  glGetInfoLogARB =                       NULL;
PFNGLGETATTACHEDOBJECTSARBPROC          glGetAttachedObjectsARB =               NULL;
PFNGLGETUNIFORMLOCATIONARBPROC          glGetUniformLocationARB =               NULL;
PFNGLGETACTIVEUNIFORMARBPROC            glGetActiveUniformARB =                 NULL;
PFNGLGETUNIFORMFVARBPROC                glGetUniformfvARB =                     NULL;
PFNGLGETUNIFORMIVARBPROC                glGetUniformivARB =                     NULL;
PFNGLGETSHADERSOURCEARBPROC             glGetShaderSourceARB =                  NULL;

PFNGLBINDATTRIBLOCATIONARBPROC          glBindAttribLocationARB =               NULL;
PFNGLGETACTIVEATTRIBARBPROC             glGetActiveAttribARB =                  NULL;
PFNGLGETATTRIBLOCATIONARBPROC           glGetAttribLocationARB =                NULL;

PFNGLACTIVETEXTUREARBPROC               glActiveTextureARB =                    NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC         glClientActiveTextureARB =              NULL;
PFNGLMULTITEXCOORD1DARBPROC             glMultiTexCoord1dARB =                  NULL;
PFNGLMULTITEXCOORD1DVARBPROC            glMultiTexCoord1dvARB =                 NULL;
PFNGLMULTITEXCOORD1FARBPROC             glMultiTexCoord1fARB =                  NULL;
PFNGLMULTITEXCOORD1FVARBPROC            glMultiTexCoord1fvARB =                 NULL;
PFNGLMULTITEXCOORD1IARBPROC             glMultiTexCoord1iARB =                  NULL;
PFNGLMULTITEXCOORD1IVARBPROC            glMultiTexCoord1ivARB =                 NULL;
PFNGLMULTITEXCOORD1SARBPROC             glMultiTexCoord1sARB =                  NULL;
PFNGLMULTITEXCOORD1SVARBPROC            glMultiTexCoord1svARB =                 NULL;
PFNGLMULTITEXCOORD2DARBPROC             glMultiTexCoord2dARB =                  NULL;
PFNGLMULTITEXCOORD2DVARBPROC            glMultiTexCoord2dvARB =                 NULL;
PFNGLMULTITEXCOORD2FARBPROC             glMultiTexCoord2fARB =                  NULL;
PFNGLMULTITEXCOORD2FVARBPROC            glMultiTexCoord2fvARB =                 NULL;
PFNGLMULTITEXCOORD2IARBPROC             glMultiTexCoord2iARB =                  NULL;
PFNGLMULTITEXCOORD2IVARBPROC            glMultiTexCoord2ivARB =                 NULL;
PFNGLMULTITEXCOORD2SARBPROC             glMultiTexCoord2sARB =                  NULL;
PFNGLMULTITEXCOORD2SVARBPROC            glMultiTexCoord2svARB =                 NULL;
PFNGLMULTITEXCOORD3DARBPROC             glMultiTexCoord3dARB =                  NULL;
PFNGLMULTITEXCOORD3DVARBPROC            glMultiTexCoord3dvARB =                 NULL;
PFNGLMULTITEXCOORD3FARBPROC             glMultiTexCoord3fARB =                  NULL;
PFNGLMULTITEXCOORD3FVARBPROC            glMultiTexCoord3fvARB =                 NULL;
PFNGLMULTITEXCOORD3IARBPROC             glMultiTexCoord3iARB =                  NULL;
PFNGLMULTITEXCOORD3IVARBPROC            glMultiTexCoord3ivARB =                 NULL;
PFNGLMULTITEXCOORD3SARBPROC             glMultiTexCoord3sARB =                  NULL;
PFNGLMULTITEXCOORD3SVARBPROC            glMultiTexCoord3svARB =                 NULL;
PFNGLMULTITEXCOORD4DARBPROC             glMultiTexCoord4dARB =                  NULL;
PFNGLMULTITEXCOORD4DVARBPROC            glMultiTexCoord4dvARB =                 NULL;
PFNGLMULTITEXCOORD4FARBPROC             glMultiTexCoord4fARB =                  NULL;
PFNGLMULTITEXCOORD4FVARBPROC            glMultiTexCoord4fvARB =                 NULL;
PFNGLMULTITEXCOORD4IARBPROC             glMultiTexCoord4iARB =                  NULL;
PFNGLMULTITEXCOORD4IVARBPROC            glMultiTexCoord4ivARB =                 NULL;
PFNGLMULTITEXCOORD4SARBPROC             glMultiTexCoord4sARB =                  NULL;
PFNGLMULTITEXCOORD4SVARBPROC            glMultiTexCoord4svARB =                 NULL;

PFNGLBINDBUFFERARBPROC                  glBindBufferARB =                       NULL;
PFNGLDELETEBUFFERSARBPROC               glDeleteBuffersARB =                    NULL;
PFNGLGENBUFFERSARBPROC                  glGenBuffersARB =                       NULL;
PFNGLISBUFFERARBPROC                    glIsBufferARB =                         NULL;
PFNGLBUFFERDATAARBPROC                  glBufferDataARB =                       NULL;
PFNGLBUFFERSUBDATAARBPROC               glBufferSubDataARB =                    NULL;
PFNGLGETBUFFERSUBDATAARBPROC            glGetBufferSubDataARB =                 NULL;
PFNGLMAPBUFFERARBPROC                   glMapBufferARB =                        NULL;
PFNGLUNMAPBUFFERARBPROC                 glUnmapBufferARB =                      NULL;
PFNGLGETBUFFERPARAMETERIVARBPROC        glGetBufferParameterivARB =             NULL;
PFNGLGETBUFFERPOINTERVARBPROC           glGetBufferPointervARB =                NULL;

PFNGLBINDVERTEXARRAYPROC                glBindVertexArray =                     NULL;
PFNGLDELETEVERTEXARRAYSPROC             glDeleteVertexArray =                   NULL;
PFNGLGENVERTEXARRAYSPROC                glGenVertexArray =                      NULL;
PFNGLISVERTEXARRAYPROC                  glIsVertexArray =                       NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC        glEnableVertexAttribArray =             NULL;

PFNGLBINDPROGRAMARBPROC                 glBindProgramARB =                      NULL;
PFNGLTEXIMAGE3DPROC                     glTexImage3D=                           NULL;
PFNGLBLENDEQUATIONSEPARATEPROC          glBlendEquationSeparate=                NULL;
PFNGLBLENDFUNCSEPARATEPROC              glBlendFuncSeparate=                    NULL;
PFNGLBLENDEQUATIONPROC                  glBlendEquation=                        NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC       glDisableVertexAttribArray=             NULL;
PFNGLGETVERTEXATTRIBIVPROC              glGetVertexAttribiv=                    NULL;
PFNGLGETVERTEXATTRIBFVPROC              glGetVertexAttribfv=                    NULL;
#endif

char *engine_gl_ext_str = NULL;

/**
 * Get addresses of GL functions and initialise engine_gl_ext_str string.
 */
void InitGLExtFuncs()
{
    const char* buf = (const char*)glGetString(GL_EXTENSIONS);                  ///@PARANOID: I do not know exactly, how much time returned string pointer is valid, so I made a copy;
    engine_gl_ext_str = (char*)malloc(strlen(buf) + 1);
    strcpy(engine_gl_ext_str, buf);
    
#ifndef GL_GLEXT_PROTOTYPES        
    /// VBO funcs
    if(IsGLExtensionSupported("GL_ARB_vertex_buffer_object"))
    {
        SAFE_GET_PROC(glBindBufferARB, PFNGLBINDBUFFERARBPROC, "glBindBufferARB");
        SAFE_GET_PROC(glDeleteBuffersARB, PFNGLDELETEBUFFERSARBPROC, "glDeleteBuffersARB");
        SAFE_GET_PROC(glGenBuffersARB, PFNGLGENBUFFERSARBPROC, "glGenBuffersARB");
        SAFE_GET_PROC(glIsBufferARB, PFNGLISBUFFERARBPROC, "glIsBufferARB");
        SAFE_GET_PROC(glBufferDataARB, PFNGLBUFFERDATAARBPROC, "glBufferDataARB");
        SAFE_GET_PROC(glBufferSubDataARB, PFNGLBUFFERSUBDATAARBPROC, "glBufferSubDataARB");
        SAFE_GET_PROC(glGetBufferSubDataARB, PFNGLGETBUFFERSUBDATAARBPROC, "glGetBufferSubDataARB");
        SAFE_GET_PROC(glMapBufferARB, PFNGLMAPBUFFERARBPROC, "glMapBufferARB");
        SAFE_GET_PROC(glUnmapBufferARB, PFNGLUNMAPBUFFERARBPROC, "glUnmapBufferARB");    
        SAFE_GET_PROC(glGetBufferParameterivARB, PFNGLGETBUFFERPARAMETERIVARBPROC, "glGetBufferParameterivARB");
        SAFE_GET_PROC(glGetBufferPointervARB, PFNGLGETBUFFERPOINTERVARBPROC, "glGetBufferPointervARB");
        
        SAFE_GET_PROC(glActiveTextureARB, PFNGLACTIVETEXTUREARBPROC, "glActiveTextureARB");
        SAFE_GET_PROC(glClientActiveTextureARB, PFNGLCLIENTACTIVETEXTUREARBPROC, "glClientActiveTextureARB");

        SAFE_GET_PROC(glMultiTexCoord1dARB, PFNGLMULTITEXCOORD1DARBPROC, "glMultiTexCoord1dARB");
        SAFE_GET_PROC(glMultiTexCoord1dvARB, PFNGLMULTITEXCOORD1DVARBPROC, "glMultiTexCoord1dvARB");
        SAFE_GET_PROC(glMultiTexCoord1fARB, PFNGLMULTITEXCOORD1FARBPROC, "glMultiTexCoord1fARB");
        SAFE_GET_PROC(glMultiTexCoord1fvARB, PFNGLMULTITEXCOORD1FVARBPROC, "glMultiTexCoord1fvARB");
        SAFE_GET_PROC(glMultiTexCoord1iARB, PFNGLMULTITEXCOORD1IARBPROC, "glMultiTexCoord1iARB");
        SAFE_GET_PROC(glMultiTexCoord1ivARB, PFNGLMULTITEXCOORD1IVARBPROC, "glMultiTexCoord1ivARB");
        SAFE_GET_PROC(glMultiTexCoord1sARB, PFNGLMULTITEXCOORD1SARBPROC, "glMultiTexCoord1sARB");
        SAFE_GET_PROC(glMultiTexCoord1svARB, PFNGLMULTITEXCOORD1SVARBPROC, "glMultiTexCoord1svARB");

        SAFE_GET_PROC(glMultiTexCoord2dARB, PFNGLMULTITEXCOORD2DARBPROC, "glMultiTexCoord2dARB");
        SAFE_GET_PROC(glMultiTexCoord2dvARB, PFNGLMULTITEXCOORD2DVARBPROC, "glMultiTexCoord2dvARB");
        SAFE_GET_PROC(glMultiTexCoord2fARB, PFNGLMULTITEXCOORD2FARBPROC, "glMultiTexCoord2fARB");
        SAFE_GET_PROC(glMultiTexCoord2fvARB, PFNGLMULTITEXCOORD2FVARBPROC, "glMultiTexCoord2fvARB");
        SAFE_GET_PROC(glMultiTexCoord2iARB, PFNGLMULTITEXCOORD2IARBPROC, "glMultiTexCoord2iARB");
        SAFE_GET_PROC(glMultiTexCoord2ivARB, PFNGLMULTITEXCOORD2IVARBPROC, "glMultiTexCoord2ivARB");
        SAFE_GET_PROC(glMultiTexCoord2sARB, PFNGLMULTITEXCOORD2SARBPROC, "glMultiTexCoord2sARB");
        SAFE_GET_PROC(glMultiTexCoord2svARB, PFNGLMULTITEXCOORD2SVARBPROC, "glMultiTexCoord2svARB");

        SAFE_GET_PROC(glMultiTexCoord3dARB, PFNGLMULTITEXCOORD3DARBPROC, "glMultiTexCoord3dARB");
        SAFE_GET_PROC(glMultiTexCoord3dvARB, PFNGLMULTITEXCOORD3DVARBPROC, "glMultiTexCoord3dvARB");
        SAFE_GET_PROC(glMultiTexCoord3fARB, PFNGLMULTITEXCOORD3FARBPROC, "glMultiTexCoord3fARB");
        SAFE_GET_PROC(glMultiTexCoord3fvARB, PFNGLMULTITEXCOORD3FVARBPROC, "glMultiTexCoord3fvARB");
        SAFE_GET_PROC(glMultiTexCoord3iARB, PFNGLMULTITEXCOORD3IARBPROC, "glMultiTexCoord3iARB");
        SAFE_GET_PROC(glMultiTexCoord3ivARB, PFNGLMULTITEXCOORD3IVARBPROC, "glMultiTexCoord3ivARB");
        SAFE_GET_PROC(glMultiTexCoord3sARB, PFNGLMULTITEXCOORD3SARBPROC, "glMultiTexCoord3sARB");
        SAFE_GET_PROC(glMultiTexCoord3svARB, PFNGLMULTITEXCOORD3SVARBPROC, "glMultiTexCoord3svARB");

        SAFE_GET_PROC(glMultiTexCoord4dARB, PFNGLMULTITEXCOORD4DARBPROC, "glMultiTexCoord4dARB");
        SAFE_GET_PROC(glMultiTexCoord4dvARB, PFNGLMULTITEXCOORD4DVARBPROC, "glMultiTexCoord4dvARB");
        SAFE_GET_PROC(glMultiTexCoord4fARB, PFNGLMULTITEXCOORD4FARBPROC, "glMultiTexCoord4fARB");
        SAFE_GET_PROC(glMultiTexCoord4fvARB, PFNGLMULTITEXCOORD4FVARBPROC, "glMultiTexCoord4fvARB");
        SAFE_GET_PROC(glMultiTexCoord4iARB, PFNGLMULTITEXCOORD4IARBPROC, "glMultiTexCoord4iARB");
        SAFE_GET_PROC(glMultiTexCoord4ivARB, PFNGLMULTITEXCOORD4IVARBPROC, "glMultiTexCoord4ivARB");
        SAFE_GET_PROC(glMultiTexCoord4sARB, PFNGLMULTITEXCOORD4SARBPROC, "glMultiTexCoord4sARB");
        SAFE_GET_PROC(glMultiTexCoord4svARB, PFNGLMULTITEXCOORD4SVARBPROC, "glMultiTexCoord4svARB");
    }
    if(IsGLExtensionSupported("GL_ARB_shading_language_100"))
    {
        SAFE_GET_PROC(glDeleteObjectARB, PFNGLDELETEOBJECTARBPROC, "glDeleteObjectARB");
        SAFE_GET_PROC(glGetHandleARB, PFNGLGETHANDLEARBPROC, "glGetHandleARB");
        SAFE_GET_PROC(glDetachObjectARB, PFNGLDETACHOBJECTARBPROC, "glDetachObjectARB");
        SAFE_GET_PROC(glCreateShaderObjectARB, PFNGLCREATESHADEROBJECTARBPROC, "glCreateShaderObjectARB");
        SAFE_GET_PROC(glShaderSourceARB, PFNGLSHADERSOURCEARBPROC, "glShaderSourceARB");
        SAFE_GET_PROC(glCompileShaderARB, PFNGLCOMPILESHADERARBPROC, "glCompileShaderARB");
        SAFE_GET_PROC(glCreateProgramObjectARB, PFNGLCREATEPROGRAMOBJECTARBPROC, "glCreateProgramObjectARB");
        SAFE_GET_PROC(glAttachObjectARB, PFNGLATTACHOBJECTARBPROC, "glAttachObjectARB");
        SAFE_GET_PROC(glLinkProgramARB, PFNGLLINKPROGRAMARBPROC, "glLinkProgramARB");
        SAFE_GET_PROC(glUseProgramObjectARB, PFNGLUSEPROGRAMOBJECTARBPROC, "glUseProgramObjectARB");
        SAFE_GET_PROC(glValidateProgramARB, PFNGLVALIDATEPROGRAMARBPROC, "glValidateProgramARB");
        SAFE_GET_PROC(glUniform1fARB, PFNGLUNIFORM1FARBPROC, "glUniform1fARB");
        SAFE_GET_PROC(glUniform2fARB, PFNGLUNIFORM2FARBPROC, "glUniform2fARB");
        SAFE_GET_PROC(glUniform3fARB, PFNGLUNIFORM3FARBPROC, "glUniform3fARB");
        SAFE_GET_PROC(glUniform4fARB, PFNGLUNIFORM4FARBPROC, "glUniform4fARB");
        SAFE_GET_PROC(glUniform1iARB, PFNGLUNIFORM1IARBPROC, "glUniform1iARB");
        SAFE_GET_PROC(glUniform2iARB, PFNGLUNIFORM2IARBPROC, "glUniform2iARB");
        SAFE_GET_PROC(glUniform3iARB, PFNGLUNIFORM3IARBPROC, "glUniform3iARB");
        SAFE_GET_PROC(glUniform4iARB, PFNGLUNIFORM4IARBPROC, "glUniform4iARB");
        SAFE_GET_PROC(glUniform1fvARB, PFNGLUNIFORM1FVARBPROC, "glUniform1fvARB");
        SAFE_GET_PROC(glUniform2fvARB, PFNGLUNIFORM2FVARBPROC, "glUniform2fvARB");
        SAFE_GET_PROC(glUniform3fvARB, PFNGLUNIFORM3FVARBPROC, "glUniform3fvARB");
        SAFE_GET_PROC(glUniform4fvARB, PFNGLUNIFORM4FVARBPROC, "glUniform4fvARB");
        SAFE_GET_PROC(glUniform1ivARB, PFNGLUNIFORM1IVARBPROC, "glUniform1ivARB");
        SAFE_GET_PROC(glUniform2ivARB, PFNGLUNIFORM2IVARBPROC, "glUniform2ivARB");
        SAFE_GET_PROC(glUniform3ivARB, PFNGLUNIFORM3IVARBPROC, "glUniform3ivARB");
        SAFE_GET_PROC(glUniform4ivARB, PFNGLUNIFORM4IVARBPROC, "glUniform4ivARB");
        SAFE_GET_PROC(glUniformMatrix2fvARB, PFNGLUNIFORMMATRIX2FVARBPROC, "glUniformMatrix2fvARB");
        SAFE_GET_PROC(glUniformMatrix3fvARB, PFNGLUNIFORMMATRIX3FVARBPROC, "glUniformMatrix3fvARB");
        SAFE_GET_PROC(glUniformMatrix4fvARB, PFNGLUNIFORMMATRIX4FVARBPROC, "glUniformMatrix4fvARB");
        SAFE_GET_PROC(glGetObjectParameterfvARB, PFNGLGETOBJECTPARAMETERFVARBPROC, "glGetObjectParameterfvARB");
        SAFE_GET_PROC(glGetObjectParameterivARB, PFNGLGETOBJECTPARAMETERIVARBPROC, "glGetObjectParameterivARB");
        SAFE_GET_PROC(glGetInfoLogARB, PFNGLGETINFOLOGARBPROC, "glGetInfoLogARB");
        SAFE_GET_PROC(glGetAttachedObjectsARB, PFNGLGETATTACHEDOBJECTSARBPROC, "glGetAttachedObjectsARB");
        SAFE_GET_PROC(glGetUniformLocationARB, PFNGLGETUNIFORMLOCATIONARBPROC, "glGetUniformLocationARB");
        SAFE_GET_PROC(glGetActiveUniformARB, PFNGLGETACTIVEUNIFORMARBPROC, "glGetActiveUniformARB");
        SAFE_GET_PROC(glGetUniformfvARB, PFNGLGETUNIFORMFVARBPROC, "glGetUniformfvARB");
        SAFE_GET_PROC(glGetUniformivARB, PFNGLGETUNIFORMIVARBPROC, "glGetUniformivARB");
        SAFE_GET_PROC(glGetShaderSourceARB, PFNGLGETSHADERSOURCEARBPROC, "glGetShaderSourceARB");

        SAFE_GET_PROC(glBindAttribLocationARB, PFNGLBINDATTRIBLOCATIONARBPROC, "glBindAttribLocationARB");
        SAFE_GET_PROC(glGetActiveAttribARB, PFNGLGETACTIVEATTRIBARBPROC, "glGetActiveAttribARB");
        SAFE_GET_PROC(glGetAttribLocationARB, PFNGLGETATTRIBLOCATIONARBPROC, "glGetAttribLocationARB");
    }
    
    /*ANT menu library GL funcs*/
    /*SAFE_GET_PROC(glBindVertexArray, PFNGLBINDVERTEXARRAYPROC, "glBindVertexArray");
    SAFE_GET_PROC(glDeleteVertexArray, PFNGLDELETEVERTEXARRAYSPROC, "glDeleteVertexArray");
    SAFE_GET_PROC(glGenVertexArray, PFNGLGENVERTEXARRAYSPROC, "glGenVertexArray");
    SAFE_GET_PROC(glIsVertexArray, PFNGLISVERTEXARRAYPROC, "glIsVertexArray");
    SAFE_GET_PROC(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC, "glEnableVertexAttribArray");        
    
    SAFE_GET_PROC(glBindProgramARB, PFNGLBINDPROGRAMARBPROC, "glBindProgramARB");
    SAFE_GET_PROC(glTexImage3D, PFNGLTEXIMAGE3DPROC, "glTexImage3D");
    SAFE_GET_PROC(glBlendEquationSeparate, PFNGLBLENDEQUATIONSEPARATEPROC, "glBlendEquationSeparate");
    SAFE_GET_PROC(glBlendFuncSeparate, PFNGLBLENDFUNCSEPARATEPROC, "glBlendFuncSeparate");
    SAFE_GET_PROC(glBlendEquation, PFNGLBLENDEQUATIONPROC, "glBlendEquation");
    SAFE_GET_PROC(glDisableVertexAttribArray, PFNGLDISABLEVERTEXATTRIBARRAYPROC, "glDisableVertexAttribArray");
    SAFE_GET_PROC(glGetVertexAttribiv, PFNGLGETVERTEXATTRIBIVPROC, "glGetVertexAttribiv");
    SAFE_GET_PROC(glGetVertexAttribfv, PFNGLGETVERTEXATTRIBFVPROC, "glGetVertexAttribfv");*/
#endif
}

/**
 * Use this function after InitGLExtFuncs()!!!
 * @param ext - extension name
 * @return 1 if extension "ext" is supported by your video card. In other cases returns 0.
 */
int IsGLExtensionSupported(const char *ext)
{
    char *ch = engine_gl_ext_str, *chh;
    int len = strlen(ext);
    
    if(ch && ch[0])
    {
        while((chh = strstr(ch, ext)) != NULL)
        {
            ch = chh + len;
            if((*ch == ' ' || *ch == 0) && (chh == engine_gl_ext_str || *(chh - 1) == ' '))
            {
                return 1;
            }
        }
    }

    return 0;
}

