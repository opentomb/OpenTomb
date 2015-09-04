
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
#define SAFE_GET_PROC(func, type, name) func = (type)SDL_GL_GetProcAddress(name)


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
    SAFE_GET_PROC(qglClearIndex, PFNGLCLEARINDEXPROC, "glClearIndex");
    SAFE_GET_PROC(qglClearColor, PFNGLCLEARCOLORPROC, "glClearColor");
    SAFE_GET_PROC(qglClear, PFNGLCLEARPROC, "glClear");
    SAFE_GET_PROC(qglIndexMask, PFNGLINDEXMASKPROC, "glIndexMask");
    SAFE_GET_PROC(qglColorMask, PFNGLCOLORMASKPROC, "glColorMask");
    SAFE_GET_PROC(qglAlphaFunc, PFNGLALPHAFUNCPROC, "glAlphaFunc");
    SAFE_GET_PROC(qglBlendFunc, PFNGLBLENDFUNCPROC, "glBlendFunc");
    SAFE_GET_PROC(qglLogicOp, PFNGLLOGICOPPROC, "glLogicOp");
    SAFE_GET_PROC(qglCullFace, PFNGLCULLFACEPROC, "glCullFace");
    SAFE_GET_PROC(qglFrontFace, PFNGLFRONTFACEPROC, "glFrontFace");
    SAFE_GET_PROC(qglPushAttrib, PFNGLPUSHATTRIBPROC, "glPushAttrib");
    SAFE_GET_PROC(qglPointSize, PFNGLPOINTSIZEPROC, "glPointSize");
    SAFE_GET_PROC(qglLineWidth, PFNGLLINEWIDTHPROC, "glLineWidth");
    SAFE_GET_PROC(qglLineStipple, PFNGLLINESTIPPLEPROC, "glLineStipple");
    SAFE_GET_PROC(qglPolygonMode, PFNGLPOLYGONMODEPROC, "glPolygonMode");
    SAFE_GET_PROC(qglPolygonOffset, PFNGLPOLYGONOFFSETPROC, "glPolygonOffset");
    SAFE_GET_PROC(qglPolygonStipple, PFNGLPOLYGONSTIPPLEPROC, "glPolygonStipple");
    SAFE_GET_PROC(qglGetPolygonStipple, PFNGLGETPOLYGONSTIPPLEPROC, "glGetPolygonStipple");
    SAFE_GET_PROC(qglEdgeFlag, PFNGLEDGEFLAGPROC, "glEdgeFlag");
    SAFE_GET_PROC(qglEdgeFlagv, PFNGLEDGEFLAGVPROC, "glEdgeFlagv");
    SAFE_GET_PROC(qglScissor, PFNGLSCISSORPROC, "glScissor");
    SAFE_GET_PROC(qglClipPlane, PFNGLCLIPPLANEPROC, "glClipPlane");
    SAFE_GET_PROC(qglGetClipPlane, PFNGLGETCLIPPLANEPROC, "glGetClipPlane");
    SAFE_GET_PROC(qglDrawBuffer, PFNGLDRAWBUFFERPROC, "glDrawBuffer");
    SAFE_GET_PROC(qglReadBuffer, PFNGLREADBUFFERPROC, "glReadBuffer");
    SAFE_GET_PROC(qglEnable, PFNGLENABLEPROC, "glEnable");
    SAFE_GET_PROC(qglDisable, PFNGLDISABLEPROC, "glDisable");
    SAFE_GET_PROC(qglIsEnabled, PFNGLISENABLEDPROC, "glIsEnabled");
    SAFE_GET_PROC(qglEnableClientState, PFNGLENABLECLIENTSTATEPROC, "glEnableClientState");
    SAFE_GET_PROC(qglDisableClientState, PFNGLDISABLECLIENTSTATEPROC, "glDisableClientState");
    SAFE_GET_PROC(qglGetError, PFNGLGETERRORPROC, "glGetError");
    SAFE_GET_PROC(qglGetString, PFNGLGETSTRINGPROC, "glGetString");
    SAFE_GET_PROC(qglGetBooleanv, PFNGLGETBOOLEANVPROC, "glGetBooleanv");
    SAFE_GET_PROC(qglGetDoublev, PFNGLGETDOUBLEVPROC, "glGetDoublev");
    SAFE_GET_PROC(qglGetFloatv, PFNGLGETFLOATVPROC, "glGetFloatv");
    SAFE_GET_PROC(qglGetIntegerv, PFNGLGETIINTEGERVPROC, "glGetIntegerv");
    SAFE_GET_PROC(qglPushAttrib, PFNGLPUSHATTRIBPROC, "glPushAttrib");
    SAFE_GET_PROC(qglPopAttrib, PFNGLPOPATTRIBPROC, "glPopAttrib");
    SAFE_GET_PROC(qglPushClientAttrib, PFNGLPUSHCLIENTATTRIBPROC, "glPushClientAttrib");  /* 1.1 */
    SAFE_GET_PROC(qglPopClientAttrib, PFNGLPOPCLIENTATTRIBPROC, "glPopClientAttrib");  /* 1.1 */
    SAFE_GET_PROC(qglRenderMode, PFNGLRENDERMODEPROC, "glRenderMode");
    SAFE_GET_PROC(qglFinish, PFNGLFINISHPROC, "glFinish");
    SAFE_GET_PROC(qglFlush, PFNGLFLUSHPROC, "glFlush");
    SAFE_GET_PROC(qglHint, PFNGLHINTPROC, "glHint");

    /* Depth Buffer */
    SAFE_GET_PROC(qglClearDepth, PFNGLCLEARDEPTHPROC, "glClearDepth");
    SAFE_GET_PROC(qglDepthFunc, PFNGLDEPTHFUNCPROC, "glDepthFunc");
    SAFE_GET_PROC(qglDepthMask, PFNGLDEPTHMASKPROC, "glDepthMask");
    SAFE_GET_PROC(qglDepthRange, PFNGLDEPTHRANGEPROC, "glDepthRange");

    /* Accumulation Buffer */
    SAFE_GET_PROC(qglClearAccum, PFNGLCLEARACCUMPROC, "glClearAccum");               
    SAFE_GET_PROC(qglAccum, PFNGLACCUMPROC, "glAccum");

    /* Transformation */
    SAFE_GET_PROC(qglMatrixMode, PFNGLMATRIXMODEPROC, "glMatrixMode");
    SAFE_GET_PROC(qglOrtho, PFNGLORTHOPROC, "glOrtho");
    SAFE_GET_PROC(qglFrustum, PFNGLFRUSTUMPROC, "glFrustum");
    SAFE_GET_PROC(qglViewport, PFNGLVIEWPORTPROC, "glViewport");
    SAFE_GET_PROC(qglPushMatrix, PFNGLPUSHMATRIXPROC, "glPushMatrix");
    SAFE_GET_PROC(qglPopMatrix, PFNGLPOPMATRIXPROC, "glPopMatrix");
    SAFE_GET_PROC(qglLoadIdentity, PFNGLLOADIDENTITYPROC, "glLoadIdentity");
    SAFE_GET_PROC(qglLoadMatrixd, PFNGLLOADMATRIXDPROC, "glLoadMatrixd");
    SAFE_GET_PROC(qglLoadMatrixf, PFNGLLOADMATRIXFPROC, "glLoadMatrixf");
    SAFE_GET_PROC(qglMultMatrixd, PFNGLMULTMATRIXDPROC, "glMultMatrixd");
    SAFE_GET_PROC(qglMultMatrixf, PFNGLMULTMATRIXFPROC, "glMultMatrixf");
    SAFE_GET_PROC(qglRotated, PFNGLROTATEDPROC, "glRotated");
    SAFE_GET_PROC(qglRotatef, PFNGLROTATEFPROC, "glRotatef");
    SAFE_GET_PROC(qglScaled, PFNGLSCALEDPROC, "glScaled");
    SAFE_GET_PROC(qglScalef, PFNGLSCALEFPROC, "glScalef");
    SAFE_GET_PROC(qglTranslated, PFNGLTRANSLATEDPROC, "glTranslated");
    SAFE_GET_PROC(qglTranslatef, PFNGLTRANSLATEFPROC, "glTranslatef");
    
    /* Raster functions */
    SAFE_GET_PROC(qglPixelZoom, PFNGLPIXELZOOMPROC, "glPixelZoom");
    SAFE_GET_PROC(qglPixelStoref, PFNGLPIXELSTOREFPROC, "glPixelStoref");
    SAFE_GET_PROC(qglPixelStorei, PFNGLPIXELSTOREIPROC, "glPixelStorei");
    SAFE_GET_PROC(qglPixelTransferf, PFNGLPIXELTRANSFERFPROC, "glPixelTransferf");
    SAFE_GET_PROC(qglPixelTransferi, PFNGLPIXELTRANSFERIPROC, "glPixelTransferi");
    SAFE_GET_PROC(qglPixelMapfv, PFNGLPIXELMAPFVPROC, "glPixelMapfv");
    SAFE_GET_PROC(qglPixelMapuiv, PFNGLPIXELMAPUIVPROC, "glPixelMapuiv");
    SAFE_GET_PROC(qglPixelMapusv, PFNGLPIXELMAPUSVPROC, "glPixelMapusv");
    SAFE_GET_PROC(qglGetPixelMapfv, PFNGLGETPIXELMAPFVPROC, "glGetPixelMapfv");
    SAFE_GET_PROC(qglGetPixelMapuiv, PFNGLGETPIXELMAPUIVPROC, "glGetPixelMapuiv");
    SAFE_GET_PROC(qglGetPixelMapusv, PFNGLGETPIXELMAPUSVPROC, "glGetPixelMapusv");
    SAFE_GET_PROC(qglBitmap, PFNGLBITMAPPROC, "glBitmap");
    SAFE_GET_PROC(qglReadPixels, PFNGLREADPIXELSPROC, "glReadPixels");
    SAFE_GET_PROC(qglDrawPixels, PFNGLDRAWPIXELSPROC, "glDrawPixels");
    SAFE_GET_PROC(qglCopyPixels, PFNGLCOPYPIXELSPROC, "glCopyPixels");

    /* Stenciling */
    SAFE_GET_PROC(qglStencilFunc, PFNGLSTENCILFUNCPROC, "glStencilFunc");
    SAFE_GET_PROC(qglStencilMask, PFNGLSTENCILMASKPROC, "glStencilMask");
    SAFE_GET_PROC(qglStencilOp, PFNGLSTENCILOPPROC, "glStencilOp");
    SAFE_GET_PROC(qglClearStencil, PFNGLCLEARSTENCILPROC, "glClearStencil");

    /* Texture mapping */
    SAFE_GET_PROC(qglTexGend, PFNGLTEXGENDPROC, "glTexGend");
    SAFE_GET_PROC(qglTexGenf, PFNGLTEXGENFPROC, "glTexGenf");
    SAFE_GET_PROC(qglTexGeni, PFNGLTEXGENIPROC, "glTexGeni");
    SAFE_GET_PROC(qglTexGendv, PFNGLTEXGENDVPROC, "glTexGendv");
    SAFE_GET_PROC(qglTexGenfv, PFNGLTEXGENFVPROC, "glTexGenfv");
    SAFE_GET_PROC(qglTexGeniv, PFNGLTEXGENIVPROC, "glTexGeniv");
    SAFE_GET_PROC(qglGetTexGendv, PFNGLGETTEXGENDVPROC, "glGetTexGendv");
    SAFE_GET_PROC(qglGetTexGenfv, PFNGLGETTEXGENFVPROC, "glGetTexGenfv");
    SAFE_GET_PROC(qglGetTexGeniv, PFNGLGETTEXGENIVPROC, "glGetTexGeniv");
    SAFE_GET_PROC(qglTexEnvf, PFNGLTEXENVFPROC, "glTexEnvf");
    SAFE_GET_PROC(qglTexEnvi, PFNGLTEXENVIPROC, "glTexEnvi");
    SAFE_GET_PROC(qglTexEnvfv, PFNGLTEXENVFVPROC, "glTexEnvfv");
    SAFE_GET_PROC(qglTexEnviv, PFNGLTEXENVIVPROC, "glTexEnviv");
    SAFE_GET_PROC(qglGetTexEnvfv, PFNGLGETTEXENVFVPROC, "glGetTexEnvfv");
    SAFE_GET_PROC(qglGetTexEnviv, PFNGLGETTEXENVIVPROC, "glGetTexEnviv");
    SAFE_GET_PROC(qglTexParameterf, PFNGLTEXPARAMETERFPROC, "glTexParameterf");
    SAFE_GET_PROC(qglTexParameteri, PFNGLTEXPARAMETERIPROC, "glTexParameteri");
    SAFE_GET_PROC(qglTexParameterfv, PFNGLTEXPARAMETERFVPROC, "glTexParameterfv");
    SAFE_GET_PROC(qglTexParameteriv, PFNGLTEXPARAMETERIVPROC, "glTexParameteriv");
    SAFE_GET_PROC(qglGetTexParameterfv, PFNGLGETTEXPARAMETERFVPROC, "glGetTexParameterfv");
    SAFE_GET_PROC(qglGetTexParameteriv, PFNGLGETTEXPARAMETERIVPROC, "glGetTexParameteriv");
    SAFE_GET_PROC(qglGetTexLevelParameterfv, PFNGLGETTEXLEVELPARAMETERFVPROC, "glGetTexLevelParameterfv");
    SAFE_GET_PROC(qglGetTexLevelParameteriv, PFNGLGETTEXLEVELPARAMETERIVPROC, "glGetTexLevelParameteriv");
    SAFE_GET_PROC(qglTexImage1D, PFNGLTEXIMAGE1DPROC, "glTexImage1D");
    SAFE_GET_PROC(qglTexImage2D, PFNGLTEXIMAGE2DPROC, "glTexImage2D");
    SAFE_GET_PROC(qglGetTexImage, PFNGLGETTEXIMAGEPROC, "glGetTexImage");
    
    /* 1.1 functions */
    /* texture objects */
    SAFE_GET_PROC(qglGenTextures, PFNGLGENTEXTURESPROC, "glGenTextures");
    SAFE_GET_PROC(qglDeleteTextures, PFNGLDELETETEXTURESPROC, "glDeleteTextures");
    SAFE_GET_PROC(qglBindTexture, PFNGLBINDTEXTUREPROC, "glBindTexture");
    SAFE_GET_PROC(qglPrioritizeTextures, PFNGLPRIORITIZETEXTURESPROC, "glPrioritizeTextures");
    SAFE_GET_PROC(qglAreTexturesResident, PFNGLARETEXTURESRESIDENTPROC, "glAreTexturesResident");
    SAFE_GET_PROC(qglIsTexture, PFNGLISTEXTUREPROC, "glIsTexture");
    /* texture mapping */
    SAFE_GET_PROC(qglTexSubImage1D, PFNGLTEXSUBIMAGE1DPROC, "glTexSubImage1D");
    SAFE_GET_PROC(qglTexSubImage2D, PFNGLTEXSUBIMAGE2DPROC, "glTexSubImage2D");
    SAFE_GET_PROC(qglCopyTexImage1D, PFNGLCOPYTEXIMAGE1DPROC, "glCopyTexImage1D");
    SAFE_GET_PROC(qglCopyTexImage2D, PFNGLCOPYTEXIMAGE2DPROC, "glCopyTexImage2D");
    SAFE_GET_PROC(qglCopyTexSubImage1D, PFNGLCOPYTEXSUBIMAGE1DPROC, "glCopyTexSubImage1D");
    SAFE_GET_PROC(qglCopyTexSubImage2D, PFNGLCOPYTEXSUBIMAGE2DPROC, "glCopyTexSubImage2D");
    /* vertex arrays */
    SAFE_GET_PROC(qglVertexPointer, PFNGLVERTEXPOINTERPROC, "glVertexPointer");
    SAFE_GET_PROC(qglNormalPointer, PFNGLNORMALPOINTERPROC, "glNormalPointer");
    SAFE_GET_PROC(qglColorPointer, PFNGLCOLORPOINTERPROC, "glColorPointer");
    SAFE_GET_PROC(qglIndexPointer, PFNGLINDEXPOINTERPROC, "glIndexPointer");
    SAFE_GET_PROC(qglTexCoordPointer, PFNGLTEXCOORDPOINTERPROC, "glTexCoordPointer");
    SAFE_GET_PROC(qglEdgeFlagPointer, PFNGLEDGEFLAGPOINTERPROC, "glEdgeFlagPointer");
    SAFE_GET_PROC(qglGetPointerv, PFNGLGETPOINTERVPROC, "glGetPointerv");
    SAFE_GET_PROC(qglArrayElement, PFNGLARRAYELEMENTPROC, "glArrayElement");
    SAFE_GET_PROC(qglDrawArrays, PFNGLDRAWARRAYSPROC, "glDrawArrays");
    SAFE_GET_PROC(qglDrawElements, PFNGLDRAWELEMENTSPROC, "glDrawElements");
    SAFE_GET_PROC(qglInterleavedArrays, PFNGLINTERLEAVEDARRAYSPROC, "glInterleavedArrays");
    
    const char* buf = (const char*)qglGetString(GL_EXTENSIONS);
    engine_gl_ext_str = (char*)malloc(strlen(buf) + 1);
    strcpy(engine_gl_ext_str, buf);
    
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
        SAFE_GET_PROC(qglBindBufferARB, PFNGLBINDBUFFERARBPROC, "glBindBufferARB");
        SAFE_GET_PROC(qglDeleteBuffersARB, PFNGLDELETEBUFFERSARBPROC, "glDeleteBuffersARB");
        SAFE_GET_PROC(qglGenBuffersARB, PFNGLGENBUFFERSARBPROC, "glGenBuffersARB");
        SAFE_GET_PROC(qglIsBufferARB, PFNGLISBUFFERARBPROC, "glIsBufferARB");
        SAFE_GET_PROC(qglBufferDataARB, PFNGLBUFFERDATAARBPROC, "glBufferDataARB");
        SAFE_GET_PROC(qglBufferSubDataARB, PFNGLBUFFERSUBDATAARBPROC, "glBufferSubDataARB");
        SAFE_GET_PROC(qglGetBufferSubDataARB, PFNGLGETBUFFERSUBDATAARBPROC, "glGetBufferSubDataARB");
        SAFE_GET_PROC(qglMapBufferARB, PFNGLMAPBUFFERARBPROC, "glMapBufferARB");
        SAFE_GET_PROC(qglUnmapBufferARB, PFNGLUNMAPBUFFERARBPROC, "glUnmapBufferARB");
        SAFE_GET_PROC(qglGetBufferParameterivARB, PFNGLGETBUFFERPARAMETERIVARBPROC, "glGetBufferParameterivARB");
        SAFE_GET_PROC(qglGetBufferPointervARB, PFNGLGETBUFFERPOINTERVARBPROC, "glGetBufferPointervARB");

        SAFE_GET_PROC(qglActiveTextureARB, PFNGLACTIVETEXTUREARBPROC, "glActiveTextureARB");
        SAFE_GET_PROC(qglClientActiveTextureARB, PFNGLCLIENTACTIVETEXTUREARBPROC, "glClientActiveTextureARB");

        SAFE_GET_PROC(qglMultiTexCoord1dARB, PFNGLMULTITEXCOORD1DARBPROC, "glMultiTexCoord1dARB");
        SAFE_GET_PROC(qglMultiTexCoord1dvARB, PFNGLMULTITEXCOORD1DVARBPROC, "glMultiTexCoord1dvARB");
        SAFE_GET_PROC(qglMultiTexCoord1fARB, PFNGLMULTITEXCOORD1FARBPROC, "glMultiTexCoord1fARB");
        SAFE_GET_PROC(qglMultiTexCoord1fvARB, PFNGLMULTITEXCOORD1FVARBPROC, "glMultiTexCoord1fvARB");
        SAFE_GET_PROC(qglMultiTexCoord1iARB, PFNGLMULTITEXCOORD1IARBPROC, "glMultiTexCoord1iARB");
        SAFE_GET_PROC(qglMultiTexCoord1ivARB, PFNGLMULTITEXCOORD1IVARBPROC, "glMultiTexCoord1ivARB");
        SAFE_GET_PROC(qglMultiTexCoord1sARB, PFNGLMULTITEXCOORD1SARBPROC, "glMultiTexCoord1sARB");
        SAFE_GET_PROC(qglMultiTexCoord1svARB, PFNGLMULTITEXCOORD1SVARBPROC, "glMultiTexCoord1svARB");

        SAFE_GET_PROC(qglMultiTexCoord2dARB, PFNGLMULTITEXCOORD2DARBPROC, "glMultiTexCoord2dARB");
        SAFE_GET_PROC(qglMultiTexCoord2dvARB, PFNGLMULTITEXCOORD2DVARBPROC, "glMultiTexCoord2dvARB");
        SAFE_GET_PROC(qglMultiTexCoord2fARB, PFNGLMULTITEXCOORD2FARBPROC, "glMultiTexCoord2fARB");
        SAFE_GET_PROC(qglMultiTexCoord2fvARB, PFNGLMULTITEXCOORD2FVARBPROC, "glMultiTexCoord2fvARB");
        SAFE_GET_PROC(qglMultiTexCoord2iARB, PFNGLMULTITEXCOORD2IARBPROC, "glMultiTexCoord2iARB");
        SAFE_GET_PROC(qglMultiTexCoord2ivARB, PFNGLMULTITEXCOORD2IVARBPROC, "glMultiTexCoord2ivARB");
        SAFE_GET_PROC(qglMultiTexCoord2sARB, PFNGLMULTITEXCOORD2SARBPROC, "glMultiTexCoord2sARB");
        SAFE_GET_PROC(qglMultiTexCoord2svARB, PFNGLMULTITEXCOORD2SVARBPROC, "glMultiTexCoord2svARB");

        SAFE_GET_PROC(qglMultiTexCoord3dARB, PFNGLMULTITEXCOORD3DARBPROC, "glMultiTexCoord3dARB");
        SAFE_GET_PROC(qglMultiTexCoord3dvARB, PFNGLMULTITEXCOORD3DVARBPROC, "glMultiTexCoord3dvARB");
        SAFE_GET_PROC(qglMultiTexCoord3fARB, PFNGLMULTITEXCOORD3FARBPROC, "glMultiTexCoord3fARB");
        SAFE_GET_PROC(qglMultiTexCoord3fvARB, PFNGLMULTITEXCOORD3FVARBPROC, "glMultiTexCoord3fvARB");
        SAFE_GET_PROC(qglMultiTexCoord3iARB, PFNGLMULTITEXCOORD3IARBPROC, "glMultiTexCoord3iARB");
        SAFE_GET_PROC(qglMultiTexCoord3ivARB, PFNGLMULTITEXCOORD3IVARBPROC, "glMultiTexCoord3ivARB");
        SAFE_GET_PROC(qglMultiTexCoord3sARB, PFNGLMULTITEXCOORD3SARBPROC, "glMultiTexCoord3sARB");
        SAFE_GET_PROC(qglMultiTexCoord3svARB, PFNGLMULTITEXCOORD3SVARBPROC, "glMultiTexCoord3svARB");

        SAFE_GET_PROC(qglMultiTexCoord4dARB, PFNGLMULTITEXCOORD4DARBPROC, "glMultiTexCoord4dARB");
        SAFE_GET_PROC(qglMultiTexCoord4dvARB, PFNGLMULTITEXCOORD4DVARBPROC, "glMultiTexCoord4dvARB");
        SAFE_GET_PROC(qglMultiTexCoord4fARB, PFNGLMULTITEXCOORD4FARBPROC, "glMultiTexCoord4fARB");
        SAFE_GET_PROC(qglMultiTexCoord4fvARB, PFNGLMULTITEXCOORD4FVARBPROC, "glMultiTexCoord4fvARB");
        SAFE_GET_PROC(qglMultiTexCoord4iARB, PFNGLMULTITEXCOORD4IARBPROC, "glMultiTexCoord4iARB");
        SAFE_GET_PROC(qglMultiTexCoord4ivARB, PFNGLMULTITEXCOORD4IVARBPROC, "glMultiTexCoord4ivARB");
        SAFE_GET_PROC(qglMultiTexCoord4sARB, PFNGLMULTITEXCOORD4SARBPROC, "glMultiTexCoord4sARB");
        SAFE_GET_PROC(qglMultiTexCoord4svARB, PFNGLMULTITEXCOORD4SVARBPROC, "glMultiTexCoord4svARB");

        SAFE_GET_PROC(qglBindVertexArray, PFNGLBINDVERTEXARRAYPROC, "glBindVertexArray");
        SAFE_GET_PROC(qglDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC, "glDeleteVertexArrays");
        SAFE_GET_PROC(qglGenVertexArrays, PFNGLGENVERTEXARRAYSPROC, "glGenVertexArrays");
        SAFE_GET_PROC(qglIsVertexArray, PFNGLISVERTEXARRAYPROC, "glIsVertexArray");

        SAFE_GET_PROC(qglGenerateMipmap, PFNGLGENERATEMIPMAPPROC, "glGenerateMipmap");
    }
    else
    {
        fprintf(stderr, "VBOs not supported");
        abort();
    }
    if(IsGLExtensionSupported("GL_ARB_shading_language_100"))
    {
        SAFE_GET_PROC(qglDeleteObjectARB, PFNGLDELETEOBJECTARBPROC, "glDeleteObjectARB");
        SAFE_GET_PROC(qglGetHandleARB, PFNGLGETHANDLEARBPROC, "glGetHandleARB");
        SAFE_GET_PROC(qglDetachObjectARB, PFNGLDETACHOBJECTARBPROC, "glDetachObjectARB");
        SAFE_GET_PROC(qglCreateShaderObjectARB, PFNGLCREATESHADEROBJECTARBPROC, "glCreateShaderObjectARB");
        SAFE_GET_PROC(qglShaderSourceARB, PFNGLSHADERSOURCEARBPROC, "glShaderSourceARB");
        SAFE_GET_PROC(qglCompileShaderARB, PFNGLCOMPILESHADERARBPROC, "glCompileShaderARB");
        SAFE_GET_PROC(qglCreateProgramObjectARB, PFNGLCREATEPROGRAMOBJECTARBPROC, "glCreateProgramObjectARB");
        SAFE_GET_PROC(qglAttachObjectARB, PFNGLATTACHOBJECTARBPROC, "glAttachObjectARB");
        SAFE_GET_PROC(qglLinkProgramARB, PFNGLLINKPROGRAMARBPROC, "glLinkProgramARB");
        SAFE_GET_PROC(qglUseProgramObjectARB, PFNGLUSEPROGRAMOBJECTARBPROC, "glUseProgramObjectARB");
        SAFE_GET_PROC(qglValidateProgramARB, PFNGLVALIDATEPROGRAMARBPROC, "glValidateProgramARB");
        SAFE_GET_PROC(qglUniform1fARB, PFNGLUNIFORM1FARBPROC, "glUniform1fARB");
        SAFE_GET_PROC(qglUniform2fARB, PFNGLUNIFORM2FARBPROC, "glUniform2fARB");
        SAFE_GET_PROC(qglUniform3fARB, PFNGLUNIFORM3FARBPROC, "glUniform3fARB");
        SAFE_GET_PROC(qglUniform4fARB, PFNGLUNIFORM4FARBPROC, "glUniform4fARB");
        SAFE_GET_PROC(qglUniform1iARB, PFNGLUNIFORM1IARBPROC, "glUniform1iARB");
        SAFE_GET_PROC(qglUniform2iARB, PFNGLUNIFORM2IARBPROC, "glUniform2iARB");
        SAFE_GET_PROC(qglUniform3iARB, PFNGLUNIFORM3IARBPROC, "glUniform3iARB");
        SAFE_GET_PROC(qglUniform4iARB, PFNGLUNIFORM4IARBPROC, "glUniform4iARB");
        SAFE_GET_PROC(qglUniform1fvARB, PFNGLUNIFORM1FVARBPROC, "glUniform1fvARB");
        SAFE_GET_PROC(qglUniform2fvARB, PFNGLUNIFORM2FVARBPROC, "glUniform2fvARB");
        SAFE_GET_PROC(qglUniform3fvARB, PFNGLUNIFORM3FVARBPROC, "glUniform3fvARB");
        SAFE_GET_PROC(qglUniform4fvARB, PFNGLUNIFORM4FVARBPROC, "glUniform4fvARB");
        SAFE_GET_PROC(qglUniform1ivARB, PFNGLUNIFORM1IVARBPROC, "glUniform1ivARB");
        SAFE_GET_PROC(qglUniform2ivARB, PFNGLUNIFORM2IVARBPROC, "glUniform2ivARB");
        SAFE_GET_PROC(qglUniform3ivARB, PFNGLUNIFORM3IVARBPROC, "glUniform3ivARB");
        SAFE_GET_PROC(qglUniform4ivARB, PFNGLUNIFORM4IVARBPROC, "glUniform4ivARB");
        SAFE_GET_PROC(qglUniformMatrix2fvARB, PFNGLUNIFORMMATRIX2FVARBPROC, "glUniformMatrix2fvARB");
        SAFE_GET_PROC(qglUniformMatrix3fvARB, PFNGLUNIFORMMATRIX3FVARBPROC, "glUniformMatrix3fvARB");
        SAFE_GET_PROC(qglUniformMatrix4fvARB, PFNGLUNIFORMMATRIX4FVARBPROC, "glUniformMatrix4fvARB");
        SAFE_GET_PROC(qglGetObjectParameterfvARB, PFNGLGETOBJECTPARAMETERFVARBPROC, "glGetObjectParameterfvARB");
        SAFE_GET_PROC(qglGetObjectParameterivARB, PFNGLGETOBJECTPARAMETERIVARBPROC, "glGetObjectParameterivARB");
        SAFE_GET_PROC(qglGetInfoLogARB, PFNGLGETINFOLOGARBPROC, "glGetInfoLogARB");
        SAFE_GET_PROC(qglGetAttachedObjectsARB, PFNGLGETATTACHEDOBJECTSARBPROC, "glGetAttachedObjectsARB");
        SAFE_GET_PROC(qglGetUniformLocationARB, PFNGLGETUNIFORMLOCATIONARBPROC, "glGetUniformLocationARB");
        SAFE_GET_PROC(qglGetActiveUniformARB, PFNGLGETACTIVEUNIFORMARBPROC, "glGetActiveUniformARB");
        SAFE_GET_PROC(qglGetUniformfvARB, PFNGLGETUNIFORMFVARBPROC, "glGetUniformfvARB");
        SAFE_GET_PROC(qglGetUniformivARB, PFNGLGETUNIFORMIVARBPROC, "glGetUniformivARB");
        SAFE_GET_PROC(qglGetShaderSourceARB, PFNGLGETSHADERSOURCEARBPROC, "glGetShaderSourceARB");

        SAFE_GET_PROC(qglBindAttribLocationARB, PFNGLBINDATTRIBLOCATIONARBPROC, "glBindAttribLocationARB");
        SAFE_GET_PROC(qglGetActiveAttribARB, PFNGLGETACTIVEATTRIBARBPROC, "glGetActiveAttribARB");
        SAFE_GET_PROC(qglGetAttribLocationARB, PFNGLGETATTRIBLOCATIONARBPROC, "glGetAttribLocationARB");
        SAFE_GET_PROC(qglEnableVertexAttribArrayARB, PFNGLENABLEVERTEXATTRIBARRAYARBPROC, "glEnableVertexAttribArrayARB");
        SAFE_GET_PROC(qglDisableVertexAttribArrayARB, PFNGLDISABLEVERTEXATTRIBARRAYARBPROC, "glDisableVertexAttribArrayARB");

        SAFE_GET_PROC(qglVertexAttribPointerARB, PFNGLVERTEXATTRIBPOINTERARBPROC, "glVertexAttribPointerARB");
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
    for( ; ; )
    {
        GLenum  glErr = qglGetError();
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
        Sys_DebugLog(GL_LOG_FILENAME, "source loaded");
        qglCompileShaderARB(ShaderObj);
        Sys_DebugLog(GL_LOG_FILENAME, "trying to compile");
        if(checkOpenGLError())
        {
            return 0;
        }
        qglGetObjectParameterivARB(ShaderObj, GL_OBJECT_COMPILE_STATUS_ARB, &compileStatus);
        printInfoLog(ShaderObj);
    }
    return compileStatus != 0;
}

int loadShaderFromFile(GLhandleARB ShaderObj, const char *fileName, const char *additionalDefines)
{
    FILE *file;
    GLint size = 0;
    int ret = 0;

    Sys_DebugLog(GL_LOG_FILENAME, "GL_Loading %s", fileName);
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
    fread(buf, 1, size, file);
    buf[size] = 0;
    fclose(file);

    //printf ( "source = %s\n", buf );
    ret = loadShaderFromBuff(ShaderObj, buf, additionalDefines);
    free(buf);
    return ret;
}

void BindWhiteTexture()
{
    qglBindTexture(GL_TEXTURE_2D, whiteTexture);
}
