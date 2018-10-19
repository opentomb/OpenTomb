
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "../gl_util.h"
#include "../gl_font.h"
#include "../gl_text.h"
#include "../vmath.h"
#include "../console.h"
#include "gui_obj.h"


typedef struct gui_console_data_s
{
    int16_t         lines_scroll;
    uint16_t        show;
}gui_console_data_t, *gui_console_data_p;


void handle_screen_resized_console(struct gui_object_s *obj, int w, int h)
{
    obj->w = w;
    obj->x = 0;
    obj->y = h - obj->h;
    if(obj->y > h)
    {
        obj->y = h;
    }
    Gui_LayoutObjects(obj);
}


void handle_delete_console_data(void *data)
{
    free(data);
}


static void Gui_FillConsoleLog(gui_object_p log, console_params_p cp, gui_console_data_p data)
{
    uint16_t lines_count = 0; 
    char **lines_buff = NULL;
    uint16_t *lines_styles = NULL;
    gui_object_p *line = &log->childs;
    gui_object_p prev = NULL;
    
    gl_tex_font_p gl_font = GLText_GetFont(FONT_CONSOLE);
    int32_t ascender = cp->spacing * glf_get_ascender(gl_font) / 64.0f;
    int32_t descender = cp->spacing * glf_get_descender(gl_font) / 64.0f;
    int32_t max_lines = cp->height / (ascender - descender);
    uint16_t begin = (lines_count <= max_lines) ? 0 : (lines_count - max_lines);
    
    Con_GetLines(&lines_count, &lines_buff, &lines_styles);
    
    if(data->lines_scroll >= lines_count)
    {
        data->lines_scroll = (lines_count) ? (lines_count - 1) : (0);
    }
    begin = (begin > data->lines_scroll) ? (begin - data->lines_scroll) : (0);
    
    for(uint16_t i = begin; i < lines_count - data->lines_scroll; ++i, line = &(*line)->next)
    {
        if(!*line)
        {
            *line = Gui_CreateObject();
            (*line)->prev = prev;
            (*line)->parent = log;
            (*line)->flags.word_wrap = 0x01;
            (*line)->flags.autoheight = 0x01;
        }
        
        Gui_SetExternalObjectLabel(*line, lines_buff[i], FONT_CONSOLE, lines_styles[i]);
        (*line)->label->line_height = cp->spacing;
        (*line)->flags.hide = 0x00;
        (*line)->flags.draw_label = 0x01;
        
        prev = *line;
    }
    
    for(; *line; line = &(*line)->next)
    {
        (*line)->flags.hide = 0x01;
    }
}


void Gui_RefreshConsole(gui_object_p *con_root, int w, int h)
{
    console_params_t console;
    gui_object_p log = NULL;
    gui_object_p edit = NULL;
    
    Con_GetParams(&console);
    if(!*con_root)
    {
        gui_console_data_p data = (gui_console_data_p)malloc(sizeof(gui_console_data_t));
        data->lines_scroll = 0;
        data->show = 0x00;
        *con_root = Gui_CreateObject();
        (*con_root)->data = data;
        (*con_root)->handlers.screen_resized = handle_screen_resized_console;
        (*con_root)->handlers.delete_user_data = handle_delete_console_data;
        (*con_root)->flags.layout = GUI_LAYOUT_VERTICAL;
        (*con_root)->flags.v_content_align = GUI_ALIGN_BOTTOM;
        (*con_root)->flags.h_content_align = GUI_ALIGN_CENTER;
        (*con_root)->flags.hide = 0x01;
        (*con_root)->flags.fixed_w = 0x01;
        (*con_root)->flags.fixed_h = 0x01;
        (*con_root)->flags.fit_inside = 0x01;
        
        log = Gui_CreateChildObject(*con_root);
        log->flags.clip_children = 0x01;
        log->flags.layout = GUI_LAYOUT_VERTICAL;
        log->flags.v_content_align = GUI_ALIGN_BOTTOM;
        log->flags.h_content_align = GUI_ALIGN_LEFT;
        log->margin_left = 8;
        log->margin_right = 8;
        
        edit = Gui_CreateChildObject(*con_root);
        edit->flags.draw_background = 0x01;
        edit->flags.fixed_h = 0x01;
        edit->flags.edit_text = 0x01;
        edit->flags.word_wrap = 0x01;
        edit->flags.autoheight = 0x01;
        edit->flags.h_content_align = GUI_ALIGN_LEFT;
        edit->margin_left = 8;
        edit->margin_right = 8;
        Gui_SetObjectLabel(edit, "", FONT_CONSOLE, FONTSTYLE_CONSOLE_EVENT);
    }
    
    (*con_root)->h = console.height;
    
    log = (*con_root)->childs;
    vec4_copy(log->color_background, console.background_color);
    log->flags.draw_background = 0x01;
    log->weight_y = 1;
    Gui_FillConsoleLog(log, &console, (gui_console_data_p)(*con_root)->data);
    
    edit = (*con_root)->childs->next;
    vec4_copy(edit->color_background, console.background_color);
    edit->flags.draw_background = 0x01;
    edit->flags.draw_label = 0x01;
    edit->label->line_height = console.spacing;
    
    (*con_root)->handlers.screen_resized(*con_root, w, h);
}

void Gui_ConScrollInternal(gui_object_p con_root, int value)
{
    if(con_root)
    {
        gui_console_data_p data = (gui_console_data_p)con_root->data;
        data->lines_scroll += value;
        data->lines_scroll = (data->lines_scroll < 0) ? (0) : (data->lines_scroll);
    }
}

void Gui_HandleEditConsole(int cmd, uint32_t key, void *data)
{
    gui_object_p console = (gui_object_p)data;
    gui_object_p edit = console->childs->next;
    char *exec = NULL;
    switch(cmd)
    {
        case GUI_COMMAND_ACTIVATE:
            Con_Exec(edit->label->text);
            edit->label->text[0] = 0;
            edit->label->cursor_pos = 0;
            break;
            
        case GUI_COMMAND_CLOSE:
            break;
            
        case GUI_COMMAND_UP:
        case GUI_COMMAND_DOWN:
            exec = Con_ListExecHistory(cmd == GUI_COMMAND_UP);
            if(exec && *exec)
            {
                Gui_SetObjectLabel(edit, exec, FONT_CONSOLE, FONTSTYLE_CONSOLE_EVENT);
            }
            break;

        default:
            Gui_ApplyEditCommands(edit, cmd, key);
            break;
    }
}