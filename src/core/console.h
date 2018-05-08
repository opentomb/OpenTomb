
#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CON_MIN_LINE_INTERVAL 0.5
#define CON_MAX_LINE_INTERVAL 4.0

typedef struct console_params_s
{
    uint8_t         background_color[4];
    uint16_t        commands_count;
    uint16_t        lines_count;
    uint16_t        height;
    uint16_t        show;
    float           spacing;
    float           show_cursor_period;
}console_params_t, *console_params_p;

void Con_Init();
void Con_SetExecFunction(int(*exec_cmd)(char *ch));
void Con_InitFont();
void Con_InitGlobals();
void Con_Destroy();
void Con_GetParams(console_params_p cp);

float Con_GetLineInterval();
void  Con_SetLineInterval(float interval);
void Con_SetHeight(uint16_t value);
uint16_t Con_GetHeight();
void Con_SetBackgroundColor(float color[4]);
void Con_SetShowCursorPeriod(float time);

void Con_SetLinesHistorySize(uint16_t count);
void Con_SetCommandsHistorySize(uint16_t count);

void Con_Scroll(int value);
void Con_Filter(char *text);
void Con_Edit(int key);
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
