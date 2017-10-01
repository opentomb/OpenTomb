
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_keycode.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <memory.h>

#include "utf8_32.h"
#include "gl_font.h"
#include "gl_util.h"
#include "console.h"
#include "system.h"
#include "vmath.h"
#include "gl_text.h"


static struct
{
    GLfloat                     background_color[4];
    GLfloat                     edit_font_color[4];

    uint32_t                    edit_size;
    char                       *edit_buff;

    uint16_t                    lines_count;
    uint16_t                    commands_count;
    uint16_t                    lines_buff_size;
    uint16_t                    commands_buff_size;
    uint16_t                   *lines_styles;
    char                      **lines_buff;
    char                      **commands_buff;
    
    int                       (*exec_cmd)(char *ch);        // Exec function pointer

    uint16_t                    line_height;                // Height, including spacing
    uint16_t                    height;                     // Console height in pixels

    float                       spacing;                    // Line spacing
    float                       cursor_time;                // Current cursor draw time
    float                       show_cursor_period;

    int16_t                     lines_scroll;
    int16_t                     cursor_pos;                 // Current cursor position, in symbols
    int16_t                     command_pos;
    uint16_t                    show_cursor : 1;            // Cursor visibility flag
    uint16_t                    show_console : 1;           // Visibility flag
} con_base;


static GLuint                   backgroundBuffer = 0;
static GLuint                   cursorBuffer = 0;

static void Con_FillBackgroundBuffer();
static void Con_DrawBackground();
static void Con_DrawCursor(GLint x, GLint y);
static void Con_AddCommandToHistory(const char *text);

void Con_Init()
{
    con_base.exec_cmd = NULL;

    con_base.edit_size = 4096;
    con_base.edit_buff = (char*)calloc(con_base.edit_size, sizeof(char));
    
    con_base.lines_buff_size = 128;
    con_base.lines_buff = (char**)calloc(con_base.lines_buff_size, sizeof(char*));
    con_base.lines_styles = (uint16_t*)calloc(con_base.lines_buff_size, sizeof(uint16_t));
    con_base.commands_buff_size = 128;   
    con_base.commands_buff = (char**)calloc(con_base.commands_buff_size, sizeof(char*));
    
    con_base.lines_scroll = 0;
    con_base.lines_count = 0;
    con_base.commands_count = 0;
    con_base.command_pos = 0;
    con_base.height = 240;

    // spacing check
    if(con_base.spacing < CON_MIN_LINE_INTERVAL)
    {
        con_base.spacing = CON_MIN_LINE_INTERVAL;
    }
    if(con_base.spacing > CON_MAX_LINE_INTERVAL)
    {
        con_base.spacing = CON_MAX_LINE_INTERVAL;
    }
}


void Con_SetExecFunction(int(*exec_cmd)(char *ch))
{
    con_base.exec_cmd = exec_cmd;
}


void Con_InitFont()
{
    qglGenBuffersARB(1, &backgroundBuffer);
    qglGenBuffersARB(1, &cursorBuffer);
    if(!GLText_GetFont(FONT_CONSOLE))
    {
        GLText_AddFont(0, 12, "resource/fonts/DroidSansMono.ttf");
    }
    if(GLText_GetFont(FONT_CONSOLE))
    {
        Con_SetLineInterval(con_base.spacing);
    }
    Con_FillBackgroundBuffer();
}


void Con_InitGlobals()
{
    con_base.background_color[0] = 1.0f;
    con_base.background_color[1] = 0.9f;
    con_base.background_color[2] = 0.7f;
    con_base.background_color[3] = 0.4f;

    con_base.edit_font_color[0] = 0.8f;
    con_base.edit_font_color[1] = 0.9f;
    con_base.edit_font_color[2] = 0.7f;
    con_base.edit_font_color[3] = 1.0f;

    con_base.exec_cmd        = NULL;
    con_base.spacing         = CON_MIN_LINE_INTERVAL;
    con_base.height          = 240;

    con_base.show_cursor_period = 0.5f;
    con_base.cursor_pos = 0;
}


void Con_Destroy()
{
    if(con_base.edit_buff)
    {
        free(con_base.edit_buff);
    }
    con_base.edit_buff = NULL;
    con_base.edit_size = 0;

    qglDeleteBuffersARB(1, &backgroundBuffer);
    qglDeleteBuffersARB(1, &cursorBuffer);
    backgroundBuffer = 0;
    cursorBuffer = 0;
    
    con_base.lines_count = 0;
    if(con_base.lines_buff)
    {
        for(uint16_t i = 0; i < con_base.lines_buff_size; ++i)
        {
            if(con_base.lines_buff[i])
            {
                free(con_base.lines_buff[i]);
            }
        }
        free(con_base.lines_buff);
        con_base.lines_buff = NULL;
        con_base.lines_buff_size = 0;
    }
    
    con_base.commands_count = 0;
    if(con_base.commands_buff)
    {
        for(uint16_t i = 0; i < con_base.commands_buff_size; ++i)
        {
            if(con_base.commands_buff[i])
            {
                free(con_base.commands_buff[i]);
            }
        }
        free(con_base.commands_buff);
        con_base.commands_buff = NULL;
        con_base.commands_buff_size = 0;
    }
}


float Con_GetLineInterval()
{
    return con_base.spacing;
}


void Con_SetLineInterval(float interval)
{
    if((interval >= CON_MIN_LINE_INTERVAL) && (interval <= CON_MAX_LINE_INTERVAL))
    {
        gl_tex_font_p gl_font = GLText_GetFont(FONT_CONSOLE);
        con_base.spacing = interval;
        if(gl_font)
        {
            con_base.line_height = (1.0 + con_base.spacing) * gl_font->font_size;
        }
    }
}


void Con_SetHeight(uint16_t value)
{
    if((value >= con_base.line_height) && (value <= screen_info.h))
    {
        con_base.height = value;
        Con_FillBackgroundBuffer();
    }
}


uint16_t Con_GetHeight()
{
    return con_base.height;
}


void Con_SetBackgroundColor(float color[4])
{
    vec4_copy(con_base.background_color, color);
}


void Con_SetShowCursorPeriod(float time)
{
    con_base.show_cursor_period = time;
}


void Con_SetLinesHistorySize(uint16_t count)
{
    if((count >= 16) && (count <= 32767) && (count != con_base.lines_buff_size))
    {
        char **new_buff = (char**)calloc(count, sizeof(char*));
        for(uint16_t i = 0; i < con_base.lines_count; ++i)
        {
            if(i < count)
            {
                new_buff[i] = con_base.lines_buff[i];
            }
            else
            {
                free(con_base.lines_buff[i]);
            }
        }
        free(con_base.lines_buff);
        con_base.lines_buff = new_buff;
        con_base.lines_buff_size = count;
        con_base.lines_count = (con_base.lines_count < count) ? (con_base.lines_count) : (count - 1);
    }
}


void Con_SetCommandsHistorySize(uint16_t count)
{
    if((count >= 16) && (count <= 32767) && (count != con_base.commands_buff_size))
    {
        char **new_buff = (char**)calloc(count, sizeof(char*));
        for(uint16_t i = 0; i < con_base.commands_count; ++i)
        {
            if(i < count)
            {
                new_buff[i] = con_base.commands_buff[i];
            }
            else
            {
                free(con_base.commands_buff[i]);
            }
        }
        free(con_base.commands_buff);
        con_base.commands_buff = new_buff;
        con_base.commands_buff_size = count;
        con_base.commands_count = (con_base.commands_count < count) ? (con_base.commands_count) : (count - 1);
    }
}


void Con_Scroll(int value)
{
    con_base.lines_scroll += value;
    con_base.lines_scroll = (con_base.lines_scroll < 0) ? (0) : (con_base.lines_scroll);
}


void Con_Filter(char *text)
{
    if(text)
    {
        uint8_t *utf8 = (uint8_t*)text;
        uint32_t utf32;
        while(*utf8)
        {
            utf8 = utf8_to_utf32(utf8, &utf32);
            Con_Edit(utf32);
        }
    }
}


void Con_Edit(int key)
{
    if(key == SDLK_UNKNOWN || key == SDLK_BACKQUOTE || key == SDLK_BACKSLASH)
    {
        return;
    }

    if((key == SDLK_RETURN) && con_base.edit_buff)
    {
        Con_AddCommandToHistory(con_base.edit_buff);
        if(con_base.exec_cmd && !con_base.exec_cmd(con_base.edit_buff))
        {
            Con_AddLine(con_base.edit_buff, 0);
        }
        con_base.edit_buff[0] = 0;
        con_base.cursor_pos = 0;
        con_base.command_pos = (con_base.commands_count > 0) ? (con_base.commands_count - 1) : (0);
        return;
    }

    con_base.cursor_time = 0.0f;
    con_base.show_cursor = 1;

    uint32_t oldLength = utf8_strlen(con_base.edit_buff);

    switch(key)
    {
        case SDLK_PAGEUP:
            Con_Scroll(1);
            break;
        
        case SDLK_PAGEDOWN:
            Con_Scroll(-1);
            break;
            
        case SDLK_UP:
            if(con_base.commands_count > 0)
            {
                strncpy(con_base.edit_buff, con_base.commands_buff[con_base.command_pos], con_base.edit_size);
                con_base.cursor_pos = utf8_strlen(con_base.edit_buff);
                con_base.command_pos++;
                con_base.command_pos = (con_base.command_pos >= con_base.commands_count) ? (0) : (con_base.command_pos);
            }
            break;

        case SDLK_DOWN:
            if(con_base.commands_count > 0)
            {
                strncpy(con_base.edit_buff, con_base.commands_buff[con_base.command_pos], con_base.edit_size);
                con_base.cursor_pos = utf8_strlen(con_base.edit_buff);
                con_base.command_pos--;
                con_base.command_pos = (con_base.command_pos < 0) ? (con_base.commands_count - 1) : (con_base.command_pos);
            }
            break;

        case SDLK_LEFT:
            if(con_base.cursor_pos > 0)
            {
                con_base.cursor_pos--;
            }
            break;

        case SDLK_RIGHT:
            if(con_base.cursor_pos < oldLength)
            {
                con_base.cursor_pos++;
            }
            break;

        case SDLK_HOME:
            con_base.cursor_pos = 0;
            break;

        case SDLK_END:
            con_base.cursor_pos = oldLength;
            break;

        case SDLK_BACKSPACE:
            if(con_base.cursor_pos > 0)
            {
                con_base.cursor_pos--;
                utf8_delete_char((uint8_t*)con_base.edit_buff, con_base.cursor_pos);
            }
            break;

        case SDLK_DELETE:
            if((con_base.cursor_pos < oldLength))
            {
                utf8_delete_char((uint8_t*)con_base.edit_buff, con_base.cursor_pos);
            }
            break;

        default:
            if(oldLength >= con_base.edit_size - 8)
            {
                char *new_buff = (char*)calloc(con_base.edit_size * 2, sizeof(char));
                if(new_buff)
                {
                    memcpy(new_buff, con_base.edit_buff, con_base.edit_size);
                    con_base.edit_buff = new_buff;
                    con_base.edit_size *= 2;
                }
            }
            if((oldLength < con_base.edit_size - 8) && (key >= SDLK_SPACE))
            {
                utf8_insert_char((uint8_t*)con_base.edit_buff, key, con_base.cursor_pos, con_base.edit_size);
                ++oldLength;
                con_base.cursor_pos++;
            }
            break;
    }
}


void Con_AddCommandToHistory(const char *text)
{
    if(text && *text)
    {
        uint32_t len = strlen(text);
        char *str = (char*)malloc((len + 1) * sizeof(char));
        memcpy(str, text, len);
        str[len] = 0;
        if(con_base.commands_count + 1 < con_base.commands_buff_size)
        {
            con_base.commands_buff[con_base.commands_count++] = str;
        }
        else
        {
            free(con_base.commands_buff[0]);
            for(uint16_t i = 1; i < con_base.commands_buff_size; ++i)
            {
                con_base.commands_buff[i - 1] = con_base.commands_buff[i];
            }
            con_base.commands_buff[con_base.commands_buff_size - 1] = str;
        }
    }
}


void Con_AddLine(const char *text, uint16_t font_style)
{
    if(text && *text)
    {
        uint32_t len = strlen(text);
        char *str = (char*)malloc((len + 1) * sizeof(char));
        memcpy(str, text, len);
        str[len] = 0;
        if(con_base.lines_count < con_base.lines_buff_size)
        {
            con_base.lines_styles[con_base.lines_count] = font_style;
            con_base.lines_buff[con_base.lines_count++] = str;
        }
        else
        {
            free(con_base.lines_buff[0]);
            for(uint16_t i = 1; i < con_base.lines_buff_size; ++i)
            {
                con_base.lines_buff[i - 1] = con_base.lines_buff[i];
                con_base.lines_styles[i - 1] = con_base.lines_styles[i];
            }
            con_base.lines_buff[con_base.lines_buff_size - 1] = str;
            con_base.lines_styles[con_base.lines_buff_size - 1] = font_style;
        }
    }
}


void Con_AddText(const char *text, uint16_t font_style)
{
    if(text)
    {
        char buf[4096], ch;
        size_t i, j, text_size = strlen(text);

        buf[0] = 0;
        for(i = 0, j = 0; i < text_size; i++)
        {
            ch = text[i];
            if((ch == 10) || (ch == 13))
            {
                j = (j < sizeof(buf)) ? (j) : (sizeof(buf) - 1);
                buf[j] = 0;
                buf[sizeof(buf) - 1] = 0;
                if((j > 0) && ((buf[0] != 10) && (buf[0] != 13) && ((buf[0] > 31) || (buf[1] > 32))))
                {
                    Con_AddLine(buf, font_style);
                }
                j = 0;
            }
            else if(j < sizeof(buf) - 1)
            {
               buf[j++] = ch;
            }
        }

        buf[sizeof(buf) - 1] = 0;
        if(j < sizeof(buf))
        {
            buf[j] = 0;
        }
        if((j > 0) && ((buf[0] != 10) && (buf[0] != 13) && ((buf[0] > 31) || (buf[1] > 32))))
        {
            Con_AddLine(buf, font_style);
        }
    }
}


void Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];

    va_start(argptr, fmt);
    vsnprintf(buf, sizeof(buf), fmt, argptr);
    buf[sizeof(buf) - 1] = 0;
    va_end(argptr);
    Con_AddLine(buf, FONTSTYLE_CONSOLE_NOTIFY);
}


void Con_Warning(const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];

    va_start(argptr, fmt);
    vsnprintf(buf, sizeof(buf), fmt, argptr);
    buf[sizeof(buf) - 1] = 0;
    va_end(argptr);
    Con_AddLine(buf, FONTSTYLE_CONSOLE_WARNING);
}


void Con_Notify(const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];

    va_start(argptr, fmt);
    vsnprintf(buf, sizeof(buf), fmt, argptr);
    buf[sizeof(buf) - 1] = 0;
    va_end(argptr);
    Con_AddLine(buf, FONTSTYLE_CONSOLE_NOTIFY);
}


void Con_UpdateResize()
{
    Con_SetLineInterval(con_base.spacing);
    Con_FillBackgroundBuffer();
}


void Con_Clean()
{
    for(uint16_t i = 0; i < con_base.lines_count; i++)
    {
        free(con_base.lines_buff[i]);
        con_base.lines_buff[i] = NULL;
    }
    con_base.lines_count = 0;
}

/*
 * CONSOLE DRAW FUNCTIONS
 */
void Con_Draw(float time)
{
    gl_tex_font_p gl_font = GLText_GetFont(FONT_CONSOLE);
    if(gl_font && con_base.show_console)
    {
        GLfloat y;
        int32_t con_bottom, cursor_x = 8;
        int32_t w_pt = (screen_info.w - 16) * 64;
        int32_t total_chars = 0;
        char *begin = con_base.edit_buff;
        char *end = con_base.edit_buff;
        int cursor_line = 0;
        int n_sym = 0;
        int n_lines = 1;

        con_base.height = (con_base.height > screen_info.h) ? (screen_info.h) : (con_base.height);

        for(char *ch = glf_get_string_for_width(gl_font, con_base.edit_buff, w_pt, &n_sym); *begin; ch = glf_get_string_for_width(gl_font, ch, w_pt, &n_sym))
        {
            n_lines += (*ch) ? (1) : (0);
            if(con_base.cursor_pos > total_chars + n_sym)
            {
                ++cursor_line;
            }
            else if(con_base.cursor_pos >= total_chars)
            {
                cursor_x += glf_get_string_len(gl_font, begin, con_base.cursor_pos - total_chars) / 64;
            }
            total_chars += n_sym;
            begin = ch;
        }

        con_bottom = screen_info.h - con_base.height + 0.2f * (GLfloat)con_base.line_height;
        y = (n_lines - cursor_line) * con_base.line_height - con_base.height;
        if(y > 0)
        {
            con_bottom -= y;
        }

        con_base.cursor_time += time;
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        BindWhiteTexture();
        Con_DrawBackground();
        Con_DrawCursor(cursor_x, con_bottom + (n_lines - cursor_line - 1) * con_base.line_height);

        begin = con_base.edit_buff;
        n_sym = -1;
        for(int line = n_lines - 1; line >= 0; --line)
        {
            y = con_bottom + line * con_base.line_height;
            if(n_lines > 1)
            {
                end = glf_get_string_for_width(gl_font, begin, w_pt, &n_sym);
            }
            if(y < screen_info.h - con_base.height)
            {
                break;
            }
            if(y < screen_info.h)
            {
                vec4_copy(gl_font->gl_font_color, con_base.edit_font_color);
                glf_render_str(gl_font, 8, y, begin, n_sym);
            }
            begin = end;
        }
        
        con_base.lines_scroll = (con_base.lines_scroll + 1 > con_base.lines_count) ? (con_base.lines_count - 1) : (con_base.lines_scroll);
        con_base.lines_scroll = (con_base.lines_scroll < 0) ? (0) : (con_base.lines_scroll);
        for(uint16_t i = con_base.lines_scroll; i < con_base.lines_count; i++)
        {
            char *str = con_base.lines_buff[con_base.lines_count - i - 1];
            gl_fontstyle_p style = GLText_GetFontStyle(con_base.lines_styles[con_base.lines_count - i - 1]);
            begin = str;
            end = str;
            for(char *ch = glf_get_string_for_width(gl_font, begin, w_pt, &n_sym); style && *begin; ch = glf_get_string_for_width(gl_font, ch, w_pt, &n_sym))
            {
                y += con_base.line_height;
                if(y > screen_info.h)
                {
                    return;
                }
                vec4_copy(gl_font->gl_font_color, style->font_color);
                glf_render_str(gl_font, 8, y, begin, n_sym);
                begin = ch;
            }
        }
    }
}


void Con_FillBackgroundBuffer()
{
    if(backgroundBuffer)
    {
        GLfloat x0 = 0.0f;
        GLfloat y0 = screen_info.h - con_base.height;
        GLfloat x1 = screen_info.w;
        GLfloat y1 = screen_info.h;
        GLfloat *v, backgroundArray[32];

        v = backgroundArray;
       *v++ = x0; *v++ = y0;
        vec4_copy(v, con_base.background_color);
        v += 4;
       *v++ = 0.0f; *v++ = 0.0f;

       *v++ = x1; *v++ = y0;
        vec4_copy(v, con_base.background_color);
        v += 4;
       *v++ = 1.0f; *v++ = 0.0f;

       *v++ = x1; *v++ = y1;
        vec4_copy(v, con_base.background_color);
        v += 4;
       *v++ = 1.0f; *v++ = 1.0f;

       *v++ = x0; *v++ = y1;
        vec4_copy(v, con_base.background_color);
        v += 4;
       *v++ = 0.0f; *v++ = 1.0f;
        qglBindBufferARB(GL_ARRAY_BUFFER, backgroundBuffer);
        qglBufferDataARB(GL_ARRAY_BUFFER, 32 * sizeof(GLfloat), backgroundArray, GL_STATIC_DRAW);
    }
}


void Con_DrawBackground()
{
    if(backgroundBuffer)
    {
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, backgroundBuffer);
        qglVertexPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), (void *)0);
        qglColorPointer(4, GL_FLOAT, 8 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
        qglTexCoordPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), (void *)(6 * sizeof(GLfloat)));
        qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}


void Con_DrawCursor(GLint x, GLint y)
{
    if(con_base.show_cursor_period)
    {
        if(con_base.cursor_time > con_base.show_cursor_period)
        {
            con_base.cursor_time = 0.0f;
            con_base.show_cursor = !con_base.show_cursor;
        }
    }

    if(con_base.show_cursor && cursorBuffer)
    {
        GLfloat line_w = 1.0f;
        GLfloat cursor_array[16];
        GLfloat *v = cursor_array;

       *v++ = (GLfloat)x;
       *v++ = (GLfloat)y - 0.1 * (GLfloat)con_base.line_height;
        v[0] = 1.0; v[1] = 1.0; v[2] = 1.0; v[3] = 0.7;             v += 4;
        v[0] = 0.0; v[1] = 0.0;                                     v += 2;
       *v++ = (GLfloat)x;
       *v++ = (GLfloat)y + 0.7 * (GLfloat)con_base.line_height;
        v[0] = 1.0; v[1]= 1.0; v[2] = 1.0; v[3] = 0.7;              v += 4;
        v[0] = 0.0; v[1] = 0.0;

        qglGetFloatv(GL_LINE_WIDTH, &line_w);
        qglLineWidth(1.0f);        
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, cursorBuffer);
        qglBufferDataARB(GL_ARRAY_BUFFER, 16 * sizeof(GLfloat), cursor_array, GL_DYNAMIC_DRAW);
        qglVertexPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), (void *)0);
        qglColorPointer(4, GL_FLOAT, 8 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
        qglTexCoordPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), (void *)(6 * sizeof(GLfloat)));
        qglDrawArrays(GL_LINES, 0, 2);
        qglLineWidth(line_w);
    }
}


int Con_IsShown()
{
    return con_base.show_console;
}


void Con_SetShown(int value)
{
    con_base.show_console = value;
}
