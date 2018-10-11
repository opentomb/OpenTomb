
#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdarg.h>

#define CON_MIN_LINE_INTERVAL 0.5
#define CON_MAX_LINE_INTERVAL 4.0

typedef struct console_params_s
{
    uint8_t         background_color[4];
    uint16_t        commands_count;
    uint16_t        lines_count;
    uint16_t        height;
    float           spacing;
}console_params_t, *console_params_p;

void Con_Init();
void Con_SetExecFunction(int(*exec_cmd)(char *ch));
void Con_InitGlobals();
void Con_Destroy();
void Con_SetParams(console_params_p cp);
void Con_GetParams(console_params_p cp);
void Con_GetLines(uint16_t *lines_count, char ***lines_buff, uint16_t **lines_styles);
char *Con_ListExecHistory(int direction);

void Con_SetLinesHistorySize(uint16_t count);
void Con_SetCommandsHistorySize(uint16_t count);

void Con_Exec(char *text);
void Con_AddLine(const char *text, uint16_t font_style);
void Con_AddText(const char *text, uint16_t font_style);
void Con_Printf(const char *fmt, ...);
void Con_Warning(const char *fmt, ...);
void Con_Notify(const char *fmt, ...);

void Con_Clean();

#ifdef	__cplusplus
}
#endif
#endif
