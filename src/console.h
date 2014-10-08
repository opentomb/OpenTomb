
#ifndef CONSOLE_H
#define CONSOLE_H

#include "ftgl/FTGLTextureFont.h"

#include <stdint.h>
#include "script.h"

#define CON_MIN_LOG 16
#define CON_MAX_LOG 128

#define CON_MIN_LINES 64
#define CON_MAX_LINES 256

#define CON_MIN_LINE_SIZE 80
#define CON_MAX_LINE_SIZE 256

#define CON_MIN_LINE_INTERVAL 1.2
#define CON_MAX_LINE_INTERVAL 4.0

typedef struct console_info_s
{
    char                        font_path[255];             // Font file path
    FTGLTextureFont            *font;                       // Texture font renderer

    uint16_t                    font_size;
    GLfloat                     font_color[4];
    GLfloat                     background_color[4];

    int                         log_lines_count;            // Amount of log lines to use
    int                         log_pos;                    // Current log position
    int                         line_size;                  // Console line size
    int                         shown_lines_count;          // Amount of shown lines
    int                         showing_lines;              // Amount of visible lines
    float                       spacing;                    // Line spacing
    float                       show_cursor_period;
    float                       cursor_time;                // Current cursor draw time
    int8_t                      show_cursor;                // Cursor visibility flag

    int16_t                     line_height;                // Height, including spacing
    int16_t                     cursor_pos;                 // Current cursor position, in symbols
    int16_t                     cursor_x;                   // Cursor position in pixels
    int16_t                     cursor_y;
    char                      **shown_lines;                // Console text
    char                      **log_lines;                  // Console lines
    int8_t                      inited;                     // Ready-to-use flag
    int8_t                      show;                       // Visibility flag
}console_info_t, *console_info_p;

extern console_info_t con_base;

void Con_Init();
void Con_Destroy();

void Con_SetFontSize(int size);
void Con_SetLineInterval(float interval);

void Con_Draw();
void Con_DrawBackground();
void Con_DrawCursor();

void Con_Edit(int key);
void Con_CalcCursorPosition();
void Con_AddLog(const char *text);
void Con_AddLine(const char *text);
void Con_AddText(const char *text);
void Con_Printf(const char *fmt, ...);

void Con_Clean();

#endif
