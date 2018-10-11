
#ifndef ENGINE_GUI_OBJ_H
#define ENGINE_GUI_OBJ_H

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

#include "../core/gl_util.h"
#include "../core/gl_font.h"

#define GUI_ALIGN_NONE      (0)
#define GUI_ALIGN_CENTER    (1)
#define GUI_ALIGN_TOP       (2)
#define GUI_ALIGN_BOTTOM    (3)
#define GUI_ALIGN_LEFT      (2)
#define GUI_ALIGN_RIGHT     (3)

#define GUI_LAYOUT_NONE         (0)
#define GUI_LAYOUT_VERTICAL     (1)
#define GUI_LAYOUT_HORIZONTAL   (2)

#define GUI_COMMAND_NONE        (0)
#define GUI_COMMAND_TEXT        (1)
#define GUI_COMMAND_OPEN        (2)
#define GUI_COMMAND_CLOSE       (3)
#define GUI_COMMAND_LEFT        (4)
#define GUI_COMMAND_RIGHT       (5)
#define GUI_COMMAND_UP          (6)
#define GUI_COMMAND_DOWN        (7)
#define GUI_COMMAND_HOME        (8)
#define GUI_COMMAND_END         (9)
#define GUI_COMMAND_DELETE      (10)
#define GUI_COMMAND_BACKSPACE   (11)
#define GUI_COMMAND_ACTIVATE    (12)
#define GUI_COMMAND_DEACTIVATE  (13)

struct gui_object_s;


typedef struct gui_object_flags_s
{
    uint32_t    hide : 1;
    uint32_t    draw_background : 1;
    uint32_t    draw_border : 1;
    uint32_t    draw_label : 1;
    uint32_t    word_wrap : 1;
    uint32_t    fixed_w : 1;
    uint32_t    fixed_h : 1;
    uint32_t    fit_inside : 1;
    uint32_t    v_content_align : 4;
    uint32_t    h_content_align : 4;
    uint32_t    v_self_align : 4;
    uint32_t    h_self_align : 4;
    uint32_t    layout : 2;
    uint32_t    clip_children : 1;
    uint32_t    edit_text : 1;
}gui_object_flags_t, *gui_object_flags_p;

typedef struct gui_handlers_s
{
    int  (*do_command)(struct gui_object_s *obj, int cmd);
    void (*screen_resized)(struct gui_object_s *obj, int w, int h);
    void (*delete_user_data)(void *data);
}gui_handlers_t, *gui_handlers_p;

typedef struct gui_object_text_s
{
    char                       *text;
    float                       line_height;
    uint16_t                    font_id;
    uint16_t                    style_id;
    uint16_t                    text_size;
    uint16_t                    cursor_pos;
}gui_object_text_t, *gui_object_text_p;

typedef struct gui_object_s
{
    int16_t         x;
    int16_t         y;
    int16_t         w;
    int16_t         h;
    int16_t         weight_x;
    int16_t         weight_y;
    int16_t         content_dx;
    int16_t         content_dy;
    int16_t         margin_left;
    int16_t         margin_right;
    int16_t         margin_top;
    int16_t         margin_bottom;
    
    struct gui_handlers_s       handlers;
    void                       *data;
    struct gui_object_text_s   *label;
    
    uint16_t                    border_width;
    uint16_t                    spacing;
    
    struct gui_object_flags_s   flags;

    uint8_t                     color_border[4];
    uint8_t                     color_background[4];    
    
    struct gui_object_s        *parent;
    struct gui_object_s        *next;
    struct gui_object_s        *prev;
    struct gui_object_s        *childs;
} gui_object_t, *gui_object_p;


gui_object_p Gui_CreateObject();
void Gui_DeleteObject(gui_object_p obj);
void Gui_DeleteObjects(gui_object_p root);

gui_object_p Gui_CreateChildObject(gui_object_p root);
void Gui_DeleteChildObject(gui_object_p obj);

void Gui_SetObjectLabel(gui_object_p obj, const char *text, uint16_t font_id, uint16_t style_id);
void Gui_SetExternalObjectLabel(gui_object_p obj, const char *text, uint16_t font_id, uint16_t style_id);
void Gui_DrawObjects(gui_object_p root);
void Gui_LayoutVertical(gui_object_p root);
void Gui_LayoutHorizontal(gui_object_p root);
void Gui_LayoutObjects(gui_object_p root);
void Gui_EnsureVisible(gui_object_p obj);

void Gui_ApplyEditCommands(gui_object_p edit, int cmd, uint32_t key);

#ifdef	__cplusplus
}
#endif

#endif
