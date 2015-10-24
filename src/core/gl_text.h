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
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

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

typedef struct gl_text_line_s
{
    char                       *text;
    uint16_t                    text_size;

    uint16_t                    font_id;
    uint16_t                    style_id;

    GLfloat                     x;
    uint8_t                     x_align;
    GLfloat                     absXoffset;
    GLfloat                     y;
    uint8_t                     y_align;
    GLfloat                     absYoffset;

    GLfloat                     rect[4];    //x0, yo, x1, y1

    int8_t                      show;

    struct gl_text_line_s     *next;
    struct gl_text_line_s     *prev;
} gl_text_line_t, *gl_text_line_p;


/**
 * Draws text using a FONT_SECONDARY.
 */
void GLText_InitTempLines();
void GLText_DestroyTempLines();

void GLText_UpdateResize();
void GLText_RenderStringLine(gl_text_line_p l);
void GLText_RenderStrings();

void GLText_AddLine(gl_text_line_p line);
void GLText_DeleteLine(gl_text_line_p line);
void GLText_MoveLine(gl_text_line_p line);
gl_text_line_p GLText_OutTextXY(GLfloat x, GLfloat y, const char *fmt, ...);


#ifdef	__cplusplus
}
#endif

#endif	/* GL_TEXT_H */

