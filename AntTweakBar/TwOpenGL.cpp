//  ---------------------------------------------------------------------------
//
//  @file       TwOpenGL.cpp
//  @author     Philippe Decaudin
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//
//  ---------------------------------------------------------------------------


#include "TwPrecomp.h"
#include "TwOpenGL.h"
#include "TwMgr.h"
#include "../gl_util.h"

using namespace std;

GLuint g_SmallFontTexID = 0;
GLuint g_NormalFontTexID = 0;
GLuint g_LargeFontTexID = 0;

//  ---------------------------------------------------------------------------
//  Extensions

#ifndef GL_ARRAY_BUFFER_ARB
#   define GL_ARRAY_BUFFER_ARB 0x8892
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER_ARB
#   define GL_ELEMENT_ARRAY_BUFFER_ARB 0x8893
#endif
#ifndef GL_ARRAY_BUFFER_BINDING_ARB
#   define GL_ARRAY_BUFFER_BINDING_ARB 0x8894
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB
#   define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB 0x8895
#endif
#ifndef GL_VERTEX_PROGRAM_ARB
#   define GL_VERTEX_PROGRAM_ARB 0x8620
#endif
#ifndef GL_FRAGMENT_PROGRAM_ARB
#   define GL_FRAGMENT_PROGRAM_ARB 0x8804
#endif
#ifndef GL_PROGRAM_OBJECT_ARB
#   define GL_PROGRAM_OBJECT_ARB 0x8B40
#endif
#ifndef GL_TEXTURE_3D
#   define GL_TEXTURE_3D 0x806F
#endif
#ifndef GL_TEXTURE0_ARB
#   define GL_TEXTURE0_ARB 0x84C0
#endif
#ifndef GL_ACTIVE_TEXTURE_ARB
#   define GL_ACTIVE_TEXTURE_ARB 0x84E0
#endif
#ifndef GL_CLIENT_ACTIVE_TEXTURE_ARB
#   define GL_CLIENT_ACTIVE_TEXTURE_ARB 0x84E1
#endif
#ifndef GL_MAX_TEXTURE_UNITS_ARB
#   define GL_MAX_TEXTURE_UNITS_ARB 0x84E2
#endif
#ifndef GL_MAX_TEXTURE_COORDS
#   define GL_MAX_TEXTURE_COORDS 0x8871
#endif
#ifndef GL_TEXTURE_RECTANGLE_ARB
#   define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#endif
#ifndef GL_FUNC_ADD
#   define GL_FUNC_ADD 0x8006
#endif
#ifndef GL_BLEND_EQUATION
#   define GL_BLEND_EQUATION 0x8009
#endif
#ifndef GL_BLEND_EQUATION_RGB
#   define GL_BLEND_EQUATION_RGB GL_BLEND_EQUATION
#endif
#ifndef GL_BLEND_EQUATION_ALPHA
#   define GL_BLEND_EQUATION_ALPHA 0x883D
#endif
#ifndef GL_BLEND_SRC_RGB
#   define GL_BLEND_SRC_RGB 0x80C9
#endif
#ifndef GL_BLEND_DST_RGB
#   define GL_BLEND_DST_RGB 0x80C8
#endif
#ifndef GL_BLEND_SRC_ALPHA
#   define GL_BLEND_SRC_ALPHA 0x80CB
#endif
#ifndef GL_BLEND_DST_ALPHA
#   define GL_BLEND_DST_ALPHA 0x80CA
#endif
#ifndef GL_VERTEX_ARRAY_BINDING
#   define GL_VERTEX_ARRAY_BINDING 0x85B5
#endif
#ifndef GL_MAX_VERTEX_ATTRIBS
#    define GL_MAX_VERTEX_ATTRIBS 0x8869
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_ENABLED
#    define GL_VERTEX_ATTRIB_ARRAY_ENABLED 0x8622
#endif

//  ---------------------------------------------------------------------------

#ifdef _DEBUG
    static void CheckGLError(const char *file, int line, const char *func)
    {
        int err=0;
        char msg[256];
        while( (err=glGetError())!=0 )
        {
            sprintf(msg, "%s(%d) : [%s] GL_ERROR=0x%x\n", file, line, func, err);
            #ifdef ANT_WINDOWS
                OutputDebugString(msg);
            #endif
            fprintf(stderr, msg);
        }
    }
#   ifdef __FUNCTION__
#       define CHECK_GL_ERROR CheckGLError(__FILE__, __LINE__, __FUNCTION__)
#   else
#       define CHECK_GL_ERROR CheckGLError(__FILE__, __LINE__, "")
#   endif
#else
#   define CHECK_GL_ERROR ((void)(0))
#endif

//  ---------------------------------------------------------------------------

static GLuint BindFont(const CTexFont *_Font)
{
    GLuint TexID = 0;
    glGenTextures(1, &TexID);
    glBindTexture(GL_TEXTURE_2D, TexID);
    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelTransferf(GL_ALPHA_SCALE, 1);
    glPixelTransferf(GL_ALPHA_BIAS, 0);
    glPixelTransferf(GL_RED_BIAS, 1);
    glPixelTransferf(GL_GREEN_BIAS, 1);
    glPixelTransferf(GL_BLUE_BIAS, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, _Font->m_TexWidth, _Font->m_TexHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, _Font->m_TexBytes);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelTransferf(GL_ALPHA_BIAS, 0);
    glPixelTransferf(GL_RED_BIAS, 0);
    glPixelTransferf(GL_GREEN_BIAS, 0);
    glPixelTransferf(GL_BLUE_BIAS, 0);

    return TexID;
}

static void UnbindFont(GLuint _FontTexID)
{
    if( _FontTexID>0 )
        glDeleteTextures(1, &_FontTexID);
}

//  ---------------------------------------------------------------------------

int CTwGraphOpenGL::Init()
{
    m_Drawing = false;
    m_FontTexID = 0;
    m_FontTex = NULL;
    m_MaxClipPlanes = -1;
    m_SupportTexRect = false; // updated in BeginDraw

    return 1;
}

//  ---------------------------------------------------------------------------

int CTwGraphOpenGL::Shut()
{
    assert(m_Drawing==false);

    UnbindFont(m_FontTexID);

    return 1;
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::BeginDraw(int _WndWidth, int _WndHeight)
{
    assert(m_Drawing==false && _WndWidth>0 && _WndHeight>0);
    m_Drawing = true;
    m_WndWidth = _WndWidth;
    m_WndHeight = _WndHeight;

    CHECK_GL_ERROR;

    static bool s_SupportTexRectChecked = false;
    if (!s_SupportTexRectChecked) 
    {
        m_SupportTexRect = IsGLExtensionSupported("GL_ARB_texture_rectangle");
        s_SupportTexRectChecked = true;
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    if( glActiveTextureARB )
    {
        glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &m_PrevActiveTextureARB);
        glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE_ARB, &m_PrevClientActiveTextureARB);
        GLint maxTexUnits = 1;
        glGetIntegerv(GL_MAX_TEXTURE_COORDS, &maxTexUnits); // was GL_MAX_TEXTURE_UNITS_ARB
        if( maxTexUnits<1 ) 
            maxTexUnits = 1;
        else if( maxTexUnits > MAX_TEXTURES )
            maxTexUnits = MAX_TEXTURES;
        GLint i;
        for( i=0; i<maxTexUnits; ++i )
        {
            glActiveTextureARB(GL_TEXTURE0_ARB+i);
            m_PrevActiveTexture1D[i] = glIsEnabled(GL_TEXTURE_1D);
            m_PrevActiveTexture2D[i] = glIsEnabled(GL_TEXTURE_2D);
            m_PrevActiveTexture3D[i] = glIsEnabled(GL_TEXTURE_3D);
            glDisable(GL_TEXTURE_1D);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_3D);
        }
        glActiveTextureARB(GL_TEXTURE0_ARB);

        for( i=0; i<maxTexUnits; i++ )
        {
            glClientActiveTextureARB(GL_TEXTURE0_ARB+i);
            m_PrevClientTexCoordArray[i] = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        glClientActiveTextureARB(GL_TEXTURE0_ARB);
    }

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    GLint Vp[4];
    glGetIntegerv(GL_VIEWPORT, Vp);
    /*
    if( _WndWidth>0 && _WndHeight>0 )
    {
        Vp[0] = 0;
        Vp[1] = 0;
        Vp[2] = _WndWidth;
        Vp[3] = _WndHeight;
        glViewport(Vp[0], Vp[1], Vp[2], Vp[3]);
    }
    glLoadIdentity();
    //glOrtho(Vp[0], Vp[0]+Vp[2]-1, Vp[1]+Vp[3]-1, Vp[1], -1, 1); // Doesn't work
    glOrtho(Vp[0], Vp[0]+Vp[2], Vp[1]+Vp[3], Vp[1], -1, 1);
    */
    if( _WndWidth>0 && _WndHeight>0 )
    {
        Vp[0] = 0;
        Vp[1] = 0;
        Vp[2] = _WndWidth-1;
        Vp[3] = _WndHeight-1;
        glViewport(Vp[0], Vp[1], Vp[2], Vp[3]);
    }
    glLoadIdentity();
    glOrtho(Vp[0], Vp[0]+Vp[2], Vp[1]+Vp[3], Vp[1], -1, 1);
    glGetIntegerv(GL_VIEWPORT, m_ViewportInit);
    glGetFloatv(GL_PROJECTION_MATRIX, m_ProjMatrixInit);

    glGetFloatv(GL_LINE_WIDTH, &m_PrevLineWidth);
    glDisable(GL_POLYGON_STIPPLE);
    glLineWidth(1);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_LINE_STIPPLE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &m_PrevTexEnv);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glGetIntegerv(GL_POLYGON_MODE, m_PrevPolygonMode);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_ALPHA_TEST);
    //glEnable(GL_ALPHA_TEST);
    //glAlphaFunc(GL_GREATER, 0);
    glDisable(GL_FOG);
    glDisable(GL_LOGIC_OP);
    glDisable(GL_SCISSOR_TEST);
    if( m_MaxClipPlanes<0 )
    {
        glGetIntegerv(GL_MAX_CLIP_PLANES, &m_MaxClipPlanes);
        if( m_MaxClipPlanes<0 || m_MaxClipPlanes>255 )
            m_MaxClipPlanes = 6;
    }
    for( GLint i=0; i<m_MaxClipPlanes; ++i )
        glDisable(GL_CLIP_PLANE0+i);
    m_PrevTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_PrevTexture);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_INDEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_EDGE_FLAG_ARRAY);

    if( glBindVertexArray!=NULL )
    {
        m_PrevVertexArray = 0;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&m_PrevVertexArray);
        glBindVertexArray(0);
    }
    if( glBindBufferARB!=NULL )
    {
        m_PrevArrayBufferARB = m_PrevElementArrayBufferARB = 0;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING_ARB, &m_PrevArrayBufferARB);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB, &m_PrevElementArrayBufferARB);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    if( glBindProgramARB!=NULL )
    {
        m_PrevVertexProgramARB = glIsEnabled(GL_VERTEX_PROGRAM_ARB);
        m_PrevFragmentProgramARB = glIsEnabled(GL_FRAGMENT_PROGRAM_ARB);
        glDisable(GL_VERTEX_PROGRAM_ARB);
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
    }
    if( glGetHandleARB!=NULL && glUseProgramObjectARB!=NULL )
    {
        m_PrevProgramObjectARB = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
        glUseProgramObjectARB(0);
    }
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    if( glTexImage3D!=NULL )
    {
        m_PrevTexture3D = glIsEnabled(GL_TEXTURE_3D);
        glDisable(GL_TEXTURE_3D);
    }

    if( m_SupportTexRect )
    {
        m_PrevTexRectARB = glIsEnabled(GL_TEXTURE_RECTANGLE_ARB);
        glDisable(GL_TEXTURE_RECTANGLE_ARB);
    }
    if( glBlendEquationSeparate!=NULL )
    {
        glGetIntegerv(GL_BLEND_EQUATION_RGB, &m_PrevBlendEquationRGB);
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &m_PrevBlendEquationAlpha);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    }
    if( glBlendFuncSeparate!=NULL )
    {
        glGetIntegerv(GL_BLEND_SRC_RGB, &m_PrevBlendSrcRGB);
        glGetIntegerv(GL_BLEND_DST_RGB, &m_PrevBlendDstRGB);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &m_PrevBlendSrcAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &m_PrevBlendDstAlpha);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    if( glBlendEquation!=NULL )
    {
        glGetIntegerv(GL_BLEND_EQUATION, &m_PrevBlendEquation);
        glBlendEquation(GL_FUNC_ADD);
    }
    if( glDisableVertexAttribArray!=NULL )
    {
        GLint maxVertexAttribs;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
        if(maxVertexAttribs>MAX_VERTEX_ATTRIBS)
            maxVertexAttribs=MAX_VERTEX_ATTRIBS;
       
        for(int i=0; i<maxVertexAttribs; i++)
        {
            glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &m_PrevEnabledVertexAttrib[i]);
            glDisableVertexAttribArray(i);
        }
    }

    CHECK_GL_ERROR;
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::EndDraw()
{
    assert(m_Drawing==true);
    m_Drawing = false;

    glBindTexture(GL_TEXTURE_2D, m_PrevTexture);
    if( glBindVertexArray!=NULL )
        glBindVertexArray(m_PrevVertexArray);
    if( glBindBufferARB!=NULL )
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_PrevArrayBufferARB);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_PrevElementArrayBufferARB);
    }
    if( glBindProgramARB!=NULL )
    {
        if( m_PrevVertexProgramARB )
            glEnable(GL_VERTEX_PROGRAM_ARB);
        if( m_PrevFragmentProgramARB )
            glEnable(GL_FRAGMENT_PROGRAM_ARB);
    }
    if( glGetHandleARB!=NULL && glUseProgramObjectARB!=NULL )
        glUseProgramObjectARB(m_PrevProgramObjectARB);
    if( glTexImage3D!=NULL && m_PrevTexture3D )
        glEnable(GL_TEXTURE_3D);
    if( m_SupportTexRect && m_PrevTexRectARB )
        glEnable(GL_TEXTURE_RECTANGLE_ARB);
    if( glBlendEquation!=NULL )
        glBlendEquation(m_PrevBlendEquation);
    if( glBlendEquationSeparate!=NULL )
        glBlendEquationSeparate(m_PrevBlendEquationRGB, m_PrevBlendEquationAlpha);
    if( glBlendFuncSeparate!=NULL )
        glBlendFuncSeparate(m_PrevBlendSrcRGB, m_PrevBlendDstRGB, m_PrevBlendSrcAlpha, m_PrevBlendDstAlpha);
    
    glPolygonMode(GL_FRONT, m_PrevPolygonMode[0]);
    glPolygonMode(GL_BACK, m_PrevPolygonMode[1]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m_PrevTexEnv);
    glLineWidth(m_PrevLineWidth);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glPopClientAttrib();
    glPopAttrib();

    if( glActiveTextureARB )
    {
        GLint maxTexUnits = 1;
        glGetIntegerv(GL_MAX_TEXTURE_COORDS, &maxTexUnits); // was GL_MAX_TEXTURE_UNITS_ARB
        if( maxTexUnits<1 ) 
            maxTexUnits = 1;
        else if( maxTexUnits > MAX_TEXTURES )
            maxTexUnits = MAX_TEXTURES;
        GLint i;
        for( i=0; i<maxTexUnits; ++i )
        {
            glActiveTextureARB(GL_TEXTURE0_ARB+i);
            if( m_PrevActiveTexture1D[i] )
                glEnable(GL_TEXTURE_1D);
            if( m_PrevActiveTexture2D[i] )
                glEnable(GL_TEXTURE_2D);
            if( m_PrevActiveTexture3D[i] )
                glEnable(GL_TEXTURE_3D);
        }
        glActiveTextureARB(m_PrevActiveTextureARB);

        for( i=0; i<maxTexUnits; ++i )
        {
            glClientActiveTextureARB(GL_TEXTURE0_ARB+i);
            if( m_PrevClientTexCoordArray[i] )
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        glClientActiveTextureARB(m_PrevClientActiveTextureARB);
    }
    if(glEnableVertexAttribArray)
    {
        GLint maxVertexAttribs;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
        if(maxVertexAttribs>MAX_VERTEX_ATTRIBS)
            maxVertexAttribs=MAX_VERTEX_ATTRIBS;
       
        for(int i=0; i<maxVertexAttribs; i++)
        {
            if(m_PrevEnabledVertexAttrib[i]!=0)
                glEnableVertexAttribArray(i);
        }
    }

    CHECK_GL_ERROR;
}

//  ---------------------------------------------------------------------------

bool CTwGraphOpenGL::IsDrawing()
{
    return m_Drawing;
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::Restore()
{
    UnbindFont(m_FontTexID);
    m_FontTexID = 0;
    m_FontTex = NULL;
}


//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::DrawLine(int _X0, int _Y0, int _X1, int _Y1, color32 _Color0, color32 _Color1, bool _AntiAliased)
{
    assert(m_Drawing==true);
    /* 
    // border adjustment NO!!
    if(_X0<_X1)
        ++_X1;
    else if(_X0>_X1)
        ++_X0;
    if(_Y0<_Y1)
        ++_Y1;
    else if(_Y0>_Y1)
        ++_Y0;
    */
    //const GLfloat dx = +0.0f;
    const GLfloat dx = +0.5f;
    //GLfloat dy = -0.2f;
    const GLfloat dy = -0.5f;
    if( _AntiAliased )
        glEnable(GL_LINE_SMOOTH);
    else
        glDisable(GL_LINE_SMOOTH);
    glDisable(GL_TEXTURE_2D);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBegin(GL_LINES);
        glColor4ub(GLubyte(_Color0>>16), GLubyte(_Color0>>8), GLubyte(_Color0), GLubyte(_Color0>>24));
        glVertex2f((GLfloat)_X0+dx, (GLfloat)_Y0+dy);
        glColor4ub(GLubyte(_Color1>>16), GLubyte(_Color1>>8), GLubyte(_Color1), GLubyte(_Color1>>24));
        glVertex2f((GLfloat)_X1+dx, (GLfloat)_Y1+dy);
        //glVertex2i(_X0, _Y0);
        //glVertex2i(_X1, _Y1);
    glEnd();
    glDisable(GL_LINE_SMOOTH);
}
  
//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::DrawRect(int _X0, int _Y0, int _X1, int _Y1, color32 _Color00, color32 _Color10, color32 _Color01, color32 _Color11)
{
    assert(m_Drawing==true);

    /*
    // border adjustment
    if(_X0<_X1)
        ++_X1;
    else if(_X0>_X1)
        ++_X0;
    if(_Y0<_Y1)
        ++_Y1;
    else if(_Y0>_Y1)
        ++_Y0;
    */
    // border adjustment
    if(_X0<_X1)
        ++_X1;
    else if(_X0>_X1)
        ++_X0;
    if(_Y0<_Y1)
        --_Y0;
    else if(_Y0>_Y1)
        --_Y1;
    const GLfloat dx = +0.0f;
    const GLfloat dy = +0.0f;

    glDisable(GL_TEXTURE_2D);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //GLubyte r = GLubyte(_Color>>16);
    //GLubyte g = GLubyte(_Color>>8);
    //GLubyte b = GLubyte(_Color);
    //GLubyte a = GLubyte(_Color>>24);
    //glColor4ub(GLubyte(_Color>>16), GLubyte(_Color>>8), GLubyte(_Color), GLubyte(_Color>>24));
    //glColor4ub(r, g, b, a);
    glBegin(GL_QUADS);
        glColor4ub(GLubyte(_Color00>>16), GLubyte(_Color00>>8), GLubyte(_Color00), GLubyte(_Color00>>24));
        glVertex2f((GLfloat)_X0+dx, (GLfloat)_Y0+dy);
        glColor4ub(GLubyte(_Color10>>16), GLubyte(_Color10>>8), GLubyte(_Color10), GLubyte(_Color10>>24));
        glVertex2f((GLfloat)_X1+dx, (GLfloat)_Y0+dy);
        glColor4ub(GLubyte(_Color11>>16), GLubyte(_Color11>>8), GLubyte(_Color11), GLubyte(_Color11>>24));
        glVertex2f((GLfloat)_X1+dx, (GLfloat)_Y1+dy);
        glColor4ub(GLubyte(_Color01>>16), GLubyte(_Color01>>8), GLubyte(_Color01), GLubyte(_Color01>>24));
        glVertex2f((GLfloat)_X0+dx, (GLfloat)_Y1+dy);
    glEnd();
}

//  ---------------------------------------------------------------------------

void *CTwGraphOpenGL::NewTextObj()
{
    return new CTextObj;
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::DeleteTextObj(void *_TextObj)
{
    assert(_TextObj!=NULL);
    delete static_cast<CTextObj *>(_TextObj);
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::BuildText(void *_TextObj, const std::string *_TextLines, color32 *_LineColors, color32 *_LineBgColors, int _NbLines, const CTexFont *_Font, int _Sep, int _BgWidth)
{

    //return;
    assert(m_Drawing==true);
    assert(_TextObj!=NULL);
    assert(_Font!=NULL);

    if( _Font != m_FontTex )
    {
        UnbindFont(m_FontTexID);
        m_FontTexID = BindFont(_Font);
        m_FontTex = _Font;
    }
    CTextObj *TextObj = static_cast<CTextObj *>(_TextObj);
    TextObj->m_TextVerts.resize(0);
    TextObj->m_TextUVs.resize(0);
    TextObj->m_BgVerts.resize(0);
    TextObj->m_Colors.resize(0);
    TextObj->m_BgColors.resize(0);

    int x, x1, y, y1, i, Len;
    unsigned char ch;
    const unsigned char *Text;
    color32 LineColor = COLOR32_RED;
    for( int Line=0; Line<_NbLines; ++Line )
    {
        x = 0;
        y = Line * (_Font->m_CharHeight+_Sep);
        y1 = y+_Font->m_CharHeight;
        Len = (int)_TextLines[Line].length();
        Text = (const unsigned char *)(_TextLines[Line].c_str());
        if( _LineColors!=NULL )
            LineColor = (_LineColors[Line]&0xff00ff00) | GLubyte(_LineColors[Line]>>16) | (GLubyte(_LineColors[Line])<<16);

        for( i=0; i<Len; ++i )
        {
            ch = Text[i];
            x1 = x + _Font->m_CharWidth[ch];

            TextObj->m_TextVerts.push_back(Vec2(x , y ));
            TextObj->m_TextVerts.push_back(Vec2(x1, y ));
            TextObj->m_TextVerts.push_back(Vec2(x , y1));
            TextObj->m_TextVerts.push_back(Vec2(x1, y ));
            TextObj->m_TextVerts.push_back(Vec2(x1, y1));
            TextObj->m_TextVerts.push_back(Vec2(x , y1));

            TextObj->m_TextUVs.push_back(Vec2(_Font->m_CharU0[ch], _Font->m_CharV0[ch]));
            TextObj->m_TextUVs.push_back(Vec2(_Font->m_CharU1[ch], _Font->m_CharV0[ch]));
            TextObj->m_TextUVs.push_back(Vec2(_Font->m_CharU0[ch], _Font->m_CharV1[ch]));
            TextObj->m_TextUVs.push_back(Vec2(_Font->m_CharU1[ch], _Font->m_CharV0[ch]));
            TextObj->m_TextUVs.push_back(Vec2(_Font->m_CharU1[ch], _Font->m_CharV1[ch]));
            TextObj->m_TextUVs.push_back(Vec2(_Font->m_CharU0[ch], _Font->m_CharV1[ch]));

            if( _LineColors!=NULL )
            {
                TextObj->m_Colors.push_back(LineColor);
                TextObj->m_Colors.push_back(LineColor);
                TextObj->m_Colors.push_back(LineColor);
                TextObj->m_Colors.push_back(LineColor);
                TextObj->m_Colors.push_back(LineColor);
                TextObj->m_Colors.push_back(LineColor);
            }

            x = x1;
        }
        if( _BgWidth>0 )
        {
            TextObj->m_BgVerts.push_back(Vec2(-1        , y ));
            TextObj->m_BgVerts.push_back(Vec2(_BgWidth+1, y ));
            TextObj->m_BgVerts.push_back(Vec2(-1        , y1));
            TextObj->m_BgVerts.push_back(Vec2(_BgWidth+1, y ));
            TextObj->m_BgVerts.push_back(Vec2(_BgWidth+1, y1));
            TextObj->m_BgVerts.push_back(Vec2(-1        , y1));

            if( _LineBgColors!=NULL )
            {
                color32 LineBgColor = (_LineBgColors[Line]&0xff00ff00) | GLubyte(_LineBgColors[Line]>>16) | (GLubyte(_LineBgColors[Line])<<16);
                TextObj->m_BgColors.push_back(LineBgColor);
                TextObj->m_BgColors.push_back(LineBgColor);
                TextObj->m_BgColors.push_back(LineBgColor);
                TextObj->m_BgColors.push_back(LineBgColor);
                TextObj->m_BgColors.push_back(LineBgColor);
                TextObj->m_BgColors.push_back(LineBgColor);
            }
        }
    }
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::DrawText(void *_TextObj, int _X, int _Y, color32 _Color, color32 _BgColor)
{
    //return;
    assert(m_Drawing==true);
    assert(_TextObj!=NULL);
    CTextObj *TextObj = static_cast<CTextObj *>(_TextObj);

    if( TextObj->m_TextVerts.size()<4 && TextObj->m_BgVerts.size()<4 )
        return; // nothing to draw

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef((GLfloat)_X, (GLfloat)_Y, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    if( (_BgColor!=0 || TextObj->m_BgColors.size()==TextObj->m_BgVerts.size()) && TextObj->m_BgVerts.size()>=4 )
    {
        glDisable(GL_TEXTURE_2D);
        glVertexPointer(2, GL_FLOAT, 0, &(TextObj->m_BgVerts[0]));
        if( TextObj->m_BgColors.size()==TextObj->m_BgVerts.size() && _BgColor==0 )
        {
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(4, GL_UNSIGNED_BYTE, 0, &(TextObj->m_BgColors[0]));
        }
        else
        {
            glDisableClientState(GL_COLOR_ARRAY);
            glColor4ub(GLubyte(_BgColor>>16), GLubyte(_BgColor>>8), GLubyte(_BgColor), GLubyte(_BgColor>>24));
        }
        glDrawArrays(GL_TRIANGLES, 0, (int)TextObj->m_BgVerts.size());
    }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_FontTexID);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    if( TextObj->m_TextVerts.size()>=4 )
    {
        glVertexPointer(2, GL_FLOAT, 0, &(TextObj->m_TextVerts[0]));
        glTexCoordPointer(2, GL_FLOAT, 0, &(TextObj->m_TextUVs[0]));
        if( TextObj->m_Colors.size()==TextObj->m_TextVerts.size() && _Color==0 )
        {
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(4, GL_UNSIGNED_BYTE, 0, &(TextObj->m_Colors[0]));
        }
        else
        {
            glDisableClientState(GL_COLOR_ARRAY);
            glColor4ub(GLubyte(_Color>>16), GLubyte(_Color>>8), GLubyte(_Color), GLubyte(_Color>>24));
        }

        glDrawArrays(GL_TRIANGLES, 0, (int)TextObj->m_TextVerts.size());
    }
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::ChangeViewport(int _X0, int _Y0, int _Width, int _Height, int _OffsetX, int _OffsetY)
{
    if( _Width>0 && _Height>0 )
    {
        GLint vp[4];
        vp[0] = _X0;
        vp[1] = _Y0;
        vp[2] = _Width-1;
        vp[3] = _Height-1;
        glViewport(vp[0], m_WndHeight-vp[1]-vp[3], vp[2], vp[3]);

        GLint matrixMode = 0;
        glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(_OffsetX, _OffsetX+vp[2], vp[3]-_OffsetY, -_OffsetY, -1, 1);
        glMatrixMode(matrixMode);
    }
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::RestoreViewport()
{
    glViewport(m_ViewportInit[0], m_ViewportInit[1], m_ViewportInit[2], m_ViewportInit[3]);

    GLint matrixMode = 0;
    glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_ProjMatrixInit);
    glMatrixMode(matrixMode);
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::SetScissor(int _X0, int _Y0, int _Width, int _Height)
{
    if( _Width>0 && _Height>0 )
    {
        glScissor(_X0-1, m_WndHeight-_Y0-_Height, _Width-1, _Height);
        glEnable(GL_SCISSOR_TEST);
    }
    else
        glDisable(GL_SCISSOR_TEST);
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::DrawTriangles(int _NumTriangles, int *_Vertices, color32 *_Colors, Cull _CullMode)
{
    assert(m_Drawing==true);

    const GLfloat dx = +0.0f;
    const GLfloat dy = +0.0f;

    GLint prevCullFaceMode, prevFrontFace;
    glGetIntegerv(GL_CULL_FACE_MODE, &prevCullFaceMode);
    glGetIntegerv(GL_FRONT_FACE, &prevFrontFace);
    GLboolean prevCullEnable = glIsEnabled(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    if( _CullMode==CULL_CW )
        glFrontFace(GL_CCW);
    else if( _CullMode==CULL_CCW )
        glFrontFace(GL_CW);
    else
        glDisable(GL_CULL_FACE);

    glDisable(GL_TEXTURE_2D);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBegin(GL_TRIANGLES);
    for(int i=0; i<3*_NumTriangles; ++i)
    {
        color32 col = _Colors[i];
        glColor4ub(GLubyte(col>>16), GLubyte(col>>8), GLubyte(col), GLubyte(col>>24));
        glVertex2f((GLfloat)_Vertices[2*i+0]+dx, (GLfloat)_Vertices[2*i+1]+dy);
    }
    glEnd();

    glCullFace(prevCullFaceMode);
    glFrontFace(prevFrontFace);
    if( prevCullEnable )
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
}

//  ---------------------------------------------------------------------------
