
#include "menu_bar.h"

#include <SDL2/SDL.h>

#include <map>
#include <string>
#include <vector>
#include <strings.h>
#include <dirent.h>

#include "AntTweakBar/AntTweakBar.h"
#include "bullet/btBulletDynamicsCommon.h"

#include "engine.h"
#include "console.h"
#include "game.h"

extern SDL_Window* sdl_window;
extern btDiscreteDynamicsWorld* bt_engine_dynamicsWorld;


static TwBar* main_bar;

static std::map<std::string, std::vector<std::string> > group_lists;


static void SaveGameMenuBtnCB(void* client_data)
{
    const char* game_name = (const char*) client_data;
    Game_Save(game_name);
}

static void LoadGameMenuBtnCB(void* client_data)
{
    const char* game_name = (const char*) client_data;
    Game_Load(game_name);
}

static void LoadMapMenuBtnCB(void* client_data)
{
    const char* map = (const char*) client_data;
    Engine_LoadMap(map);
}

static const char* filename_from_path(const char* path)
{
    const char* sep = strrchr(path, '/');
    if(!sep || sep == path) {
        return NULL;
    }
    return sep + 1;
}

static bool has_ext(const char* str, const char* ext)
{
    const char* dot = strrchr(str, '.');
    if(!dot || dot == str) {
        return false;
    }
    return strncasecmp(dot + 1, ext, 3) == 0;
}

static int add_button(const char* path, const char* ext, const char* group, void (*callback)(void*))
{
    DIR* dir = opendir(path);
    if (dir == NULL) {
        return 0;
    }

    while (struct dirent* entry = readdir(dir))
    {
        if ((strcmp(entry->d_name, ".")  == 0) ||
            (strcmp(entry->d_name, "..") == 0) ||
            (ext && !has_ext(entry->d_name, ext)))
        {
            continue;
        }

        group_lists[group].push_back(std::string(path) + "/" + entry->d_name);
        TwAddButton(main_bar, group_lists[group].back().c_str(), callback, (void*)group_lists[group].back().c_str(), (std::string(" group='") + group + "' label='" + entry->d_name + "' ").c_str());
    }

    closedir(dir);
    return 0;
}

static std::string get_init_str(const char* name, const char* label, int size_x, int size_y, bool visible)
{
    char buf[255];
    sprintf(buf, " %s label='%s' position='%d %d' size='%d %d' alpha=%d resizable=%d movable=%d fontresizable=%d iconifiable=%d visible=%d",
            name, label, 0, 0, size_x, size_y, 64, 0, 0, 0, 0, (visible ? 1 : 0));
    return buf;
}

static void GetGravityVecZCB(void* value, void* client_data)
{
    btDiscreteDynamicsWorld* engine_dynamics = static_cast<btDiscreteDynamicsWorld*>(client_data);
    *static_cast<float*>(value) = engine_dynamics->getGravity().getZ();
}

static void SetGravityVecZCB(const void* value, void* client_data)
{
    btDiscreteDynamicsWorld* engine_dynamics = static_cast<btDiscreteDynamicsWorld*>(client_data);
    btVector3 g = engine_dynamics->getGravity();
    g.setZ(*static_cast<const float*>(value));
    engine_dynamics->setGravity(g);
}

void MenuBar_Init(int wnd_width, int wnd_height)
{
    TwInit(TW_OPENGL, NULL);
    TwWindowSize(wnd_width, wnd_height);

    main_bar = TwNewBar("Main");
    TwDefine(get_init_str("Main", "MainBar", wnd_width, wnd_height, false).c_str());

    TwAddVarCB(main_bar, "gravity", TW_TYPE_FLOAT, SetGravityVecZCB, GetGravityVecZCB, bt_engine_dynamicsWorld, " group='Engine' label='gravity' ");
    TwAddVarRW(main_bar, "free_look", TW_TYPE_BOOL32, &control_states.free_look, " group='Engine' label='free_look' ");
    TwAddVarRW(main_bar, "free_look_speed", TW_TYPE_FLOAT, &control_states.free_look_speed, " group='Engine' label='free_look_speed' min=512 max=8192 step=4.0 ");
    TwAddVarRW(main_bar, "mouse_look", TW_TYPE_BOOL32, &control_states.mouse_look, " group='Engine' label='mouse_look' ");
    TwAddVarRW(main_bar, "noclip", TW_TYPE_BOOL32, &control_states.noclip, " group='Engine' label='noclip' ");
    TwAddVarRW(main_bar, "cam_distance", TW_TYPE_FLOAT, &control_states.cam_distance, " group='Engine' label='cam_distance' min=128 max=4096 step=8.0 ");
    
    add_button("save", NULL, "Save/Load", LoadGameMenuBtnCB);
    add_button("data/tr1/data", "phd", "tr1", LoadMapMenuBtnCB);
    add_button("data/tr2/data", "tr2", "tr2", LoadMapMenuBtnCB);
    add_button("data/tr3/data", "tr2", "tr3", LoadMapMenuBtnCB);
    add_button("data/tr4/data", "tr4", "tr4", LoadMapMenuBtnCB);
    add_button("data/tr5/data", "trc", "tr5", LoadMapMenuBtnCB);

    TwDefine(" Main/tr1 group='Levels' ");
    TwDefine(" Main/tr2 group='Levels' ");
    TwDefine(" Main/tr3 group='Levels' ");
    TwDefine(" Main/tr4 group='Levels' ");
    TwDefine(" Main/tr5 group='Levels' ");

    Con_AddLine("MenuBar initialized");
}

void MenuBar_Destroy()
{
    TwTerminate();
}

bool MenuBar_Event(const SDL_Event& event)
{
    return TwEventSDL(&event) != 0;
}

void MenuBar_Render()
{
    TwDraw();
}

bool MenuBar_IsVisible()
{
    int visible = 1;
    TwGetParam(main_bar, NULL, "visible", TW_PARAM_INT32, 1, &visible);
    return visible != 0;
}

void MenuBar_Show()
{
    SDL_SetRelativeMouseMode(SDL_FALSE);
    TwDefine(" Main visible=1");
}

void MenuBar_Hide()
{
    SDL_WarpMouseInWindow(sdl_window, screen_info.w / 2, screen_info.h / 2);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    TwDefine(" Main visible=0");
}

void MenuBar_ToggleVisibility()
{
    if (MenuBar_IsVisible()) {
        MenuBar_Hide();
    } else {
        MenuBar_Show();
    }
}
