
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "script.h"
#include "../core/system.h"
#include "../core/gl_text.h"
#include "../core/console.h"
#include "../core/vmath.h"
#include "../render/camera.h"
#include "../render/render.h"
#include "../vt/tr_versions.h"
#include "../skeletal_model.h"
#include "../trigger.h"
#include "../entity.h"
#include "../engine.h"
#include "../controls.h"
#include "../game.h"
#include "../gameflow.h"
#include "../character_controller.h"


// Entity timer constants
#define TICK_IDLE           (0)
#define TICK_STOPPED        (1)
#define TICK_ACTIVE         (2)

#define LUA_EXPOSE(lua, x) do { lua_pushinteger(lua, x); lua_setglobal(lua, #x); } while(false)


/*
 * MISK
 */
char *SC_ParseToken(char *data, char *token)
{
    ///@FIXME: token may be overflowed
    int c;
    int len;

    len = 0;
    token[0] = 0;

    if(!data)
    {
        return NULL;
    }

// skip whitespace
    skipwhite:
    while((c = *data) <= ' ')
    {
        if(c == 0)
            return NULL;                    // end of file;
        data++;
    }

// skip // comments
    if (c=='/' && data[1] == '/')
    {
        while (*data && *data != '\n')
            data++;
        goto skipwhite;
    }

// handle quoted strings specially
    if (c == '\"')
    {
        data++;
        while (1)
        {
            c = *data++;
            if (c=='\"' || !c)
            {
                token[len] = 0;
                return data;
            }
            token[len] = c;
            len++;
        }
    }


// parse single characters
    if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
    {
        token[len] = c;
        len++;
        token[len] = 0;
        return data+1;
    }


// parse a regular word
    do
    {
        token[len] = c;
        data++;
        len++;
        c = *data;
        if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
        {
            break;
        }
    } while (c>32);

    token[len] = 0;
    return data;
}

float SC_ParseFloat(char **ch)
{
    char token[64];
    (*ch) = SC_ParseToken(*ch, token);
    if(token[0])
    {
        return atof(token);
    }
    return 0.0;
}

int SC_ParseInt(char **ch)
{
    char token[64];
    (*ch) = SC_ParseToken(*ch, token);
    if(token[0])
    {
        return atoi(token);
    }
    return 0;
}


bool Script_GetString(lua_State *lua, int string_index, size_t string_size, char *buffer)
{
    bool result = false;

    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "getString");

        if(lua_isfunction(lua, -1))
        {
            size_t *string_length = NULL;

            lua_pushinteger(lua, string_index);
            if (lua_CallAndLog(lua, 1, 1, 0))
            {
                const char* lua_str = lua_tolstring(lua, -1, string_length);
                strncpy(buffer, lua_str, string_size);
                result = true;
            }
        }
        lua_settop(lua, top);
    }

    return result;
}


bool Script_GetLoadingScreen(lua_State *lua, int level_index, char *pic_path)
{
    bool result = false;
    size_t  string_length  = 0;
    int     top;

    const char *real_path;

    if(lua != NULL)
    {
        top = lua_gettop(lua);

        lua_getglobal(lua, "getLoadingScreen");
        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, gameflow.getCurrentGameID());
            lua_pushinteger(lua, gameflow.getCurrentLevelID());
            lua_pushinteger(lua, level_index);
            if (lua_CallAndLog(lua, 3, 1, 0))
            {
                real_path = lua_tolstring(lua, -1, &string_length);

                // Lua returns constant string pointer, which we can't assign to
                // provided argument; so we need to straightly copy it.

                strncpy(pic_path, real_path, MAX_ENGINE_PATH);

                result = true;
            }
        }
        lua_settop(lua, top);
    }

    // If Lua wasn't able to extract file path from the script, most likely it means
    // that entry is broken or missing, or wrong track ID was specified. So we return
    // FALSE in such cases.

    return result;
}


/*
 * Gameplay functions
 */

int Script_DoTasks(lua_State *lua, float time)
{
    lua_pushnumber(lua, time);
    lua_setglobal(lua, "frame_time");

    Script_CallVoidFunc(lua, "doTasks");
    Script_CallVoidFunc(lua, "clearKeys");

    return 0;
}

void Script_AddKey(lua_State *lua, int keycode, int state)
{
    int top = lua_gettop(lua);

    lua_getglobal(lua, "addKey");

    if(!lua_isfunction(lua, -1))
    {
        lua_settop(lua, top);
        return;
    }

    lua_pushinteger(lua, keycode);
    lua_pushinteger(lua, state);
    lua_CallAndLog(lua, 2, 0, 0);

    lua_settop(lua, top);
}

bool Script_CallVoidFunc(lua_State *lua, const char* func_name, bool destroy_after_call)
{
    int top = lua_gettop(lua);

    lua_getglobal(lua, func_name);

    if(!lua_isfunction(lua, -1))
    {
        lua_settop(lua, top);
        return false;
    }

    lua_CallAndLog(lua, 0, 0, 0);

    if(destroy_after_call)
    {
        lua_pushnil(engine_lua);
        lua_setglobal(lua, func_name);
    }

    lua_settop(lua, top);
    return true;
}


/*
 * Game structures parse
 */
int Script_ParseControls(lua_State *lua, struct control_settings_s *cs)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "controls");
        lua_getfield(lua, -1, "mouse_sensitivity");
        cs->mouse_sensitivity = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "use_joy");
        cs->use_joy = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_number");
        cs->joy_number = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_rumble");
        cs->joy_rumble = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_look_axis_x");
        cs->joy_axis_map[AXIS_LOOK_X] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_look_axis_y");
        cs->joy_axis_map[AXIS_LOOK_Y] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_look_invert_x");
        cs->joy_look_invert_x = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_look_invert_y");
        cs->joy_look_invert_y = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_look_sensitivity");
        cs->joy_look_sensitivity = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_look_deadzone");
        cs->joy_look_deadzone = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_move_axis_x");
        cs->joy_axis_map[AXIS_MOVE_X] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_move_axis_y");
        cs->joy_axis_map[AXIS_MOVE_Y] = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_move_invert_x");
        cs->joy_move_invert_x = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_move_invert_y");
        cs->joy_move_invert_y = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_move_sensitivity");
        cs->joy_move_sensitivity = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "joy_move_deadzone");
        cs->joy_move_deadzone = lua_tonumber(lua, -1);
        lua_pop(lua, 1);


        lua_settop(lua, top);
        return 1;
    }

    return -1;
}

int Script_ParseScreen(lua_State *lua, struct screen_info_s *sc)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "screen");
        lua_getfield(lua, -1, "x");
        sc->x = (int16_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "y");
        sc->y = (int16_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "width");
        sc->w = (int16_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "height");
        sc->h = (int16_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "fullscreen");
        sc->fullscreen = (int8_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "debug_view_state");
        sc->debug_view_state = (int8_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "crosshair");
        sc->crosshair = (int8_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "fov");
        sc->fov = (float)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_settop(lua, top);
        return 1;
    }

    return -1;
}

int Script_ParseRender(lua_State *lua, struct render_settings_s *rs)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "render");
        lua_getfield(lua, -1, "mipmap_mode");
        rs->mipmap_mode = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "mipmaps");
        rs->mipmaps = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "lod_bias");
        rs->lod_bias = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "anisotropy");
        rs->anisotropy = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "antialias");
        rs->antialias = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "antialias_samples");
        rs->antialias_samples = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "texture_border");
        rs->texture_border = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "z_depth");
        rs->z_depth = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "fog_enabled");
        rs->fog_enabled = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "fog_start_depth");
        rs->fog_start_depth = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "fog_end_depth");
        rs->fog_end_depth = lua_tonumber(lua, -1);
        lua_pop(lua, 1);


        lua_getfield(lua, -1, "fog_color");
        if(lua_istable(lua, -1))
        {
            lua_getfield(lua, -1, "r");
            rs->fog_color[0] = (GLfloat)lua_tonumber(lua, -1) / 255.0;
            lua_pop(lua, 1);

            lua_getfield(lua, -1, "g");
            rs->fog_color[1] = (GLfloat)lua_tonumber(lua, -1) / 255.0;
            lua_pop(lua, 1);

            lua_getfield(lua, -1, "b");
            rs->fog_color[2] = (GLfloat)lua_tonumber(lua, -1) / 255.0;
            lua_pop(lua, 1);

            rs->fog_color[3] = 1.0; // Not sure if we need this at all...
        }

        if(rs->z_depth != 8 && rs->z_depth != 16 && rs->z_depth != 24)
        {
            rs->z_depth = 24;
        }

        lua_settop(lua, top);
        return 1;
    }

    return -1;
}


int Script_ParseConsole(lua_State *lua)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "console");
        lua_getfield(lua, -1, "background_color");
        if(lua_istable(lua, -1))
        {
            float color[4];
            lua_getfield(lua, -1, "r");
            color[0] = lua_tonumber(lua, -1) / 255.0;
            lua_pop(lua, 1);

            lua_getfield(lua, -1, "g");
            color[1] = lua_tonumber(lua, -1) / 255.0;
            lua_pop(lua, 1);

            lua_getfield(lua, -1, "b");
            color[2] = lua_tonumber(lua, -1) / 255.0;
            lua_pop(lua, 1);

            lua_getfield(lua, -1, "a");
            color[3] = lua_tonumber(lua, -1) / 255.0;
            lua_pop(lua, 1);
            Con_SetBackgroundColor(color);
        }
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "spacing");
        Con_SetLineInterval(lua_tonumber(lua, -1));
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "line_size");
        Con_SetMaxLineLenght(lua_tonumber(lua, -1));
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "lines_count");
        Con_SetLinesCount(lua_tonumber(lua, -1));
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "log_size");
        Con_SetLogLinesCount(lua_tonumber(lua, -1));
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "showing_lines");
        Con_SetShowingLines(lua_tonumber(lua, -1));
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "show");
        Con_SetShown(lua_tonumber(lua, -1));
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "show_cursor_period");
        Con_SetShowCursorPeriod(lua_tonumber(lua, -1));
        lua_pop(lua, 1);

        lua_settop(lua, top);
        return 1;
    }

    return -1;
}


bool lua_CallWithError(lua_State *lua, int nargs, int nresults, int errfunc, const char *cfile, int cline)
{
    if(lua_pcall(lua, nargs, nresults, errfunc) != LUA_OK)
    {
        if (lua_gettop(lua) > 0 && lua_isstring(lua, -1))
        {
            const char *luaErrorDescription = lua_tostring(lua, -1);
            Con_Warning("Lua error: %s (called from %s:%d)", luaErrorDescription, cfile, cline);
            lua_pop(lua, 1);
        }
        else
        {
            Con_Warning("Lua error without message (called from %s:%d)", cfile, cline);
        }
        return false;
    }
    return true;
}


/*
 * debug functions
 */
int lua_print(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top == 0)
    {
       Con_AddLine("nil", FONTSTYLE_CONSOLE_EVENT);
    }

    for(int i = 1; i <= top; i++)
    {
        switch(lua_type(lua, i))
        {
            case LUA_TNUMBER:
            case LUA_TSTRING:
               Con_AddLine(lua_tostring(lua, i), FONTSTYLE_CONSOLE_EVENT);
               break;

            case LUA_TBOOLEAN:
               Con_AddLine(lua_toboolean(lua, i) ? ("true") : ("false"), FONTSTYLE_CONSOLE_EVENT);
               break;

            case LUA_TFUNCTION:
                Con_AddLine("function", FONTSTYLE_CONSOLE_EVENT);
                break;

            case LUA_TTABLE:
                Con_AddLine("table", FONTSTYLE_CONSOLE_EVENT);
                break;

            case LUA_TTHREAD:
                Con_AddLine("thread", FONTSTYLE_CONSOLE_EVENT);
                break;

            case LUA_TLIGHTUSERDATA:
                Con_AddLine("light user data", FONTSTYLE_CONSOLE_EVENT);
                break;

            case LUA_TNIL:
                Con_AddLine("nil", FONTSTYLE_CONSOLE_EVENT);
                break;

            case LUA_TNONE:
                Con_AddLine("none", FONTSTYLE_CONSOLE_EVENT);
                break;

            default:
                Con_AddLine("none or nil", FONTSTYLE_CONSOLE_EVENT);
                break;
        };
    }

    return 0;
}


int lua_BindKey(lua_State *lua)
{
    int top = lua_gettop(lua);
    int act = lua_tointeger(lua, 1);

    if(top < 1 || act < 0 || act >= ACT_LASTINDEX)
    {
        Con_Warning("wrong action number");
    }

    else if(top == 2)
    {
        control_mapper.action_map[act].primary = lua_tointeger(lua, 2);
    }
    else if(top == 3)
    {
        control_mapper.action_map[act].primary   = lua_tointeger(lua, 2);
        control_mapper.action_map[act].secondary = lua_tointeger(lua, 3);
    }
    else
    {
        Con_Warning("bindKey: expecting arguments (action_id, key_id1, (key_id2))");
    }

    return 0;
}


 int lua_GetActionState(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        int act = lua_tointeger(lua, 1);

        if((act >= 0) && (act < ACT_LASTINDEX))
        {
            lua_pushinteger(lua, (int)(control_mapper.action_map[act].state));
            return 1;
        }

        Con_Warning("wrong action number");
        return 0;
    }

    Con_Warning("getActionState: expecting arguments (action_id)");
    return 0;
}


int lua_GetActionChange(lua_State *lua)
{
    if(lua_gettop(lua) >= 1)
    {
        int act = lua_tointeger(lua, 1);

        if((act >= 0) && (act < ACT_LASTINDEX))
        {
            lua_pushinteger(lua, (int)(control_mapper.action_map[act].already_pressed));
            return 1;
        }

        Con_Warning("wrong action number");
        return 0;
    }

    Con_Warning("getActionChange: expecting arguments (action_id)");
    return 0;
}


int lua_AddFont(lua_State *lua)
{
    if(lua_gettop(lua) != 3)
    {
        Con_Warning("addFont: expecting arguments (font index, font path, font size)");
        return 0;
    }

    if(!GLText_AddFont(lua_tointeger(lua, 1), lua_tointeger(lua, 3), lua_tostring(lua, 2)))
    {
        Con_Warning("can't create font with id = %d", lua_tointeger(lua, 1));
    }

    return 0;
}


int lua_AddFontStyle(lua_State *lua)
{
    if(lua_gettop(lua) < 12)
    {
        Con_Warning("addFontStyle: expecting arguments (index, R, G, B, A, shadow, fade, rect, border, bR, bG, bB, bA, hide)");
        return 0;
    }

    font_Style  style_index = (font_Style)lua_tointeger(lua, 1);
    GLfloat     color_R     = (GLfloat)lua_tonumber(lua, 2);
    GLfloat     color_G     = (GLfloat)lua_tonumber(lua, 3);
    GLfloat     color_B     = (GLfloat)lua_tonumber(lua, 4);
    GLfloat     color_A     = (GLfloat)lua_tonumber(lua, 5);
    int         shadowed    = lua_toboolean(lua, 6);
    int         rect        = lua_toboolean(lua, 7);
    GLfloat     rect_border = (GLfloat)lua_tonumber(lua, 8);
    GLfloat     rect_R      = (GLfloat)lua_tonumber(lua, 9);
    GLfloat     rect_G      = (GLfloat)lua_tonumber(lua, 10);
    GLfloat     rect_B      = (GLfloat)lua_tonumber(lua, 11);
    GLfloat     rect_A      = (GLfloat)lua_tonumber(lua, 12);

    if(!GLText_AddFontStyle(style_index,
                         color_R, color_G, color_B, color_A,
                         shadowed, rect, rect_border,
                         rect_R, rect_G, rect_B, rect_A))
    {
        Con_Warning("can't create fontstyle with id = %d", style_index);
    }

    return 0;
}


int lua_RemoveFont(lua_State *lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning("removeFont: expecting arguments (font index)");
        return 0;
    }

    if(!GLText_RemoveFont(lua_tointeger(lua, 1)))
    {
        Con_Warning("can't remove font");
    }

    return 0;
}


int lua_RemoveFontStyle(lua_State *lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning("removeFontStyle: expecting arguments (style index)");
        return 0;
    }

    if(!GLText_RemoveFontStyle(lua_tointeger(lua, 1)))
    {
        Con_Warning("can't remove font style");
    }

    return 0;
}


// Called when something goes absolutely horribly wrong in Lua, and tries
// to produce some debug output. Lua calls abort afterwards, so sending
// the output to the internal console is not an option.
static int lua_LuaPanic(lua_State *lua)
{
    if(lua_gettop(lua) < 1)
    {
        fprintf(stderr, "Fatal lua error (no details provided).\n");
    }
    else
    {
        fprintf(stderr, "Fatal lua error: %s\n", lua_tostring(lua, 1));
    }
    fflush(stderr);
    return 0;
}


void Script_LoadConstants(lua_State *lua)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        LUA_EXPOSE(lua, TR_I);
        LUA_EXPOSE(lua, TR_I_DEMO);
        LUA_EXPOSE(lua, TR_I_UB);
        LUA_EXPOSE(lua, TR_II);
        LUA_EXPOSE(lua, TR_II_DEMO);
        LUA_EXPOSE(lua, TR_III);
        LUA_EXPOSE(lua, TR_IV);
        LUA_EXPOSE(lua, TR_IV_DEMO);
        LUA_EXPOSE(lua, TR_V);
        LUA_EXPOSE(lua, TR_UNKNOWN);

        LUA_EXPOSE(lua, GAME_1);
        LUA_EXPOSE(lua, GAME_1_1);
        LUA_EXPOSE(lua, GAME_1_5);
        LUA_EXPOSE(lua, GAME_2);
        LUA_EXPOSE(lua, GAME_2_1);
        LUA_EXPOSE(lua, GAME_2_5);
        LUA_EXPOSE(lua, GAME_3);
        LUA_EXPOSE(lua, GAME_3_5);
        LUA_EXPOSE(lua, GAME_4);
        LUA_EXPOSE(lua, GAME_4_1);
        LUA_EXPOSE(lua, GAME_5);

        LUA_EXPOSE(lua, ENTITY_STATE_ENABLED);
        LUA_EXPOSE(lua, ENTITY_STATE_ACTIVE);
        LUA_EXPOSE(lua, ENTITY_STATE_VISIBLE);
        LUA_EXPOSE(lua, ENTITY_STATE_COLLIDABLE);

        LUA_EXPOSE(lua, ENTITY_TYPE_GENERIC);
        LUA_EXPOSE(lua, ENTITY_TYPE_INTERACTIVE);
        LUA_EXPOSE(lua, ENTITY_TYPE_TRIGGER_ACTIVATOR);
        LUA_EXPOSE(lua, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR);
        LUA_EXPOSE(lua, ENTITY_TYPE_PICKABLE);
        LUA_EXPOSE(lua, ENTITY_TYPE_TRAVERSE);
        LUA_EXPOSE(lua, ENTITY_TYPE_TRAVERSE_FLOOR);
        LUA_EXPOSE(lua, ENTITY_TYPE_DYNAMIC);
        LUA_EXPOSE(lua, ENTITY_TYPE_ACTOR);

        LUA_EXPOSE(lua, ENTITY_CALLBACK_NONE);
        LUA_EXPOSE(lua, ENTITY_CALLBACK_ACTIVATE);
        LUA_EXPOSE(lua, ENTITY_CALLBACK_DEACTIVATE);
        LUA_EXPOSE(lua, ENTITY_CALLBACK_COLLISION);
        LUA_EXPOSE(lua, ENTITY_CALLBACK_STAND);
        LUA_EXPOSE(lua, ENTITY_CALLBACK_HIT);

        LUA_EXPOSE(lua, ANIM_NORMAL_CONTROL);
        LUA_EXPOSE(lua, ANIM_LOOP_LAST_FRAME);
        LUA_EXPOSE(lua, ANIM_FRAME_LOCK);

        LUA_EXPOSE(lua, ANIM_TYPE_BASE);
        LUA_EXPOSE(lua, ANIM_TYPE_HEAD_TRACK);
        LUA_EXPOSE(lua, ANIM_TYPE_WEAPON_LH);
        LUA_EXPOSE(lua, ANIM_TYPE_WEAPON_RH);
        LUA_EXPOSE(lua, ANIM_TYPE_WEAPON_TH);
        LUA_EXPOSE(lua, ANIM_TYPE_MISK_1);
        LUA_EXPOSE(lua, ANIM_TYPE_MISK_2);
        LUA_EXPOSE(lua, ANIM_TYPE_MISK_3);
        LUA_EXPOSE(lua, ANIM_TYPE_MISK_4);

        LUA_EXPOSE(lua, MOVE_STATIC_POS);
        LUA_EXPOSE(lua, MOVE_KINEMATIC);
        LUA_EXPOSE(lua, MOVE_ON_FLOOR);
        LUA_EXPOSE(lua, MOVE_WADE);
        LUA_EXPOSE(lua, MOVE_QUICKSAND);
        LUA_EXPOSE(lua, MOVE_ON_WATER);
        LUA_EXPOSE(lua, MOVE_UNDERWATER);
        LUA_EXPOSE(lua, MOVE_FREE_FALLING);
        LUA_EXPOSE(lua, MOVE_CLIMBING);
        LUA_EXPOSE(lua, MOVE_MONKEYSWING);
        LUA_EXPOSE(lua, MOVE_WALLS_CLIMB);
        LUA_EXPOSE(lua, MOVE_DOZY);
        LUA_EXPOSE(lua, MOVE_FLY);

        LUA_EXPOSE(lua, TRIGGER_OP_OR);
        LUA_EXPOSE(lua, TRIGGER_OP_XOR);

        LUA_EXPOSE(lua, RESP_KILL);
        LUA_EXPOSE(lua, RESP_VERT_COLLIDE);
        LUA_EXPOSE(lua, RESP_HOR_COLLIDE);
        LUA_EXPOSE(lua, RESP_SLIDE);

        LUA_EXPOSE(lua, TICK_IDLE);
        LUA_EXPOSE(lua, TICK_STOPPED);
        LUA_EXPOSE(lua, TICK_ACTIVE);

        LUA_EXPOSE(lua, COLLISION_SHAPE_BOX);
        LUA_EXPOSE(lua, COLLISION_SHAPE_BOX_BASE);
        LUA_EXPOSE(lua, COLLISION_SHAPE_SPHERE);
        LUA_EXPOSE(lua, COLLISION_SHAPE_TRIMESH);
        LUA_EXPOSE(lua, COLLISION_SHAPE_TRIMESH_CONVEX);
        LUA_EXPOSE(lua, COLLISION_SHAPE_SINGLE_BOX);
        LUA_EXPOSE(lua, COLLISION_SHAPE_SINGLE_SPHERE);

        LUA_EXPOSE(lua, COLLISION_NONE);
        LUA_EXPOSE(lua, COLLISION_GROUP_ALL);
        LUA_EXPOSE(lua, COLLISION_GROUP_STATIC_ROOM);
        LUA_EXPOSE(lua, COLLISION_GROUP_STATIC_OBLECT);
        LUA_EXPOSE(lua, COLLISION_GROUP_KINEMATIC);
        //LUA_EXPOSE(lua, COLLISION_GROUP_GHOST);
        LUA_EXPOSE(lua, COLLISION_GROUP_TRIGGERS);
        LUA_EXPOSE(lua, COLLISION_GROUP_CHARACTERS);
        LUA_EXPOSE(lua, COLLISION_GROUP_VEHICLE);
        LUA_EXPOSE(lua, COLLISION_GROUP_BULLETS);
        LUA_EXPOSE(lua, COLLISION_GROUP_DYNAMICS);
        LUA_EXPOSE(lua, COLLISION_GROUP_DYNAMICS_NI);

        LUA_EXPOSE(lua, ENTITY_TRIGGERING_ACTIVATED);
        LUA_EXPOSE(lua, ENTITY_TRIGGERING_DEACTIVATED);
        LUA_EXPOSE(lua, ENTITY_TRIGGERING_NOT_READY);

        lua_pushstring(lua, Engine_GetBasePath());
        lua_setglobal(lua, "base_path");

        lua_settop(lua, top);
    }
}


bool Script_LuaInit()
{
    bool ret = false;
    engine_lua = luaL_newstate();

    if(engine_lua)
    {
        luaL_openlibs(engine_lua);
        Script_LoadConstants(engine_lua);
        Script_LuaRegisterFuncs(engine_lua);
        lua_atpanic(engine_lua, lua_LuaPanic);

        // Load script loading order (sic!)
        Script_DoLuaFile(engine_lua, "scripts/loadscript.lua");
        ret = true;
    }

    return ret;
}


int Script_DoLuaFile(lua_State *lua, const char *local_path)
{
    char script_path[1024];
    strncpy(script_path, Engine_GetBasePath(), sizeof(script_path));
    strncat(script_path, local_path, sizeof(script_path));
    return luaL_dofile(lua, script_path);
}


void Script_LuaClearTasks()
{
    if(engine_lua)
    {
        int top = lua_gettop(engine_lua);

        Script_CallVoidFunc(engine_lua, "clearTasks");

        lua_getglobal(engine_lua, "tlist_Clear");
        if(lua_isfunction(engine_lua, -1))
        {
            lua_CallAndLog(engine_lua, 0, 1, 0);
        }

        lua_getglobal(engine_lua, "entfuncs_Clear");
        if(lua_isfunction(engine_lua, -1))
        {
            lua_CallAndLog(engine_lua, 0, 1, 0);
        }

        lua_settop(engine_lua, top);
    }
}


void Script_LuaRegisterAnimFuncs(lua_State *lua);
void Script_LuaRegisterEntityFuncs(lua_State *lua);
void Script_LuaRegisterCharacterFuncs(lua_State *lua);
void Script_LuaRegisterWorldFuncs(lua_State *lua);
void Script_LuaRegisterAudioFuncs(lua_State *lua);


void Script_LuaRegisterFuncs(lua_State *lua)
{
    luaL_dostring(lua, CVAR_LUA_TABLE_NAME " = {};");

    lua_register(lua, "print", lua_print);

    lua_register(lua, "getActionState", lua_GetActionState);
    lua_register(lua, "getActionChange", lua_GetActionChange);

    lua_register(lua, "bind", lua_BindKey);
    lua_register(lua, "addFont", lua_AddFont);
    lua_register(lua, "removeFont", lua_RemoveFont);
    lua_register(lua, "addFontStyle", lua_AddFontStyle);
    lua_register(lua, "removeFontStyle", lua_RemoveFontStyle);

    Game_RegisterLuaFunctions(lua);

    Script_LuaRegisterAnimFuncs(lua);
    Script_LuaRegisterEntityFuncs(lua);
    Script_LuaRegisterCharacterFuncs(lua);
    Script_LuaRegisterWorldFuncs(lua);
    Script_LuaRegisterAudioFuncs(lua);
}
