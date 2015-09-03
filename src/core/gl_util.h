
/*****************************************************************
 ************** OLD GOOD QUAKE ENGINE OPENGL STYLE ***************
 *****************************************************************/

#ifndef GL_UTIL_H
#define GL_UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>    /* Header File For The OpenGL Library */
    
typedef GLenum (APIENTRYP PFNGLGETERRORPROC) (void);
typedef const GLubyte* (APIENTRYP PFNGLGETSTRINGPROC) (GLenum name);
typedef void (APIENTRYP PFNGLGETBOOLEANVPROC) (GLenum pname, GLboolean *params);
typedef void (APIENTRYP PFNGLGETDOUBLEVPROC) (GLenum pname, GLdouble *params);
typedef void (APIENTRYP PFNGLGETFLOATVPROC) (GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETIINTEGERVPROC) (GLenum pname, GLint *params);

typedef void (APIENTRYP PFNGLGENTEXTURESPROC) (GLsizei n, GLuint *textures);
typedef void (APIENTRYP PFNGLDELETETEXTURESPROC) (GLsizei n, const GLuint *textures);
typedef void (APIENTRYP PFNGLBINDTEXTURESPROC) (GLenum target, GLuint texture);
typedef void (APIENTRYP PFNGLTEXPATAMETERIPROC) (GLenum target, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLTEXPATAMETERFPROC) (GLenum target, GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLTEXIMAGE2DPROC) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    
extern PFNGLGETERRORPROC qglGetError;
extern PFNGLGETSTRINGPROC qglGetString;
extern PFNGLGETBOOLEANVPROC qglGetBooleanv;
extern PFNGLGETDOUBLEVPROC qglGetDoublev;
extern PFNGLGETFLOATVPROC qglGetFloatv;
extern PFNGLGETIINTEGERVPROC qglGetIntegerv;

extern PFNGLGENTEXTURESPROC qglGenTextures;
extern PFNGLDELETETEXTURESPROC qglDeleteTextures;
extern PFNGLBINDTEXTURESPROC qglBindTexture;
extern PFNGLTEXPATAMETERIPROC qglTexParameteri;
extern PFNGLTEXPATAMETERFPROC qglTexParameterf;
extern PFNGLTEXIMAGE2DPROC qglTexImage2D;


/*GLSL functions EXT*/
extern PFNGLDELETEOBJECTARBPROC qglDeleteObjectARB;
extern PFNGLGETHANDLEARBPROC qglGetHandleARB;
extern PFNGLDETACHOBJECTARBPROC qglDetachObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC qglCreateShaderObjectARB;
extern PFNGLSHADERSOURCEARBPROC qglShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC qglCompileShaderARB;
extern PFNGLCREATEPROGRAMOBJECTARBPROC qglCreateProgramObjectARB;
extern PFNGLATTACHOBJECTARBPROC qglAttachObjectARB;
extern PFNGLLINKPROGRAMARBPROC qglLinkProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC qglUseProgramObjectARB;
extern PFNGLVALIDATEPROGRAMARBPROC qglValidateProgramARB;
extern PFNGLUNIFORM1FARBPROC qglUniform1fARB;
extern PFNGLUNIFORM2FARBPROC qglUniform2fARB;
extern PFNGLUNIFORM3FARBPROC qglUniform3fARB;
extern PFNGLUNIFORM4FARBPROC qglUniform4fARB;
extern PFNGLUNIFORM1IARBPROC qglUniform1iARB;
extern PFNGLUNIFORM2IARBPROC qglUniform2iARB;
extern PFNGLUNIFORM3IARBPROC qglUniform3iARB;
extern PFNGLUNIFORM4IARBPROC qglUniform4iARB;
extern PFNGLUNIFORM1FVARBPROC qglUniform1fvARB;
extern PFNGLUNIFORM2FVARBPROC qglUniform2fvARB;
extern PFNGLUNIFORM3FVARBPROC qglUniform3fvARB;
extern PFNGLUNIFORM4FVARBPROC qglUniform4fvARB;
extern PFNGLUNIFORM1IVARBPROC qglUniform1ivARB;
extern PFNGLUNIFORM2IVARBPROC qglUniform2ivARB;
extern PFNGLUNIFORM3IVARBPROC qglUniform3ivARB;
extern PFNGLUNIFORM4IVARBPROC qglUniform4ivARB;
extern PFNGLUNIFORMMATRIX2FVARBPROC qglUniformMatrix2fvARB;
extern PFNGLUNIFORMMATRIX3FVARBPROC qglUniformMatrix3fvARB;
extern PFNGLUNIFORMMATRIX4FVARBPROC qglUniformMatrix4fvARB;
extern PFNGLGETOBJECTPARAMETERFVARBPROC qglGetObjectParameterfvARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC qglGetObjectParameterivARB;
extern PFNGLGETINFOLOGARBPROC qglGetInfoLogARB;
extern PFNGLGETATTACHEDOBJECTSARBPROC qglGetAttachedObjectsARB;
extern PFNGLGETUNIFORMLOCATIONARBPROC qglGetUniformLocationARB;
extern PFNGLGETACTIVEUNIFORMARBPROC qglGetActiveUniformARB;
extern PFNGLGETUNIFORMFVARBPROC qglGetUniformfvARB;
extern PFNGLGETUNIFORMIVARBPROC qglGetUniformivARB;
extern PFNGLGETSHADERSOURCEARBPROC qglGetShaderSourceARB;

extern PFNGLBINDATTRIBLOCATIONARBPROC qglBindAttribLocationARB;
extern PFNGLGETACTIVEATTRIBARBPROC qglGetActiveAttribARB;
extern PFNGLGETATTRIBLOCATIONARBPROC qglGetAttribLocationARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC qglEnableVertexAttribArrayARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC qglDisableVertexAttribArrayARB;
extern PFNGLVERTEXATTRIBPOINTERARBPROC qglVertexAttribPointerARB;


/*multitexture EXT*/
extern PFNGLACTIVETEXTUREARBPROC qglActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC qglClientActiveTextureARB;
extern PFNGLMULTITEXCOORD1DARBPROC qglMultiTexCoord1dARB;
extern PFNGLMULTITEXCOORD1DVARBPROC qglMultiTexCoord1dvARB;
extern PFNGLMULTITEXCOORD1FARBPROC qglMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD1FVARBPROC qglMultiTexCoord1fvARB;
extern PFNGLMULTITEXCOORD1IARBPROC qglMultiTexCoord1iARB;
extern PFNGLMULTITEXCOORD1IVARBPROC qglMultiTexCoord1ivARB;
extern PFNGLMULTITEXCOORD1SARBPROC qglMultiTexCoord1sARB;
extern PFNGLMULTITEXCOORD1SVARBPROC qglMultiTexCoord1svARB;
extern PFNGLMULTITEXCOORD2DARBPROC qglMultiTexCoord2dARB;
extern PFNGLMULTITEXCOORD2DVARBPROC qglMultiTexCoord2dvARB;
extern PFNGLMULTITEXCOORD2FARBPROC qglMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2FVARBPROC qglMultiTexCoord2fvARB;
extern PFNGLMULTITEXCOORD2IARBPROC qglMultiTexCoord2iARB;
extern PFNGLMULTITEXCOORD2IVARBPROC qglMultiTexCoord2ivARB;
extern PFNGLMULTITEXCOORD2SARBPROC qglMultiTexCoord2sARB;
extern PFNGLMULTITEXCOORD2SVARBPROC qglMultiTexCoord2svARB;
extern PFNGLMULTITEXCOORD3DARBPROC qglMultiTexCoord3dARB;
extern PFNGLMULTITEXCOORD3DVARBPROC qglMultiTexCoord3dvARB;
extern PFNGLMULTITEXCOORD3FARBPROC qglMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD3FVARBPROC qglMultiTexCoord3fvARB;
extern PFNGLMULTITEXCOORD3IARBPROC qglMultiTexCoord3iARB;
extern PFNGLMULTITEXCOORD3IVARBPROC qglMultiTexCoord3ivARB;
extern PFNGLMULTITEXCOORD3SARBPROC qglMultiTexCoord3sARB;
extern PFNGLMULTITEXCOORD3SVARBPROC qglMultiTexCoord3svARB;
extern PFNGLMULTITEXCOORD4DARBPROC qglMultiTexCoord4dARB;
extern PFNGLMULTITEXCOORD4DVARBPROC qglMultiTexCoord4dvARB;
extern PFNGLMULTITEXCOORD4FARBPROC qglMultiTexCoord4fARB;
extern PFNGLMULTITEXCOORD4FVARBPROC qglMultiTexCoord4fvARB;
extern PFNGLMULTITEXCOORD4IARBPROC qglMultiTexCoord4iARB;
extern PFNGLMULTITEXCOORD4IVARBPROC qglMultiTexCoord4ivARB;
extern PFNGLMULTITEXCOORD4SARBPROC qglMultiTexCoord4sARB;
extern PFNGLMULTITEXCOORD4SVARBPROC qglMultiTexCoord4svARB;

/* VBO EXT */
extern PFNGLBINDBUFFERARBPROC qglBindBufferARB;
extern PFNGLDELETEBUFFERSARBPROC qglDeleteBuffersARB;
extern PFNGLGENBUFFERSARBPROC qglGenBuffersARB;
extern PFNGLISBUFFERARBPROC qglIsBufferARB;
extern PFNGLBUFFERDATAARBPROC qglBufferDataARB;
extern PFNGLBUFFERSUBDATAARBPROC qglBufferSubDataARB;
extern PFNGLGETBUFFERSUBDATAARBPROC qglGetBufferSubDataARB;
extern PFNGLMAPBUFFERARBPROC qglMapBufferARB;
extern PFNGLUNMAPBUFFERARBPROC qglUnmapBufferARB;
extern PFNGLGETBUFFERPARAMETERIVARBPROC qglGetBufferParameterivARB;
extern PFNGLGETBUFFERPOINTERVARBPROC qglGetBufferPointervARB;

extern PFNGLBINDVERTEXARRAYPROC qglBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC qglDeleteVertexArrays;
extern PFNGLGENVERTEXARRAYSPROC qglGenVertexArrays;
extern PFNGLISVERTEXARRAYPROC qglIsVertexArray;

extern PFNGLGENERATEMIPMAPPROC qglGenerateMipmap;

void InitGLExtFuncs();
int IsGLExtensionSupported(const char *ext);

int checkOpenGLError();
void printInfoLog (GLhandleARB object);
int loadShaderFromBuff(GLhandleARB ShaderObj, const char *source, const char *additionalDefines);
int loadShaderFromFile(GLhandleARB ShaderObj, const char *fileName, const char *additionalDefines);

void BindWhiteTexture();

#ifdef	__cplusplus
}
#endif

#endif
