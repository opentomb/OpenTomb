/*
 * File:   gl_text.h
 * Author: TeslaRus
 *
 * Created on October 24, 2015, 11:34 AM
 */

#ifndef GL_TEXT_H
#define	GL_TEXT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdarg.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "gl_font.h"


#define GLTEXT_MAX_FONTSTYLES 32
#define GLTEXT_MAX_FONTS      8
    
#define GLTEXT_MAX_TEMP_LINES   (256)

// Horizontal alignment is simple side alignment, like in original TRs.
// It means that X coordinate will be either used for left, right or
// center orientation.

#define GLTEXT_ALIGN_LEFT       0
#define GLTEXT_ALIGN_RIGHT      1
#define GLTEXT_ALIGN_TOP        0
#define GLTEXT_ALIGN_BOTTOM     1
#define GLTEXT_ALIGN_CENTER     2
    
// Default line size is generally used for static in-game strings. Strings
// that are created dynamically may have variable string sizes.

#define GUI_LINE_DEFAULTSIZE 128

    
// OpenTomb has three types of fonts - primary, secondary and console
// font. This should be enough for most of the cases. However, user
// can generate and use additional font types via script, but engine
// behaviour with extra font types is undefined.
enum font_Type
{
    FONT_CONSOLE = 0,
    FONT_PRIMARY,
    FONT_SECONDARY
};
    
// This is predefined enumeration of font styles, which can be extended
// with user-defined script functions.
///@TODO: add system message console style
enum font_Style
{
    FONTSTYLE_CONSOLE_INFO = 0,
    FONTSTYLE_CONSOLE_WARNING,
    FONTSTYLE_CONSOLE_EVENT,
    FONTSTYLE_CONSOLE_NOTIFY,
    FONTSTYLE_MENU_TITLE,
    FONTSTYLE_MENU_HEADING1,
    FONTSTYLE_MENU_HEADING2,
    FONTSTYLE_MENU_ITEM_ACTIVE,
    FONTSTYLE_MENU_ITEM_INACTIVE,
    FONTSTYLE_MENU_CONTENT,
    FONTSTYLE_STATS_TITLE,
    FONTSTYLE_STATS_CONTENT,
    FONTSTYLE_NOTIFIER,
    FONTSTYLE_SAVEGAMELIST,
    FONTSTYLE_GENERIC
};


typedef struct gl_text_line_s
{
    char                       *text;
    uint16_t                    font_id;
    uint16_t                    style_id;
    uint16_t                    text_size;
    uint16_t                    x_align : 4;
    uint16_t                    y_align : 4;
    uint16_t                    show : 1;
    
    GLfloat                     line_width;
    GLfloat                     line_height;
    GLfloat                     x;
    GLfloat                     y;
    GLfloat                     rect[4];    //x0, y0, x1, y1

    struct gl_text_line_s     *next;
    struct gl_text_line_s     *prev;
} gl_text_line_t, *gl_text_line_p;


/**
 * Draws text using a FONT_SECONDARY.
 */
void GLText_Init();
void GLText_Destroy();

void GLText_UpdateResize(int w, int h, float scale);
void GLText_RenderStringLine(gl_text_line_p l);
void GLText_RenderStrings();

void GLText_AddLine(gl_text_line_p line);
void GLText_DeleteLine(gl_text_line_p line);
gl_text_line_p GLText_OutTextXY(GLfloat x, GLfloat y, const char *fmt, ...);
gl_text_line_p GLText_VOutTextXY(GLfloat x, GLfloat y, const char *fmt, va_list argptr);


int  GLText_AddFont(uint16_t index, uint16_t size, const char* path);
int  GLText_RemoveFont(uint16_t index);
int  GLText_AddFontStyle(uint16_t index,
                      GLfloat R, GLfloat G, GLfloat B, GLfloat A,
                      uint8_t shadow);
int  GLText_RemoveFontStyle(uint16_t index);
gl_tex_font_p  GLText_GetFont(uint16_t index);
gl_fontstyle_p GLText_GetFontStyle(uint16_t index);

#ifdef	__cplusplus
}
#endif

#endif	/* GL_TEXT_H */

