
#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "gl_font.h"

#define CON_MIN_LOG 16
#define CON_MAX_LOG 128

#define CON_MIN_LINES 64
#define CON_MAX_LINES 256

#define CON_MIN_LINE_SIZE 80
#define CON_MAX_LINE_SIZE 256

#define CON_MIN_LINE_INTERVAL 0.5
#define CON_MAX_LINE_INTERVAL 4.0

// This is predefined enumeration of font styles, which can be extended
// with user-defined script functions.
///@TODO: add system message console style
enum font_Style
{
        FONTSTYLE_CONSOLE_INFO,
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

#define GUI_MAX_FONTSTYLES 32   // Who even needs so many?
#define GUI_MAX_FONTS      8    // 8 fonts is PLENTY.

typedef struct console_info_s
{
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

extern console_info_t           con_base;

void Con_Init();
void Con_InitFont();
void Con_InitGlobals();
void Con_Destroy();

void Con_SetLineInterval(float interval);

void Con_Filter(char *text);
void Con_Edit(int key);
void Con_CalcCursorPosition();
void Con_AddLog(const char *text);
void Con_AddLine(const char *text, uint16_t font_style);
void Con_AddText(const char *text, uint16_t font_style);
void Con_Printf(const char *fmt, ...);
void Con_Warning(const char *fmt, ...);
void Con_Notify(const char *fmt, ...);

int  Con_AddFont(uint16_t index, uint16_t size, const char* path);
int  Con_RemoveFont(uint16_t index);
int  Con_AddFontStyle(uint16_t index,
                      GLfloat R, GLfloat G, GLfloat B, GLfloat A,
                      uint8_t shadow, uint8_t fading,
                      uint8_t rect, uint8_t rect_border,
                      GLfloat rect_R, GLfloat rect_G, GLfloat rect_B, GLfloat rect_A,
                      uint8_t hide);
int  Con_RemoveFontStyle(uint16_t index);
gl_tex_font_p  Con_GetFont(uint16_t index);
gl_fontstyle_p Con_GetFontStyle(uint16_t index);
void Con_SetScaleFonts(float scale);
void Con_UpdateFonts(float time);

void Con_Clean();

#ifdef	__cplusplus
}
#endif
#endif
