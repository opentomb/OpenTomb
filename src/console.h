
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "gui.h"
#include "gl_font.h"

#define CON_MIN_LOG 16
#define CON_MAX_LOG 128

#define CON_MIN_LINES 64
#define CON_MAX_LINES 256

#define CON_MIN_LINE_SIZE 80
#define CON_MAX_LINE_SIZE 256

#define CON_MIN_LINE_INTERVAL 0.5
#define CON_MAX_LINE_INTERVAL 4.0

typedef struct console_info_s
{
    struct gl_tex_font_s       *font;                       // Texture font renderer
    
    GLfloat                     background_color[4];

    uint16_t                    log_lines_count;            // Amount of log lines to use
    uint16_t                    log_pos;                    // Current log position
    char                      **log_lines;                  // Console lines
    
    uint16_t                    line_count;                 // Amount of shown lines
    uint16_t                   *line_style_id;
    char                      **line_text;                  // Console text
    
    uint16_t                    line_size;                  // Console line size
    int16_t                     line_height;                // Height, including spacing
    
    uint16_t                    showing_lines;              // Amount of visible lines
    float                       spacing;                    // Line spacing
    
    int16_t                     cursor_pos;                 // Current cursor position, in symbols
    int16_t                     cursor_x;                   // Cursor position in pixels
    int16_t                     cursor_y;
    float                       cursor_time;                // Current cursor draw time
    float                       show_cursor_period;
    int8_t                      show_cursor;                // Cursor visibility flag

    int8_t                      inited;                     // Ready-to-use flag
    int8_t                      show;                       // Visibility flag
}console_info_t, *console_info_p;

extern console_info_t con_base;

void Con_Init();
void Con_InitFonts();
void Con_InitGlobals();
void Con_Destroy();

void Con_SetLineInterval(float interval);

void Con_Draw();
void Con_DrawBackground();
void Con_DrawCursor();

void Con_Filter(char *text);
void Con_Edit(int key);
void Con_CalcCursorPosition();
void Con_AddLog(const char *text);
void Con_AddLine(const char *text, font_Style style = FONTSTYLE_CONSOLE_INFO);
void Con_AddText(const char *text, font_Style style = FONTSTYLE_CONSOLE_INFO);
void Con_Printf(const char *fmt, ...);
void Con_Warning(int warn_string_index, ...);
void Con_Notify(int notify_string_index, ...);

void Con_Clean();

#endif
