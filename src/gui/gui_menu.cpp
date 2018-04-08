
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "../core/system.h"
#include "../core/vmath.h"
#include "gui.h"
#include "gui_menu.h"


static void Gui_SetupMenuObj(gui_object_p root)
{
    root->w = screen_info.w * 0.5;
    root->h = screen_info.h * 0.6;
    root->x = screen_info.w * 0.25;
    root->y = screen_info.h * 0.2;
    root->color_border[0] = 65;
    root->color_border[1] = 21;
    root->color_border[2] = 22;
    root->color_border[3] = 255;
    root->color_background[0] = 32;
    root->color_background[1] = 21;
    root->color_background[2] = 22;
    root->color_background[3] = 126;
    root->border_width = 4;
    root->flags.draw_border = 0x01;
    root->flags.draw_background = 0x01;
    root->flags.clip_children = 0x00;
}

static gui_object_p Gui_AdddMenuObj(gui_object_p root)
{
    gui_object_p obj = Gui_CreateChildObject(root);
    obj->w = root->w - 2 * root->border_width - 4;
    obj->h = 40;
    vec4_copy(obj->color_border, root->color_border);
    vec4_copy(obj->color_border, root->color_border);
    obj->border_width = 4;
    return obj;
}

gui_object_p Gui_BuildSavesList()
{
    gui_object_p root = Gui_CreateObject();
    Gui_SetupMenuObj(root);

    gui_object_p obj = Gui_AdddMenuObj(root);
    obj->x = root->border_width + 2;
    obj->y = root->h - (obj->h + root->border_width + 2);
    obj->flags.draw_border = 0x01;
    Gui_SetObjectLabel(obj, "Load game:", 1, 1);
    obj->h_align = GLTEXT_ALIGN_CENTER;
    obj->v_align = GLTEXT_ALIGN_CENTER;
    obj->flags.draw_label = 0x01;
    obj->line_height = 0.8;

    gui_object_p cont = Gui_AdddMenuObj(root);
    cont->w = root->w - 2 * root->border_width;
    cont->h = root->h - obj->h - 2 * root->border_width;
    cont->x = root->border_width;
    cont->y = root->border_width;

    cont->border_width = 4;
    cont->flags.clip_children = 0x01;
    cont->flags.draw_background = 0x00;
    cont->flags.draw_border = 0x00;

    file_info_p list = Sys_ListDir("save", NULL);
    for(file_info_p it = list; it; it = it->next)
    {
        if(!it->is_dir)
        {
            obj = Gui_AdddMenuObj(cont);
            obj->w = cont->w - 2 * cont->border_width;
            obj->h = 32;
            obj->x = cont->border_width;
            obj->y = ((obj->prev) ? (obj->prev->y) : (cont->h - cont->border_width)) - obj->h;
            obj->flags.draw_border = (obj->prev) ? (0x00) : (0x01);
            obj->border_width = 3;
            obj->color_border[0] = 220;
            obj->color_border[1] = 211;
            obj->color_border[2] = 242;
            obj->color_border[3] = 255;

            Gui_SetObjectLabel(obj, it->name, 2, 2);
            obj->h_align = GLTEXT_ALIGN_CENTER;
            obj->v_align = GLTEXT_ALIGN_CENTER;
            obj->flags.draw_label = 0x01;
            obj->line_height = 0.8;
        }
    }
    Sys_ListDirFree(list);

    return root;
}

gui_object_p Gui_ListSaves(gui_object_p root, int dy)
{
    gui_object_p ret = NULL;
    if(root && root->childs && root->childs->next)
    {
        gui_object_p cont = root->childs->next;
        for(gui_object_p obj = cont->childs; obj; obj = obj->next)
        {
            if(obj->flags.draw_border)
            {
                ret = obj;
                if((dy > 0) && obj->prev)
                {
                    ret = obj->prev;
                    obj->flags.draw_border = 0x00;
                    ret->flags.draw_border = 0x01;
                    if(ret->y + obj->h + cont->content_dy + cont->border_width > cont->h)
                    {
                        cont->content_dy = cont->h - ret->h - ret->y - cont->border_width;
                    }
                }
                else if((dy < 0) && obj->next)
                {
                    ret = obj->next;
                    obj->flags.draw_border = 0x00;
                    ret->flags.draw_border = 0x01;
                    if(ret->y + cont->content_dy < cont->border_width)
                    {
                        cont->content_dy = cont->border_width - ret->y;
                    }
                }
                break;
            }
        }
    }
    return ret;
}
