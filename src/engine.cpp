
#include <SDL2/SDL.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "al/AL/al.h"
#include "al/AL/alc.h"
}

#include "vt/vt_level.h"
#include "core/system.h"
#include "core/gl_font.h"
#include "core/console.h"
#include "core/redblack.h"
#include "core/vmath.h"
#include "render.h"
#include "gui.h"
#include "gameflow.h"
#include "world.h"
#include "resource.h"
#include "engine.h"
#include "engine_lua.h"
#include "engine_bullet.h"

extern SDL_Window             *sdl_window;
extern SDL_GLContext           sdl_gl_context;
extern SDL_GameController     *sdl_controller;
extern SDL_Joystick           *sdl_joystick;
extern SDL_Haptic             *sdl_haptic;
extern ALCdevice              *al_device;
extern ALCcontext             *al_context;

struct engine_control_state_s           control_states = {0};
struct control_settings_s               control_mapper = {0};
struct audio_settings_s                 audio_settings = {0};
btScalar                                engine_frame_time = 0.0;

lua_State                              *engine_lua = NULL;
struct camera_s                         engine_camera;
struct world_s                          engine_world;


void Engine_InitDefaultGlobals()
{
    Sys_InitGlobals();
    Con_InitGlobals();
    Controls_InitGlobals();
    Game_InitGlobals();
    Render_InitGlobals();
    Audio_InitGlobals();
}

// First stage of initialization.

void Engine_Init_Pre()
{
    /* Console must be initialized previously! some functions uses CON_AddLine before GL initialization!
     * Rendering activation may be done later. */

    Sys_Init();
    Con_Init();
    Engine_LuaInit();

    lua_CallVoidFunc(engine_lua, "loadscript_pre", true);

    Gameflow_Init();

    Render_Init();
    Cam_Init(&engine_camera);
    renderer.cam = &engine_camera;

    Engine_BTInit();
}

// Second stage of initialization.

void Engine_Init_Post()
{
    lua_CallVoidFunc(engine_lua, "loadscript_post", true);

    Con_InitFont(glf_manager_get_font(con_font_manager, FONT_CONSOLE));

    Gui_Init();

    Con_AddLine("Engine inited!", FONTSTYLE_CONSOLE_EVENT);
}

void Engine_Destroy()
{
    Render_Empty(&renderer);
    Con_Destroy();
    Sys_Destroy();

    Engine_BTDestroy();

    ///-----cleanup_end-----
    if(engine_lua)
    {
        lua_close(engine_lua);
        engine_lua = NULL;
    }

    Gui_Destroy();
}


void Engine_Shutdown(int val)
{
    Engine_LuaClearTasks();
    Render_Empty(&renderer);
    World_Empty(&engine_world);
    Engine_Destroy();

    /* no more renderings */
    SDL_GL_DeleteContext(sdl_gl_context);
    SDL_DestroyWindow(sdl_window);

    if(sdl_joystick)
    {
        SDL_JoystickClose(sdl_joystick);
    }

    if(sdl_controller)
    {
        SDL_GameControllerClose(sdl_controller);
    }

    if(sdl_haptic)
    {
        SDL_HapticClose(sdl_haptic);
    }

    if(al_context)  // T4Larson <t4larson@gmail.com>: fixed
    {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(al_context);
    }

    if(al_device)
    {
        alcCloseDevice(al_device);
    }

    Sys_Destroy();
    IMG_Quit();
    SDL_Quit();

    exit(val);
}


engine_container_p Container_Create()
{
    engine_container_p ret;

    ret = (engine_container_p)malloc(sizeof(engine_container_t));
    ret->next = NULL;
    ret->object = NULL;
    ret->object_type = 0;
    return ret;
}


int Engine_GetLevelFormat(const char *name)
{
    // PLACEHOLDER: Currently, only PC levels are supported.

    return LEVEL_FORMAT_PC;
}


int Engine_GetPCLevelVersion(const char *name)
{
    int ret = TR_UNKNOWN;
    int len = strlen(name);
    FILE *ff;

    if(len < 5)
    {
        return ret;                                                             // Wrong (too short) filename
    }

    ff = fopen(name, "rb");
    if(ff)
    {
        char ext[5];
        uint8_t check[4];

        ext[0] = name[len-4];                                                   // .
        ext[1] = toupper(name[len-3]);                                          // P
        ext[2] = toupper(name[len-2]);                                          // H
        ext[3] = toupper(name[len-1]);                                          // D
        ext[4] = 0;
        fread(check, 4, 1, ff);

        if(!strncmp(ext, ".PHD", 4))                                            //
        {
            if(check[0] == 0x20 &&
               check[1] == 0x00 &&
               check[2] == 0x00 &&
               check[3] == 0x00)
            {
                ret = TR_I;                                                     // TR_I ? OR TR_I_DEMO
            }
            else
            {
                ret = TR_UNKNOWN;
            }
        }
        else if(!strncmp(ext, ".TUB", 4))
        {
            if(check[0] == 0x20 &&
               check[1] == 0x00 &&
               check[2] == 0x00 &&
               check[3] == 0x00)
            {
                ret = TR_I_UB;                                                  // TR_I_UB
            }
            else
            {
                ret = TR_UNKNOWN;
            }
        }
        else if(!strncmp(ext, ".TR2", 4))
        {
            if(check[0] == 0x2D &&
               check[1] == 0x00 &&
               check[2] == 0x00 &&
               check[3] == 0x00)
            {
                ret = TR_II;                                                    // TR_II
            }
            else if((check[0] == 0x38 || check[0] == 0x34) &&
                    (check[1] == 0x00) &&
                    (check[2] == 0x18 || check[2] == 0x08) &&
                    (check[3] == 0xFF))
            {
                ret = TR_III;                                                   // TR_III
            }
            else
            {
                ret = TR_UNKNOWN;
            }
        }
        else if(!strncmp(ext, ".TR4", 4))
        {
            if(check[0] == 0x54 &&                                              // T
               check[1] == 0x52 &&                                              // R
               check[2] == 0x34 &&                                              // 4
               check[3] == 0x00)
            {
                ret = TR_IV;                                                    // OR TR TR_IV_DEMO
            }
            else if(check[0] == 0x54 &&                                         // T
                    check[1] == 0x52 &&                                         // R
                    check[2] == 0x34 &&                                         // 4
                    check[3] == 0x63)                                           //
            {
                ret = TR_IV;                                                    // TRLE
            }
            else if(check[0] == 0xF0 &&                                         // T
                    check[1] == 0xFF &&                                         // R
                    check[2] == 0xFF &&                                         // 4
                    check[3] == 0xFF)
            {
                ret = TR_IV;                                                    // BOGUS (OpenRaider =))
            }
            else
            {
                ret = TR_UNKNOWN;
            }
        }
        else if(!strncmp(ext, ".TRC", 4))
        {
            if(check[0] == 0x54 &&                                              // T
               check[1] == 0x52 &&                                              // R
               check[2] == 0x34 &&                                              // 4
               check[3] == 0x00)
            {
                ret = TR_V;                                                     // TR_V
            }
            else
            {
                ret = TR_UNKNOWN;
            }
        }
        else                                                                    // unknown ext.
        {
            ret = TR_UNKNOWN;
        }

        fclose(ff);
    }

    return ret;
}


void Engine_GetLevelName(char *name, const char *path)
{
    int i, len, start, ext;

    if(!path || (path[0] == 0x00))
    {
        name[0] = 0x00;
        return;
    }

    ext = len = strlen(path);
    start = 0;

    for(i=len;i>=0;i--)
    {
        if(path[i] == '.')
        {
            ext = i;
        }
        if(path[i] == '\\' || path[i] == '/')
        {
            start = i + 1;
            break;
        }
    }

    for(i=start;i<ext && i-start<LEVEL_NAME_MAX_LEN-1;i++)
    {
        name[i-start] = path[i];
    }
    name[i-start] = 0;
}

void Engine_GetLevelScriptName(int game_version, char *name, const char *postfix)
{
    char level_name[LEVEL_NAME_MAX_LEN];
    Engine_GetLevelName(level_name, gameflow_manager.CurrentLevelPath);

    name[0] = 0;

    strcat(name, "scripts/level/");

    if(game_version < TR_II)
    {
        strcat(name, "tr1/");
    }
    else if(game_version < TR_III)
    {
        strcat(name, "tr2/");
    }
    else if(game_version < TR_IV)
    {
        strcat(name, "tr3/");
    }
    else if(game_version < TR_V)
    {
        strcat(name, "tr4/");
    }
    else
    {
        strcat(name, "tr5/");
    }

    for(char *ch=level_name;*ch!=0;ch++)
    {
        *ch = toupper(*ch);
    }

    strcat(name, level_name);
    if(postfix) strcat(name, postfix);
    strcat(name, ".lua");
}

bool Engine_LoadPCLevel(const char *name)
{
    VT_Level *tr_level = new VT_Level();

    int trv = Engine_GetPCLevelVersion(name);
    if(trv == TR_UNKNOWN) return false;

    tr_level->read_level(name, trv);
    tr_level->prepare_level();
    //tr_level->dump_textures();

    TR_GenWorld(&engine_world, tr_level);

    char buf[LEVEL_NAME_MAX_LEN] = {0x00};
    Engine_GetLevelName(buf, name);

    Con_Notify("loaded PC level");
    Con_Notify("version = %d, map = \"%s\"", trv, buf);
    Con_Notify("rooms count = %d", engine_world.room_count);

    delete tr_level;

    return true;
}

int Engine_LoadMap(const char *name)
{
    if(!Sys_FileFound(name, 0))
    {
        Con_Warning("file not found: \"%s\"", name);
        return 0;
    }

    Gui_DrawLoadScreen(0);

    renderer.style &= ~R_DRAW_SKYBOX;
    renderer.r_list_active_count = 0;
    renderer.world = NULL;

    strncpy(gameflow_manager.CurrentLevelPath, name, MAX_ENGINE_PATH);          // it is needed for "not in the game" levels or correct saves loading.

    Gui_DrawLoadScreen(50);

    World_Empty(&engine_world);
    World_Prepare(&engine_world);

    lua_Clean(engine_lua);

    Audio_Init();

    Gui_DrawLoadScreen(100);


    // Here we can place different platform-specific level loading routines.

    switch(Engine_GetLevelFormat(name))
    {
        case LEVEL_FORMAT_PC:
            if(Engine_LoadPCLevel(name) == false) return 0;
            break;

        case LEVEL_FORMAT_PSX:
            break;

        case LEVEL_FORMAT_DC:
            break;

        case LEVEL_FORMAT_OPENTOMB:
            break;

        default:
            break;
    }

    engine_world.id   = 0;
    engine_world.name = 0;
    engine_world.type = 0;

    Game_Prepare();

    Render_SetWorld(&engine_world);

    Gui_DrawLoadScreen(1000);

    Gui_FadeStart(FADER_LOADSCREEN, GUI_FADER_DIR_IN);
    Gui_NotifierStop();

    return 1;
}

int Engine_ExecCmd(char *ch)
{
    char token[con_base.line_size];
    char buf[con_base.line_size + 32];
    char *pch;
    int val;
    room_p r;
    room_sector_p sect;
    FILE *f;

    while(ch!=NULL)
    {
        pch = ch;
        ch = parse_token(ch, token);
        if(!strcmp(token, "help"))
        {
            Con_AddLine("Available commands:\0", FONTSTYLE_CONSOLE_WARNING);
            Con_AddLine("help - show help info\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("loadMap(\"file_name\") - load level \"file_name\"\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("save, load - save and load game state in \"file_name\"\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("exit - close program\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("cls - clean console\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("show_fps - switch show fps flag\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("spacing - read and write spacing\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("showing_lines - read and write number of showing lines\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("cvars - lua's table of cvar's, to see them type: show_table(cvars)\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("free_look - switch camera mode\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("cam_distance - camera distance to actor\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("r_wireframe, r_portals, r_frustums, r_room_boxes, r_boxes, r_normals, r_skip_room - render modes\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("playsound(id) - play specified sound\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("stopsound(id) - stop specified sound\0", FONTSTYLE_CONSOLE_NOTIFY);
            Con_AddLine("Watch out for case sensitive commands!\0", FONTSTYLE_CONSOLE_WARNING);
        }
        else if(!strcmp(token, "goto"))
        {
            control_states.free_look = 1;
            renderer.cam->pos[0] = SC_ParseFloat(&ch);
            renderer.cam->pos[1] = SC_ParseFloat(&ch);
            renderer.cam->pos[2] = SC_ParseFloat(&ch);
            return 1;
        }
        else if(!strcmp(token, "save"))
        {
            ch = parse_token(ch, token);
            if(NULL != ch)
            {
                Game_Save(token);
            }
            return 1;
        }
        else if(!strcmp(token, "load"))
        {
            ch = parse_token(ch, token);
            if(NULL != ch)
            {
                Game_Load(token);
            }
            return 1;
        }
        else if(!strcmp(token, "exit"))
        {
            Engine_Shutdown(0);
            return 1;
        }
        else if(!strcmp(token, "cls"))
        {
            Con_Clean();
            return 1;
        }
        else if(!strcmp(token, "spacing"))
        {
            ch = parse_token(ch, token);
            if(NULL == ch)
            {
                Con_Notify("spacing = %d", con_base.spacing);
                return 1;
            }
            Con_SetLineInterval(atof(token));
            return 1;
        }
        else if(!strcmp(token, "showing_lines"))
        {
            ch = parse_token(ch, token);
            if(NULL == ch)
            {
                Con_Notify("showing lines = %d", con_base.showing_lines);
                return 1;
            }
            else
            {
                val = atoi(token);
                if((val >=2 ) && (val <= con_base.line_count))
                {
                    con_base.showing_lines = val;
                    con_base.cursor_y = screen_info.h - con_base.line_height * con_base.showing_lines;
                }
                else
                {
                    Con_Warning("wrong lines count, must be in interval: (%d, %d)", 2, con_base.line_count);
                }
            }
            return 1;
        }
        else if(!strcmp(token, "r_wireframe"))
        {
            renderer.style ^= R_DRAW_WIRE;
            return 1;
        }
        else if(!strcmp(token, "r_points"))
        {
            renderer.style ^= R_DRAW_POINTS;
            return 1;
        }
        else if(!strcmp(token, "r_coll"))
        {
            renderer.style ^= R_DRAW_COLL;
            return 1;
        }
        else if(!strcmp(token, "r_normals"))
        {
            renderer.style ^= R_DRAW_NORMALS;
            return 1;
        }
        else if(!strcmp(token, "r_portals"))
        {
            renderer.style ^= R_DRAW_PORTALS;
            return 1;
        }
        else if(!strcmp(token, "r_frustums"))
        {
            renderer.style ^= R_DRAW_FRUSTUMS;
            return 1;
        }
        else if(!strcmp(token, "r_room_boxes"))
        {
            renderer.style ^= R_DRAW_ROOMBOXES;
            return 1;
        }
        else if(!strcmp(token, "r_boxes"))
        {
            renderer.style ^= R_DRAW_BOXES;
            return 1;
        }
        else if(!strcmp(token, "r_axis"))
        {
            renderer.style ^= R_DRAW_AXIS;
            return 1;
        }
        else if(!strcmp(token, "r_nullmeshes"))
        {
            renderer.style ^= R_DRAW_NULLMESHES;
            return 1;
        }
        else if(!strcmp(token, "r_dummy_statics"))
        {
            renderer.style ^= R_DRAW_DUMMY_STATICS;
            return 1;
        }
        else if(!strcmp(token, "r_skip_room"))
        {
            renderer.style ^= R_SKIP_ROOM;
            return 1;
        }
        else if(!strcmp(token, "room_info"))
        {
            r = renderer.cam->current_room;
            if(r)
            {
                sect = Room_GetSectorXYZ(r, renderer.cam->pos);
                Con_Printf("ID = %d, x_sect = %d, y_sect = %d", r->id, r->sectors_x, r->sectors_y);
                if(sect)
                {
                    Con_Printf("sect(%d, %d), inpenitrable = %d, r_up = %d, r_down = %d", sect->index_x, sect->index_y,
                               (int)(sect->ceiling == TR_METERING_WALLHEIGHT || sect->floor == TR_METERING_WALLHEIGHT), (int)(sect->sector_above != NULL), (int)(sect->sector_below != NULL));
                    for(uint32_t i=0;i<sect->owner_room->static_mesh_count;i++)
                    {
                        Con_Printf("static[%d].object_id = %d", i, sect->owner_room->static_mesh[i].object_id);
                    }
                    for(engine_container_p cont=sect->owner_room->containers;cont;cont=cont->next)
                    {
                        if(cont->object_type == OBJECT_ENTITY)
                        {
                            entity_p e = (entity_p)cont->object;
                            Con_Printf("cont[entity](%d, %d, %d).object_id = %d", (int)e->transform[12+0], (int)e->transform[12+1], (int)e->transform[12+2], e->id);
                        }
                    }
                }
            }
            return 1;
        }
        else if(!strcmp(token, "xxx"))
        {
            f = fopen("ascII.txt", "r");
            if(f)
            {
                long int size;
                char *buf;
                fseek(f, 0, SEEK_END);
                size= ftell(f);
                buf = (char*) malloc((size+1)*sizeof(char));

                fseek(f, 0, SEEK_SET);
                fread(buf, size, sizeof(char), f);
                buf[size] = 0;
                fclose(f);
                Con_Clean();
                Con_AddText(buf, FONTSTYLE_CONSOLE_INFO);
                free(buf);
            }
            else
            {
                Con_AddText("Not avaliable =(", FONTSTYLE_CONSOLE_WARNING);
            }
            return 1;
        }
        else if(token[0])
        {
            if(engine_lua)
            {
                Con_AddLine(pch, FONTSTYLE_GENERIC);
                if (luaL_dostring(engine_lua, pch) != LUA_OK)
                {
                    Con_AddLine(lua_tostring(engine_lua, -1), FONTSTYLE_CONSOLE_WARNING);
                    lua_pop(engine_lua, 1);
                }
            }
            else
            {
                snprintf(buf, con_base.line_size + 32, "Command \"%s\" not found", token);
                Con_AddLine(buf, FONTSTYLE_CONSOLE_WARNING);
            }
            return 0;
        }
    }

    return 0;
}


void Engine_InitConfig(const char *filename)
{
    lua_State *lua = luaL_newstate();

    Engine_InitDefaultGlobals();

    if(lua != NULL)
    {
        if((filename != NULL) && Sys_FileFound(filename, 0))
        {
            luaL_openlibs(lua);
            lua_register(lua, "bind", lua_BindKey);                             // get and set key bindings
            luaL_dofile(lua, filename);

            lua_ParseScreen(lua, &screen_info);
            lua_ParseRender(lua, &renderer.settings);
            lua_ParseAudio(lua, &audio_settings);
            lua_ParseConsole(lua, &con_base);
            lua_ParseControls(lua, &control_mapper);
            lua_close(lua);
        }
        else
        {
            Sys_Warn("Could not find \"%s\"", filename);
        }
    }
}


void Engine_SaveConfig()
{

}
