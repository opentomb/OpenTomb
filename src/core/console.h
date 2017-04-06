
#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#define CON_MIN_LOG 16
#define CON_MAX_LOG 128

#define CON_MIN_LINES 64
#define CON_MAX_LINES 512

#define CON_MIN_LINE_SIZE 80
#define CON_MAX_LINE_SIZE 256

#define CON_MIN_LINE_INTERVAL 0.5
#define CON_MAX_LINE_INTERVAL 4.0


#define GUI_MIN_FONT_SIZE  1
#define GUI_MAX_FONT_SIZE  72

void Con_Init();
void Con_SetExecFunction(int(*exec_cmd)(char *ch));
void Con_InitFont();
void Con_InitGlobals();
void Con_Destroy();

float Con_GetLineInterval();
void  Con_SetLineInterval(float interval);
uint16_t Con_GetShowingLines();
void Con_SetShowingLines(uint16_t value);
void Con_SetBackgroundColor(float color[4]);
void Con_SetShowCursorPeriod(float time);

void Con_SetLinesCount(uint16_t count);
void Con_SetLogLinesCount(uint16_t count);
void Con_SetMaxLineLenght(uint16_t line_size);

void Con_Filter(char *text);
void Con_Edit(int key);
void Con_CalcCursorPosition();
void Con_AddLog(const char *text);
void Con_AddLine(const char *text, uint16_t font_style);
void Con_AddText(const char *text, uint16_t font_style);
void Con_Printf(const char *fmt, ...);
void Con_Warning(const char *fmt, ...);
void Con_Notify(const char *fmt, ...);

void Con_UpdateResize();

void Con_Clean();

void Con_Draw(float time);
int  Con_IsShown();
void Con_SetShown(int value);

#ifdef	__cplusplus
}
#endif
#endif
