
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


/* Miscellaneous */
typedef void (APIENTRYP PFNGLCLEARINDEXPROC) (GLfloat c);                  
typedef void (APIENTRYP PFNGLCLEARCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);                  
typedef void (APIENTRYP PFNGLCLEARPROC) (GLbitfield mask);                  
typedef void (APIENTRYP PFNGLINDEXMASKPROC) (GLuint mask);                  
typedef void (APIENTRYP PFNGLCOLORMASKPROC) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);                  
typedef void (APIENTRYP PFNGLALPHAFUNCPROC) (GLenum func, GLclampf ref);                  
typedef void (APIENTRYP PFNGLBLENDFUNCPROC) (GLenum sfactor, GLenum dfactor);                  
typedef void (APIENTRYP PFNGLLOGICOPPROC) (GLenum opcode);                  
typedef void (APIENTRYP PFNGLCULLFACEPROC) (GLenum mode);                  
typedef void (APIENTRYP PFNGLFRONTFACEPROC) (GLenum mode);                  
typedef void (APIENTRYP PFNGLPOINTSIZEPROC) (GLfloat size);                  
typedef void (APIENTRYP PFNGLLINEWIDTHPROC) (GLfloat width);                  
typedef void (APIENTRYP PFNGLLINESTIPPLEPROC) (GLint factor, GLushort pattern);                  
typedef void (APIENTRYP PFNGLPOLYGONMODEPROC) (GLenum face, GLenum mode);                  
typedef void (APIENTRYP PFNGLPOLYGONOFFSETPROC) (GLfloat factor, GLfloat units);                  
typedef void (APIENTRYP PFNGLGETPOLYGONSTIPPLEPROC) (GLubyte *mask);    
typedef void (APIENTRYP PFNGLPOLYGONSTIPPLEPROC) (const GLubyte *mask);    
typedef void (APIENTRYP PFNGLEDGEFLAGPROC) (GLboolean flag);                  
typedef void (APIENTRYP PFNGLEDGEFLAGVPROC) (const GLboolean *flag);                  
typedef void (APIENTRYP PFNGLSCISSORPROC) (GLint x, GLint y, GLsizei width, GLsizei height);                  
typedef void (APIENTRYP PFNGLCLIPPLANEPROC) (GLenum plane, const GLdouble *equation);                  
typedef void (APIENTRYP PFNGLGETCLIPPLANEPROC) (GLenum plane, GLdouble *equation);                  
typedef void (APIENTRYP PFNGLDRAWBUFFERPROC) (GLenum mode);                  
typedef void (APIENTRYP PFNGLREADBUFFERPROC) (GLenum mode);                  
typedef void (APIENTRYP PFNGLENABLEPROC) (GLenum cap);
typedef void (APIENTRYP PFNGLDISABLEPROC) (GLenum cap);
typedef GLboolean (APIENTRYP PFNGLISENABLEDPROC) (GLenum cap);
typedef void (APIENTRYP PFNGLENABLECLIENTSTATEPROC) (GLenum cap);
typedef void (APIENTRYP PFNGLDISABLECLIENTSTATEPROC) (GLenum cap);
typedef GLenum (APIENTRYP PFNGLGETERRORPROC) (void);
typedef const GLubyte* (APIENTRYP PFNGLGETSTRINGPROC) (GLenum name);
typedef void (APIENTRYP PFNGLGETBOOLEANVPROC) (GLenum pname, GLboolean *params);
typedef void (APIENTRYP PFNGLGETDOUBLEVPROC) (GLenum pname, GLdouble *params);
typedef void (APIENTRYP PFNGLGETFLOATVPROC) (GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETIINTEGERVPROC) (GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLPUSHATTRIBPROC) (GLbitfield mask);      
typedef void (APIENTRYP PFNGLPOPATTRIBPROC) (void);                  
typedef void (APIENTRYP PFNGLPUSHCLIENTATTRIBPROC) (GLbitfield mask);                  
typedef void (APIENTRYP PFNGLPOPCLIENTATTRIBPROC) (void);                  
typedef GLint (APIENTRYP PFNGLRENDERMODEPROC) (GLenum mode);                  
typedef void (APIENTRYP PFNGLFINISHPROC) (void);                  
typedef void (APIENTRYP PFNGLFLUSHPROC) (void);                  
typedef void (APIENTRYP PFNGLHINTPROC) (GLenum target, GLenum mode);                  

/* Depth Buffer */
typedef void (APIENTRYP PFNGLCLEARDEPTHPROC) (GLclampd depth);                  
typedef void (APIENTRYP PFNGLDEPTHFUNCPROC) (GLenum func);                  
typedef void (APIENTRYP PFNGLDEPTHMASKPROC) (GLboolean flag);                  
typedef void (APIENTRYP PFNGLDEPTHRANGEPROC) (GLclampd near_val, GLclampd far_val);                  

/* Accumulation Buffer */
typedef void (APIENTRYP PFNGLCLEARACCUMPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);                  
typedef void (APIENTRYP PFNGLACCUMPROC) (GLenum op, GLfloat value);                  

/* Transformation */
typedef void (APIENTRYP PFNGLMATRIXMODEPROC) (GLenum mode);
typedef void (APIENTRYP PFNGLORTHOPROC) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
typedef void (APIENTRYP PFNGLFRUSTUMPROC) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
typedef void (APIENTRYP PFNGLVIEWPORTPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLPUSHMATRIXPROC) (void);
typedef void (APIENTRYP PFNGLPOPMATRIXPROC) (void);
typedef void (APIENTRYP PFNGLLOADIDENTITYPROC) (void);
typedef void (APIENTRYP PFNGLLOADMATRIXDPROC) (const GLdouble *m);
typedef void (APIENTRYP PFNGLLOADMATRIXFPROC) (const GLfloat *m);
typedef void (APIENTRYP PFNGLMULTMATRIXDPROC) (const GLdouble *m);
typedef void (APIENTRYP PFNGLMULTMATRIXFPROC) (const GLfloat *m);
typedef void (APIENTRYP PFNGLROTATEDPROC) (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRYP PFNGLROTATEFPROC) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRYP PFNGLSCALEDPROC) (GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRYP PFNGLSCALEFPROC) (GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRYP PFNGLTRANSLATEDPROC) (GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRYP PFNGLTRANSLATEFPROC) (GLfloat x, GLfloat y, GLfloat z);

/* Raster functions */
typedef void (APIENTRYP PFNGLPIXELZOOMPROC) (GLfloat xfactor, GLfloat yfactor);
typedef void (APIENTRYP PFNGLPIXELSTOREFPROC) (GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLPIXELSTOREIPROC) (GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLPIXELTRANSFERFPROC) (GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLPIXELTRANSFERIPROC) (GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLPIXELMAPFVPROC) (GLenum map, GLint mapsize, const GLfloat *values);
typedef void (APIENTRYP PFNGLPIXELMAPUIVPROC) (GLenum map, GLint mapsize, const GLuint *values);
typedef void (APIENTRYP PFNGLPIXELMAPUSVPROC) (GLenum map, GLint mapsize, const GLushort *values);
typedef void (APIENTRYP PFNGLGETPIXELMAPFVPROC) (GLenum map, GLfloat *values);
typedef void (APIENTRYP PFNGLGETPIXELMAPUIVPROC) (GLenum map, GLuint *values);
typedef void (APIENTRYP PFNGLGETPIXELMAPUSVPROC) (GLenum map, GLushort *values);
typedef void (APIENTRYP PFNGLBITMAPPROC) (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
typedef void (APIENTRYP PFNGLREADPIXELSPROC) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
typedef void (APIENTRYP PFNGLDRAWPIXELSPROC) (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLCOPYPIXELSPROC) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);

/* Stenciling */
typedef void (APIENTRYP PFNGLSTENCILFUNCPROC) (GLenum func, GLint ref, GLuint mask);
typedef void (APIENTRYP PFNGLSTENCILMASKPROC) (GLuint mask);
typedef void (APIENTRYP PFNGLSTENCILOPPROC) (GLenum fail, GLenum zfail, GLenum zpass);
typedef void (APIENTRYP PFNGLCLEARSTENCILPROC) (GLint s);

/* Texture mapping */
typedef void (APIENTRYP PFNGLTEXGENDPROC) (GLenum coord, GLenum pname, GLdouble param);
typedef void (APIENTRYP PFNGLTEXGENFPROC) (GLenum coord, GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLTEXGENIPROC) (GLenum coord, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLTEXGENDVPROC) (GLenum coord, GLenum pname, const GLdouble *params);
typedef void (APIENTRYP PFNGLTEXGENFVPROC) (GLenum coord, GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLTEXGENIVPROC) (GLenum coord, GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLGETTEXGENDVPROC) (GLenum coord, GLenum pname, GLdouble *params);
typedef void (APIENTRYP PFNGLGETTEXGENFVPROC) (GLenum coord, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETTEXGENIVPROC) (GLenum coord, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLTEXENVFPROC) (GLenum target, GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLTEXENVIPROC) (GLenum target, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLTEXENVFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLTEXENVIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLGETTEXENVFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETTEXENVIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLTEXPARAMETERFPROC) (GLenum target, GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLTEXPARAMETERIPROC) (GLenum target, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLTEXPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLTEXPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLGETTEXPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETTEXPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETTEXLEVELPARAMETERFVPROC) (GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETTEXLEVELPARAMETERIVPROC) (GLenum target, GLint level, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLTEXIMAGE1DPROC) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLTEXIMAGE2DPROC) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLGETTEXIMAGEPROC) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);

/* 1.1 functions */
/* texture objects */
typedef void (APIENTRYP PFNGLGENTEXTURESPROC) (GLsizei n, GLuint *textures);
typedef void (APIENTRYP PFNGLDELETETEXTURESPROC) (GLsizei n, const GLuint *textures);
typedef void (APIENTRYP PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
typedef void (APIENTRYP PFNGLPRIORITIZETEXTURESPROC) (GLsizei n, const GLuint *textures, const GLclampf *priorities);
typedef GLboolean (APIENTRYP PFNGLARETEXTURESRESIDENTPROC) (GLsizei n, const GLuint *textures, GLboolean *residences);
typedef GLboolean (APIENTRYP PFNGLISTEXTUREPROC) (GLuint texture);
/* texture mapping */
typedef void (APIENTRYP PFNGLTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLCOPYTEXIMAGE1DPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (APIENTRYP PFNGLCOPYTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (APIENTRYP PFNGLCOPYTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (APIENTRYP PFNGLCOPYTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
/* vertex arrays */
typedef void (APIENTRYP PFNGLVERTEXPOINTERPROC) (GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (APIENTRYP PFNGLNORMALPOINTERPROC) (GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (APIENTRYP PFNGLCOLORPOINTERPROC) (GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (APIENTRYP PFNGLINDEXPOINTERPROC) (GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (APIENTRYP PFNGLTEXCOORDPOINTERPROC) (GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (APIENTRYP PFNGLEDGEFLAGPOINTERPROC) (GLsizei stride, const GLvoid *ptr);
typedef void (APIENTRYP PFNGLGETPOINTERVPROC) (GLenum pname, GLvoid **params);
typedef void (APIENTRYP PFNGLARRAYELEMENTPROC) (GLint i);
typedef void (APIENTRYP PFNGLDRAWARRAYSPROC) (GLenum mode, GLint first, GLsizei count);
typedef void (APIENTRYP PFNGLDRAWELEMENTSPROC) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (APIENTRYP PFNGLINTERLEAVEDARRAYSPROC) (GLenum format, GLsizei stride, const GLvoid *pointer);

/*******************************************************************************
 ******************************************************************************
 *******************************************************************************/

/* Miscellaneous */
extern PFNGLCLEARINDEXPROC qglClearIndex;
extern PFNGLCLEARCOLORPROC qglClearColor;
extern PFNGLCLEARPROC qglClear;
extern PFNGLINDEXMASKPROC qglIndexMask;
extern PFNGLCOLORMASKPROC qglColorMask;
extern PFNGLALPHAFUNCPROC qglAlphaFunc;
extern PFNGLBLENDFUNCPROC qglBlendFunc;
extern PFNGLLOGICOPPROC qglLogicOp;
extern PFNGLCULLFACEPROC qglCullFace;
extern PFNGLFRONTFACEPROC qglFrontFace;
extern PFNGLPOINTSIZEPROC qglPointSize;
extern PFNGLLINEWIDTHPROC qglLineWidth;
extern PFNGLLINESTIPPLEPROC qglLineStipple;
extern PFNGLPOLYGONMODEPROC qglPolygonMode;
extern PFNGLPOLYGONOFFSETPROC qglPolygonOffset;
extern PFNGLPOLYGONSTIPPLEPROC qglPolygonStipple;
extern PFNGLGETPOLYGONSTIPPLEPROC qglGetPolygonStipple;
extern PFNGLEDGEFLAGPROC qglEdgeFlag;
extern PFNGLEDGEFLAGVPROC qglEdgeFlagv;
extern PFNGLSCISSORPROC qglScissor;
extern PFNGLCLIPPLANEPROC qglClipPlane;
extern PFNGLGETCLIPPLANEPROC qglGetClipPlane;
extern PFNGLDRAWBUFFERPROC qglDrawBuffer;
extern PFNGLREADBUFFERPROC qglReadBuffer;
extern PFNGLENABLEPROC qglEnable;
extern PFNGLDISABLEPROC qglDisable;
extern PFNGLISENABLEDPROC qglIsEnabled;
extern PFNGLENABLECLIENTSTATEPROC qglEnableClientState;
extern PFNGLDISABLECLIENTSTATEPROC qglDisableClientState;
extern PFNGLGETERRORPROC qglGetError;
extern PFNGLGETSTRINGPROC qglGetString;
extern PFNGLGETBOOLEANVPROC qglGetBooleanv;
extern PFNGLGETDOUBLEVPROC qglGetDoublev;
extern PFNGLGETFLOATVPROC qglGetFloatv;
extern PFNGLGETIINTEGERVPROC qglGetIntegerv;
extern PFNGLPUSHATTRIBPROC qglPushAttrib;
extern PFNGLPOPATTRIBPROC qglPopAttrib;
extern PFNGLPUSHCLIENTATTRIBPROC qglPushClientAttrib;  /* 1.1 */
extern PFNGLPOPCLIENTATTRIBPROC qglPopClientAttrib;  /* 1.1 */
extern PFNGLRENDERMODEPROC qglRenderMode;
extern PFNGLFINISHPROC qglFinish;
extern PFNGLFLUSHPROC qglFlush;
extern PFNGLHINTPROC qglHint;

/* Depth Buffer */
extern PFNGLCLEARDEPTHPROC qglClearDepth;
extern PFNGLDEPTHFUNCPROC qglDepthFunc;
extern PFNGLDEPTHMASKPROC qglDepthMask;
extern PFNGLDEPTHRANGEPROC qglDepthRange;

/* Accumulation Buffer */
extern PFNGLCLEARACCUMPROC qglClearAccum;               
extern PFNGLACCUMPROC qglAccum;

/* Transformation */
extern PFNGLMATRIXMODEPROC qglMatrixMode;
extern PFNGLORTHOPROC qglOrtho;
extern PFNGLFRUSTUMPROC qglFrustum;
extern PFNGLVIEWPORTPROC qglViewport;
extern PFNGLPUSHMATRIXPROC qglPushMatrix;
extern PFNGLPOPMATRIXPROC qglPopMatrix;
extern PFNGLLOADIDENTITYPROC qglLoadIdentity;
extern PFNGLLOADMATRIXDPROC qglLoadMatrixd;
extern PFNGLLOADMATRIXFPROC qglLoadMatrixf;
extern PFNGLMULTMATRIXDPROC qglMultMatrixd;
extern PFNGLMULTMATRIXFPROC qglMultMatrixf;
extern PFNGLROTATEDPROC qglRotated;
extern PFNGLROTATEFPROC qglRotatef;
extern PFNGLSCALEDPROC qglScaled;
extern PFNGLSCALEFPROC qglScalef;
extern PFNGLTRANSLATEDPROC qglTranslated;
extern PFNGLTRANSLATEFPROC qglTranslatef;

/* Raster functions */
extern PFNGLPIXELZOOMPROC qglPixelZoom;
extern PFNGLPIXELSTOREFPROC qglPixelStoref;
extern PFNGLPIXELSTOREIPROC qglPixelStorei;
extern PFNGLPIXELTRANSFERFPROC qglPixelTransferf;
extern PFNGLPIXELTRANSFERIPROC qglPixelTransferi;
extern PFNGLPIXELMAPFVPROC qglPixelMapfv;
extern PFNGLPIXELMAPUIVPROC qglPixelMapuiv;
extern PFNGLPIXELMAPUSVPROC qglPixelMapusv;
extern PFNGLGETPIXELMAPFVPROC qglGetPixelMapfv;
extern PFNGLGETPIXELMAPUIVPROC qglGetPixelMapuiv;
extern PFNGLGETPIXELMAPUSVPROC qglGetPixelMapusv;
extern PFNGLBITMAPPROC qglBitmap;
extern PFNGLREADPIXELSPROC qglReadPixels;
extern PFNGLDRAWPIXELSPROC qglDrawPixels;
extern PFNGLCOPYPIXELSPROC qglCopyPixels;

/* Stenciling */
extern PFNGLSTENCILFUNCPROC qglStencilFunc;
extern PFNGLSTENCILMASKPROC qglStencilMask;
extern PFNGLSTENCILOPPROC qglStencilOp;
extern PFNGLCLEARSTENCILPROC qglClearStencil;

/* Texture mapping */
extern PFNGLTEXGENDPROC qglTexGend;
extern PFNGLTEXGENFPROC qglTexGenf;
extern PFNGLTEXGENIPROC qglTexGeni;
extern PFNGLTEXGENDVPROC qglTexGendv;
extern PFNGLTEXGENFVPROC qglTexGenfv;
extern PFNGLTEXGENIVPROC qglTexGeniv;
extern PFNGLGETTEXGENDVPROC qglGetTexGendv;
extern PFNGLGETTEXGENFVPROC qglGetTexGenfv;
extern PFNGLGETTEXGENIVPROC qglGetTexGeniv;
extern PFNGLTEXENVFPROC qglTexEnvf;
extern PFNGLTEXENVIPROC qglTexEnvi;
extern PFNGLTEXENVFVPROC qglTexEnvfv;
extern PFNGLTEXENVIVPROC qglTexEnviv;
extern PFNGLGETTEXENVFVPROC qglGetTexEnvfv;
extern PFNGLGETTEXENVIVPROC qglGetTexEnviv;
extern PFNGLTEXPARAMETERFPROC qglTexParameterf;
extern PFNGLTEXPARAMETERIPROC qglTexParameteri;
extern PFNGLTEXPARAMETERFVPROC qglTexParameterfv;
extern PFNGLTEXPARAMETERIVPROC qglTexParameteriv;
extern PFNGLGETTEXPARAMETERFVPROC qglGetTexParameterfv;
extern PFNGLGETTEXPARAMETERIVPROC qglGetTexParameteriv;
extern PFNGLGETTEXLEVELPARAMETERFVPROC qglGetTexLevelParameterfv;
extern PFNGLGETTEXLEVELPARAMETERIVPROC qglGetTexLevelParameteriv;
extern PFNGLTEXIMAGE1DPROC qglTexImage1D;
extern PFNGLTEXIMAGE2DPROC qglTexImage2D;
extern PFNGLGETTEXIMAGEPROC qglGetTexImage;

/* 1.1 functions */
/* texture objects */
extern PFNGLGENTEXTURESPROC qglGenTextures;
extern PFNGLDELETETEXTURESPROC qglDeleteTextures;
extern PFNGLBINDTEXTUREPROC qglBindTexture;
extern PFNGLPRIORITIZETEXTURESPROC qglPrioritizeTextures;
extern PFNGLARETEXTURESRESIDENTPROC qglAreTexturesResident;
extern PFNGLISTEXTUREPROC qglIsTexture;
/* texture mapping */
extern PFNGLTEXSUBIMAGE1DPROC qglTexSubImage1D;
extern PFNGLTEXSUBIMAGE2DPROC qglTexSubImage2D;
extern PFNGLCOPYTEXIMAGE1DPROC qglCopyTexImage1D;
extern PFNGLCOPYTEXIMAGE2DPROC qglCopyTexImage2D;
extern PFNGLCOPYTEXSUBIMAGE1DPROC qglCopyTexSubImage1D;
extern PFNGLCOPYTEXSUBIMAGE2DPROC qglCopyTexSubImage2D;
/* vertex arrays */
extern PFNGLVERTEXPOINTERPROC qglVertexPointer;
extern PFNGLNORMALPOINTERPROC qglNormalPointer;
extern PFNGLCOLORPOINTERPROC qglColorPointer;
extern PFNGLINDEXPOINTERPROC qglIndexPointer;
extern PFNGLTEXCOORDPOINTERPROC qglTexCoordPointer;
extern PFNGLEDGEFLAGPOINTERPROC qglEdgeFlagPointer;
extern PFNGLGETPOINTERVPROC qglGetPointerv;
extern PFNGLARRAYELEMENTPROC qglArrayElement;
extern PFNGLDRAWARRAYSPROC qglDrawArrays;
extern PFNGLDRAWELEMENTSPROC qglDrawElements;
extern PFNGLINTERLEAVEDARRAYSPROC qglInterleavedArrays;

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
