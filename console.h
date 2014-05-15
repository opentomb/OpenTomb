
#ifndef CONSOLE_H
#define CONSOLE_H

#include "ftgl/FTGLBitmapFont.h"
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
    char                        font_patch[255];            // путь к файлу шрифта
    FTGLBitmapFont             *font_bitmap;                // рендерер простого шрифта
    FTGLTextureFont            *font_texture;               // рендерер сглаженного шрифта

    uint16_t                    font_size;                  // размер шрифта в чем то =)
    GLfloat                     font_color[4];
    GLfloat                     background_color[4];

    int8_t                      smooth;                     // Режим вывода текста - сглаживание вкл.

    int                         log_lines_count;            // объём лога
    int                         log_pos;                    // текущая позиция указателя в логе
    int                         line_size;                  // Размер строки в консоли
    int                         shown_lines_count;          // количество показываемых строк
    int                         showing_lines;              // количество отображаемых строк
    float                       spacing;                    // межстрочный интервал %) - это уже ворд, а не движок
    float                       show_cursor_period;
    float                       cursor_time;                // текущее время отрисовки курсора
    int8_t                      show_cursor;                // флаг видимости курсора

    int16_t                     line_height;                // высота строки с учетом отступа в пикселах
    int16_t                     cursor_pos;                 // положение курсора в строке (в смволах)
    int16_t                     cursor_x;                   // положение курсора в пикселах x
    int16_t                     cursor_y;                   // положение курсора в пикселах y
    char                      **shown_lines;                // текст консоли
    char                      **log_lines;                  // текст консоли
    int8_t                      inited;                     // флаг готовности к использованию
    int8_t                      show;                       // флаг видимости консоли
}console_info_t, *console_info_p;

extern console_info_t con_base;

void Con_InitGlobals();
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
