
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

#define GUI_MIN_FONT_SIZE  1
#define GUI_MAX_FONT_SIZE  72

#define GUI_MAX_FONTSTYLES 32   // Who even needs so many?
#define GUI_MAX_FONTS      8    // 8 fonts is PLENTY.

struct lua_State;

void Con_Init();
void Con_InitFont();
void Con_InitGlobals();
void Con_Destroy();
void Con_ParseSettings(struct lua_State *lua);

float Con_GetLineInterval();
void  Con_SetLineInterval(float interval);
uint16_t Con_GetShowingLines();
void Con_SetShowingLines(uint16_t value);

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
                      uint8_t shadow, uint8_t rect, uint8_t rect_border,
                      GLfloat rect_R, GLfloat rect_G, GLfloat rect_B, GLfloat rect_A);
int  Con_RemoveFontStyle(uint16_t index);
gl_tex_font_p  Con_GetFont(uint16_t index);
gl_fontstyle_p Con_GetFontStyle(uint16_t index);
void Con_SetScaleFonts(float scale);

void Con_Clean();

void Con_Draw(float time);
int  Con_IsShown();
void Con_SetShown(int value);

#ifdef	__cplusplus
}
#endif
#endif
