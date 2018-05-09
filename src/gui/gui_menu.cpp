
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "../core/system.h"
#include "../core/vmath.h"
#include "../controls.h"
#include "../game.h"
#include "../gameflow.h"
#include "gui.h"
#include "gui_menu.h"


static gui_object_p Gui_CreateMenuRoot();
static gui_object_p Gui_AddListItem(gui_object_p cont);
static gui_object_p Gui_ListInventoryMenu(gui_object_p root, int dy);

extern "C" int handle_to_container(struct gui_object_s *obj, enum gui_command_e cmd);
extern "C" int handle_to_item(struct gui_object_s *obj, enum gui_command_e cmd);

extern "C" int handle_on_crosshair(struct gui_object_s *obj, enum gui_command_e cmd);
extern "C" int handle_load_game_cont(struct gui_object_s *obj, enum gui_command_e cmd);
extern "C" int handle_save_game_cont(struct gui_object_s *obj, enum gui_command_e cmd);
extern "C" int handle_new_game_cont(struct gui_object_s *obj, enum gui_command_e cmd);
extern "C" int handle_home_cont(struct gui_object_s *obj, enum gui_command_e cmd);
extern "C" int handle_controls_cont(struct gui_object_s *obj, enum gui_command_e cmd);
extern "C" int handle_main_menu(struct gui_object_s *obj, enum gui_command_e cmd);
extern "C" void handle_screen_resized_inv(struct gui_object_s *obj, int w, int h);
extern "C" void handle_screen_resized_main(struct gui_object_s *obj, int w, int h);

struct gui_controls_data_s
{
    uint32_t    wait_key : 1;
    uint32_t    is_primary : 1;
};

static gui_object_p Gui_CreateMenuRoot()
{
    gui_object_p root = Gui_CreateObject();
    root->color_border[0] = 232;
    root->color_border[1] = 192;
    root->color_border[2] = 112;
    root->color_border[3] = 255;
    root->color_background[0] = 32;
    root->color_background[1] = 21;
    root->color_background[2] = 22;
    root->color_background[3] = 126;
    root->border_width = 4;

    root->spacing = 4;
    root->margin_top = 8;
    root->margin_bottom = 8;
    root->margin_left = 8;
    root->margin_right = 8;

    root->flags.draw_border = 0x01;
    root->flags.draw_background = 0x01;
    root->flags.clip_children = 0x00;
    root->flags.h_content_align = GUI_ALIGN_CENTER;
    root->flags.v_content_align = GUI_ALIGN_TOP;
    root->flags.layout = GUI_LAYOUT_VERTICAL;
    root->flags.fit_inside = 0x01;
    return root;
}

static gui_object_p Gui_AddListItem(gui_object_p cont)
{
    gui_object_p obj = Gui_CreateChildObject(cont);
    obj->h = 32;
    obj->border_width = 3;
    obj->color_border[0] = 232;
    obj->color_border[1] = 192;
    obj->color_border[2] = 112;
    obj->color_border[3] = 255;

    obj->flags.h_content_align = GUI_ALIGN_CENTER;
    obj->flags.v_content_align = GUI_ALIGN_CENTER;
    obj->flags.fixed_h = 0x01;
    obj->line_height = 0.8;
    return obj;
}

static gui_object_p Gui_AddLoadGameContainer(gui_object_p root)
{
    gui_object_p cont = Gui_CreateChildObject(root);
    cont->handlers.do_command = handle_load_game_cont;
    cont->w = root->w - root->margin_left - root->margin_right;

    cont->border_width = 0;
    cont->flags.clip_children = 0x01;
    cont->flags.draw_background = 0x00;
    cont->flags.draw_border = 0x00;
    cont->flags.layout = GUI_LAYOUT_VERTICAL;
    cont->flags.h_content_align = GUI_ALIGN_CENTER;
    cont->weight_y = 1;

    file_info_p list = Sys_ListDir("save", NULL);
    for(file_info_p it = list; it; it = it->next)
    {
        if(!it->is_dir)
        {
            gui_object_p obj = Gui_AddListItem(cont);
            obj->flags.draw_border = (obj->prev) ? (0x00) : (0x01);
            Gui_SetObjectLabel(obj, it->name, 2, 2);
            obj->flags.draw_label = 0x01;
        }
    }
    Sys_ListDirFree(list);

    return cont;
}

static gui_object_p Gui_AddSaveGameContainer(gui_object_p root)
{
    gui_object_p cont = Gui_CreateChildObject(root);
    cont->handlers.do_command = handle_save_game_cont;
    cont->w = root->w - root->margin_left - root->margin_right;

    cont->border_width = 0;
    cont->flags.clip_children = 0x01;
    cont->flags.draw_background = 0x00;
    cont->flags.draw_border = 0x00;
    cont->flags.layout = GUI_LAYOUT_VERTICAL;
    cont->flags.h_content_align = GUI_ALIGN_CENTER;
    cont->weight_y = 1;

    gui_object_p obj = Gui_AddListItem(cont);
    obj->flags.draw_border = (obj->prev) ? (0x00) : (0x01);
    Gui_SetObjectLabel(obj, "NEW SAVE", 2, 2);
    obj->flags.draw_label = 0x01;

    file_info_p list = Sys_ListDir("save", NULL);
    for(file_info_p it = list; it; it = it->next)
    {
        if(!it->is_dir)
        {
            obj = Gui_AddListItem(cont);
            obj->flags.draw_border = 0x00;
            Gui_SetObjectLabel(obj, it->name, 2, 2);
            obj->flags.draw_label = 0x01;
        }
    }
    Sys_ListDirFree(list);

    return cont;
}

static gui_object_p Gui_AddNewGameContainer(gui_object_p root)
{
    gui_object_p cont = Gui_CreateChildObject(root);
    cont->handlers.do_command = handle_new_game_cont;
    cont->w = root->w - root->margin_left - root->margin_right;

    cont->border_width = 0;
    cont->flags.clip_children = 0x01;
    cont->flags.draw_background = 0x00;
    cont->flags.draw_border = 0x00;
    cont->flags.layout = GUI_LAYOUT_VERTICAL;
    cont->flags.h_content_align = GUI_ALIGN_CENTER;
    cont->weight_y = 1;

    gui_object_p obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x01;
    Gui_SetObjectLabel(obj, "Tomb Raider I", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_1;

    obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x00;
    Gui_SetObjectLabel(obj, "Tomb Raider I UB", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_1_5;

    obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x00;
    Gui_SetObjectLabel(obj, "Tomb Raider II", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_2;

    obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x00;
    Gui_SetObjectLabel(obj, "Tomb Raider II Gold", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_2_5;

    obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x00;
    Gui_SetObjectLabel(obj, "Tomb Raider III", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_3;

    obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x00;
    Gui_SetObjectLabel(obj, "Tomb Raider III Gold", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_3_5;

    obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x00;
    Gui_SetObjectLabel(obj, "Tomb Raider IV", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_4;

    obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x00;
    Gui_SetObjectLabel(obj, "Tomb Raider V", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_5;

    return cont;
}

static gui_object_p Gui_AddHomeContainer(gui_object_p root)
{
    gui_object_p cont = Gui_CreateChildObject(root);
    cont->handlers.do_command = handle_home_cont;
    cont->w = root->w - root->margin_left - root->margin_right;

    cont->border_width = 0;
    cont->flags.clip_children = 0x01;
    cont->flags.draw_background = 0x00;
    cont->flags.draw_border = 0x00;
    cont->flags.layout = GUI_LAYOUT_VERTICAL;
    cont->flags.h_content_align = GUI_ALIGN_CENTER;
    cont->weight_y = 1;

    gui_object_p obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x01;
    Gui_SetObjectLabel(obj, "Tomb Raider I", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_1;

    obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x00;
    Gui_SetObjectLabel(obj, "Tomb Raider I UB", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_1_5;

    obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x00;
    Gui_SetObjectLabel(obj, "Tomb Raider II", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_2;

    obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x00;
    Gui_SetObjectLabel(obj, "Tomb Raider II Gold", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_2_5;

    obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x00;
    Gui_SetObjectLabel(obj, "Tomb Raider III", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_3;

    obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x00;
    Gui_SetObjectLabel(obj, "Tomb Raider III Gold", 2, 2);
    obj->flags.draw_label = 0x01;
    obj->data = (void*)GAME_3_5;

    return cont;
}

static void Gui_AddControlsHObj(gui_object_p cont, int ctrl)
{
    if((ctrl >= 0) && (ctrl < ACT_LASTINDEX))
    {
        control_action_p act = control_states.actions + ctrl;
        gui_object_p obj = Gui_AddListItem(cont);
        char buff[128];
        obj->data = act;
        obj->border_width = 2;
        obj->spacing = 4;
        obj->margin_bottom = 4;
        obj->margin_top = 4;
        obj->margin_left = 4;
        obj->margin_right = 4;
        obj->flags.draw_border = 0x00;
        obj->flags.fit_inside = 0x01;
        obj->flags.layout = GUI_LAYOUT_HORIZONTAL;
        gui_object_p name = Gui_CreateChildObject(obj);
        Controls_ActionToStr(buff, (enum ACTIONS)ctrl);
        Gui_SetObjectLabel(name, buff, 2, 2);
        name->flags.draw_label = 0x01;
        name->line_height = 0.8f;
        name->weight_x = 1;
        name->flags.v_content_align = GUI_ALIGN_CENTER;
        name->flags.h_content_align = GUI_ALIGN_CENTER;
        gui_object_p value = Gui_CreateChildObject(obj);
        Controls_KeyToStr(buff, act->primary);
        Gui_SetObjectLabel(value, buff, 2, 2);
        value->flags.draw_label = 0x01;
        value->border_width = 2;
        value->line_height = 0.8f;
        value->weight_x = 1;
        value->flags.v_content_align = GUI_ALIGN_CENTER;
        value->flags.h_content_align = GUI_ALIGN_CENTER;
        value = Gui_CreateChildObject(obj);
        Controls_KeyToStr(buff, act->secondary);
        Gui_SetObjectLabel(value, buff, 2, 2);
        value->flags.draw_label = 0x01;
        value->border_width = 2;
        value->line_height = 0.8f;
        value->weight_x = 1;
        value->flags.v_content_align = GUI_ALIGN_CENTER;
        value->flags.h_content_align = GUI_ALIGN_CENTER;
    }
}

static gui_object_p Gui_AddControlsContainer(gui_object_p root)
{
    gui_object_p cont = Gui_CreateChildObject(root);
    struct gui_controls_data_s *params = (struct gui_controls_data_s*)calloc(1, sizeof(struct gui_controls_data_s));
    params->is_primary = 0x01;
    cont->handlers.do_command = handle_controls_cont;
    cont->handlers.delete_user_data = free;
    cont->data = params;
    cont->w = root->w - root->margin_left - root->margin_right;

    cont->border_width = 0;
    cont->flags.clip_children = 0x01;
    cont->flags.draw_background = 0x00;
    cont->flags.draw_border = 0x00;
    cont->flags.layout = GUI_LAYOUT_VERTICAL;
    cont->flags.h_content_align = GUI_ALIGN_CENTER;
    cont->weight_y = 1;

    for(int i = 0; i < ACT_LASTINDEX; ++i)
    {
        Gui_AddControlsHObj(cont, i);
    }
    cont->childs->flags.draw_border = 0x01;
    cont->childs->childs->next->flags.draw_border = 0x01;

    return cont;
}

static gui_object_p Gui_AddGraphicsContainer(gui_object_p root)
{
    gui_object_p cont = Gui_CreateChildObject(root);
    cont->handlers.do_command = handle_to_item;
    cont->w = root->w - root->margin_left - root->margin_right;

    cont->border_width = 0;
    cont->flags.clip_children = 0x01;
    cont->flags.draw_background = 0x00;
    cont->flags.draw_border = 0x00;
    cont->flags.layout = GUI_LAYOUT_VERTICAL;
    cont->flags.h_content_align = GUI_ALIGN_CENTER;
    cont->weight_y = 1;

    gui_object_p obj = Gui_AddListItem(cont);
    Gui_SetObjectLabel(obj, "Draw crosshair", 2, 2);
    obj->handlers.do_command = handle_on_crosshair;
    obj->flags.draw_label = 0x01;
    obj->flags.draw_border = 0x01;

    return cont;
}

gui_object_p Gui_BuildMainMenu()
{
    gui_object_p root = Gui_CreateMenuRoot();
    gui_object_p title = Gui_CreateChildObject(root);
    root->handlers.do_command = handle_main_menu;
    root->handlers.screen_resized = handle_screen_resized_main;

    title->h = 48;
    title->flags.draw_border = 0x01;
    title->border_width = 4;
    title->spacing = 4;
    title->margin_top = 6;
    title->margin_bottom = 6;
    title->margin_left = 6;
    title->margin_right = 6;
    title->flags.layout = GUI_LAYOUT_HORIZONTAL;
    title->flags.h_content_align = GUI_ALIGN_LEFT;
    title->flags.v_content_align = GUI_ALIGN_CENTER;
    title->flags.draw_label = 0x00;
    title->flags.draw_border = 0x01;
    title->flags.fixed_h = 0x01;
    title->flags.clip_children = 0x01;

    gui_object_p cont = Gui_CreateChildObject(root);
    cont->margin_left = 6;
    cont->margin_right = 6;
    cont->margin_top = 6;
    cont->margin_bottom = 6;
    cont->border_width = 4;
    cont->flags.clip_children = 0x01;
    cont->flags.fit_inside = 0x01;
    cont->flags.draw_background = 0x00;
    cont->flags.draw_border = 0x00;
    cont->flags.layout = GUI_LAYOUT_VERTICAL;
    cont->flags.h_content_align = GUI_ALIGN_CENTER;
    cont->weight_y = 1;

    // fill menu
    gui_object_p obj = Gui_CreateChildObject(title);
    obj->data = Gui_AddNewGameContainer(cont);
    title->data = obj;
    Gui_SetObjectLabel(obj, "New Game", 1, 1);
    obj->w = 172;
    obj->line_height = 0.8;
    obj->border_width = 4;
    obj->flags.fixed_w = 0x01;
    obj->flags.draw_border = 0x01;
    obj->flags.draw_label = 0x01;
    obj->flags.h_content_align = GUI_ALIGN_CENTER;
    obj->flags.v_content_align = GUI_ALIGN_CENTER;

    obj = Gui_CreateChildObject(title);
    obj->data = Gui_AddLoadGameContainer(cont);
    ((gui_object_p)obj->data)->flags.hide = 0x01;
    Gui_SetObjectLabel(obj, "Load Game", 1, 1);
    obj->w = 172;
    obj->line_height = 0.8;
    obj->border_width = 2;
    obj->flags.fixed_w = 0x01;
    obj->flags.draw_border = 0x01;
    obj->flags.draw_label = 0x01;
    obj->flags.h_content_align = GUI_ALIGN_CENTER;
    obj->flags.v_content_align = GUI_ALIGN_CENTER;

    obj = Gui_CreateChildObject(title);
    obj->data = Gui_AddHomeContainer(cont);
    ((gui_object_p)obj->data)->flags.hide = 0x01;
    Gui_SetObjectLabel(obj, "Home", 1, 1);
    obj->w = 172;
    obj->line_height = 0.8;
    obj->border_width = 2;
    obj->flags.fixed_w = 0x01;
    obj->flags.draw_border = 0x01;
    obj->flags.draw_label = 0x01;
    obj->flags.h_content_align = GUI_ALIGN_CENTER;
    obj->flags.v_content_align = GUI_ALIGN_CENTER;

    obj = Gui_CreateChildObject(title);
    obj->data = Gui_AddGraphicsContainer(cont);
    ((gui_object_p)obj->data)->flags.hide = 0x01;
    Gui_SetObjectLabel(obj, "Graphics", 1, 1);
    obj->w = 172;
    obj->line_height = 0.8;
    obj->border_width = 2;
    obj->flags.fixed_w = 0x01;
    obj->flags.draw_border = 0x01;
    obj->flags.draw_label = 0x01;
    obj->flags.h_content_align = GUI_ALIGN_CENTER;
    obj->flags.v_content_align = GUI_ALIGN_CENTER;

    obj = Gui_CreateChildObject(title);
    obj->data = Gui_AddControlsContainer(cont);
    ((gui_object_p)obj->data)->flags.hide = 0x01;
    Gui_SetObjectLabel(obj, "Controls", 1, 1);
    obj->w = 172;
    obj->line_height = 0.8;
    obj->border_width = 2;
    obj->flags.fixed_w = 0x01;
    obj->flags.draw_border = 0x01;
    obj->flags.draw_label = 0x01;
    obj->flags.h_content_align = GUI_ALIGN_CENTER;
    obj->flags.v_content_align = GUI_ALIGN_CENTER;

    root->handlers.screen_resized(root, screen_info.w, screen_info.h);

    return root;
}

gui_object_p Gui_BuildLoadGameMenu()
{
    gui_object_p root = Gui_CreateMenuRoot();
    gui_object_p obj = Gui_CreateChildObject(root);

    root->handlers.screen_resized = handle_screen_resized_inv;
    root->handlers.do_command = handle_to_container;
    obj->h = 40;
    obj->flags.draw_border = 0x01;
    Gui_SetObjectLabel(obj, "Load game:", 1, 1);
    obj->border_width = 4;
    obj->flags.h_content_align = GUI_ALIGN_CENTER;
    obj->flags.v_content_align = GUI_ALIGN_CENTER;
    obj->flags.draw_label = 0x01;
    obj->flags.draw_border = 0x01;
    obj->flags.fixed_h = 0x01;
    obj->line_height = 0.8;

    Gui_AddLoadGameContainer(root);
    handle_screen_resized_inv(root, screen_info.w, screen_info.h);
    Gui_LayoutObjects(root);

    return root;
}

gui_object_p Gui_BuildSaveGameMenu()
{
    gui_object_p root = Gui_CreateMenuRoot();
    gui_object_p obj = Gui_CreateChildObject(root);

    root->handlers.screen_resized = handle_screen_resized_inv;
    root->handlers.do_command = handle_to_container;
    obj->h = 40;
    obj->flags.draw_border = 0x01;
    Gui_SetObjectLabel(obj, "Save game:", 1, 1);
    obj->border_width = 4;
    obj->flags.h_content_align = GUI_ALIGN_CENTER;
    obj->flags.v_content_align = GUI_ALIGN_CENTER;
    obj->flags.draw_label = 0x01;
    obj->flags.draw_border = 0x01;
    obj->flags.fixed_h = 0x01;
    obj->line_height = 0.8;

    Gui_AddSaveGameContainer(root);
    handle_screen_resized_inv(root, screen_info.w, screen_info.h);
    Gui_LayoutObjects(root);

    return root;
}

gui_object_p Gui_BuildNewGameMenu()
{
    gui_object_p root = Gui_CreateMenuRoot();
    gui_object_p obj = Gui_CreateChildObject(root);

    root->handlers.screen_resized = handle_screen_resized_inv;
    root->handlers.do_command = handle_to_container;
    obj->h = 40;
    obj->flags.draw_border = 0x01;
    Gui_SetObjectLabel(obj, "New game", 1, 1);
    obj->border_width = 4;
    obj->flags.h_content_align = GUI_ALIGN_CENTER;
    obj->flags.v_content_align = GUI_ALIGN_CENTER;
    obj->flags.draw_label = 0x01;
    obj->flags.draw_border = 0x01;
    obj->flags.fixed_h = 0x01;
    obj->line_height = 0.8;

    Gui_AddNewGameContainer(root);
    handle_screen_resized_inv(root, screen_info.w, screen_info.h);
    Gui_LayoutObjects(root);

    return root;
}

gui_object_p Gui_BuildControlsMenu()
{
    gui_object_p root = Gui_CreateMenuRoot();
    gui_object_p obj = Gui_CreateChildObject(root);

    root->handlers.screen_resized = handle_screen_resized_inv;
    root->handlers.do_command = handle_to_container;
    obj->h = 40;
    obj->flags.draw_border = 0x01;
    Gui_SetObjectLabel(obj, "Controls", 1, 1);
    obj->border_width = 4;
    obj->flags.h_content_align = GUI_ALIGN_CENTER;
    obj->flags.v_content_align = GUI_ALIGN_CENTER;
    obj->flags.draw_label = 0x01;
    obj->flags.draw_border = 0x01;
    obj->flags.fixed_h = 0x01;
    obj->line_height = 0.8;

    Gui_AddControlsContainer(root);
    handle_screen_resized_inv(root, screen_info.w, screen_info.h);
    Gui_LayoutObjects(root);

    return root;
}

static void Gui_AddKeyValueHObj(gui_object_p cont, const char *name_str, const char *value_str)
{
    gui_object_p obj = Gui_AddListItem(cont);
    obj->flags.draw_border = 0x00;
    obj->flags.fit_inside = 0x01;
    obj->flags.layout = GUI_LAYOUT_HORIZONTAL;
    gui_object_p name = Gui_CreateChildObject(obj);
    Gui_SetObjectLabel(name, name_str, 2, 2);
    name->flags.draw_label = 0x01;
    name->weight_x = 1;
    name->flags.v_content_align = GUI_ALIGN_CENTER;
    name->flags.h_content_align = GUI_ALIGN_CENTER;
    gui_object_p value = Gui_CreateChildObject(obj);
    Gui_SetObjectLabel(value, value_str, 2, 2);
    value->flags.draw_label = 0x01;
    value->weight_x = 1;
    value->flags.v_content_align = GUI_ALIGN_CENTER;
    value->flags.h_content_align = GUI_ALIGN_CENTER;
}

gui_object_p Gui_BuildStatisticsMenu()
{
    gui_object_p root = Gui_CreateMenuRoot();
    gui_object_p obj = Gui_CreateChildObject(root);

    root->handlers.screen_resized = handle_screen_resized_inv;
    obj->h = 40;
    obj->flags.draw_border = 0x01;
    Gui_SetObjectLabel(obj, "Statistics", 1, 1);
    obj->border_width = 4;
    obj->flags.h_content_align = GUI_ALIGN_CENTER;
    obj->flags.v_content_align = GUI_ALIGN_CENTER;
    obj->flags.draw_label = 0x01;
    obj->flags.draw_border = 0x01;
    obj->flags.fixed_h = 0x01;
    obj->line_height = 0.8;

    gui_object_p cont = Gui_CreateChildObject(root);
    cont->handlers.do_command = handle_new_game_cont;
    cont->w = root->w - root->margin_left - root->margin_right;

    cont->border_width = 0;
    cont->flags.clip_children = 0x01;
    cont->flags.draw_background = 0x00;
    cont->flags.draw_border = 0x00;
    cont->flags.layout = GUI_LAYOUT_VERTICAL;
    cont->flags.v_content_align = GUI_ALIGN_TOP;
    cont->flags.h_content_align = GUI_ALIGN_CENTER;
    cont->weight_y = 1;

    char buff[128];
    Gui_AddKeyValueHObj(cont, "Level time", "Too late");
    {
        int secrets = 0;
        for(int i = 0; i < GF_MAX_SECRETS; ++i)
        {
            secrets += Gameflow_GetSecretStateAtIndex(i);
        }
        snprintf(buff, sizeof(buff), "%d", secrets);
        Gui_AddKeyValueHObj(cont, "Secrets found", buff);
    }
    Gui_AddKeyValueHObj(cont, "Items found", "N/A");
    Gui_AddKeyValueHObj(cont, "Kills", "N/A");
    Gui_AddKeyValueHObj(cont, "Meds used", "N/A");

    handle_screen_resized_inv(root, screen_info.w, screen_info.h);
    Gui_LayoutObjects(root);

    return root;
}

gui_object_p Gui_ListInventoryMenu(gui_object_p root, int dy)
{
    gui_object_p ret = NULL;
    if(root)
    {
        for(gui_object_p obj = root->childs; obj; obj = obj->next)
        {
            if(obj->flags.draw_border)
            {
                ret = obj;
                if((dy > 0) && obj->prev)
                {
                    ret = obj->prev;
                    obj->flags.draw_border = 0x00;
                    ret->flags.draw_border = 0x01;
                }
                else if((dy < 0) && obj->next)
                {
                    ret = obj->next;
                    obj->flags.draw_border = 0x00;
                    ret->flags.draw_border = 0x01;
                }
                break;
            }
        }
        Gui_EnsureVisible(ret);
    }
    return ret;
}

// HANDLERS
extern "C" int handle_to_container(struct gui_object_s *obj, enum gui_command_e cmd)
{
    if(obj && obj->childs && obj->childs->next && obj->childs->next->handlers.do_command)
    {
        return obj->childs->next->handlers.do_command(obj->childs->next, cmd);
    }
    return 0;
}

extern "C" int handle_to_item(struct gui_object_s *obj, enum gui_command_e cmd)
{
    int ret = 0;
    if(cmd == UP)
    {
        ret = (Gui_ListInventoryMenu(obj, 1)) ? (1) : (0);
    }
    else if(cmd == DOWN)
    {
        ret = (Gui_ListInventoryMenu(obj, -1)) ? (1) : (0);
    }
    else if(cmd == ACTIVATE)
    {
        gui_object_p item = Gui_ListInventoryMenu(obj, 0);
        if(item && item->handlers.do_command)
        {
            ret = item->handlers.do_command(item, cmd);
        }
    }
    return ret;
}

extern "C" void handle_screen_resized_inv(struct gui_object_s *obj, int w, int h)
{
    obj->w = w * 0.50f;
    obj->h = h * 0.60f;
    obj->x = w * 0.25f;
    obj->y = h * 0.20f;
    Gui_LayoutObjects(obj);
}

extern "C" void handle_screen_resized_main(struct gui_object_s *obj, int w, int h)
{
    obj->w = w * 0.80f;
    obj->h = h * 0.90f;
    obj->x = w * 0.10f;
    obj->y = h * 0.05f;
    Gui_LayoutObjects(obj);
}

extern "C" int handle_main_menu(struct gui_object_s *obj, enum gui_command_e cmd)
{
    gui_object_p title = obj->childs;
    gui_object_p curr_in_title = (gui_object_p)title->data;
    gui_object_p curr_menu = (curr_in_title) ? ((gui_object_p)curr_in_title->data) : (NULL);
    gui_object_p cont = title->next;
    if(!cont->handlers.do_command)
    {
        title->flags.draw_border = 0x01;
        cont->flags.draw_border = 0x00;
        if((cmd == LEFT) && (curr_in_title->prev))
        {
            curr_in_title->border_width = 2;
            curr_in_title = curr_in_title->prev;
            title->data = curr_in_title;
            curr_in_title->border_width = 4;
            Gui_EnsureVisible(curr_in_title);

            for(gui_object_p it = cont->childs; it; it = it->next)
            {
                it->flags.hide = (it != curr_in_title->data) ? (0x01) : (0x00);
            }
            Gui_LayoutObjects(cont);
        }
        else if((cmd == RIGHT) && (curr_in_title->next))
        {
            curr_in_title->border_width = 2;
            curr_in_title = curr_in_title->next;
            title->data = curr_in_title;
            curr_in_title->border_width = 4;
            Gui_EnsureVisible(curr_in_title);

            for(gui_object_p it = cont->childs; it; it = it->next)
            {
                it->flags.hide = (it != curr_in_title->data) ? (0x01) : (0x00);
            }
            Gui_LayoutObjects(cont);
        }
        else if(curr_menu && (cmd == ACTIVATE))
        {
            title->flags.draw_border = 0x00;
            cont->flags.draw_border = 0x01;
            cont->handlers.do_command = curr_menu->handlers.do_command;
        }
        else if(cmd == CLOSE)
        {
            return 0;
        }
    }
    else if(!cont->handlers.do_command(curr_menu, cmd) && (cmd == CLOSE))
    {
        title->flags.draw_border = 0x01;
        cont->flags.draw_border = 0x00;
        cont->handlers.do_command = NULL;
    }

    return 1;
}

extern "C" int handle_load_game_cont(struct gui_object_s *obj, enum gui_command_e cmd)
{
    int ret = 0;
    if(cmd == UP)
    {
        ret = (Gui_ListInventoryMenu(obj, 1)) ? (1) : (0);
    }
    else if(cmd == DOWN)
    {
        ret = (Gui_ListInventoryMenu(obj, -1)) ? (1) : (0);
    }
    else if(cmd == ACTIVATE)
    {
        gui_object_p item = Gui_ListInventoryMenu(obj, 0);
        if(item && item->text)
        {
            ret = Game_Load(item->text);
            Gui_SetCurrentMenu(NULL);
        }
    }
    return ret;
}

extern "C" int handle_save_game_cont(struct gui_object_s *obj, enum gui_command_e cmd)
{
    int ret = 0;
    if(cmd == UP)
    {
        ret = (Gui_ListInventoryMenu(obj, 1)) ? (1) : (0);
    }
    else if(cmd == DOWN)
    {
        ret = (Gui_ListInventoryMenu(obj, -1)) ? (1) : (0);
    }
    else if(cmd == ACTIVATE)
    {
        gui_object_p item = Gui_ListInventoryMenu(obj, 0);
        if(item && item->text)
        {
            ret = Game_Save(item->text);
        }
    }
    return ret;
}

extern "C" int handle_new_game_cont(struct gui_object_s *obj, enum gui_command_e cmd)
{
    int ret = 0;
    if(cmd == UP)
    {
        ret = (Gui_ListInventoryMenu(obj, 1)) ? (1) : (0);
    }
    else if(cmd == DOWN)
    {
        ret = (Gui_ListInventoryMenu(obj, -1)) ? (1) : (0);
    }
    else if(cmd == ACTIVATE)
    {
        gui_object_p item = Gui_ListInventoryMenu(obj, 0);
        if(item && item->text)
        {
            int game_id = 0x7FFF & (uint64_t)item->data;
            Gameflow_SetGame(game_id, 1);
            Gui_SetCurrentMenu(NULL);
            switch(game_id)
            {
                case GAME_1:
                case GAME_1_1:
                case GAME_1_5:
                    Gameflow_Send(GF_OP_STARTFMV, 0);
                    Gameflow_Send(GF_OP_STARTFMV, 2);
                    Gameflow_Send(GF_OP_STARTFMV, 3);
                    break;

                case GAME_2:
                case GAME_2_1:
                case GAME_2_5:
                    Gameflow_Send(GF_OP_STARTFMV, 0);
                    Gameflow_Send(GF_OP_STARTFMV, 2);
                    break;

                case GAME_3:
                case GAME_3_5:
                    Gameflow_Send(GF_OP_STARTFMV, 0);
                    Gameflow_Send(GF_OP_STARTFMV, 2);
                    break;
            }
            ret = 1;
        }
    }
    return ret;
}

extern "C" int handle_home_cont(struct gui_object_s *obj, enum gui_command_e cmd)
{
    int ret = 0;
    if(cmd == UP)
    {
        ret = (Gui_ListInventoryMenu(obj, 1)) ? (1) : (0);
    }
    else if(cmd == DOWN)
    {
        ret = (Gui_ListInventoryMenu(obj, -1)) ? (1) : (0);
    }
    else if(cmd == ACTIVATE)
    {
        gui_object_p item = Gui_ListInventoryMenu(obj, 0);
        if(item && item->text)
        {
            int game_id = 0x7FFF & (uint64_t)item->data;
            Gameflow_SetGame(game_id, 0);
            Gui_SetCurrentMenu(NULL);
            switch(game_id)
            {
                case GAME_1:
                case GAME_1_1:
                case GAME_1_5:
                    Gameflow_Send(GF_OP_STARTFMV, 1);
                    break;

                case GAME_2:
                case GAME_2_1:
                case GAME_2_5:
                    Gameflow_Send(GF_OP_STARTFMV, 1);
                    break;

                case GAME_3:
                case GAME_3_5:
                    Gameflow_Send(GF_OP_STARTFMV, 1);
                    break;
            }
            ret = 1;
        }
    }
    return ret;
}

extern "C" int handle_controls_cont(struct gui_object_s *obj, enum gui_command_e cmd)
{
    int ret = 0;
    struct gui_controls_data_s *params = (struct gui_controls_data_s*)obj->data;

    if(params->wait_key)
    {
        gui_object_p curr = Gui_ListInventoryMenu(obj, 0);
        control_action_p act = (control_action_p)curr->data;

        curr = curr->childs->next;
        curr = (params->is_primary) ? (curr) : (curr->next);
        curr->color_border[0] = 188;
        curr->color_border[1] = 0;
        curr->color_border[2] = 0;

        if(control_states.last_key)
        {
            char buff[128];
            Controls_KeyToStr(buff, control_states.last_key);
            Gui_SetObjectLabel(curr, buff, 2, 2);
            if(params->is_primary)
            {
                act->primary = control_states.last_key;
            }
            else
            {
                act->secondary = control_states.last_key;
            }
            params->wait_key = 0x00;
        }

        if(cmd == CLOSE)
        {
            params->wait_key = 0x00;
            ret = 1;
        }

        if(params->wait_key == 0x00)
        {
            curr->color_border[0] = curr->parent->color_border[0];
            curr->color_border[1] = curr->parent->color_border[1];
            curr->color_border[2] = curr->parent->color_border[2];
        }
    }
    else if(cmd == UP)
    {
        gui_object_p curr = Gui_ListInventoryMenu(obj, 1);
        if(curr->next)
        {
            for(gui_object_p it = curr->next->childs; it; it = it->next)
            {
                it->flags.draw_border = 0x00;
            }
            ret = 1;
        }
        if(params->is_primary)
        {
            curr->childs->next->flags.draw_border = 0x01;
        }
        else
        {
            curr->childs->next->next->flags.draw_border = 0x01;
        }
    }
    else if(cmd == DOWN)
    {
        gui_object_p curr = Gui_ListInventoryMenu(obj, -1);
        if(curr->prev)
        {
            for(gui_object_p it = curr->prev->childs; it; it = it->next)
            {
                it->flags.draw_border = 0x00;
            }
            ret = 1;
        }
        if(params->is_primary)
        {
            curr->childs->next->flags.draw_border = 0x01;
        }
        else
        {
            curr->childs->next->next->flags.draw_border = 0x01;
        }
    }
    else if(cmd == LEFT)
    {
        gui_object_p curr = Gui_ListInventoryMenu(obj, 0);
        curr->childs->next->next->flags.draw_border = 0x00;
        curr->childs->next->flags.draw_border = 0x01;
        params->is_primary = 0x01;
        ret = 1;
    }
    else if(cmd == RIGHT)
    {
        gui_object_p curr = Gui_ListInventoryMenu(obj, 0);
        curr->childs->next->next->flags.draw_border = 0x01;
        curr->childs->next->flags.draw_border = 0x00;
        params->is_primary = 0x00;
        ret = 1;
    }
    else if(cmd == ACTIVATE)
    {
        params->wait_key = 0x01;
    }
    return ret;
}

extern "C" int handle_on_crosshair(struct gui_object_s *obj, enum gui_command_e cmd)
{
    screen_info.crosshair = !screen_info.crosshair;
    return 1;
}
