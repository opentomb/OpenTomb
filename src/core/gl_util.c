
/*****************************************************************
 ************** OLD GOOD QUAKE ENGINE OPENGL STYLE ***************
 *****************************************************************/

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_opengl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "gl_util.h"
#include "system.h"

#define GL_LOG_FILENAME "gl_log.txt"


/* Miscellaneous */
PFNGLCLEARINDEXPROC                     qglClearIndex = NULL;
PFNGLCLEARCOLORPROC                     qglClearColor = NULL;
PFNGLCLEARPROC                          qglClear = NULL;
PFNGLINDEXMASKPROC                      qglIndexMask = NULL;
PFNGLCOLORMASKPROC                      qglColorMask = NULL;
PFNGLALPHAFUNCPROC                      qglAlphaFunc = NULL;
PFNGLBLENDFUNCPROC                      qglBlendFunc = NULL;
PFNGLLOGICOPPROC                        qglLogicOp = NULL;
PFNGLCULLFACEPROC                       qglCullFace = NULL;
PFNGLFRONTFACEPROC                      qglFrontFace = NULL;
PFNGLPOINTSIZEPROC                      qglPointSize = NULL;
PFNGLLINEWIDTHPROC                      qglLineWidth = NULL;
PFNGLLINESTIPPLEPROC                    qglLineStipple = NULL;
PFNGLPOLYGONMODEPROC                    qglPolygonMode = NULL;
PFNGLPOLYGONOFFSETPROC                  qglPolygonOffset = NULL;
PFNGLPOLYGONSTIPPLEPROC                 qglPolygonStipple = NULL;
PFNGLGETPOLYGONSTIPPLEPROC              qglGetPolygonStipple = NULL;
PFNGLEDGEFLAGPROC                       qglEdgeFlag = NULL;
PFNGLEDGEFLAGVPROC                      qglEdgeFlagv = NULL;
PFNGLSCISSORPROC                        qglScissor = NULL;
PFNGLCLIPPLANEPROC                      qglClipPlane = NULL;
PFNGLGETCLIPPLANEPROC                   qglGetClipPlane = NULL;
PFNGLDRAWBUFFERPROC                     qglDrawBuffer = NULL;
PFNGLREADBUFFERPROC                     qglReadBuffer = NULL;
PFNGLENABLEPROC                         qglEnable = NULL;
PFNGLDISABLEPROC                        qglDisable = NULL;
PFNGLISENABLEDPROC                      qglIsEnabled = NULL;
PFNGLENABLECLIENTSTATEPROC              qglEnableClientState = NULL;
PFNGLDISABLECLIENTSTATEPROC             qglDisableClientState = NULL;
PFNGLGETERRORPROC                       qglGetError = NULL;
PFNGLGETSTRINGPROC                      qglGetString = NULL;
PFNGLGETBOOLEANVPROC                    qglGetBooleanv = NULL;
PFNGLGETDOUBLEVPROC                     qglGetDoublev = NULL;
PFNGLGETFLOATVPROC                      qglGetFloatv = NULL;
PFNGLGETIINTEGERVPROC                   qglGetIntegerv = NULL;
PFNGLPUSHATTRIBPROC                     qglPushAttrib = NULL;
PFNGLPOPATTRIBPROC                      qglPopAttrib = NULL;
PFNGLPUSHCLIENTATTRIBPROC               qglPushClientAttrib = NULL;
PFNGLPOPCLIENTATTRIBPROC                qglPopClientAttrib = NULL;
PFNGLRENDERMODEPROC                     qglRenderMode = NULL;
PFNGLFINISHPROC                         qglFinish = NULL;
PFNGLFLUSHPROC                          qglFlush = NULL;
PFNGLHINTPROC                           qglHint = NULL;

/* Depth Buffer */
PFNGLCLEARDEPTHPROC                     qglClearDepth = NULL;
PFNGLDEPTHFUNCPROC                      qglDepthFunc = NULL;
PFNGLDEPTHMASKPROC                      qglDepthMask = NULL;
PFNGLDEPTHRANGEPROC                     qglDepthRange = NULL;

/* Accumulation Buffer */
PFNGLCLEARACCUMPROC                     qglClearAccum = NULL;               
PFNGLACCUMPROC                          qglAccum = NULL;

/* Transformation */
PFNGLMATRIXMODEPROC                     qglMatrixMode = NULL;
PFNGLORTHOPROC                          qglOrtho = NULL;
PFNGLFRUSTUMPROC                        qglFrustum = NULL;
PFNGLVIEWPORTPROC                       qglViewport = NULL;
PFNGLPUSHMATRIXPROC                     qglPushMatrix = NULL;
PFNGLPOPMATRIXPROC                      qglPopMatrix = NULL;
PFNGLLOADIDENTITYPROC                   qglLoadIdentity = NULL;
PFNGLLOADMATRIXDPROC                    qglLoadMatrixd = NULL;
PFNGLLOADMATRIXFPROC                    qglLoadMatrixf = NULL;
PFNGLMULTMATRIXDPROC                    qglMultMatrixd = NULL;
PFNGLMULTMATRIXFPROC                    qglMultMatrixf = NULL;
PFNGLROTATEDPROC                        qglRotated = NULL;
PFNGLROTATEFPROC                        qglRotatef = NULL;
PFNGLSCALEDPROC                         qglScaled = NULL;
PFNGLSCALEFPROC                         qglScalef = NULL;
PFNGLTRANSLATEDPROC                     qglTranslated = NULL;
PFNGLTRANSLATEFPROC                     qglTranslatef = NULL;

/* Raster functions */
PFNGLPIXELZOOMPROC                      qglPixelZoom = NULL;
PFNGLPIXELSTOREFPROC                    qglPixelStoref = NULL;
PFNGLPIXELSTOREIPROC                    qglPixelStorei = NULL;
PFNGLPIXELTRANSFERFPROC                 qglPixelTransferf = NULL;
PFNGLPIXELTRANSFERIPROC                 qglPixelTransferi = NULL;
PFNGLPIXELMAPFVPROC                     qglPixelMapfv = NULL;
PFNGLPIXELMAPUIVPROC                    qglPixelMapuiv = NULL;
PFNGLPIXELMAPUSVPROC                    qglPixelMapusv = NULL;
PFNGLGETPIXELMAPFVPROC                  qglGetPixelMapfv = NULL;
PFNGLGETPIXELMAPUIVPROC                 qglGetPixelMapuiv = NULL;
PFNGLGETPIXELMAPUSVPROC                 qglGetPixelMapusv = NULL;
PFNGLBITMAPPROC                         qglBitmap = NULL;
PFNGLREADPIXELSPROC                     qglReadPixels = NULL;
PFNGLDRAWPIXELSPROC                     qglDrawPixels = NULL;
PFNGLCOPYPIXELSPROC                     qglCopyPixels = NULL;

/* Stenciling */
PFNGLSTENCILFUNCPROC                    qglStencilFunc = NULL;
PFNGLSTENCILMASKPROC                    qglStencilMask = NULL;
PFNGLSTENCILOPPROC                      qglStencilOp = NULL;
PFNGLCLEARSTENCILPROC                   qglClearStencil = NULL;

/* Texture mapping */
PFNGLTEXGENDPROC                        qglTexGend = NULL;
PFNGLTEXGENFPROC                        qglTexGenf = NULL;
PFNGLTEXGENIPROC                        qglTexGeni = NULL;
PFNGLTEXGENDVPROC                       qglTexGendv = NULL;
PFNGLTEXGENFVPROC                       qglTexGenfv = NULL;
PFNGLTEXGENIVPROC                       qglTexGeniv = NULL;
PFNGLGETTEXGENDVPROC                    qglGetTexGendv = NULL;
PFNGLGETTEXGENFVPROC                    qglGetTexGenfv = NULL;
PFNGLGETTEXGENIVPROC                    qglGetTexGeniv = NULL;
PFNGLTEXENVFPROC                        qglTexEnvf = NULL;
PFNGLTEXENVIPROC                        qglTexEnvi = NULL;
PFNGLTEXENVFVPROC                       qglTexEnvfv = NULL;
PFNGLTEXENVIVPROC                       qglTexEnviv = NULL;
PFNGLGETTEXENVFVPROC                    qglGetTexEnvfv = NULL;
PFNGLGETTEXENVIVPROC                    qglGetTexEnviv = NULL;
PFNGLTEXPARAMETERFPROC                  qglTexParameterf = NULL;
PFNGLTEXPARAMETERIPROC                  qglTexParameteri = NULL;
PFNGLTEXPARAMETERFVPROC                 qglTexParameterfv = NULL;
PFNGLTEXPARAMETERIVPROC                 qglTexParameteriv = NULL;
PFNGLGETTEXPARAMETERFVPROC              qglGetTexParameterfv = NULL;
PFNGLGETTEXPARAMETERIVPROC              qglGetTexParameteriv = NULL;
PFNGLGETTEXLEVELPARAMETERFVPROC         qglGetTexLevelParameterfv = NULL;
PFNGLGETTEXLEVELPARAMETERIVPROC         qglGetTexLevelParameteriv = NULL;
PFNGLTEXIMAGE1DPROC                     qglTexImage1D = NULL;
PFNGLTEXIMAGE2DPROC                     qglTexImage2D = NULL;
PFNGLGETTEXIMAGEPROC                    qglGetTexImage = NULL;

/* 1.1 functions */
/* texture objects */
PFNGLGENTEXTURESPROC                    qglGenTextures = NULL;
PFNGLDELETETEXTURESPROC                 qglDeleteTextures = NULL;
PFNGLBINDTEXTUREPROC                    qglBindTexture = NULL;
PFNGLPRIORITIZETEXTURESPROC             qglPrioritizeTextures = NULL;
PFNGLARETEXTURESRESIDENTPROC            qglAreTexturesResident = NULL;
PFNGLISTEXTUREPROC                      qglIsTexture = NULL;
/* texture mapping */
PFNGLTEXSUBIMAGE1DPROC                  qglTexSubImage1D = NULL;
PFNGLTEXSUBIMAGE2DPROC                  qglTexSubImage2D = NULL;
PFNGLCOPYTEXIMAGE1DPROC                 qglCopyTexImage1D = NULL;
PFNGLCOPYTEXIMAGE2DPROC                 qglCopyTexImage2D = NULL;
PFNGLCOPYTEXSUBIMAGE1DPROC              qglCopyTexSubImage1D = NULL;
PFNGLCOPYTEXSUBIMAGE2DPROC              qglCopyTexSubImage2D = NULL;
/* vertex arrays */
PFNGLVERTEXPOINTERPROC                  qglVertexPointer = NULL;
PFNGLNORMALPOINTERPROC                  qglNormalPointer = NULL;
PFNGLCOLORPOINTERPROC                   qglColorPointer = NULL;
PFNGLINDEXPOINTERPROC                   qglIndexPointer = NULL;
PFNGLTEXCOORDPOINTERPROC                qglTexCoordPointer = NULL;
PFNGLEDGEFLAGPOINTERPROC                qglEdgeFlagPointer = NULL;
PFNGLGETPOINTERVPROC                    qglGetPointerv = NULL;
PFNGLARRAYELEMENTPROC                   qglArrayElement = NULL;
PFNGLDRAWARRAYSPROC                     qglDrawArrays = NULL;
PFNGLDRAWELEMENTSPROC                   qglDrawElements = NULL;
PFNGLINTERLEAVEDARRAYSPROC              qglInterleavedArrays = NULL;

/*ARB*/
PFNGLDELETEOBJECTARBPROC                qglDeleteObjectARB = NULL;
PFNGLGETHANDLEARBPROC                   qglGetHandleARB = NULL;
PFNGLDETACHOBJECTARBPROC                qglDetachObjectARB = NULL;
PFNGLCREATESHADEROBJECTARBPROC          qglCreateShaderObjectARB = NULL;
PFNGLSHADERSOURCEARBPROC                qglShaderSourceARB = NULL;
PFNGLCOMPILESHADERARBPROC               qglCompileShaderARB = NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC         qglCreateProgramObjectARB = NULL;
PFNGLATTACHOBJECTARBPROC                qglAttachObjectARB = NULL;
PFNGLLINKPROGRAMARBPROC                 qglLinkProgramARB = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC            qglUseProgramObjectARB = NULL;
PFNGLVALIDATEPROGRAMARBPROC             qglValidateProgramARB = NULL;
PFNGLUNIFORM1FARBPROC                   qglUniform1fARB = NULL;
PFNGLUNIFORM2FARBPROC                   qglUniform2fARB = NULL;
PFNGLUNIFORM3FARBPROC                   qglUniform3fARB = NULL;
PFNGLUNIFORM4FARBPROC                   qglUniform4fARB = NULL;
PFNGLUNIFORM1IARBPROC                   qglUniform1iARB = NULL;
PFNGLUNIFORM2IARBPROC                   qglUniform2iARB = NULL;
PFNGLUNIFORM3IARBPROC                   qglUniform3iARB = NULL;
PFNGLUNIFORM4IARBPROC                   qglUniform4iARB = NULL;
PFNGLUNIFORM1FVARBPROC                  qglUniform1fvARB = NULL;
PFNGLUNIFORM2FVARBPROC                  qglUniform2fvARB = NULL;
PFNGLUNIFORM3FVARBPROC                  qglUniform3fvARB = NULL;
PFNGLUNIFORM4FVARBPROC                  qglUniform4fvARB = NULL;
PFNGLUNIFORM1IVARBPROC                  qglUniform1ivARB = NULL;
PFNGLUNIFORM2IVARBPROC                  qglUniform2ivARB = NULL;
PFNGLUNIFORM3IVARBPROC                  qglUniform3ivARB = NULL;
PFNGLUNIFORM4IVARBPROC                  qglUniform4ivARB = NULL;
PFNGLUNIFORMMATRIX2FVARBPROC            qglUniformMatrix2fvARB = NULL;
PFNGLUNIFORMMATRIX3FVARBPROC            qglUniformMatrix3fvARB = NULL;
PFNGLUNIFORMMATRIX4FVARBPROC            qglUniformMatrix4fvARB = NULL;
PFNGLGETOBJECTPARAMETERFVARBPROC        qglGetObjectParameterfvARB = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC        qglGetObjectParameterivARB = NULL;
PFNGLGETINFOLOGARBPROC                  qglGetInfoLogARB = NULL;
PFNGLGETATTACHEDOBJECTSARBPROC          qglGetAttachedObjectsARB = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC          qglGetUniformLocationARB = NULL;
PFNGLGETACTIVEUNIFORMARBPROC            qglGetActiveUniformARB = NULL;
PFNGLGETUNIFORMFVARBPROC                qglGetUniformfvARB = NULL;
PFNGLGETUNIFORMIVARBPROC                qglGetUniformivARB = NULL;
PFNGLGETSHADERSOURCEARBPROC             qglGetShaderSourceARB = NULL;

PFNGLBINDATTRIBLOCATIONARBPROC          qglBindAttribLocationARB = NULL;
PFNGLGETACTIVEATTRIBARBPROC             qglGetActiveAttribARB = NULL;
PFNGLGETATTRIBLOCATIONARBPROC           qglGetAttribLocationARB = NULL;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC     qglEnableVertexAttribArrayARB = NULL;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC     qglDisableVertexAttribArrayARB = NULL;
PFNGLVERTEXATTRIBPOINTERARBPROC         qglVertexAttribPointerARB = NULL;

PFNGLACTIVETEXTUREARBPROC               qglActiveTextureARB = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC         qglClientActiveTextureARB = NULL;
PFNGLMULTITEXCOORD1DARBPROC             qglMultiTexCoord1dARB = NULL;
PFNGLMULTITEXCOORD1DVARBPROC            qglMultiTexCoord1dvARB = NULL;
PFNGLMULTITEXCOORD1FARBPROC             qglMultiTexCoord1fARB = NULL;
PFNGLMULTITEXCOORD1FVARBPROC            qglMultiTexCoord1fvARB = NULL;
PFNGLMULTITEXCOORD1IARBPROC             qglMultiTexCoord1iARB = NULL;
PFNGLMULTITEXCOORD1IVARBPROC            qglMultiTexCoord1ivARB = NULL;
PFNGLMULTITEXCOORD1SARBPROC             qglMultiTexCoord1sARB = NULL;
PFNGLMULTITEXCOORD1SVARBPROC            qglMultiTexCoord1svARB = NULL;
PFNGLMULTITEXCOORD2DARBPROC             qglMultiTexCoord2dARB = NULL;
PFNGLMULTITEXCOORD2DVARBPROC            qglMultiTexCoord2dvARB = NULL;
PFNGLMULTITEXCOORD2FARBPROC             qglMultiTexCoord2fARB = NULL;
PFNGLMULTITEXCOORD2FVARBPROC            qglMultiTexCoord2fvARB = NULL;
PFNGLMULTITEXCOORD2IARBPROC             qglMultiTexCoord2iARB = NULL;
PFNGLMULTITEXCOORD2IVARBPROC            qglMultiTexCoord2ivARB = NULL;
PFNGLMULTITEXCOORD2SARBPROC             qglMultiTexCoord2sARB = NULL;
PFNGLMULTITEXCOORD2SVARBPROC            qglMultiTexCoord2svARB = NULL;
PFNGLMULTITEXCOORD3DARBPROC             qglMultiTexCoord3dARB = NULL;
PFNGLMULTITEXCOORD3DVARBPROC            qglMultiTexCoord3dvARB = NULL;
PFNGLMULTITEXCOORD3FARBPROC             qglMultiTexCoord3fARB = NULL;
PFNGLMULTITEXCOORD3FVARBPROC            qglMultiTexCoord3fvARB = NULL;
PFNGLMULTITEXCOORD3IARBPROC             qglMultiTexCoord3iARB = NULL;
PFNGLMULTITEXCOORD3IVARBPROC            qglMultiTexCoord3ivARB = NULL;
PFNGLMULTITEXCOORD3SARBPROC             qglMultiTexCoord3sARB = NULL;
PFNGLMULTITEXCOORD3SVARBPROC            qglMultiTexCoord3svARB = NULL;
PFNGLMULTITEXCOORD4DARBPROC             qglMultiTexCoord4dARB = NULL;
PFNGLMULTITEXCOORD4DVARBPROC            qglMultiTexCoord4dvARB = NULL;
PFNGLMULTITEXCOORD4FARBPROC             qglMultiTexCoord4fARB = NULL;
PFNGLMULTITEXCOORD4FVARBPROC            qglMultiTexCoord4fvARB = NULL;
PFNGLMULTITEXCOORD4IARBPROC             qglMultiTexCoord4iARB = NULL;
PFNGLMULTITEXCOORD4IVARBPROC            qglMultiTexCoord4ivARB = NULL;
PFNGLMULTITEXCOORD4SARBPROC             qglMultiTexCoord4sARB = NULL;
PFNGLMULTITEXCOORD4SVARBPROC            qglMultiTexCoord4svARB = NULL;

PFNGLBINDBUFFERARBPROC                  qglBindBufferARB = NULL;
PFNGLDELETEBUFFERSARBPROC               qglDeleteBuffersARB = NULL;
PFNGLGENBUFFERSARBPROC                  qglGenBuffersARB = NULL;
PFNGLISBUFFERARBPROC                    qglIsBufferARB = NULL;
PFNGLBUFFERDATAARBPROC                  qglBufferDataARB = NULL;
PFNGLBUFFERSUBDATAARBPROC               qglBufferSubDataARB = NULL;
PFNGLGETBUFFERSUBDATAARBPROC            qglGetBufferSubDataARB = NULL;
PFNGLMAPBUFFERARBPROC                   qglMapBufferARB = NULL;
PFNGLUNMAPBUFFERARBPROC                 qglUnmapBufferARB = NULL;
PFNGLGETBUFFERPARAMETERIVARBPROC        qglGetBufferParameterivARB = NULL;
PFNGLGETBUFFERPOINTERVARBPROC           qglGetBufferPointervARB = NULL;

PFNGLBINDVERTEXARRAYPROC                qglBindVertexArray = NULL;
PFNGLDELETEVERTEXARRAYSPROC             qglDeleteVertexArrays = NULL;
PFNGLGENVERTEXARRAYSPROC                qglGenVertexArrays = NULL;
PFNGLISVERTEXARRAYPROC                  qglIsVertexArray = NULL;

PFNGLGENERATEMIPMAPEXTPROC              qglGenerateMipmap = NULL;

static char *engine_gl_ext_str = NULL;
static GLuint whiteTexture = 0;

/**
 * Get addresses of GL functions and initialise engine_gl_ext_str string.
 */
void InitGLExtFuncs()
{
    // white texture data for coloured polygons and debug lines.
    const GLubyte whtx[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    /* Miscellaneous */
    qglClearIndex = (PFNGLCLEARINDEXPROC)SDL_GL_GetProcAddress("glClearIndex");
    qglClearColor = (PFNGLCLEARCOLORPROC)SDL_GL_GetProcAddress("glClearColor");
    qglClear = (PFNGLCLEARPROC)SDL_GL_GetProcAddress("glClear");
    qglIndexMask = (PFNGLINDEXMASKPROC)SDL_GL_GetProcAddress("glIndexMask");
    qglColorMask = (PFNGLCOLORMASKPROC)SDL_GL_GetProcAddress("glColorMask");
    qglAlphaFunc = (PFNGLALPHAFUNCPROC)SDL_GL_GetProcAddress("glAlphaFunc");
    qglBlendFunc = (PFNGLBLENDFUNCPROC)SDL_GL_GetProcAddress("glBlendFunc");
    qglLogicOp = (PFNGLLOGICOPPROC)SDL_GL_GetProcAddress("glLogicOp");
    qglCullFace = (PFNGLCULLFACEPROC)SDL_GL_GetProcAddress("glCullFace");
    qglFrontFace = (PFNGLFRONTFACEPROC)SDL_GL_GetProcAddress("glFrontFace");
    qglPushAttrib = (PFNGLPUSHATTRIBPROC)SDL_GL_GetProcAddress("glPushAttrib");
    qglPointSize = (PFNGLPOINTSIZEPROC)SDL_GL_GetProcAddress("glPointSize");
    qglLineWidth = (PFNGLLINEWIDTHPROC)SDL_GL_GetProcAddress("glLineWidth");
    qglLineStipple = (PFNGLLINESTIPPLEPROC)SDL_GL_GetProcAddress("glLineStipple");
    qglPolygonMode = (PFNGLPOLYGONMODEPROC)SDL_GL_GetProcAddress("glPolygonMode");
    qglPolygonOffset = (PFNGLPOLYGONOFFSETPROC)SDL_GL_GetProcAddress("glPolygonOffset");
    qglPolygonStipple = (PFNGLPOLYGONSTIPPLEPROC)SDL_GL_GetProcAddress("glPolygonStipple");
    qglGetPolygonStipple = (PFNGLGETPOLYGONSTIPPLEPROC)SDL_GL_GetProcAddress("glGetPolygonStipple");
    qglEdgeFlag = (PFNGLEDGEFLAGPROC)SDL_GL_GetProcAddress("glEdgeFlag");
    qglEdgeFlagv = (PFNGLEDGEFLAGVPROC)SDL_GL_GetProcAddress("glEdgeFlagv");
    qglScissor = (PFNGLSCISSORPROC)SDL_GL_GetProcAddress("glScissor");
    qglClipPlane = (PFNGLCLIPPLANEPROC)SDL_GL_GetProcAddress("glClipPlane");
    qglGetClipPlane = (PFNGLGETCLIPPLANEPROC)SDL_GL_GetProcAddress("glGetClipPlane");
    qglDrawBuffer = (PFNGLDRAWBUFFERPROC)SDL_GL_GetProcAddress("glDrawBuffer");
    qglReadBuffer = (PFNGLREADBUFFERPROC)SDL_GL_GetProcAddress("glReadBuffer");
    qglEnable = (PFNGLENABLEPROC)SDL_GL_GetProcAddress("glEnable");
    qglDisable = (PFNGLDISABLEPROC)SDL_GL_GetProcAddress("glDisable");
    qglIsEnabled = (PFNGLISENABLEDPROC)SDL_GL_GetProcAddress("glIsEnabled");
    qglEnableClientState = (PFNGLENABLECLIENTSTATEPROC)SDL_GL_GetProcAddress("glEnableClientState");
    qglDisableClientState = (PFNGLDISABLECLIENTSTATEPROC)SDL_GL_GetProcAddress("glDisableClientState");
    qglGetError = (PFNGLGETERRORPROC)SDL_GL_GetProcAddress("glGetError");
    qglGetString = (PFNGLGETSTRINGPROC)SDL_GL_GetProcAddress("glGetString");
    qglGetBooleanv = (PFNGLGETBOOLEANVPROC)SDL_GL_GetProcAddress("glGetBooleanv");
    qglGetDoublev = (PFNGLGETDOUBLEVPROC)SDL_GL_GetProcAddress("glGetDoublev");
    qglGetFloatv = (PFNGLGETFLOATVPROC)SDL_GL_GetProcAddress("glGetFloatv");
    qglGetIntegerv = (PFNGLGETIINTEGERVPROC)SDL_GL_GetProcAddress("glGetIntegerv");
    qglPushAttrib = (PFNGLPUSHATTRIBPROC)SDL_GL_GetProcAddress("glPushAttrib");
    qglPopAttrib = (PFNGLPOPATTRIBPROC)SDL_GL_GetProcAddress("glPopAttrib");
    qglPushClientAttrib = (PFNGLPUSHCLIENTATTRIBPROC)SDL_GL_GetProcAddress("glPushClientAttrib");  /* 1.1 */
    qglPopClientAttrib = (PFNGLPOPCLIENTATTRIBPROC)SDL_GL_GetProcAddress("glPopClientAttrib");  /* 1.1 */
    qglRenderMode = (PFNGLRENDERMODEPROC)SDL_GL_GetProcAddress("glRenderMode");
    qglFinish = (PFNGLFINISHPROC)SDL_GL_GetProcAddress("glFinish");
    qglFlush = (PFNGLFLUSHPROC)SDL_GL_GetProcAddress("glFlush");
    qglHint = (PFNGLHINTPROC)SDL_GL_GetProcAddress("glHint");

    /* Depth Buffer */
    qglClearDepth = (PFNGLCLEARDEPTHPROC)SDL_GL_GetProcAddress("glClearDepth");
    qglDepthFunc = (PFNGLDEPTHFUNCPROC)SDL_GL_GetProcAddress("glDepthFunc");
    qglDepthMask = (PFNGLDEPTHMASKPROC)SDL_GL_GetProcAddress("glDepthMask");
    qglDepthRange = (PFNGLDEPTHRANGEPROC)SDL_GL_GetProcAddress("glDepthRange");

    /* Accumulation Buffer */
    qglClearAccum = (PFNGLCLEARACCUMPROC)SDL_GL_GetProcAddress("glClearAccum");               
    qglAccum = (PFNGLACCUMPROC)SDL_GL_GetProcAddress("glAccum");

    /* Transformation */
    qglMatrixMode = (PFNGLMATRIXMODEPROC)SDL_GL_GetProcAddress("glMatrixMode");
    qglOrtho = (PFNGLORTHOPROC)SDL_GL_GetProcAddress("glOrtho");
    qglFrustum = (PFNGLFRUSTUMPROC)SDL_GL_GetProcAddress("glFrustum");
    qglViewport = (PFNGLVIEWPORTPROC)SDL_GL_GetProcAddress("glViewport");
    qglPushMatrix = (PFNGLPUSHMATRIXPROC)SDL_GL_GetProcAddress("glPushMatrix");
    qglPopMatrix = (PFNGLPOPMATRIXPROC)SDL_GL_GetProcAddress("glPopMatrix");
    qglLoadIdentity = (PFNGLLOADIDENTITYPROC)SDL_GL_GetProcAddress("glLoadIdentity");
    qglLoadMatrixd = (PFNGLLOADMATRIXDPROC)SDL_GL_GetProcAddress("glLoadMatrixd");
    qglLoadMatrixf = (PFNGLLOADMATRIXFPROC)SDL_GL_GetProcAddress("glLoadMatrixf");
    qglMultMatrixd = (PFNGLMULTMATRIXDPROC)SDL_GL_GetProcAddress("glMultMatrixd");
    qglMultMatrixf = (PFNGLMULTMATRIXFPROC)SDL_GL_GetProcAddress("glMultMatrixf");
    qglRotated = (PFNGLROTATEDPROC)SDL_GL_GetProcAddress("glRotated");
    qglRotatef = (PFNGLROTATEFPROC)SDL_GL_GetProcAddress("glRotatef");
    qglScaled = (PFNGLSCALEDPROC)SDL_GL_GetProcAddress("glScaled");
    qglScalef = (PFNGLSCALEFPROC)SDL_GL_GetProcAddress("glScalef");
    qglTranslated = (PFNGLTRANSLATEDPROC)SDL_GL_GetProcAddress("glTranslated");
    qglTranslatef = (PFNGLTRANSLATEFPROC)SDL_GL_GetProcAddress("glTranslatef");
    
    /* Raster functions */
    qglPixelZoom = (PFNGLPIXELZOOMPROC)SDL_GL_GetProcAddress("glPixelZoom");
    qglPixelStoref = (PFNGLPIXELSTOREFPROC)SDL_GL_GetProcAddress("glPixelStoref");
    qglPixelStorei = (PFNGLPIXELSTOREIPROC)SDL_GL_GetProcAddress("glPixelStorei");
    qglPixelTransferf = (PFNGLPIXELTRANSFERFPROC)SDL_GL_GetProcAddress("glPixelTransferf");
    qglPixelTransferi = (PFNGLPIXELTRANSFERIPROC)SDL_GL_GetProcAddress("glPixelTransferi");
    qglPixelMapfv = (PFNGLPIXELMAPFVPROC)SDL_GL_GetProcAddress("glPixelMapfv");
    qglPixelMapuiv = (PFNGLPIXELMAPUIVPROC)SDL_GL_GetProcAddress("glPixelMapuiv");
    qglPixelMapusv = (PFNGLPIXELMAPUSVPROC)SDL_GL_GetProcAddress("glPixelMapusv");
    qglGetPixelMapfv = (PFNGLGETPIXELMAPFVPROC)SDL_GL_GetProcAddress("glGetPixelMapfv");
    qglGetPixelMapuiv = (PFNGLGETPIXELMAPUIVPROC)SDL_GL_GetProcAddress("glGetPixelMapuiv");
    qglGetPixelMapusv = (PFNGLGETPIXELMAPUSVPROC)SDL_GL_GetProcAddress("glGetPixelMapusv");
    qglBitmap = (PFNGLBITMAPPROC)SDL_GL_GetProcAddress("glBitmap");
    qglReadPixels = (PFNGLREADPIXELSPROC)SDL_GL_GetProcAddress("glReadPixels");
    qglDrawPixels = (PFNGLDRAWPIXELSPROC)SDL_GL_GetProcAddress("glDrawPixels");
    qglCopyPixels = (PFNGLCOPYPIXELSPROC)SDL_GL_GetProcAddress("glCopyPixels");

    /* Stenciling */
    qglStencilFunc = (PFNGLSTENCILFUNCPROC)SDL_GL_GetProcAddress("glStencilFunc");
    qglStencilMask = (PFNGLSTENCILMASKPROC)SDL_GL_GetProcAddress("glStencilMask");
    qglStencilOp = (PFNGLSTENCILOPPROC)SDL_GL_GetProcAddress("glStencilOp");
    qglClearStencil = (PFNGLCLEARSTENCILPROC)SDL_GL_GetProcAddress("glClearStencil");

    /* Texture mapping */
    qglTexGend = (PFNGLTEXGENDPROC)SDL_GL_GetProcAddress("glTexGend");
    qglTexGenf = (PFNGLTEXGENFPROC)SDL_GL_GetProcAddress("glTexGenf");
    qglTexGeni = (PFNGLTEXGENIPROC)SDL_GL_GetProcAddress("glTexGeni");
    qglTexGendv = (PFNGLTEXGENDVPROC)SDL_GL_GetProcAddress("glTexGendv");
    qglTexGenfv = (PFNGLTEXGENFVPROC)SDL_GL_GetProcAddress("glTexGenfv");
    qglTexGeniv = (PFNGLTEXGENIVPROC)SDL_GL_GetProcAddress("glTexGeniv");
    qglGetTexGendv = (PFNGLGETTEXGENDVPROC)SDL_GL_GetProcAddress("glGetTexGendv");
    qglGetTexGenfv = (PFNGLGETTEXGENFVPROC)SDL_GL_GetProcAddress("glGetTexGenfv");
    qglGetTexGeniv = (PFNGLGETTEXGENIVPROC)SDL_GL_GetProcAddress("glGetTexGeniv");
    qglTexEnvf = (PFNGLTEXENVFPROC)SDL_GL_GetProcAddress("glTexEnvf");
    qglTexEnvi = (PFNGLTEXENVIPROC)SDL_GL_GetProcAddress("glTexEnvi");
    qglTexEnvfv = (PFNGLTEXENVFVPROC)SDL_GL_GetProcAddress("glTexEnvfv");
    qglTexEnviv = (PFNGLTEXENVIVPROC)SDL_GL_GetProcAddress("glTexEnviv");
    qglGetTexEnvfv = (PFNGLGETTEXENVFVPROC)SDL_GL_GetProcAddress("glGetTexEnvfv");
    qglGetTexEnviv = (PFNGLGETTEXENVIVPROC)SDL_GL_GetProcAddress("glGetTexEnviv");
    qglTexParameterf = (PFNGLTEXPARAMETERFPROC)SDL_GL_GetProcAddress("glTexParameterf");
    qglTexParameteri = (PFNGLTEXPARAMETERIPROC)SDL_GL_GetProcAddress("glTexParameteri");
    qglTexParameterfv = (PFNGLTEXPARAMETERFVPROC)SDL_GL_GetProcAddress("glTexParameterfv");
    qglTexParameteriv = (PFNGLTEXPARAMETERIVPROC)SDL_GL_GetProcAddress("glTexParameteriv");
    qglGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC)SDL_GL_GetProcAddress("glGetTexParameterfv");
    qglGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC)SDL_GL_GetProcAddress("glGetTexParameteriv");
    qglGetTexLevelParameterfv = (PFNGLGETTEXLEVELPARAMETERFVPROC)SDL_GL_GetProcAddress("glGetTexLevelParameterfv");
    qglGetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIVPROC)SDL_GL_GetProcAddress("glGetTexLevelParameteriv");
    qglTexImage1D = (PFNGLTEXIMAGE1DPROC)SDL_GL_GetProcAddress("glTexImage1D");
    qglTexImage2D = (PFNGLTEXIMAGE2DPROC)SDL_GL_GetProcAddress("glTexImage2D");
    qglGetTexImage = (PFNGLGETTEXIMAGEPROC)SDL_GL_GetProcAddress("glGetTexImage");
    
    /* 1.1 functions */
    /* texture objects */
    qglGenTextures = (PFNGLGENTEXTURESPROC)SDL_GL_GetProcAddress("glGenTextures");
    qglDeleteTextures = (PFNGLDELETETEXTURESPROC)SDL_GL_GetProcAddress("glDeleteTextures");
    qglBindTexture = (PFNGLBINDTEXTUREPROC)SDL_GL_GetProcAddress("glBindTexture");
    qglPrioritizeTextures = (PFNGLPRIORITIZETEXTURESPROC)SDL_GL_GetProcAddress("glPrioritizeTextures");
    qglAreTexturesResident = (PFNGLARETEXTURESRESIDENTPROC)SDL_GL_GetProcAddress("glAreTexturesResident");
    qglIsTexture = (PFNGLISTEXTUREPROC)SDL_GL_GetProcAddress("glIsTexture");
    /* texture mapping */
    qglTexSubImage1D = (PFNGLTEXSUBIMAGE1DPROC)SDL_GL_GetProcAddress("glTexSubImage1D");
    qglTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)SDL_GL_GetProcAddress("glTexSubImage2D");
    qglCopyTexImage1D = (PFNGLCOPYTEXIMAGE1DPROC)SDL_GL_GetProcAddress("glCopyTexImage1D");
    qglCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC)SDL_GL_GetProcAddress("glCopyTexImage2D");
    qglCopyTexSubImage1D = (PFNGLCOPYTEXSUBIMAGE1DPROC)SDL_GL_GetProcAddress("glCopyTexSubImage1D");
    qglCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC)SDL_GL_GetProcAddress("glCopyTexSubImage2D");
    /* vertex arrays */
    qglVertexPointer = (PFNGLVERTEXPOINTERPROC)SDL_GL_GetProcAddress("glVertexPointer");
    qglNormalPointer = (PFNGLNORMALPOINTERPROC)SDL_GL_GetProcAddress("glNormalPointer");
    qglColorPointer = (PFNGLCOLORPOINTERPROC)SDL_GL_GetProcAddress("glColorPointer");
    qglIndexPointer = (PFNGLINDEXPOINTERPROC)SDL_GL_GetProcAddress("glIndexPointer");
    qglTexCoordPointer = (PFNGLTEXCOORDPOINTERPROC)SDL_GL_GetProcAddress("glTexCoordPointer");
    qglEdgeFlagPointer = (PFNGLEDGEFLAGPOINTERPROC)SDL_GL_GetProcAddress("glEdgeFlagPointer");
    qglGetPointerv = (PFNGLGETPOINTERVPROC)SDL_GL_GetProcAddress("glGetPointerv");
    qglArrayElement = (PFNGLARRAYELEMENTPROC)SDL_GL_GetProcAddress("glArrayElement");
    qglDrawArrays = (PFNGLDRAWARRAYSPROC)SDL_GL_GetProcAddress("glDrawArrays");
    qglDrawElements = (PFNGLDRAWELEMENTSPROC)SDL_GL_GetProcAddress("glDrawElements");
    qglInterleavedArrays = (PFNGLINTERLEAVEDARRAYSPROC)SDL_GL_GetProcAddress("glInterleavedArrays");
    
    const char *buf = (const char*)qglGetString(GL_EXTENSIONS);
    if(buf)
    {
        size_t buf_size = strlen(buf) + 1;
        engine_gl_ext_str = (char*)malloc(buf_size);
        strncpy(engine_gl_ext_str, buf, buf_size);
    }
    
    qglGenTextures(1, &whiteTexture);
    qglBindTexture(GL_TEXTURE_2D, whiteTexture);
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, whtx);
    qglTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, whtx);
    qglTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whtx);
    qglBindTexture(GL_TEXTURE_2D, 0);

    /// VBO funcs
    if(IsGLExtensionSupported("GL_ARB_vertex_buffer_object"))
    {
        qglBindBufferARB = (PFNGLBINDBUFFERARBPROC)SDL_GL_GetProcAddress("glBindBufferARB");
        qglDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)SDL_GL_GetProcAddress("glDeleteBuffersARB");
        qglGenBuffersARB = (PFNGLGENBUFFERSARBPROC)SDL_GL_GetProcAddress("glGenBuffersARB");
        qglIsBufferARB = (PFNGLISBUFFERARBPROC)SDL_GL_GetProcAddress("glIsBufferARB");
        qglBufferDataARB = (PFNGLBUFFERDATAARBPROC)SDL_GL_GetProcAddress("glBufferDataARB");
        qglBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)SDL_GL_GetProcAddress("glBufferSubDataARB");
        qglGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC)SDL_GL_GetProcAddress("glGetBufferSubDataARB");
        qglMapBufferARB = (PFNGLMAPBUFFERARBPROC)SDL_GL_GetProcAddress("glMapBufferARB");
        qglUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)SDL_GL_GetProcAddress("glUnmapBufferARB");
        qglGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)SDL_GL_GetProcAddress("glGetBufferParameterivARB");
        qglGetBufferPointervARB = (PFNGLGETBUFFERPOINTERVARBPROC)SDL_GL_GetProcAddress("glGetBufferPointervARB");

        qglActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glActiveTextureARB");
        qglClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glClientActiveTextureARB");

        qglMultiTexCoord1dARB = (PFNGLMULTITEXCOORD1DARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1dARB");
        qglMultiTexCoord1dvARB = (PFNGLMULTITEXCOORD1DVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1dvARB");
        qglMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1fARB");
        qglMultiTexCoord1fvARB = (PFNGLMULTITEXCOORD1FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1fvARB");
        qglMultiTexCoord1iARB = (PFNGLMULTITEXCOORD1IARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1iARB");
        qglMultiTexCoord1ivARB = (PFNGLMULTITEXCOORD1IVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1ivARB");
        qglMultiTexCoord1sARB = (PFNGLMULTITEXCOORD1SARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1sARB");
        qglMultiTexCoord1svARB = (PFNGLMULTITEXCOORD1SVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1svARB");

        qglMultiTexCoord2dARB = (PFNGLMULTITEXCOORD2DARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2dARB");
        qglMultiTexCoord2dvARB = (PFNGLMULTITEXCOORD2DVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2dvARB");
        qglMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
        qglMultiTexCoord2fvARB = (PFNGLMULTITEXCOORD2FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2fvARB");
        qglMultiTexCoord2iARB = (PFNGLMULTITEXCOORD2IARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2iARB");
        qglMultiTexCoord2ivARB = (PFNGLMULTITEXCOORD2IVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2ivARB");
        qglMultiTexCoord2sARB = (PFNGLMULTITEXCOORD2SARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2sARB");
        qglMultiTexCoord2svARB = (PFNGLMULTITEXCOORD2SVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2svARB");

        qglMultiTexCoord3dARB = (PFNGLMULTITEXCOORD3DARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3dARB");
        qglMultiTexCoord3dvARB = (PFNGLMULTITEXCOORD3DVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3dvARB");
        qglMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3fARB");
        qglMultiTexCoord3fvARB = (PFNGLMULTITEXCOORD3FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3fvARB");
        qglMultiTexCoord3iARB = (PFNGLMULTITEXCOORD3IARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3iARB");
        qglMultiTexCoord3ivARB = (PFNGLMULTITEXCOORD3IVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3ivARB");
        qglMultiTexCoord3sARB = (PFNGLMULTITEXCOORD3SARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3sARB");
        qglMultiTexCoord3svARB = (PFNGLMULTITEXCOORD3SVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3svARB");

        qglMultiTexCoord4dARB = (PFNGLMULTITEXCOORD4DARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4dARB");
        qglMultiTexCoord4dvARB = (PFNGLMULTITEXCOORD4DVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4dvARB");
        qglMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4fARB");
        qglMultiTexCoord4fvARB = (PFNGLMULTITEXCOORD4FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4fvARB");
        qglMultiTexCoord4iARB = (PFNGLMULTITEXCOORD4IARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4iARB");
        qglMultiTexCoord4ivARB = (PFNGLMULTITEXCOORD4IVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4ivARB");
        qglMultiTexCoord4sARB = (PFNGLMULTITEXCOORD4SARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4sARB");
        qglMultiTexCoord4svARB = (PFNGLMULTITEXCOORD4SVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4svARB");

        qglBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
        qglDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
        qglGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
        qglIsVertexArray = (PFNGLISVERTEXARRAYPROC)SDL_GL_GetProcAddress("glIsVertexArray");

        qglGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)SDL_GL_GetProcAddress("glGenerateMipmap");
    }
    else
    {
        fprintf(stderr, "VBOs not supported");
        abort();
    }
    if(IsGLExtensionSupported("GL_ARB_shading_language_100"))
    {
        qglDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)SDL_GL_GetProcAddress("glDeleteObjectARB");
        qglGetHandleARB = (PFNGLGETHANDLEARBPROC)SDL_GL_GetProcAddress("glGetHandleARB");
        qglDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)SDL_GL_GetProcAddress("glDetachObjectARB");
        qglCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)SDL_GL_GetProcAddress("glCreateShaderObjectARB");
        qglShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)SDL_GL_GetProcAddress("glShaderSourceARB");
        qglCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)SDL_GL_GetProcAddress("glCompileShaderARB");
        qglCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glCreateProgramObjectARB");
        qglAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)SDL_GL_GetProcAddress("glAttachObjectARB");
        qglLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)SDL_GL_GetProcAddress("glLinkProgramARB");
        qglUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glUseProgramObjectARB");
        qglValidateProgramARB = (PFNGLVALIDATEPROGRAMARBPROC)SDL_GL_GetProcAddress("glValidateProgramARB");
        qglUniform1fARB = (PFNGLUNIFORM1FARBPROC)SDL_GL_GetProcAddress("glUniform1fARB");
        qglUniform2fARB = (PFNGLUNIFORM2FARBPROC)SDL_GL_GetProcAddress("glUniform2fARB");
        qglUniform3fARB = (PFNGLUNIFORM3FARBPROC)SDL_GL_GetProcAddress("glUniform3fARB");
        qglUniform4fARB = (PFNGLUNIFORM4FARBPROC)SDL_GL_GetProcAddress("glUniform4fARB");
        qglUniform1iARB = (PFNGLUNIFORM1IARBPROC)SDL_GL_GetProcAddress("glUniform1iARB");
        qglUniform2iARB = (PFNGLUNIFORM2IARBPROC)SDL_GL_GetProcAddress("glUniform2iARB");
        qglUniform3iARB = (PFNGLUNIFORM3IARBPROC)SDL_GL_GetProcAddress("glUniform3iARB");
        qglUniform4iARB = (PFNGLUNIFORM4IARBPROC)SDL_GL_GetProcAddress("glUniform4iARB");
        qglUniform1fvARB = (PFNGLUNIFORM1FVARBPROC)SDL_GL_GetProcAddress("glUniform1fvARB");
        qglUniform2fvARB = (PFNGLUNIFORM2FVARBPROC)SDL_GL_GetProcAddress("glUniform2fvARB");
        qglUniform3fvARB = (PFNGLUNIFORM3FVARBPROC)SDL_GL_GetProcAddress("glUniform3fvARB");
        qglUniform4fvARB = (PFNGLUNIFORM4FVARBPROC)SDL_GL_GetProcAddress("glUniform4fvARB");
        qglUniform1ivARB = (PFNGLUNIFORM1IVARBPROC)SDL_GL_GetProcAddress("glUniform1ivARB");
        qglUniform2ivARB = (PFNGLUNIFORM2IVARBPROC)SDL_GL_GetProcAddress("glUniform2ivARB");
        qglUniform3ivARB = (PFNGLUNIFORM3IVARBPROC)SDL_GL_GetProcAddress("glUniform3ivARB");
        qglUniform4ivARB = (PFNGLUNIFORM4IVARBPROC)SDL_GL_GetProcAddress("glUniform4ivARB");
        qglUniformMatrix2fvARB = (PFNGLUNIFORMMATRIX2FVARBPROC)SDL_GL_GetProcAddress("glUniformMatrix2fvARB");
        qglUniformMatrix3fvARB = (PFNGLUNIFORMMATRIX3FVARBPROC)SDL_GL_GetProcAddress("glUniformMatrix3fvARB");
        qglUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)SDL_GL_GetProcAddress("glUniformMatrix4fvARB");
        qglGetObjectParameterfvARB = (PFNGLGETOBJECTPARAMETERFVARBPROC)SDL_GL_GetProcAddress("glGetObjectParameterfvARB");
        qglGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)SDL_GL_GetProcAddress("glGetObjectParameterivARB");
        qglGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)SDL_GL_GetProcAddress("glGetInfoLogARB");
        qglGetAttachedObjectsARB = (PFNGLGETATTACHEDOBJECTSARBPROC)SDL_GL_GetProcAddress("glGetAttachedObjectsARB");
        qglGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)SDL_GL_GetProcAddress("glGetUniformLocationARB");
        qglGetActiveUniformARB = (PFNGLGETACTIVEUNIFORMARBPROC)SDL_GL_GetProcAddress("glGetActiveUniformARB");
        qglGetUniformfvARB = (PFNGLGETUNIFORMFVARBPROC)SDL_GL_GetProcAddress("glGetUniformfvARB");
        qglGetUniformivARB = (PFNGLGETUNIFORMIVARBPROC)SDL_GL_GetProcAddress("glGetUniformivARB");
        qglGetShaderSourceARB = (PFNGLGETSHADERSOURCEARBPROC)SDL_GL_GetProcAddress("glGetShaderSourceARB");

        qglBindAttribLocationARB = (PFNGLBINDATTRIBLOCATIONARBPROC)SDL_GL_GetProcAddress("glBindAttribLocationARB");
        qglGetActiveAttribARB = (PFNGLGETACTIVEATTRIBARBPROC)SDL_GL_GetProcAddress("glGetActiveAttribARB");
        qglGetAttribLocationARB = (PFNGLGETATTRIBLOCATIONARBPROC)SDL_GL_GetProcAddress("glGetAttribLocationARB");
        qglEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArrayARB");
        qglDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)SDL_GL_GetProcAddress("glDisableVertexAttribArrayARB");

        qglVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARBPROC)SDL_GL_GetProcAddress("glVertexAttribPointerARB");
    }
    else
    {
        Sys_Error("Shaders not supported");
    }
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

/*
 * Shaders generation section
 */
int checkOpenGLError()
{
    int ret = 0;
    
    for(GLenum  glErr = qglGetError(); glErr != GL_NO_ERROR; glErr = qglGetError())
    {
        ret = 1;
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
    
    return ret;
}


void printInfoLog (GLhandleARB object)
{
    GLint       logLength     = 0;
    GLint       charsWritten  = 0;
    GLcharARB * infoLog;

    checkOpenGLError();                         // check for OpenGL errors
    qglGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength);

    if (logLength > 0)
    {
        infoLog = (GLcharARB*)malloc(logLength);
        qglGetInfoLogARB(object, logLength, &charsWritten, infoLog);
        Sys_DebugLog(GL_LOG_FILENAME, "GL_InfoLog[%d]:", charsWritten);
        Sys_DebugLog(GL_LOG_FILENAME, "%s", (const char*)infoLog);
        free(infoLog);
    }
}


int loadShaderFromBuff(GLhandleARB ShaderObj, const char *source, const char *additionalDefines)
{
    GLint compileStatus = 0;
    if(source)
    {
        GLint source_size = strlen(source);
        if (additionalDefines)
        {
            const char *bufs[2] = { additionalDefines, source };
            const GLint lengths[2] = { (GLint) strlen(additionalDefines), source_size };
            qglShaderSourceARB(ShaderObj, 2, bufs, lengths);
        }
        else
        {
            qglShaderSourceARB(ShaderObj, 1, (const char **)&source, &source_size);
        }
        //Sys_DebugLog(GL_LOG_FILENAME, "source loaded");
        qglCompileShaderARB(ShaderObj);
        //Sys_DebugLog(GL_LOG_FILENAME, "trying to compile");
        if(checkOpenGLError())
        {
            return 0;
        }
        qglGetObjectParameterivARB(ShaderObj, GL_OBJECT_COMPILE_STATUS_ARB, &compileStatus);
        //printInfoLog(ShaderObj);
    }
    return compileStatus != 0;
}


int loadShaderFromFile(GLhandleARB ShaderObj, const char *fileName, const char *additionalDefines)
{
    FILE *file;
    GLint size = 0;
    int ret = 0;

    //Sys_DebugLog(GL_LOG_FILENAME, "GL_Loading %s", fileName);
    file = fopen (fileName, "rb");
    if (file == NULL)
    {
        Sys_DebugLog(GL_LOG_FILENAME, "Error opening %s", fileName);
        return ret;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);

    if(size < 1)
    {
        fclose(file);
        Sys_DebugLog(GL_LOG_FILENAME, "Error loading file %s: size < 1", fileName);
        return ret;
    }

    char *buf = (char*)malloc(size + 1);
    fseek(file, 0, SEEK_SET);
    if(size != fread(buf, 1, size, file))
    {
        Sys_DebugLog(GL_LOG_FILENAME, "Error loading file %s", fileName);
    }
    buf[size] = 0;
    fclose(file);

    ret = loadShaderFromBuff(ShaderObj, buf, additionalDefines);
    free(buf);
    return ret;
}


void BindWhiteTexture()
{
    qglBindTexture(GL_TEXTURE_2D, whiteTexture);
}
