
#ifndef ENGINE_GUI_OBJ_H
#define ENGINE_GUI_OBJ_H

#include <stdint.h>

#include "../core/gl_util.h"
#include "../core/gl_font.h"
#include "../core/gl_text.h"

typedef struct gui_object_flags_s
{
    uint32_t    hide : 1;
    uint32_t    draw_background : 1;
    uint32_t    draw_border : 1;
    uint32_t    draw_label : 1;
    uint32_t    word_wrap : 1;
    uint32_t    clip_children : 1;
    uint32_t    layout : 2;
}gui_object_flags_t, *gui_object_flags_p;

typedef struct gui_object_s
{
    int16_t         x;
    int16_t         y;
    int16_t         w;
    int16_t         h;
    int16_t         content_w;
    int16_t         content_h;
    int16_t         content_dx;
    int16_t         content_dy;
    
    void                       *data;
    char                       *text;
    float                       line_height;
    uint16_t                    font_id;
    uint16_t                    style_id;
    uint16_t                    text_size;
    uint8_t                     border_width;
    uint8_t                     v_align : 4;
    uint8_t                     h_align : 4;
    
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
void Gui_DrawObjects(gui_object_p root);
void Gui_LayoutVertical(gui_object_p root, int16_t spacing, int16_t margin);
void Gui_LayoutHorizontal(gui_object_p root, int16_t spacing, int16_t margin);

#endif
