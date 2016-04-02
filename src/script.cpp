
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "al/AL/al.h"
#include "al/AL/alc.h"
}

#include "core/system.h"
#include "core/gl_text.h"
#include "core/console.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "render/camera.h"
#include "render/render.h"
#include "vt/tr_versions.h"
#include "mesh.h"
#include "skeletal_model.h"
#include "inventory.h"
#include "trigger.h"
#include "room.h"
#include "script.h"
#include "entity.h"
#include "world.h"
#include "engine.h"
#include "physics.h"
#include "controls.h"
#include "game.h"
#include "gameflow.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "audio.h"
#include "gui.h"
#include "engine_string.h"

// Response constants
#define RESP_KILL           (0)
#define RESP_VERT_COLLIDE   (1)
#define RESP_HOR_COLLIDE    (2)
#define RESP_SLIDE          (3)

// Entity timer constants
#define TICK_IDLE           (0)
#define TICK_STOPPED        (1)
#define TICK_ACTIVE         (2)


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

/*
 *   Specific functions to get specific parameters from script.
 */
 int Script_GetGlobalSound(lua_State *lua, int global_sound_id)
{
    lua_Integer sound_id = 0;

    if(lua)
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "getGlobalSound");

        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, World_GetVersion());
            lua_pushinteger(lua, global_sound_id);
            if(lua_CallAndLog(lua, 2, 1, 0))
            {
                sound_id = lua_tointeger(lua, -1);
            }
        }
        lua_settop(lua, top);
    }

    return (int)sound_id;
}

int Script_GetSecretTrackNumber(lua_State *lua)
{
    lua_Integer track_number = 0;

    if(lua)
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "getSecretTrackNumber");

        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, World_GetVersion());
            if(lua_CallAndLog(lua, 1, 1, 0))
            {
                track_number = lua_tointeger(lua, -1);
            }
        }
        lua_settop(lua, top);
    }

    return (int)track_number;
}

int Script_GetNumTracks(lua_State *lua)
{
    lua_Integer num_tracks = 0;

    if(lua)
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "getNumTracks");

        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, World_GetVersion());
            if(lua_CallAndLog(lua, 1, 1, 0))
                num_tracks = lua_tointeger(lua, -1);
        }
        lua_settop(lua, top);
    }

    return (int)num_tracks;
}


bool Script_GetOverridedSamplesInfo(lua_State *lua, int *num_samples, int *num_sounds, char *sample_name_mask)
{
    bool result = false;

    if(lua)
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "getOverridedSamplesInfo");
        const char *real_path;

        if (lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, World_GetVersion());
            if (lua_CallAndLog(lua, 1, 3, 0))
            {
                size_t string_length = 0;
                real_path   = lua_tolstring(lua, -1, &string_length);
               *num_sounds  = lua_tointeger(lua, -2);
               *num_samples = lua_tointeger(lua, -3);
                strncpy(sample_name_mask, real_path, 255);

                if((*num_sounds != -1) && (*num_samples != -1) && (strcmp(real_path, "NONE") != 0))
                    result = true;
            }
        }
        lua_settop(lua, top);
    }

    // If Lua environment doesn't exist or script function returned -1 in one of the
    // fields, it means that corresponding sample override table is missing or not
    // valid - hence, return false.

    return result;
}


bool Script_GetOverridedSample(lua_State *lua, int sound_id, int *first_sample_number, int *samples_count)
{
    bool result = false;

    if(lua)
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "getOverridedSample");

        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, World_GetVersion());
            lua_pushinteger(lua, gameflow_manager.CurrentLevelID);
            lua_pushinteger(lua, sound_id);
            if(lua_CallAndLog(lua, 3, 2, 0))
            {
                *first_sample_number = (int)lua_tointeger(lua, -2);
                *samples_count       = (int)lua_tointeger(lua, -1);

                if((*first_sample_number != -1) && (*samples_count != -1))
                    result = true;
            }
        }
        lua_settop(lua, top);
    }

    return result;
}


bool Script_GetSoundtrack(lua_State *lua, int track_index, char *file_path, int *load_method, int *stream_type)
{
    bool result = false;

    if(lua)
    {
        int top = lua_gettop(lua);
        const char *real_path;

        lua_getglobal(lua, "getTrackInfo");

        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, World_GetVersion());
            lua_pushinteger(lua, track_index);
            if(lua_CallAndLog(lua, 2, 3, 0))
            {
                size_t string_length  = 0;
                real_path   = lua_tolstring(lua, -3, &string_length);
               *stream_type = (int)lua_tointeger(lua, -2);
               *load_method = (int)lua_tointeger(lua, -1);

                // Lua returns constant string pointer, which we can't assign to
                // provided argument; so we need to straightly copy it.

                strncpy(file_path, real_path, 255);

                if(*stream_type != -1)
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
            lua_pushinteger(lua, gameflow_manager.CurrentGameID);
            lua_pushinteger(lua, gameflow_manager.CurrentLevelID);
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

int Script_ExecEntity(lua_State *lua, int id_callback, int id_object, int id_activator)
{
    int top = lua_gettop(lua);
    int ret = -1;

    lua_getglobal(lua, "execEntity");
    if(lua_isfunction(lua, -1))
    {
        int argn = 0;
        lua_pushinteger(lua, id_callback);  argn++;
        lua_pushinteger(lua, id_object);    argn++;

        if(id_activator >= 0)
        {
            lua_pushinteger(lua, id_activator);
            argn++;
        }

        lua_CallAndLog(lua, argn, 0, 0);
        ret = 1;
    }
    lua_settop(lua, top);
    //Sys_Warn("Broken \"execEntity\" script function");
    return ret;
}

void Script_LoopEntity(lua_State *lua, int object_id)
{
    entity_p ent = World_GetEntityByID(object_id);
    if((lua) && (ent->state_flags & ENTITY_STATE_ACTIVE))
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "loopEntity");
        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, object_id);
            lua_CallAndLog(lua, 1, 0, 0);
        }
        lua_settop(lua, top);
    }
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

        sc->w_unit = (GLfloat)sc->w / SYS_SCREEN_METERING_RESOLUTION;
        sc->h_unit = (GLfloat)sc->h / SYS_SCREEN_METERING_RESOLUTION;
        lua_getfield(lua, -1, "fullscreen");
        sc->FS_flag = (int8_t)lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "debug_info");
        sc->show_debuginfo = (int8_t)lua_tonumber(lua, -1);
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

int Script_ParseAudio(lua_State *lua, struct audio_settings_s *as)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "audio");
        lua_getfield(lua, -1, "music_volume");
        as->music_volume = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "sound_volume");
        as->sound_volume = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "use_effects");
        as->use_effects  = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "listener_is_player");
        as->listener_is_player = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "stream_buffer_size");
        as->stream_buffer_size = (lua_tonumber(lua, -1)) * 1024;
        lua_pop(lua, 1);

        if(as->stream_buffer_size <= 0)
        {
            as->stream_buffer_size = 128 * 1024;
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
        char errormessage[4096];
        if (lua_gettop(lua) > 0 && lua_isstring(lua, -1))
        {
            const char *luaErrorDescription = lua_tostring(lua, -1);
            snprintf(errormessage, sizeof(errormessage), "Lua error: %s (called from %s:%d)", luaErrorDescription, cfile, cline);
            lua_pop(lua, 1);
        }
        else
        {
            snprintf(errormessage, sizeof(errormessage), "Lua error without message (called from %s:%d)", cfile, cline);
        }
        Con_AddLine(errormessage, FONTSTYLE_CONSOLE_WARNING);
        return false;
    }
    return true;
}


/*
 * *****************************************************************************
 */

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
        Con_Warning("expecting arguments (action_id, key_id1, (key_id2))");
    }

    return 0;
}


 int lua_DumpModel(lua_State * lua)
 {
     int id = 0;
     if(lua_gettop(lua) > 0)
     {
         id = lua_tointeger(lua, 1);
     }

     skeletal_model_p sm = World_GetModelByID(id);
     if(sm == NULL)
     {
        Con_Printf("wrong model id = %d", id);
        return 0;
     }

     for(int i = 0; i < sm->mesh_count; i++)
     {
         Con_Printf("mesh[%d] = %d", i, sm->mesh_tree[i].mesh_base->id);
     }

     return 0;
 }


int lua_DumpRoom(lua_State * lua)
{
    room_p r = NULL;

    if(lua_gettop(lua) == 0)
    {
        r = engine_camera.current_room;
    }
    else
    {
        uint32_t id = lua_tointeger(lua, 1);
        r = World_GetRoomByID(id);
        if(!r)
        {
            Con_Warning("wrong room id = %d", id);
            return 0;
        }
    }

    if(r != NULL)
    {
        room_sector_p rs = r->sectors;
        Sys_DebugLog("room_dump.txt", "ROOM = %d, (%d x %d), bottom = %g, top = %g, pos(%g, %g)", r->id, r->sectors_x, r->sectors_y, r->bb_min[2], r->bb_max[2], r->transform[12 + 0], r->transform[12 + 1]);
        Sys_DebugLog("room_dump.txt", "flag = 0x%X, alt_room = %d, base_room = %d", r->flags, (r->alternate_room != NULL) ? (r->alternate_room->id) : (-1), (r->base_room != NULL) ? (r->base_room->id) : (-1));
        for(uint32_t i = 0; i < r->sectors_count; i++, rs++)
        {
            Sys_DebugLog("room_dump.txt", "(%d,%d)\tfloor = %d, ceiling = %d, portal = %d", rs->index_x, rs->index_y, rs->floor, rs->ceiling, (rs->portal_to_room) ? (rs->portal_to_room->id) : (-1));
        }
        for(static_mesh_p sm = r->content->static_mesh; sm < r->content->static_mesh + r->content->static_mesh_count;sm++)
        {
            Sys_DebugLog("room_dump.txt", "static_mesh = %d", sm->object_id);
        }
        for(engine_container_p cont = r->content->containers; cont; cont = cont->next)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                entity_p ent = (entity_p)cont->object;
                Sys_DebugLog("room_dump.txt", "entity: id = %d, model = %d", ent->id, ent->bf->animations.model->id);
            }
        }
    }

    return 0;
}


int lua_SetRoomEnabled(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("wrong arguments number, shoul be (room_id, state)");
        return 0;
    }

    uint32_t id = lua_tointeger(lua, 1);
    room_p r = World_GetRoomByID(id);
    if(!r)
    {
        Con_Warning("wrong room id = %d", id);
        return 0;
    }

    if(lua_tointeger(lua, 2) == 0)
    {
        Room_Disable(r);
    }
    else
    {
        Room_Enable(r);
    }

    return 0;
}

/*
 * Base engine functions
 */
int lua_SetModelCollisionMap(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning("wrong arguments number, shoul be (model_id, map_index, value)");
        return 0;
    }

    skeletal_model_p model = World_GetModelByID(lua_tointeger(lua, 1));
    if(model == NULL)
    {
        Con_Warning("wrong model id = %d", lua_tointeger(lua, 1));
        return 0;
    }

    int arg = lua_tointeger(lua, 2);
    int val = lua_tointeger(lua, 3);
    if((arg >= 0) && (arg < model->mesh_count) &&
       (val >= 0) && (val < model->mesh_count))
    {
        model->collision_map[arg] = val;
    }

    return 0;
}


/*
 * Base Entity trigger functions
 */
int lua_ActivateEntity(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 6)
    {
        Con_Warning("expecting arguments (object_id, activator_id, trigger_mask, trigger_op, trigger_lock, trigger_timer)");
        return 0;
    }

    entity_p object             = World_GetEntityByID(lua_tointeger(lua, 1));
    entity_p activator          = World_GetEntityByID(lua_tointeger(lua, 2));
    uint16_t trigger_mask       = lua_tointeger(lua, 3);
    uint16_t trigger_op         = lua_tointeger(lua, 4);
    uint16_t trigger_lock       = lua_tointeger(lua, 5);
    uint16_t trigger_timer      = lua_tointeger(lua, 6);

    if(object)
    {
        Entity_Activate(object, activator, trigger_mask, trigger_op, trigger_lock, trigger_timer);
    }

    return 0;
}


int lua_DeactivateEntity(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning("expecting arguments (object_id, activator_id)");
        return 0;
    }

    entity_p object             = World_GetEntityByID(lua_tointeger(lua, 1));
    entity_p activator          = World_GetEntityByID(lua_tointeger(lua, 2));

    if(object)
    {
        Entity_Deactivate(object, activator);
    }

    return 0;
}


int lua_NoFixEntityCollision(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_Warning("Expecting arguments (entity_id)");
        return 0;
    }

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));

    if(ent)
    {
        ent->no_fix_all = 0x01;
    }

    return 0;
}


int lua_EnableEntity(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("missed argument entity_id");
        return 0;
    }

    entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
    if(ent)
    {
        Entity_Enable(ent);
    }

    return 0;
}


int lua_DisableEntity(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("missed argument entity_id");
        return 0;
    }

    entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
    if(ent)
    {
        Entity_Disable(ent);
    }

    return 0;
}


int lua_SetEntityCollision(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_Warning("missed argument entity_id");
        return 0;
    }

    entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
    if(ent)
    {
        if((top >= 2) && (lua_tointeger(lua, 2)))
        {
            Entity_EnableCollision(ent);
        }
        else
        {
            Entity_DisableCollision(ent);
        }
    }

    return 0;
}


int lua_SetEntityCollisionFlags(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning("expecting arguments (entity_id, collision_type, collision_shape)");
        return 0;
    }

    uint32_t id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);
    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    uint16_t collision_type = lua_tointeger(lua, 2);
    uint16_t collision_shape = lua_tointeger(lua, 3);
    if((ent->self->collision_type & 0x0001) && !(collision_type & 0x0001))
    {
        Entity_DisableCollision(ent);
    }
    else if(!(ent->self->collision_type & 0x0001) && (collision_type & 0x0001))
    {
        Entity_EnableCollision(ent);
    }
    ent->self->collision_type = collision_type;
    ent->self->collision_shape = collision_shape;
    if(Physics_GetBodiesCount(ent->physics) != ent->bf->bone_tag_count)
    {
        ent->self->collision_shape = COLLISION_SHAPE_SINGLE_BOX;
    }

    return 0;
}


int lua_GetEntitySectorFlags(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No entity specified - return.
    entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));

    if(ent && (ent->current_sector))
    {
        lua_pushinteger(lua, ent->current_sector->flags);
        return 1;
    }
    return 0;
}


int lua_GetEntitySectorIndex(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No entity specified - return.
    entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));

    if(ent && (ent->current_sector))
    {
        lua_pushinteger(lua, ent->current_sector->trig_index);
        return 1;
    }
    return 0;
}


int lua_GetEntitySectorMaterial(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No entity specified - return.
    entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));

    if(ent && (ent->current_sector))
    {
        lua_pushinteger(lua, ent->current_sector->material);
        return 1;
    }
    return 0;
}


int lua_SameRoom(lua_State *lua)
{
    if(lua_gettop(lua) != 2)
    {
        Con_Warning("expecting arguments (ent_id1, ent_id2)");
        return 0;
    }

    entity_p ent1 = World_GetEntityByID(lua_tonumber(lua, 1));
    entity_p ent2 = World_GetEntityByID(lua_tonumber(lua, 1));

    if(ent1 && ent2)
    {
        lua_pushboolean(lua, ent1->self->room == ent2->self->room);
        return 1;
    }

    lua_pushboolean(lua, 0);
    return 1;
}


int lua_NewSector(lua_State *lua)
{
    if(lua_gettop(lua) > 0)
    {
        entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
        if(ent)
        {
            lua_pushboolean(lua, ent->current_sector == ent->last_sector);
            return 1;
        }
    }

    return 0;   // No argument specified - return.
}


int lua_GetGravity(lua_State * lua)
{
    float g[3];
    Physics_GetGravity(g);
    lua_pushnumber(lua, g[0]);
    lua_pushnumber(lua, g[1]);
    lua_pushnumber(lua, g[2]);

    return 3;
}


int lua_SetGravity(lua_State * lua)                                             // function to be exported to Lua
{
    float g[3];

    switch(lua_gettop(lua))
    {
        case 0:                                                                 // get value
            Physics_GetGravity(g);
            Con_Printf("gravity = (%.3f, %.3f, %.3f)", g[0], g[1], g[2]);
            break;

        case 1:                                                                 // set z only value
            g[0] = 0.0;
            g[1] = 0.0;
            g[2] = lua_tonumber(lua, 1);
            Physics_SetGravity(g);
            Con_Printf("gravity = (%.3f, %.3f, %.3f)", g[0], g[1], g[2]);
            break;

        case 3:                                                                 // set xyz value
            g[0] = lua_tonumber(lua, 1);
            g[1] = lua_tonumber(lua, 2);
            g[2] = lua_tonumber(lua, 3);
            Physics_SetGravity(g);
            Con_Printf("gravity = (%.3f, %.3f, %.3f)", g[0], g[1], g[2]);
            break;

        default:
            Con_Warning("expecting arguments none or (oz), or (ox, oy, oz)");
            break;
    };

    return 0;
}


int lua_DropEntity(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning("expecting arguments (entity_id, time, (only_room))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);
    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    float move[3], g[3], t, time = lua_tonumber(lua, 2);
    float from[3], to[3];
    collision_result_t cb;

    Physics_GetGravity(g);
    vec3_mul_scalar(move, ent->speed, time);
    t = 0.5 * time * time;
    vec3_add_mul(move, move, g, t);
    ent->speed[0] += g[0] * time;
    ent->speed[1] += g[1] * time;
    ent->speed[2] += g[2] * time;

    Mat4_vec3_mul_macro(from, ent->transform, ent->bf->centre);
    from[2] = ent->transform[12 + 2];
    vec3_add(to, from, move);
    to[2] -= (ent->bf->bb_max[2] - ent->bf->bb_min[2]);

    if(Physics_RayTest(&cb, from, to, ent->self))
    {
        bool only_room = (top > 2) ? (lua_toboolean(lua, 3)) : (false);

        if((!only_room) || ((only_room) && (cb.obj) && (cb.obj->object_type == OBJECT_ROOM_BASE)))
        {
            ent->transform[12 + 2] = cb.point[2];
            lua_pushboolean(lua, 1);
        }
        else
        {
            lua_pushboolean(lua, 0);
        }
    }
    else
    {
        vec3_add_to(ent->transform + 12, move);
        lua_pushboolean(lua, 0);
    }

    Entity_UpdateRigidBody(ent, 1);
    return 1;
}


int lua_GetEntityModelID(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No argument - return.

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
    if(ent == NULL) return 0;

    if(ent->bf->animations.model)
    {
        lua_pushinteger(lua, ent->bf->animations.model->id);
        return 1;
    }
    return 0;
}


int lua_GetEntityActivationOffset(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No argument - return.

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
    if(ent == NULL) return 0;

    lua_pushnumber(lua, ent->activation_offset[0]);
    lua_pushnumber(lua, ent->activation_offset[1]);
    lua_pushnumber(lua, ent->activation_offset[2]);
    lua_pushnumber(lua, ent->activation_offset[3]);

    return 4;
}


int lua_SetEntityActivationOffset(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_AddLine("not set entity id", FONTSTYLE_GENERIC);
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);
    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    if(top >= 4)
    {
        ent->activation_offset[0] = lua_tonumber(lua, 2);
        ent->activation_offset[1] = lua_tonumber(lua, 3);
        ent->activation_offset[2] = lua_tonumber(lua, 4);
    }
    if(top >= 5)
    {
        ent->activation_offset[3] = lua_tonumber(lua, 5);
    }

    return 0;
}


int lua_SetCharacterTarget(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top == 0)
    {
        Con_Warning("expecting arguments (entity_id, (target_id))");
        return 0;
    }

    uint32_t character_id = lua_tointeger(lua, 1);
    entity_p ent     = World_GetEntityByID(character_id);

    if(!ent || !ent->character)
    {
        Con_Warning("no character with id = %d", character_id);
        return 0;
    }

    ent->character->target_id = (top > 1) ? (lua_tointeger(lua, 2)) : (ENTITY_ID_NONE);

    return 0;
}


int lua_GetCharacterParam(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("expecting arguments (entity_id, param)");
        return 0;
    }

    int id         = lua_tointeger(lua, 1);
    int parameter  = lua_tointeger(lua, 2);
    entity_p ent   = World_GetEntityByID(id);

    if(parameter >= PARAM_LASTINDEX)
    {
        Con_Warning("wrong option index, expecting id < %d", PARAM_LASTINDEX);
        return 0;
    }

    if(ent && ent->character)
    {
        lua_pushnumber(lua, Character_GetParam(ent, parameter));
        return 1;
    }
    else
    {
        Con_Warning("no character with id = %d", id);
        return 0;
    }
}


int lua_SetCharacterParam(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 3)
    {
        Con_Warning("expecting arguments (entity_id, param, value, (max_value))");
        return 0;
    }

    int id           = lua_tointeger(lua, 1);
    int parameter    = lua_tointeger(lua, 2);
    entity_p ent     = World_GetEntityByID(id);

    if(parameter >= PARAM_LASTINDEX)
    {
        Con_Warning("wrong option index, expecting id < %d", PARAM_LASTINDEX);
        return 0;
    }

    if(!ent || !ent->character)
    {
        Con_Warning("no character with id = %d", id);
        return 0;
    }
    else if(top == 3)
    {
        Character_SetParam(ent, parameter, lua_tonumber(lua, 3));
    }
    else
    {
        ent->character->parameters.param[parameter] = lua_tonumber(lua, 3);
        ent->character->parameters.maximum[parameter] = lua_tonumber(lua, 4);
    }

    return 0;
}


int lua_GetCharacterCombatMode(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;
    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));

    if(ent && ent->character)
    {
        lua_pushnumber(lua, ent->character->weapon_current_state);
        return 1;
    }

    return 0;
}


int lua_ChangeCharacterParam(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning("expecting arguments (entity_id, param, value)");
        return 0;
    }

    int id         = lua_tointeger(lua, 1);
    int parameter  = lua_tointeger(lua, 2);
    int value      = lua_tonumber(lua, 3);
    entity_p ent   = World_GetEntityByID(id);

    if(parameter >= PARAM_LASTINDEX)
    {
        Con_Warning("wrong option index, expecting id < %d", PARAM_LASTINDEX);
        return 0;
    }

    if(ent && ent->character)
    {
        Character_ChangeParam(ent, parameter, value);
    }
    else
    {
        Con_Warning("no character with id = %d", id);
    }

    return 0;
}


int lua_AddCharacterHair(lua_State *lua)
{
    if(lua_gettop(lua) != 2)
    {
        Con_Warning("expecting arguments (entity_id, hair_setup_index)");
    }
    else
    {
        int ent_id       = lua_tointeger(lua, 1);
        int setup_index  = lua_tointeger(lua, 2);

        entity_p ent   = World_GetEntityByID(ent_id);

        if(ent && ent->character)
        {
            hair_setup_s *hair_setup = Hair_GetSetup(lua, setup_index);

            if(!hair_setup)
            {
                Con_Warning("wrong hair setup index = %d", setup_index);
            }
            else
            {
                ent->character->hair_count++;
                ent->character->hairs = (struct hair_s**)realloc(ent->character->hairs, (sizeof(struct hair_s*) * ent->character->hair_count));
                ent->character->hairs[ent->character->hair_count-1] = Hair_Create(hair_setup, ent->physics);
                if(!ent->character->hairs[ent->character->hair_count-1])
                {
                    Con_Warning("can not create hair for entity_id = %d", ent_id);
                }
            }
            free(hair_setup);
        }
        else
        {
            Con_Warning("no character with id = %d", ent_id);
        }
    }
    return 0;
}


int lua_ResetCharacterHair(lua_State *lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }
    else
    {
        int ent_id   = lua_tointeger(lua, 1);
        entity_p ent = World_GetEntityByID(ent_id);

        if(ent && ent->character)
        {
            if(ent->character->hairs)
            {
                for(int i = 0; i < ent->character->hair_count; i++)
                {
                    Hair_Delete(ent->character->hairs[i]);
                    ent->character->hairs[i] = NULL;
                }
                free(ent->character->hairs);
                ent->character->hairs = NULL;
                ent->character->hair_count = 0;
            }
            else
            {
                Con_Warning("can not create hair for entity_id = %d", ent_id);
            }
        }
        else
        {
            Con_Warning("no character with id = %d", ent_id);
        }
    }
    return 0;
}


int lua_AddEntityRagdoll(lua_State *lua)
{
    if(lua_gettop(lua) != 2)
    {
        Con_Warning("expecting arguments (entity_id, ragdoll_setup_index)");
    }
    else
    {
        int ent_id       = lua_tointeger(lua, 1);
        int setup_index  = lua_tointeger(lua, 2);

        entity_p ent   = World_GetEntityByID(ent_id);

        if(ent)
        {
            struct rd_setup_s *ragdoll_setup = Ragdoll_GetSetup(lua, setup_index);
            if(ragdoll_setup)
            {
                if(!Ragdoll_Create(ent->physics, ent->bf, ragdoll_setup))
                {
                    Con_Warning("can not create ragdoll for entity_id = %d", ent_id);
                }
                ent->type_flags |=  ENTITY_TYPE_DYNAMIC;
                Ragdoll_DeleteSetup(ragdoll_setup);
            }
            else
            {
                Con_Warning("no ragdoll setup with id = %d", setup_index);
            }
        }
        else
        {
            Con_Warning("no entity with id = %d", ent_id);
        }
    }
    return 0;
}


int lua_RemoveEntityRagdoll(lua_State *lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }
    else
    {
        int ent_id   = lua_tointeger(lua, 1);
        entity_p ent = World_GetEntityByID(ent_id);

        if(ent)
        {
            if(!Ragdoll_Delete(ent->physics))
            {
                Con_Warning("can not remove ragdoll for entity_id = %d", ent_id);
            }
            ent->type_flags &= ~ENTITY_TYPE_DYNAMIC;
        }
        else
        {
            Con_Warning("no entity with id = %d", ent_id);
        }
    }
    return 0;
}


int lua_GetSecretStatus(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No parameter specified - return

    int secret_number = lua_tointeger(lua, 1);
    if((secret_number > TR_GAMEFLOW_MAX_SECRETS) || (secret_number < 0)) return 0;   // No such secret - return

    lua_pushinteger(lua, (int)gameflow_manager.SecretsTriggerMap[secret_number]);
    return 1;
}


int lua_SetSecretStatus(lua_State *lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No parameter specified - return

    int secret_number = lua_tointeger(lua, 1);
    if((secret_number > TR_GAMEFLOW_MAX_SECRETS) || (secret_number < 0)) return 0;   // No such secret - return

    gameflow_manager.SecretsTriggerMap[secret_number] = lua_tointeger(lua, 2);
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

    Con_Warning("expecting arguments (action_id)");
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

    Con_Warning("expecting arguments (action_id)");
    return 0;
}


int lua_GetLevelVersion(lua_State *lua)
{
    lua_pushinteger(lua, World_GetVersion());
    return 1;
}


int lua_AddFont(lua_State *lua)
{
    if(lua_gettop(lua) != 3)
    {
        Con_Warning("expecting arguments (font index, font path, font size)");
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
        Con_Warning("expecting arguments (index, R, G, B, A, shadow, fade, rect, border, bR, bG, bB, bA, hide)");
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
        Con_Warning("expecting arguments (font index)");
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
        Con_Warning("expecting arguments (style index)");
        return 0;
    }

    if(!GLText_RemoveFontStyle(lua_tointeger(lua, 1)))
    {
        Con_Warning("can't remove font style");
    }

    return 0;
}


int lua_AddItem(lua_State * lua)
{
    int top, count;
    top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning("expecting arguments (entity_id, item_id, items_count)");
        return 0;
    }

    if(top >= 3)
    {
        count = lua_tointeger(lua, 3);
    }
    else
    {
        count = -1;
    }

    int entity_id = lua_tointeger(lua, 1);
    int item_id = lua_tointeger(lua, 2);

    entity_p ent = World_GetEntityByID(entity_id);

    if(ent && ent->character)
    {
        entity_p player = World_GetPlayer();
        lua_pushinteger(lua, Inventory_AddItem(&ent->character->inventory, item_id, count));
        if(!player || ent->id == player->id)
        {
            Gui_NotifierStart(item_id);
        }
        return 1;
    }

    Con_Warning("no entity with id = %d", entity_id);
    return 0;
}


int lua_RemoveItem(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning("expecting arguments (entity_id, item_id, items_count)");
        return 0;
    }

    int entity_id = lua_tointeger(lua, 1);
    int item_id = lua_tointeger(lua, 2);
    int count = lua_tointeger(lua, 3);

    entity_p ent = World_GetEntityByID(entity_id);

    if(ent && ent->character)
    {
        lua_pushinteger(lua, Inventory_RemoveItem(&ent->character->inventory, item_id, count));
        return 1;
    }

    Con_Warning("no entity with id = %d", entity_id);
    return 0;
}


int lua_RemoveAllItems(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int entity_id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(entity_id);

    if(ent && ent->character)
    {
        Inventory_RemoveAllItems(&ent->character->inventory);
    }
    else
    {
        Con_Warning("no entity with id = %d", entity_id);
    }

    return 0;
}


int lua_GetItemsCount(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("expecting arguments (entity_id, item_id)");
        return 0;
    }
    int entity_id = lua_tointeger(lua, 1);
    int item_id = lua_tointeger(lua, 2);

    entity_p ent = World_GetEntityByID(entity_id);

    if(ent && ent->character)
    {
        lua_pushinteger(lua, Inventory_GetItemsCount(ent->character->inventory, item_id));
        return 1;
    }
    else
    {
        Con_Warning("no entity with id = %d", entity_id);
        return 0;
    }

}


int lua_CreateBaseItem(lua_State * lua)
{
    if(lua_gettop(lua) < 5)
    {
        Con_Warning("expecting arguments (item_id, model_id, world_model_id, type, count, (name))");
        return 0;
    }

    int item_id         = lua_tointeger(lua, 1);
    int model_id        = lua_tointeger(lua, 2);
    int world_model_id  = lua_tointeger(lua, 3);
    int type            = lua_tointeger(lua, 4);
    int count           = lua_tointeger(lua, 5);

    World_CreateItem(item_id, model_id, world_model_id, type, count, lua_tostring(lua, 6));

    return 0;
}


int lua_DeleteBaseItem(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (item_id)");
    }
    else
    {
        World_DeleteItem(lua_tointeger(lua, 1));
    }
    return 0;
}


int lua_PrintItems(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int entity_id = lua_tointeger(lua, 1);
    entity_p  ent = World_GetEntityByID(entity_id);
    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", entity_id);
        return 0;
    }

    if(ent->character)
    {
        inventory_node_p i = ent->character->inventory;
        for(; i; i = i->next)
        {
            Con_Printf("item[id = %d]: count = %d", i->id, i->count);
        }
    }
    return 0;
}


int lua_SetStateChangeRange(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 6)
    {
        Con_Warning("expecting arguments (model_id, anim_num, state_id, dispatch_num, start_frame, end_frame, (next_anim), (next_frame))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p model = World_GetModelByID(id);

    if(model == NULL)
    {
        Con_Warning("no skeletal model with id = %d", id);
        return 0;
    }

    int anim = lua_tointeger(lua, 2);
    int state = lua_tointeger(lua, 3);
    int dispatch = lua_tointeger(lua, 4);
    int frame_low = lua_tointeger(lua, 5);
    int frame_high = lua_tointeger(lua, 6);

    if((anim < 0) || (anim + 1 > model->animation_count))
    {
        Con_Warning("wrong anim number = %d", anim);
        return 0;
    }

    animation_frame_p af = model->animations + anim;
    for(uint16_t i = 0; i < af->state_change_count; i++)
    {
        if(af->state_change[i].id == (uint32_t)state)
        {
            if((dispatch >= 0) && (dispatch < af->state_change[i].anim_dispatch_count))
            {
                af->state_change[i].anim_dispatch[dispatch].frame_low = frame_low;
                af->state_change[i].anim_dispatch[dispatch].frame_high = frame_high;
                if(top >= 8)
                {
                    af->state_change[i].anim_dispatch[dispatch].next_anim = lua_tointeger(lua, 7);
                    af->state_change[i].anim_dispatch[dispatch].next_frame = lua_tointeger(lua, 8);
                }
            }
            else
            {
                Con_Warning("wrong anim dispatch number = %d", dispatch);
            }
            break;
        }
    }

    return 0;
}


int lua_GetAnimCommandTransform(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning("expecting arguments (model_id, anim_num, frame_num)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    int anim = lua_tointeger(lua, 2);
    int frame = lua_tointeger(lua, 3);
    skeletal_model_p model = World_GetModelByID(id);
    if(model == NULL)
    {
        Con_Warning("no skeletal model with id = %d", id);
        return 0;
    }

    if((anim < 0) || (anim + 1 > model->animation_count))
    {
        Con_Warning("wrong anim number = %d", anim);
        return 0;
    }

    if(frame < 0)                                                               // it is convenient to use -1 as a last frame number
    {
        frame = (int)model->animations[anim].frames_count + frame;
    }

    if((frame < 0) || (frame + 1 > model->animations[anim].frames_count))
    {
        Con_Warning("wrong anim frame number = %d", frame);
        return 0;
    }

    lua_pushinteger(lua, model->animations[anim].frames[frame].command);
    lua_pushnumber(lua, model->animations[anim].frames[frame].move[0]);
    lua_pushnumber(lua, model->animations[anim].frames[frame].move[1]);
    lua_pushnumber(lua, model->animations[anim].frames[frame].move[2]);

    return 4;
}


int lua_SetAnimCommandTransform(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 4)
    {
        Con_Warning("expecting arguments (model_id, anim_num, frame_num, flag, (dx, dy, dz))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    int anim = lua_tointeger(lua, 2);
    int frame = lua_tointeger(lua, 3);
    skeletal_model_p model = World_GetModelByID(id);
    if(model == NULL)
    {
        Con_Warning("no skeletal model with id = %d", id);
        return 0;
    }

    if((anim < 0) || (anim + 1 > model->animation_count))
    {
        Con_Warning("wrong anim number = %d", anim);
        return 0;
    }

    if(frame < 0)                                                               // it is convenient to use -1 as a last frame number
    {
        frame = (int)model->animations[anim].frames_count + frame;
    }

    if((frame < 0) || (frame + 1 > model->animations[anim].frames_count))
    {
        Con_Warning("wrong frame number = %d", frame);
        return 0;
    }

    model->animations[anim].frames[frame].command = 0x00ff & lua_tointeger(lua, 4);
    if(top >= 7)
    {
        model->animations[anim].frames[frame].move[0] = lua_tonumber(lua, 5);
        model->animations[anim].frames[frame].move[1] = lua_tonumber(lua, 6);
        model->animations[anim].frames[frame].move[2] = lua_tonumber(lua, 7);
    }

    return 0;
}


int lua_SpawnEntity(lua_State * lua)
{
    if(lua_gettop(lua) < 5)
    {
        Con_Warning("expecting arguments (model_id1, room_id, x, y, z, (ax, ay, az))");
        return 0;
    }

    float pos[3], ang[3];
    int model_id = lua_tointeger(lua, 1);
    int room_id = lua_tointeger(lua, 2);
    pos[0] = lua_tonumber(lua, 3);
    pos[1] = lua_tonumber(lua, 4);
    pos[2] = lua_tonumber(lua, 5);
    ang[0] = lua_tonumber(lua, 6);
    ang[1] = lua_tonumber(lua, 7);
    ang[2] = lua_tonumber(lua, 8);

    int32_t ov_id = -1;
    if(lua_isnumber(lua, 9))
    {
        ov_id = lua_tointeger(lua, 9);
    }

    uint32_t id = World_SpawnEntity(model_id, room_id, pos, ang, ov_id);
    if(id == 0xFFFFFFFF)
    {
        lua_pushnil(lua);
    }
    else
    {
        lua_pushinteger(lua, id);
    }

    return 1;
}

/*
 * Moveables script control section
 */
int lua_GetEntityVector(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("expecting arguments (entity_id1, entity_id2)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p e1 = World_GetEntityByID(id);
    if(e1 == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }
    id = lua_tointeger(lua, 2);
    entity_p e2 = World_GetEntityByID(id);
    if(e2 == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushnumber(lua, e2->transform[12+0] - e1->transform[12+0]);
    lua_pushnumber(lua, e2->transform[12+1] - e1->transform[12+1]);
    lua_pushnumber(lua, e2->transform[12+2] - e1->transform[12+2]);
    return 3;
}


int lua_GetEntityDistance(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("expecting arguments (entity_id1, entity_id2)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p e1 = World_GetEntityByID(id);
    if(e1 == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }
    id = lua_tointeger(lua, 2);
    entity_p e2 = World_GetEntityByID(id);
    if(e2 == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushnumber(lua, vec3_dist(e1->transform + 12, e2->transform + 12));
    return 1;
}


int lua_GetEntityDirDot(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("expecting arguments (id1, id2)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p e1 = World_GetEntityByID(id);
    if(e1 == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }
    id = lua_tointeger(lua, 2);
    entity_p e2 = World_GetEntityByID(id);
    if(e2 == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushnumber(lua, vec3_dot(e1->transform + 4, e2->transform + 4));
    return 1;
}


int lua_GetEntityPosition(lua_State * lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);
    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushnumber(lua, ent->transform[12+0]);
    lua_pushnumber(lua, ent->transform[12+1]);
    lua_pushnumber(lua, ent->transform[12+2]);
    lua_pushnumber(lua, ent->angles[0]);
    lua_pushnumber(lua, ent->angles[1]);
    lua_pushnumber(lua, ent->angles[2]);

    return 6;
}


int lua_GetEntityAngles(lua_State * lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushnumber(lua, ent->angles[0]);
    lua_pushnumber(lua, ent->angles[1]);
    lua_pushnumber(lua, ent->angles[2]);

    return 3;
}


int lua_GetEntityScaling(lua_State * lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushnumber(lua, ent->scaling[0]);
    lua_pushnumber(lua, ent->scaling[1]);
    lua_pushnumber(lua, ent->scaling[2]);

    return 3;
}


int lua_SetEntityScaling(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 4)
    {
        Con_Warning("expecting arguments (entity_id, x_scaling, y_scaling, z_scaling)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
    }
    else
    {
        ent->scaling[0] = lua_tonumber(lua, 2);
        ent->scaling[1] = lua_tonumber(lua, 3);
        ent->scaling[2] = lua_tonumber(lua, 4);

        Physics_SetCollisionScale(ent->physics, ent->scaling);
        Entity_UpdateRigidBody(ent, 1);
    }

    return 0;
}


int lua_SimilarSector(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 5)
    {
        Con_Warning("expecting arguments (entity_id, dx, dy, dz, ignore_doors, (ceiling))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    float dx = lua_tonumber(lua, 2);
    float dy = lua_tonumber(lua, 3);
    float dz = lua_tonumber(lua, 4);

    float next_pos[3];

    next_pos[0] = ent->transform[12+0] + (dx * ent->transform[0+0] + dy * ent->transform[4+0] + dz * ent->transform[8+0]);
    next_pos[1] = ent->transform[12+1] + (dx * ent->transform[0+1] + dy * ent->transform[4+1] + dz * ent->transform[8+1]);
    next_pos[2] = ent->transform[12+2] + (dx * ent->transform[0+2] + dy * ent->transform[4+2] + dz * ent->transform[8+2]);

    room_sector_p curr_sector = Room_GetSectorRaw(ent->self->room, ent->transform+12);
    room_sector_p next_sector = Room_GetSectorRaw(ent->self->room, next_pos);

    curr_sector = Sector_GetPortalSectorTarget(curr_sector);
    next_sector = Sector_GetPortalSectorTarget(next_sector);

    bool ignore_doors = lua_toboolean(lua, 5);

    if((top >= 6) && lua_toboolean(lua, 6))
    {
        lua_pushboolean(lua, Sectors_SimilarCeiling(curr_sector, next_sector, ignore_doors));
    }
    else
    {
        lua_pushboolean(lua, Sectors_SimilarFloor(curr_sector, next_sector, ignore_doors));
    }

    return 1;
}


int lua_GetSectorHeight(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_Warning("expecting arguments (entity_id, (ceiling), (dx, dy, dz))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    bool ceiling = false;
    if(top > 1) ceiling = lua_toboolean(lua, 2);

    float pos[3];
    vec3_copy(pos, ent->transform+12);

    if(top > 2)
    {
        float dx = lua_tonumber(lua, 2);
        float dy = lua_tonumber(lua, 3);
        float dz = lua_tonumber(lua, 4);

        pos[0] += dx * ent->transform[0+0] + dy * ent->transform[4+0] + dz * ent->transform[8+0];
        pos[1] += dx * ent->transform[0+1] + dy * ent->transform[4+1] + dz * ent->transform[8+1];
        pos[2] += dx * ent->transform[0+2] + dy * ent->transform[4+2] + dz * ent->transform[8+2];
    }

    room_sector_p curr_sector = Room_GetSectorRaw(ent->self->room, pos);
    curr_sector = Sector_GetPortalSectorTarget(curr_sector);
    float point[3];
    (ceiling) ? (Sector_LowestCeilingCorner(curr_sector, point)) : (Sector_HighestFloorCorner(curr_sector, point));

    lua_pushnumber(lua, point[2]);
    return 1;
}


int lua_SectorTriggerClear(lua_State * lua)
{
    int id, sx, sy, top;

    top = lua_gettop(lua);

    if(top < 3)
    {
        Con_AddLine("wrong arguments number, must be (room_id, index_x, index_y)", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    id = lua_tointeger(lua, 1);
    sx = lua_tointeger(lua, 2);
    sy = lua_tointeger(lua, 3);
    room_sector_p rs = World_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    if(rs->trigger)
    {
        for(trigger_command_p current_command = rs->trigger->commands; current_command; )
        {
            trigger_command_p next_command = current_command->next;
            current_command->next = NULL;
            free(current_command);
            current_command = next_command;
        }
        free(rs->trigger);
        rs->trigger = NULL;
    }

    return 0;
}


int lua_SectorAddTrigger(lua_State * lua)
{
    int id, sx, sy, top;

    top = lua_gettop(lua);

    if(top < 8)
    {
        Con_AddLine("wrong arguments number, must be (room_id, index_x, index_y, function, sub_function, mask, once, timer)", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    id = lua_tointeger(lua, 1);
    sx = lua_tointeger(lua, 2);
    sy = lua_tointeger(lua, 3);
    room_sector_p rs = World_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }
    if(rs->trigger)
    {
        Con_AddLine("sector trigger already exists", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    rs->trigger = (trigger_header_p)malloc(sizeof(trigger_header_t));
    rs->trigger->commands = NULL;
    rs->trigger->function_value = lua_tointeger(lua, 4);
    rs->trigger->sub_function = lua_tointeger(lua, 5);
    rs->trigger->mask = lua_tointeger(lua, 6);
    rs->trigger->once = lua_tointeger(lua, 7);
    rs->trigger->timer = lua_tointeger(lua, 8);

    return 0;
}


int lua_SectorAddTriggerCommand(lua_State * lua)
{
    int id, sx, sy, top;

    top = lua_gettop(lua);

    if(top < 6)
    {
        Con_AddLine("wrong arguments number, must be (room_id, index_x, index_y, function, operands, once, (cam_index, cam_move, cam_timer))", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    id = lua_tointeger(lua, 1);
    sx = lua_tointeger(lua, 2);
    sy = lua_tointeger(lua, 3);
    room_sector_p rs = World_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }
    if(!rs->trigger)
    {
        Con_AddLine("sector has no base trigger", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    trigger_command_p cmd = (trigger_command_p)malloc(sizeof(trigger_command_t));

    cmd->next = NULL;
    cmd->function = lua_tointeger(lua, 4);
    cmd->operands = lua_tointeger(lua, 5);
    cmd->once = lua_tointeger(lua, 6);
    cmd->cam_index = 0;
    cmd->cam_move = 0;
    cmd->cam_timer = 0;

    if(top >= 9)
    {
        cmd->cam_index = lua_tointeger(lua, 7);
        cmd->cam_move = lua_tointeger(lua, 8);
        cmd->cam_timer = lua_tointeger(lua, 9);
    }

    trigger_command_p *last = &rs->trigger->commands;
    for(; *last; last = &(*last)->next);
    *last = cmd;

    return 0;
}


int lua_SetEntityPosition(lua_State * lua)
{
    switch(lua_gettop(lua))
    {
        case 4:
            {
                int id = lua_tointeger(lua, 1);
                entity_p ent = World_GetEntityByID(id);
                if(ent == NULL)
                {
                    Con_Warning("no entity with id = %d", id);
                    return 0;
                }
                ent->transform[12+0] = lua_tonumber(lua, 2);
                ent->transform[12+1] = lua_tonumber(lua, 3);
                ent->transform[12+2] = lua_tonumber(lua, 4);
                if(ent->character)
                {
                    Character_UpdatePlatformPreStep(ent);
                }
            }
            return 0;

        case 7:
            {
                int id = lua_tointeger(lua, 1);
                entity_p ent = World_GetEntityByID(id);
                if(ent == NULL)
                {
                    Con_Warning("no entity with id = %d", id);
                    return 0;
                }
                ent->transform[12+0] = lua_tonumber(lua, 2);
                ent->transform[12+1] = lua_tonumber(lua, 3);
                ent->transform[12+2] = lua_tonumber(lua, 4);
                ent->angles[0] = lua_tonumber(lua, 5);
                ent->angles[1] = lua_tonumber(lua, 6);
                ent->angles[2] = lua_tonumber(lua, 7);
                Entity_UpdateTransform(ent);
                if(ent->character)
                {
                    Character_UpdatePlatformPreStep(ent);
                }
            }
            return 0;

        default:
            Con_Warning("expecting arguments (entity_id, x, y, z, (fi_x, fi_y, fi_z))");
            return 0;
    }

    return 0;
}


int lua_SetEntityAngles(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning("expecting arguments (entity_id, fi_x, (fi_y, fi_z))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
    }
    else
    {
        ent->angles[0] = lua_tonumber(lua, 2);

        if(top > 2)
        {
            ent->angles[1] = lua_tonumber(lua, 3);
            ent->angles[2] = lua_tonumber(lua, 4);
        }

        Entity_UpdateTransform(ent);
    }

    return 0;
}


int lua_MoveEntityGlobal(lua_State * lua)
{
    switch(lua_gettop(lua))
    {
        case 4:
            {
                int id = lua_tointeger(lua, 1);
                entity_p ent = World_GetEntityByID(id);
                if(ent == NULL)
                {
                    Con_Printf("can not find entity with id = %d", id);
                    return 0;
                }
                ent->transform[12+0] += lua_tonumber(lua, 2);
                ent->transform[12+1] += lua_tonumber(lua, 3);
                ent->transform[12+2] += lua_tonumber(lua, 4);
                Entity_UpdateRigidBody(ent, 1);
            }
            return 0;

        default:
            Con_Warning("expecting arguments (entity_id, x, y, z)");
            return 0;
    }

    return 0;
}


int lua_MoveEntityLocal(lua_State * lua)
{
    if(lua_gettop(lua) < 4)
    {
        Con_Warning("expecting arguments (entity_id, dx, dy, dz)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    float dx = lua_tonumber(lua, 2);
    float dy = lua_tonumber(lua, 3);
    float dz = lua_tonumber(lua, 4);

    ent->transform[12+0] += dx * ent->transform[0+0] + dy * ent->transform[4+0] + dz * ent->transform[8+0];
    ent->transform[12+1] += dx * ent->transform[0+1] + dy * ent->transform[4+1] + dz * ent->transform[8+1];
    ent->transform[12+2] += dx * ent->transform[0+2] + dy * ent->transform[4+2] + dz * ent->transform[8+2];

    Entity_UpdateRigidBody(ent, 1);

    return 0;
}


int lua_MoveEntityToSink(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("expecting arguments (entity_id, sink_id)");
        return 0;
    }

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));

    if(ent)
    {
        Entity_MoveToSink(ent, lua_tointeger(lua, 2));
    }

    return 0;
}


int lua_MoveEntityToEntity(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 3)
    {
        Con_Warning("expecting arguments (entity_to_move_id, entity_id, speed, (ignore_z))");
        return 0;
    }

    entity_p ent1 = World_GetEntityByID(lua_tointeger(lua, 1));
    entity_p ent2 = World_GetEntityByID(lua_tointeger(lua, 2));
    float speed_mult = lua_tonumber(lua, 3);
    float *ent1_pos = ent1->transform + 12;
    float *ent2_pos = ent2->transform + 12;
    float t, speed[3];

    vec3_sub(speed, ent2_pos, ent1_pos);
    t = vec3_abs(speed);
    if(t == 0.0) t = 1.0; // Prevents division by zero.
    t = speed_mult / t;

    ent1->transform[12+0] += speed[0] * t;
    ent1->transform[12+1] += speed[1] * t;

    ///@FIXME: blood tears
    bool ignore_z = (top > 3) ? (lua_toboolean(lua, 4)) : (false);
    if(!ignore_z) ent1->transform[12+2] += speed[2] * t;

    if(ent1->character) Character_UpdatePlatformPreStep(ent1);
    Entity_UpdateRigidBody(ent1, 1);

    return 0;
}


int lua_RotateEntity(lua_State *lua)
{
    int top = lua_gettop(lua);

    if((top > 4) || (top < 2))
    {
        Con_Warning("expecting arguments (ent_id, rot_x, (rot_y, rot_z))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
    }
    else
    {
        ent->angles[0] += lua_tonumber(lua, 2);

        if(top == 4)
        {
             ent->angles[1] += lua_tonumber(lua, 3);
             ent->angles[2] += lua_tonumber(lua, 4);
        }

        Entity_UpdateTransform(ent);
        Entity_UpdateRigidBody(ent, 1);
    }

    return 0;
}


int lua_GetEntitySpeed(lua_State * lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushnumber(lua, ent->speed[0]);
    lua_pushnumber(lua, ent->speed[1]);
    lua_pushnumber(lua, ent->speed[2]);
    return 3;
}


int lua_GetEntitySpeedLinear(lua_State * lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushnumber(lua, vec3_abs(ent->speed));
    return 1;
}


int lua_SetEntitySpeed(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning("expecting arguments (id, speed_x, (speed_y, speed_z))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
    }
    else
    {
        ent->speed[0] = lua_tonumber(lua, 2);

        if(top > 2)
        {
            ent->speed[1] = lua_tonumber(lua, 3);
            ent->speed[2] = lua_tonumber(lua, 4);
        }

        Character_UpdateCurrentSpeed(ent);
    }

    return 0;
}


int lua_SetEntityAnim(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning("expecting arguments (entity_id, anim_id, (frame_number, another_model))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    if(top >= 3)
    {
        Entity_SetAnimation(ent, lua_tointeger(lua, 2), lua_tointeger(lua, 3));
    }
    else if(top == 2)
    {
        Entity_SetAnimation(ent, lua_tointeger(lua, 2), 0);
    }

    return 0;
}


int lua_SetEntityAnimFlag(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top != 2)
    {
        Con_Warning("expecting arguments (entity_id, anim_flag)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    ent->bf->animations.anim_frame_flags = lua_tointeger(lua,2);

    return 0;
}


int lua_SetEntityBodyPartFlag(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 3)
    {
        Con_Warning("expecting arguments (entity_id, bone_id, body_part_flag)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    int bone_id = lua_tointeger(lua, 2);
    if((bone_id < 0) || (bone_id >= ent->bf->bone_tag_count))
    {
        Con_Warning("wrong bone index = %d", bone_id);
        return 0;
    }

    ent->bf->bone_tags[bone_id].body_part = lua_tointeger(lua, 3);

    return 0;
}


int lua_SetModelBodyPartFlag(lua_State * lua)
{
    int top = lua_gettop(lua);

    if(top < 3)
    {
        Con_Warning("expecting arguments (model_id, bone_id, body_part_flag)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p model = World_GetModelByID(id);

    if(model == NULL)
    {
        Con_Warning("no skeletal model with id = %d", id);
        return 0;
    }

    int bone_id = lua_tointeger(lua, 2);
    if((bone_id < 0) || (bone_id >= model->mesh_count))
    {
        Con_Warning("wrong bone index = %d", bone_id);
        return 0;
    }

    model->mesh_tree[bone_id].body_part = lua_tointeger(lua, 3);

    return 0;
}


int lua_GetEntityAnim(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, ent->bf->animations.current_animation);
    lua_pushinteger(lua, ent->bf->animations.current_frame);
    lua_pushinteger(lua, ent->bf->animations.model->animations[ent->bf->animations.current_animation].frames_count);

    return 3;
}


int lua_CanTriggerEntity(lua_State * lua)
{
    int id;
    int top = lua_gettop(lua);
    float pos[3], offset[3], r;

    if(top < 2)
    {
        lua_pushboolean(lua, 0);
        return 1;
    }

    id = lua_tointeger(lua, 1);
    entity_p e1 = World_GetEntityByID(id);
    if(e1 == NULL || !e1->character || !e1->character->cmd.action)
    {
        lua_pushboolean(lua, 0);
        return 1;
    }

    id = lua_tointeger(lua, 2);
    entity_p e2 = World_GetEntityByID(id);
    if((e2 == NULL) || (e1 == e2))
    {
        lua_pushboolean(lua, 0);
        return 1;
    }

    r = e2->activation_offset[3];
    if(top >= 3)
    {
        r = lua_tonumber(lua, 3);
    }
    r *= r;
    vec3_copy(offset, e2->activation_offset);
    if(top >= 4)
    {
        offset[0] = lua_tonumber(lua, 4);
        offset[1] = lua_tonumber(lua, 5);
        offset[2] = lua_tonumber(lua, 6);
    }

    Mat4_vec3_mul_macro(pos, e2->transform, offset);
    if((vec3_dot(e1->transform+4, e2->transform+4) > 0.75) &&
       (vec3_dist_sq(e1->transform+12, pos) < r))
    {
        lua_pushboolean(lua, 1);
        return 1;
    }

    lua_pushboolean(lua, 0);
    return 1;
}


int lua_GetEntityVisibility(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, (ent->state_flags & ENTITY_STATE_VISIBLE) != 0);

    return 1;
}


int lua_SetEntityVisibility(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("expecting arguments (entity_id, value)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    if(lua_tointeger(lua, 2) != 0)
    {
        ent->state_flags |= ENTITY_STATE_VISIBLE;
    }
    else
    {
        ent->state_flags &= ~ENTITY_STATE_VISIBLE;
    }

    return 0;
}


int lua_GetEntityEnability(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushboolean(lua, (ent->state_flags & ENTITY_STATE_ENABLED) != 0);

    return 1;
}


int lua_GetEntityActivity(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushboolean(lua, (ent->state_flags & ENTITY_STATE_ACTIVE) != 0);

    return 1;
}


int lua_SetEntityActivity(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("expecting arguments (entity_id, value)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    if(lua_tointeger(lua, 2) != 0)
    {
        ent->state_flags |= ENTITY_STATE_ACTIVE;
    }
    else
    {
        ent->state_flags &= ~ENTITY_STATE_ACTIVE;
    }

    return 0;
}


int lua_GetEntityTriggerLayout(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
    if(ent == NULL) return 0;   // No entity found - return.

    lua_pushinteger(lua, (ent->trigger_layout & ENTITY_TLAYOUT_MASK));          // mask
    lua_pushinteger(lua, (ent->trigger_layout & ENTITY_TLAYOUT_EVENT) >> 5);    // event
    lua_pushinteger(lua, (ent->trigger_layout & ENTITY_TLAYOUT_LOCK) >> 6);     // lock

    return 3;
}


int lua_SetEntityTriggerLayout(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning("expecting arguments (entity_id, layout) or (entity_id, mask, event, once)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    if(top == 2)
    {
        ent->trigger_layout = (uint8_t)lua_tointeger(lua, 2);
    }
    else if(top == 4)
    {
        uint8_t trigger_layout = ent->trigger_layout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_MASK);  trigger_layout ^= (uint8_t)lua_tointeger(lua, 2);          // mask  - 00011111
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_EVENT); trigger_layout ^= ((uint8_t)lua_tointeger(lua, 3)) << 5;   // event - 00100000
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_LOCK);  trigger_layout ^= ((uint8_t)lua_tointeger(lua, 4)) << 6;   // lock  - 01000000
        ent->trigger_layout = trigger_layout;
    }

    return 0;
}


int lua_SetEntityLock(lua_State * lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No arguments provided - return.

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
    if(ent)
    {
        uint8_t trigger_layout = ent->trigger_layout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_LOCK);  trigger_layout ^= ((uint8_t)lua_tointeger(lua, 2)) << 6;   // lock  - 01000000
        ent->trigger_layout = trigger_layout;
    }
    return 0;
}


int lua_GetEntityLock(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No argument provided - return.

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
    if(ent)
    {
        lua_pushinteger(lua, ((ent->trigger_layout & ENTITY_TLAYOUT_LOCK) >> 6));      // lock
        return 1;
    }
    return 0;
}


int lua_SetEntityEvent(lua_State * lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No arguments provided - return.

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
    if(ent)
    {
        uint8_t trigger_layout = ent->trigger_layout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_EVENT);
        trigger_layout ^= ((uint8_t)lua_tointeger(lua, 2)) << 5;   // event - 00100000
        ent->trigger_layout = trigger_layout;
    }
    return 0;
}


int lua_GetEntityEvent(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No argument provided - return.

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
    if(ent)
    {
        lua_pushinteger(lua, ((ent->trigger_layout & ENTITY_TLAYOUT_EVENT) >> 5));    // event
        return 1;
    }
    return 0;
}


int lua_GetEntityMask(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No argument provided - return.

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
    if(ent)
    {
        lua_pushinteger(lua, (ent->trigger_layout & ENTITY_TLAYOUT_MASK));      // mask
        return 1;
    }
    return 0;
}


int lua_SetEntityMask(lua_State * lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No arguments provided - return.

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
    if(ent)
    {
        uint8_t trigger_layout = ent->trigger_layout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_MASK);  trigger_layout ^= (uint8_t)lua_tointeger(lua, 2);   // mask  - 00011111
        ent->trigger_layout = trigger_layout;
    }
    return 0;
}


int lua_GetEntitySectorStatus(lua_State *lua)
{
    if(lua_gettop(lua) < 1) return 0;

    entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
    if(ent)
    {
        lua_pushinteger(lua, ((ent->trigger_layout & ENTITY_TLAYOUT_SSTATUS) >> 7));
        return 1;
    }
    return 0;
}


int lua_SetEntitySectorStatus(lua_State *lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No arguments specified - return.

    entity_p ent = World_GetEntityByID(lua_tonumber(lua, 1));
    if(ent)
    {
        uint8_t trigger_layout = ent->trigger_layout;
        trigger_layout &= ~(uint8_t)(ENTITY_TLAYOUT_SSTATUS);
        trigger_layout ^=  ((uint8_t)lua_tointeger(lua, 2)) << 7;   // sector_status  - 10000000
        ent->trigger_layout = trigger_layout;
    }
    return 0;
}


int lua_GetEntityOCB(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No argument provided - return.

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
    if(ent == NULL) return 0;   // No entity found - return.

    lua_pushinteger(lua, ent->OCB);
    return 1;
}


int lua_SetEntityOCB(lua_State * lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No arguments provided - return.

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
    if(ent == NULL) return 0;   // No entity found - return.

    ent->OCB = lua_tointeger(lua, 2);
    return 0;
}


int lua_GetEntityFlags(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, ent->state_flags);
    lua_pushinteger(lua, ent->type_flags);
    lua_pushinteger(lua, ent->callback_flags);

    return 3;
}


int lua_SetEntityFlags(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning("expecting arguments (entity_id, state_flags, type_flags, (callback_flags))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    if(!lua_isnil(lua, 2))
    {
        ent->state_flags = lua_tointeger(lua, 2);
    }
    if(!lua_isnil(lua, 3))
    {
        ent->type_flags = lua_tointeger(lua, 3);
    }
    if(!lua_isnil(lua, 4))
    {
        ent->callback_flags = lua_tointeger(lua, 4);
    }

    return 0;
}


int lua_GetEntityTypeFlag(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_Warning("expecting arguments (entity_id, (type_flag))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    if(top == 1)
    {
        lua_pushinteger(lua, ent->type_flags);
    }
    else
    {
        lua_pushinteger(lua, (ent->type_flags & (uint16_t)(lua_tointeger(lua, 2))));
    }

    return 1;
}


int lua_SetEntityTypeFlag(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning("expecting arguments (entity_id, type_flag, (value))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    if(top == 2)
    {
        ent->type_flags ^= (uint16_t)lua_tointeger(lua, 2);
    }
    else
    {
        if(lua_tointeger(lua, 3) == 1)
        {
            ent->type_flags |=  (uint16_t)lua_tointeger(lua, 2);
        }
        else
        {
            ent->type_flags &= ~(uint16_t)lua_tointeger(lua, 2);
        }
    }

    return 0;
}


int lua_GetEntityStateFlag(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_Warning("expecting arguments (entity_id, (state_flag))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    if(top == 1)
    {
        lua_pushinteger(lua, ent->state_flags);
    }
    else
    {
        lua_pushinteger(lua, (ent->state_flags & (uint16_t)(lua_tointeger(lua, 2))));
    }

    return 1;
}


int lua_SetEntityStateFlag(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning("expecting arguments (entity_id, state_flag, (value))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    if(top == 2)
    {
        ent->state_flags ^= (uint16_t)lua_tointeger(lua, 2);
    }
    else
    {
        if(lua_tointeger(lua, 3) == 1)
        {
            ent->state_flags |=  (uint16_t)lua_tointeger(lua, 2);
        }
        else
        {
            ent->state_flags &= ~(uint16_t)lua_tointeger(lua, 2);
        }
    }

    return 0;
}


int lua_GetEntityCallbackFlag(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_Warning("expecting arguments (entity_id, (callback_flag))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    if(top == 1)
    {
        lua_pushinteger(lua, ent->callback_flags);
    }
    else
    {
        lua_pushinteger(lua, (ent->callback_flags & (uint32_t)(lua_tointeger(lua, 2))));
    }

    return 1;
}


int lua_SetEntityCallbackFlag(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Warning("expecting arguments (entity_id, callback_flag, (value))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    if(top == 2)
    {
        ent->callback_flags ^= (uint32_t)lua_tointeger(lua, 2);
    }
    else
    {
        if(lua_tointeger(lua, 3) == 1)
        {
            ent->callback_flags |=  (uint16_t)lua_tointeger(lua, 2);
        }
        else
        {
            ent->callback_flags &= ~(uint32_t)lua_tointeger(lua, 2);
        }
    }

    return 0;
}


int lua_GetEntityTimer(lua_State * lua)
{
    if(lua_gettop(lua) < 1) return 0;   // No arguments provided - return.

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
    if(ent == NULL)
    {
        return 0;   // No entity found - return.
    }

    lua_pushnumber(lua, ent->timer);
    return 1;
}


int lua_SetEntityTimer(lua_State * lua)
{
    if(lua_gettop(lua) < 2) return 0;   // No arguments provided - return.

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));
    if(ent == NULL)
    {
        return 0;   // No entity found - return.
    }

    ent->timer = lua_tonumber(lua, 2);
    return 0;
}


int lua_GetEntityMoveType(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, ent->move_type);

    return 1;
}


int lua_SetEntityMoveType(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("expecting arguments (entity_id, move_type)");
        return 0;
    }

    entity_p ent = World_GetEntityByID(lua_tointeger(lua, 1));

    if(ent == NULL)
    {
        return 0;
    }
    ent->move_type = lua_tointeger(lua, 2);

    return 0;
}


int lua_GetEntityResponse(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("expecting arguments (entity_id, response_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent && ent->character)
    {
        switch(lua_tointeger(lua, 2))
        {
            case 0:
                lua_pushinteger(lua, ent->character->resp.kill);
                break;
            case 1:
                lua_pushinteger(lua, ent->character->resp.vertical_collide);
                break;
            case 2:
                lua_pushinteger(lua, ent->character->resp.horizontal_collide);
                break;
            case 3:
                lua_pushinteger(lua, ent->character->resp.slide);
                break;
            default:
                lua_pushinteger(lua, 0);
                break;
        }
        return 1;
    }
    else
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }
}


int lua_SetEntityResponse(lua_State * lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Warning("expecting arguments (entity_id, response_id, value)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent && ent->character)
    {
        int8_t value = (int8_t)lua_tointeger(lua, 3);

        switch(lua_tointeger(lua, 2))
        {
            case RESP_KILL:
                ent->character->resp.kill = value;
                break;

            case RESP_VERT_COLLIDE:
                ent->character->resp.vertical_collide = value;
                break;

            case RESP_HOR_COLLIDE:
                ent->character->resp.horizontal_collide = value;
                break;

            case RESP_SLIDE:
                ent->character->resp.slide = value;
                break;

            default:
                break;
        }
    }
    else
    {
        Con_Warning("no entity with id = %d", id);
    }

    return 0;
}


int lua_GetEntityState(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, ent->bf->animations.last_state);

    return 1;
}


int lua_GetEntityModel(lua_State * lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, ent->bf->animations.model->id);

    return 1;
}


int lua_SetEntityState(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("expecting arguments (entity_id, value)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    ent->bf->animations.next_state = lua_tointeger(lua, 2);
    if(!lua_isnil(lua, 3))
    {
        ent->bf->animations.last_state = lua_tointeger(lua, 3);
    }

    return 0;
}


int lua_SetEntityRoomMove(lua_State * lua)
{
    if(lua_gettop(lua) < 4)
    {
        Con_Warning("expecting arguments (entity_id, room_id, move_type, dir_flag)");
        return 0;
    }

    uint32_t id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);
    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    room_p room = NULL;
    if(!lua_isnil(lua, 2) && (room = World_GetRoomByID(lua_tointeger(lua, 2))))
    {
        if(ent == World_GetPlayer())
        {
            ent->self->room = room;
        }
        else if(ent->self->room != room)
        {
            if(ent->self->room != NULL)
            {
                Room_RemoveObject(ent->self->room, ent->self);
            }
            Room_AddObject(room, ent->self);
        }
    }
    Entity_UpdateRoomPos(ent);

    if(!lua_isnil(lua, 3))
    {
        ent->move_type = lua_tointeger(lua, 3);
    }
    if(!lua_isnil(lua, 4))
    {
        ent->dir_flag = lua_tointeger(lua, 4);
    }

    return 0;
}


int lua_GetEntityMeshCount(lua_State *lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent == NULL)
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }

    lua_pushinteger(lua, ent->bf->bone_tag_count);
    return 1;
}


int lua_SetEntityMeshswap(lua_State * lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Warning("expecting arguments (id_dest, id_src)");
        return 0;
    }

    int id_dest = lua_tointeger(lua, 1);
    int id_src = lua_tointeger(lua, 2);

    entity_p         ent_dest;
    skeletal_model_p model_src;

    ent_dest   = World_GetEntityByID(id_dest);
    model_src  = World_GetModelByID(id_src);

    int meshes_to_copy = (ent_dest->bf->bone_tag_count > model_src->mesh_count) ? (model_src->mesh_count) : (ent_dest->bf->bone_tag_count);

    for(int i = 0; i < meshes_to_copy; i++)
    {
        ent_dest->bf->bone_tags[i].mesh_base = model_src->mesh_tree[i].mesh_base;
        ent_dest->bf->bone_tags[i].mesh_skin = model_src->mesh_tree[i].mesh_skin;
    }

    return 0;
}


int lua_SetModelMeshReplaceFlag(lua_State *lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Printf("Wrong arguments count. Must be (id_model, bone_num, flag)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p sm = World_GetModelByID(id);
    if(sm != NULL)
    {
        int bone = lua_tointeger(lua, 2);
        if((bone >= 0) && (bone < sm->mesh_count))
        {
            sm->mesh_tree[bone].replace_mesh = lua_tointeger(lua, 3);
        }
        else
        {
            Con_Printf("wrong bone number = %d", bone);
        }
    }
    else
    {
        Con_Printf("can not find model with id = %d", id);
    }

    return 0;
}


int lua_SetModelAnimReplaceFlag(lua_State *lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Printf("Wrong arguments count. Must be (id_model, bone_num, flag)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p sm = World_GetModelByID(id);
    if(sm != NULL)
    {
        int bone = lua_tointeger(lua, 2);
        if((bone >= 0) && (bone < sm->mesh_count))
        {
            sm->mesh_tree[bone].replace_anim = lua_tointeger(lua, 3);
        }
        else
        {
            Con_Printf("wrong bone number = %d", bone);
        }
    }
    else
    {
        Con_Printf("can not find model with id = %d", id);
    }

    return 0;
}


int lua_CopyMeshFromModelToModel(lua_State *lua)
{
    if(lua_gettop(lua) < 4)
    {
        Con_Printf("Wrong arguments count. Must be (id_model1, id_model2, bone_num1, bone_num2)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p sm1 = World_GetModelByID(id);
    if(sm1 == NULL)
    {
        Con_Printf("can not find model with id = %d", id);
        return 0;
    }

    id = lua_tointeger(lua, 2);
    skeletal_model_p sm2 = World_GetModelByID(id);
    if(sm2 == NULL)
    {
        Con_Printf("can not find model with id = %d", id);
        return 0;
    }

    int bone1 = lua_tointeger(lua, 3);
    int bone2 = lua_tointeger(lua, 4);

    if((bone1 >= 0) && (bone1 < sm1->mesh_count) && (bone2 >= 0) && (bone2 < sm2->mesh_count))
    {
        sm1->mesh_tree[bone1].mesh_base = sm2->mesh_tree[bone2].mesh_base;
    }
    else
    {
        Con_AddLine("wrong bone number = %d", FONTSTYLE_GENERIC);
    }

    return 0;
}


int lua_PushEntityBody(lua_State *lua)
{
    if(lua_gettop(lua) != 5)
    {
        Con_Printf("Wrong arguments count. Must be (entity_id, body_number, h_force, v_force, reset_flag)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);
    int body_number = lua_tointeger(lua, 2);

    if(ent && (body_number < ent->bf->bone_tag_count) && (ent->type_flags & ENTITY_TYPE_DYNAMIC))
    {
        float h_force = lua_tonumber(lua, 3);
        float t       = ent->angles[0] * M_PI / 180.0;
        float speed[3];

        speed[0] = -sinf(t) * h_force;
        speed[1] =  cosf(t) * h_force;
        speed[2] =  lua_tonumber(lua, 4);

        /*if(lua_toboolean(lua, 5))
            ent->physics->bt_body[body_number]->clearForces();*/
        Physics_PushBody(ent->physics, speed, body_number);
        //ent->physics->bt_body[body_number]->setLinearVelocity(angle);
        //ent->physics->bt_body[body_number]->setAngularVelocity(angle / 1024.0);
    }
    else
    {
        Con_Printf("Can't apply force to entity %d - no entity, body, or entity is not kinematic!", id);
    }

    return 0;
}


int lua_SetEntityBodyMass(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(lua_gettop(lua) < 3)
    {
        Con_Printf("Wrong arguments count. Must be (entity_id, body_number, (mass / each body mass))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    int body_number = lua_tointeger(lua, 2);
    body_number = (body_number < 1) ? (1) : (body_number);

    if(ent && (ent->bf->bone_tag_count >= body_number))
    {
        uint16_t argn  = 3;
        bool dynamic = false;
        float mass = 0.0;

        for(int i = 0; i < body_number; i++)
        {
            if(top >= argn) mass = lua_tonumber(lua, argn++);
            if(mass > 0.0) dynamic = true;
            Physics_SetBodyMass(ent->physics, mass, i);
        }
        Entity_UpdateRigidBody(ent, 1);

        if(dynamic)
        {
            ent->type_flags |=  ENTITY_TYPE_DYNAMIC;
        }
        else
        {
            ent->type_flags &= ~ENTITY_TYPE_DYNAMIC;
        }
    }
    else
    {
        Con_Printf("Can't find entity %d or body number is more than %d", id, body_number);
    }

    return 0;
}


int lua_LockEntityBodyLinearFactor(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 2)
    {
        Con_Printf("Wrong arguments count. Must be [entity_id, body_number, (vertical_factor)]");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);
    int body_number = lua_tointeger(lua, 2);

    if(ent && (body_number < ent->bf->bone_tag_count) && (ent->type_flags & ENTITY_TYPE_DYNAMIC))
    {
        float factor[3], t    = ent->angles[0] * M_PI / 180.0;
        factor[0] = fabs(sinf(t));
        factor[1] = fabs(cosf(t));
        factor[2] = 1.0;

        if(top >= 3)
        {
            factor[2] = fabs(lua_tonumber(lua, 3));
            factor[2] = (factor[2] > 1.0) ? (1.0) : (factor[2]);
        }
        Physics_SetLinearFactor(ent->physics, factor, body_number);
    }
    else
    {
        Con_Printf("Can't apply force to entity %d - no entity, body, or entity is not dynamic!", id);
    }

    return 0;
}


int lua_SetCharacterWeaponModel(lua_State *lua)
{
    if(lua_gettop(lua) < 3)
    {
        Con_Printf("Wrong arguments count. Must be (id_entity, id_weapon_model, armed_state)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent && ent->character)
    {
        Character_SetWeaponModel(ent, lua_tointeger(lua, 2), lua_tointeger(lua, 3));
    }
    else
    {
        Con_Printf("can not find entity with id = %d", id);
    }

    return 0;
}


int lua_GetCharacterCurrentWeapon(lua_State *lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (entity_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent && ent->character)
    {
        lua_pushinteger(lua, ent->character->current_weapon);
        return 1;
    }
    else
    {
        Con_Warning("no entity with id = %d", id);
        return 0;
    }
}


int lua_SetCharacterCurrentWeapon(lua_State *lua)
{
    if(lua_gettop(lua) < 2)
    {
        Con_Printf("Wrong arguments count. Must be (id_entity, id_weapon)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    entity_p ent = World_GetEntityByID(id);

    if(ent && ent->character)
    {
        ent->character->current_weapon = lua_tointeger(lua, 2);
    }
    else
    {
        Con_Printf("can not find entity with id = %d", id);
    }

    return 0;
}

/*
 * Camera functions
 */
int lua_CamShake(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        float power = lua_tonumber(lua, 1);
        float time  = lua_tonumber(lua, 2);
        //Cam_Shake(&engine_camera, power, time);
    }

    return 0;
}


int lua_PlayFlyby(lua_State *lua)
{
    if(lua_gettop(lua) >= 2)
    {
        uint32_t flyby_id   = lua_tointeger(lua, 1);
        int      once       = lua_tointeger(lua, 2);
        Game_PlayFlyBy(flyby_id, once);
    }

    return 0;
}


int lua_FlashSetup(lua_State *lua)
{
    /*if(lua_gettop(lua) != 6) return 0;

    Gui_FadeSetup(FADER_EFFECT,
                  (uint8_t)(lua_tointeger(lua, 1)),
                  (uint8_t)(lua_tointeger(lua, 2)), (uint8_t)(lua_tointeger(lua, 3)), (uint8_t)(lua_tointeger(lua, 4)),
                  BM_MULTIPLY,
                  (uint16_t)(lua_tointeger(lua, 5)), (uint16_t)(lua_tointeger(lua, 6)));*/
    return 0;
}


int lua_FlashStart(lua_State *lua)
{
    //Gui_FadeStart(FADER_EFFECT, GUI_FADER_DIR_TIMED);
    return 0;
}

/*
 * General gameplay functions
 */
int lua_PlayStream(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_Warning("expecting arguments (stream_id, (mask))");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    uint8_t mask = 0;
    if(top >= 2) mask = lua_tointeger(lua, 2);

    if(id < 0)
    {
        Con_Warning("wrong stream id");
        return 0;
    }

    if(mask)
    {
        Audio_StreamPlay(id, mask);
    }
    else
    {
        Audio_StreamPlay(id);
    }

    return 0;
}


int lua_PlaySound(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_Warning("expecting arguments (sound_id, (entity_id))");
        return 0;
    }

    uint32_t id  = lua_tointeger(lua, 1);           // uint_t can't been less zero, reduce number of comparations

    int ent_id = -1;
    if((top >= 2) && World_GetEntityByID(ent_id = lua_tointeger(lua, 2)) == NULL)
    {
        ent_id = -1;
    }

    int result;

    if(ent_id >= 0)
    {
        result = Audio_Send(id, TR_AUDIO_EMITTER_ENTITY, ent_id);
    }
    else
    {
        result = Audio_Send(id, TR_AUDIO_EMITTER_GLOBAL);
    }

    if(result < 0)
    {
        switch(result)
        {
            case TR_AUDIO_SEND_NOCHANNEL:
                Con_Warning("send ignored: no free channels");
                break;

            case TR_AUDIO_SEND_NOSAMPLE:
                Con_Warning("send ignored: no sample");
                break;
        }
    }

    return 0;
}


int lua_StopSound(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top < 1)
    {
        Con_Warning("expecting arguments (sound_id, (entity_id))");
        return 0;
    }

    uint32_t id  = lua_tointeger(lua, 1);
    int ent_id = -1;
    if((top >= 2) && World_GetEntityByID(ent_id = lua_tointeger(lua, 2)) == NULL)
    {
        ent_id = -1;
    }

    int result;

    if(ent_id == -1)
    {
        result = Audio_Kill(id, TR_AUDIO_EMITTER_GLOBAL);
    }
    else
    {
        result = Audio_Kill(id, TR_AUDIO_EMITTER_ENTITY, ent_id);
    }

    if(result < 0) Con_Warning("audio with id = %d not played", id);

    return 0;
}


int lua_GetLevel(lua_State *lua)
{
    lua_pushinteger(lua, gameflow_manager.CurrentLevelID);
    return 1;
}


int lua_SetLevel(lua_State *lua)
{
    if(lua_gettop(lua) != 1)
    {
        Con_Warning("expecting arguments (level_id)");
        return 0;
    }

    int id  = lua_tointeger(lua, 1);
    Con_Notify("level was changed to %d", id);

    Game_LevelTransition(id);
    Gameflow_Send(TR_GAMEFLOW_OP_LEVELCOMPLETE, id);    // Next level

    return 0;
}


int lua_SetGame(lua_State *lua)
{
    int top = lua_gettop(lua);
    if(top < 1)
    {
        Con_Warning("expecting arguments (gameversion, (level_id))");
        return 0;
    }

    gameflow_manager.CurrentGameID = lua_tointeger(lua, 1);
    if(!lua_isnil(lua, 2))
    {
        gameflow_manager.CurrentLevelID = lua_tointeger(lua, 2);
    }

    lua_getglobal(lua, "getTitleScreen");
    if(lua_isfunction(lua, -1))
    {
        lua_pushnumber(lua, gameflow_manager.CurrentGameID);
        if (lua_CallAndLog(lua, 1, 1, 0))
        {
            //Gui_FadeAssignPic(FADER_LOADSCREEN, lua_tostring(lua, -1));
            lua_pop(lua, 1);
        }
    }
    lua_settop(lua, top);

    Con_Notify("level was changed to %d", gameflow_manager.CurrentGameID);
    Game_LevelTransition(gameflow_manager.CurrentLevelID);
    Gameflow_Send(TR_GAMEFLOW_OP_LEVELCOMPLETE, gameflow_manager.CurrentLevelID);

    return 0;
}


int lua_LoadMap(lua_State *lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (map_name, (game_id, map_id))");
        return 0;
    }

    if(lua_isstring(lua, 1))
    {
        const char *s = lua_tostring(lua, 1);
        if((s != NULL) && (s[0] != 0))
        {
            if(!lua_isnil(lua, 2))
            {
                gameflow_manager.CurrentGameID = lua_tointeger(lua, 2);
            }
            if(!lua_isnil(lua, 3))
            {
                gameflow_manager.CurrentLevelID = lua_tointeger(lua, 3);
            }
            char file_path[MAX_ENGINE_PATH];
            Script_GetLoadingScreen(lua, gameflow_manager.CurrentLevelID, file_path);
            if(!Gui_LoadScreenAssignPic(file_path))
            {
                Gui_LoadScreenAssignPic("resource/graphics/legal.png");
            }
            Engine_LoadMap(s);
        }
    }

    return 0;
}


/*
 * Flipped (alternate) room functions
 */
int lua_SetFlipState(lua_State *lua)
{
    if(lua_gettop(lua) != 2)
    {
        Con_Warning("expecting arguments (flip_index, flip_state)");
        return 0;
    }

    uint32_t flip_index = (uint32_t)lua_tointeger(lua, 1);
    uint32_t flip_state = (uint32_t)lua_tointeger(lua, 2);
    return World_SetFlipState(flip_index, flip_state);
}


int lua_SetFlipMap(lua_State *lua)
{
    if(lua_gettop(lua) != 3)
    {
        Con_Warning("expecting arguments (flip_index, flip_mask, flip_operation)");
        return 0;
    }

    uint32_t flip_index = (uint32_t)lua_tointeger(lua, 1);
    uint8_t  flip_mask = (uint32_t)lua_tointeger(lua, 2);
    uint8_t  flip_operation = (uint32_t)lua_tointeger(lua, 3);

    return World_SetFlipMap(flip_index, flip_mask, flip_operation);
}


int lua_GetFlipMap(lua_State *lua)
{
    if(lua_gettop(lua) == 1)
    {
        uint32_t flip_index = (uint32_t)lua_tointeger(lua, 1);
        uint32_t flip_map = World_GetFlipMap(flip_index);

        if(flip_map == 0xFFFFFFFF)
        {
            Con_Warning("wrong flipmap index");
            return 0;
        }

        lua_pushinteger(lua, flip_map);
        return 1;
    }
    else
    {
        Con_Warning("expecting arguments (flip_index)");
        return 0;
    }
}


int lua_GetFlipState(lua_State *lua)
{
    if(lua_gettop(lua) == 1)
    {
        uint32_t flip_index = (uint32_t)lua_tointeger(lua, 1);
        uint32_t flip_state = World_GetFlipState(flip_index);

        if(flip_state == 0xFFFFFFFF)
        {
            Con_Warning("wrong flipmap index");
            return 0;
        }

        lua_pushinteger(lua, flip_state);
        return 1;
    }
    else
    {
        Con_Warning("expecting arguments (flip_index)");
        return 0;
    }
}

/*
 * Generate UV rotate animations
 */
int lua_genUVRotateAnimation(lua_State *lua)
{
    if(lua_gettop(lua) < 1)
    {
        Con_Warning("expecting arguments (model_id)");
        return 0;
    }

    int id = lua_tointeger(lua, 1);
    skeletal_model_p model = World_GetModelByID(id);

    if(model)
    {
        polygon_p p = model->mesh_tree->mesh_base->transparency_polygons;
        if((p) && (p->anim_id == 0))
        {
            anim_seq_t seq;
            anim_seq_p tmp_seq;
            uint32_t seqs_conut;
            World_GetAnimSeqInfo(&tmp_seq, &seqs_conut);

            // Fill up new sequence with frame list.
            seq.anim_type         = TR_ANIMTEXTURE_FORWARD;
            seq.frame_lock        = false;         // by default anim is playing
            seq.uvrotate          = true;
            seq.reverse_direction = false;         // Needed for proper reverse-type start-up.
            seq.frame_rate        = 0.025 * 16;    // Should be passed as 1 / FPS.
            seq.frame_time        = 0.0;           // Reset frame time to initial state.
            seq.current_frame     = 0;             // Reset current frame to zero.
            seq.frames_count      = 1;
            seq.frame_list        = (uint32_t*)calloc(seq.frames_count, sizeof(uint32_t));
            seq.frame_list[0]     = 0;
            seq.frames            = (tex_frame_p)calloc(seq.frames_count, sizeof(tex_frame_t));

            float v_min, v_max;
            v_min = v_max = p->vertices->tex_coord[1];
            for(uint16_t j = 1; j < p->vertex_count; j++)
            {
                if(p->vertices[j].tex_coord[1] > v_max)
                {
                    v_max = p->vertices[j].tex_coord[1];
                }
                if(p->vertices[j].tex_coord[1] < v_min)
                {
                    v_min = p->vertices[j].tex_coord[1];
                }
            }

            seq.frames->uvrotate_max = 0.5 * (v_max - v_min);
            seq.frames->current_uvrotate = 0.0f;
            seq.frames->texture_index = p->texture_index;
            seq.frames->mat[0] = 1.0;
            seq.frames->mat[1] = 0.0;
            seq.frames->mat[2] = 0.0;
            seq.frames->mat[3] = 1.0;
            seq.frames->move[0] = 0.0;
            seq.frames->move[1] = 0.0;

            for(; p; p = p->next)
            {
                p->anim_id = seqs_conut + 1;
                for(uint16_t j = 0; j < p->vertex_count; j++)
                {
                    p->vertices[j].tex_coord[1] = v_min + 0.5 * (p->vertices[j].tex_coord[1] - v_min) + seq.frames->uvrotate_max;
                }
            }

            if(!World_AddAnimSeq(&seq))
            {
                if(seq.frames_count)
                {
                    free(seq.frame_list);
                    seq.frame_list = NULL;
                    free(seq.frames);
                    seq.frames = NULL;
                }
                seq.frames_count = 0;
            }
        }
    }

    return 0;
}


// Called when something goes absolutely horribly wrong in Lua, and tries
// to produce some debug output. Lua calls abort afterwards, so sending
// the output to the internal console is not an option.
static int lua_LuaPanic(lua_State *lua)
{
    if (lua_gettop(lua) < 1)
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

        lua_pushinteger(lua, TR_I);
        lua_setglobal(lua, "TR_I");
        lua_pushinteger(lua, TR_I_DEMO);
        lua_setglobal(lua, "TR_I_DEMO");
        lua_pushinteger(lua, TR_I_UB);
        lua_setglobal(lua, "TR_I_UB");
        lua_pushinteger(lua, TR_II);
        lua_setglobal(lua, "TR_II");
        lua_pushinteger(lua, TR_II_DEMO);
        lua_setglobal(lua, "TR_II_DEMO");
        lua_pushinteger(lua, TR_III);
        lua_setglobal(lua, "TR_III");
        lua_pushinteger(lua, TR_IV);
        lua_setglobal(lua, "TR_IV");
        lua_pushinteger(lua, TR_IV_DEMO);
        lua_setglobal(lua, "TR_IV_DEMO");
        lua_pushinteger(lua, TR_V);
        lua_setglobal(lua, "TR_V");
        lua_pushinteger(lua, TR_UNKNOWN);
        lua_setglobal(lua, "TR_UNKNOWN");

        lua_pushinteger(lua, GAME_1);
        lua_setglobal(lua, "GAME_1");
        lua_pushinteger(lua, GAME_1_1);
        lua_setglobal(lua, "GAME_1_1");
        lua_pushinteger(lua, GAME_1_5);
        lua_setglobal(lua, "GAME_1_5");
        lua_pushinteger(lua, GAME_2);
        lua_setglobal(lua, "GAME_2");
        lua_pushinteger(lua, GAME_2_1);
        lua_setglobal(lua, "GAME_2_1");
        lua_pushinteger(lua, GAME_2_5);
        lua_setglobal(lua, "GAME_2_5");
        lua_pushinteger(lua, GAME_3);
        lua_setglobal(lua, "GAME_3");
        lua_pushinteger(lua, GAME_3_5);
        lua_setglobal(lua, "GAME_3_5");
        lua_pushinteger(lua, GAME_4);
        lua_setglobal(lua, "GAME_4");
        lua_pushinteger(lua, GAME_4_1);
        lua_setglobal(lua, "GAME_4_1");
        lua_pushinteger(lua, GAME_5);
        lua_setglobal(lua, "GAME_5");

        lua_pushinteger(lua, ENTITY_STATE_ENABLED);
        lua_setglobal(lua, "ENTITY_STATE_ENABLED");
        lua_pushinteger(lua, ENTITY_STATE_ACTIVE);
        lua_setglobal(lua, "ENTITY_STATE_ACTIVE");
        lua_pushinteger(lua, ENTITY_STATE_VISIBLE);
        lua_setglobal(lua, "ENTITY_STATE_VISIBLE");

        lua_pushinteger(lua, ENTITY_TYPE_GENERIC);
        lua_setglobal(lua, "ENTITY_TYPE_GENERIC");
        lua_pushinteger(lua, ENTITY_TYPE_INTERACTIVE);
        lua_setglobal(lua, "ENTITY_TYPE_INTERACTIVE");
        lua_pushinteger(lua, ENTITY_TYPE_TRIGGER_ACTIVATOR);
        lua_setglobal(lua, "ENTITY_TYPE_TRIGGER_ACTIVATOR");
        lua_pushinteger(lua, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR);
        lua_setglobal(lua, "ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR");
        lua_pushinteger(lua, ENTITY_TYPE_PICKABLE);
        lua_setglobal(lua, "ENTITY_TYPE_PICKABLE");
        lua_pushinteger(lua, ENTITY_TYPE_TRAVERSE);
        lua_setglobal(lua, "ENTITY_TYPE_TRAVERSE");
        lua_pushinteger(lua, ENTITY_TYPE_TRAVERSE_FLOOR);
        lua_setglobal(lua, "ENTITY_TYPE_TRAVERSE_FLOOR");
        lua_pushinteger(lua, ENTITY_TYPE_DYNAMIC);
        lua_setglobal(lua, "ENTITY_TYPE_DYNAMIC");
        lua_pushinteger(lua, ENTITY_TYPE_ACTOR);
        lua_setglobal(lua, "ENTITY_TYPE_ACTOR");

        lua_pushinteger(lua, ENTITY_CALLBACK_NONE);
        lua_setglobal(lua, "ENTITY_CALLBACK_NONE");
        lua_pushinteger(lua, ENTITY_CALLBACK_ACTIVATE);
        lua_setglobal(lua, "ENTITY_CALLBACK_ACTIVATE");
        lua_pushinteger(lua, ENTITY_CALLBACK_DEACTIVATE);
        lua_setglobal(lua, "ENTITY_CALLBACK_DEACTIVATE");
        lua_pushinteger(lua, ENTITY_CALLBACK_COLLISION);
        lua_setglobal(lua, "ENTITY_CALLBACK_COLLISION");
        lua_pushinteger(lua, ENTITY_CALLBACK_STAND);
        lua_setglobal(lua, "ENTITY_CALLBACK_STAND");
        lua_pushinteger(lua, ENTITY_CALLBACK_HIT);
        lua_setglobal(lua, "ENTITY_CALLBACK_HIT");


        lua_pushinteger(lua, ANIM_NORMAL_CONTROL);
        lua_setglobal(lua, "ANIM_NORMAL_CONTROL");
        lua_pushinteger(lua, ANIM_LOOP_LAST_FRAME);
        lua_setglobal(lua, "ANIM_LOOP_LAST_FRAME");
        lua_pushinteger(lua, ANIM_FRAME_LOCK);
        lua_setglobal(lua, "ANIM_FRAME_LOCK");

        lua_pushinteger(lua, MOVE_STATIC_POS);
        lua_setglobal(lua, "MOVE_STATIC_POS");
        lua_pushinteger(lua, MOVE_KINEMATIC);
        lua_setglobal(lua, "MOVE_KINEMATIC");
        lua_pushinteger(lua, MOVE_ON_FLOOR);
        lua_setglobal(lua, "MOVE_ON_FLOOR");
        lua_pushinteger(lua, MOVE_WADE);
        lua_setglobal(lua, "MOVE_WADE");
        lua_pushinteger(lua, MOVE_QUICKSAND);
        lua_setglobal(lua, "MOVE_QUICKSAND");
        lua_pushinteger(lua, MOVE_ON_WATER);
        lua_setglobal(lua, "MOVE_ON_WATER");
        lua_pushinteger(lua, MOVE_UNDERWATER);
        lua_setglobal(lua, "MOVE_UNDERWATER");
        lua_pushinteger(lua, MOVE_FREE_FALLING);
        lua_setglobal(lua, "MOVE_FREE_FALLING");
        lua_pushinteger(lua, MOVE_CLIMBING);
        lua_setglobal(lua, "MOVE_CLIMBING");
        lua_pushinteger(lua, MOVE_MONKEYSWING);
        lua_setglobal(lua, "MOVE_MONKEYSWING");
        lua_pushinteger(lua, MOVE_WALLS_CLIMB);
        lua_setglobal(lua, "MOVE_WALLS_CLIMB");
        lua_pushinteger(lua, MOVE_DOZY);
        lua_setglobal(lua, "MOVE_DOZY");

        lua_pushinteger(lua, TRIGGER_OP_OR);
        lua_setglobal(lua, "TRIGGER_OP_OR");
        lua_pushinteger(lua, TRIGGER_OP_XOR);
        lua_setglobal(lua, "TRIGGER_OP_XOR");

        lua_pushinteger(lua, RESP_KILL);
        lua_setglobal(lua, "RESP_KILL");
        lua_pushinteger(lua, RESP_VERT_COLLIDE);
        lua_setglobal(lua, "RESP_VERT_COLLIDE");
        lua_pushinteger(lua, RESP_HOR_COLLIDE);
        lua_setglobal(lua, "RESP_HOR_COLLIDE");
        lua_pushinteger(lua, RESP_SLIDE);
        lua_setglobal(lua, "RESP_SLIDE");

        lua_pushinteger(lua, TICK_IDLE);
        lua_setglobal(lua, "TICK_IDLE");
        lua_pushinteger(lua, TICK_STOPPED);
        lua_setglobal(lua, "TICK_STOPPED");
        lua_pushinteger(lua, TICK_ACTIVE);
        lua_setglobal(lua, "TICK_ACTIVE");

        lua_pushinteger(lua, COLLISION_SHAPE_BOX);
        lua_setglobal(lua, "COLLISION_SHAPE_BOX");
        lua_pushinteger(lua, COLLISION_SHAPE_BOX_BASE);
        lua_setglobal(lua, "COLLISION_SHAPE_BOX_BASE");
        lua_pushinteger(lua, COLLISION_SHAPE_SPHERE);
        lua_setglobal(lua, "COLLISION_SHAPE_SPHERE");
        lua_pushinteger(lua, COLLISION_SHAPE_TRIMESH);
        lua_setglobal(lua, "COLLISION_SHAPE_TRIMESH");
        lua_pushinteger(lua, COLLISION_SHAPE_TRIMESH_CONVEX);
        lua_setglobal(lua, "COLLISION_SHAPE_TRIMESH_CONVEX");
        lua_pushinteger(lua, COLLISION_SHAPE_SINGLE_BOX);
        lua_setglobal(lua, "COLLISION_SHAPE_SINGLE_BOX");
        lua_pushinteger(lua, COLLISION_SHAPE_SINGLE_SPHERE);
        lua_setglobal(lua, "COLLISION_SHAPE_SINGLE_SPHERE");

        lua_pushinteger(lua, COLLISION_TYPE_NONE);
        lua_setglobal(lua, "COLLISION_TYPE_NONE");
        lua_pushinteger(lua, COLLISION_TYPE_STATIC);
        lua_setglobal(lua, "COLLISION_TYPE_STATIC");
        lua_pushinteger(lua, COLLISION_TYPE_KINEMATIC);
        lua_setglobal(lua, "COLLISION_TYPE_KINEMATIC");
        lua_pushinteger(lua, COLLISION_TYPE_DYNAMIC);
        lua_setglobal(lua, "COLLISION_TYPE_DYNAMIC");
        lua_pushinteger(lua, COLLISION_TYPE_ACTOR);
        lua_setglobal(lua, "COLLISION_TYPE_ACTOR");
        lua_pushinteger(lua, COLLISION_TYPE_VEHICLE);
        lua_setglobal(lua, "COLLISION_TYPE_VEHICLE");
        lua_pushinteger(lua, COLLISION_TYPE_GHOST);
        lua_setglobal(lua, "COLLISION_TYPE_GHOST");

        lua_settop(lua, top);
    }
}


bool Script_LuaInit()
{
    engine_lua = luaL_newstate();

    if(engine_lua != NULL)
    {
        luaL_openlibs(engine_lua);
        Script_LoadConstants(engine_lua);
        Script_LuaRegisterFuncs(engine_lua);
        lua_atpanic(engine_lua, lua_LuaPanic);

        // Load script loading order (sic!)

        luaL_dofile(engine_lua, "scripts/loadscript.lua");

        return true;
    }
    else
    {
        return false;
    }
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


void Script_LuaRegisterFuncs(lua_State *lua)
{
    /*
     * register globals
     */
    char cvar_init[64]; cvar_init[0] = 0;
    strcat(cvar_init, CVAR_LUA_TABLE_NAME); strcat(cvar_init, " = {};");
    luaL_dostring(lua, cvar_init);

    Game_RegisterLuaFunctions(lua);

    // Register script functions

    lua_register(lua, "print", lua_print);
    lua_register(lua, "dumpModel", lua_DumpModel);
    lua_register(lua, "dumpRoom", lua_DumpRoom);
    lua_register(lua, "setRoomEnabled", lua_SetRoomEnabled);

    lua_register(lua, "playSound", lua_PlaySound);
    lua_register(lua, "stopSound", lua_StopSound);

    lua_register(lua, "playStream", lua_PlayStream);

    lua_register(lua, "setLevel", lua_SetLevel);
    lua_register(lua, "getLevel", lua_GetLevel);

    lua_register(lua, "setGame", lua_SetGame);
    lua_register(lua, "loadMap", lua_LoadMap);

    lua_register(lua, "camShake", lua_CamShake);
    lua_register(lua, "playFlyby", lua_PlayFlyby);

    lua_register(lua, "flashSetup", lua_FlashSetup);
    lua_register(lua, "flashStart", lua_FlashStart);

    lua_register(lua, "getLevelVersion", lua_GetLevelVersion);

    lua_register(lua, "setFlipMap", lua_SetFlipMap);
    lua_register(lua, "getFlipMap", lua_GetFlipMap);
    lua_register(lua, "setFlipState", lua_SetFlipState);
    lua_register(lua, "getFlipState", lua_GetFlipState);

    lua_register(lua, "setModelCollisionMap", lua_SetModelCollisionMap);
    lua_register(lua, "getAnimCommandTransform", lua_GetAnimCommandTransform);
    lua_register(lua, "setAnimCommandTransform", lua_SetAnimCommandTransform);
    lua_register(lua, "setStateChangeRange", lua_SetStateChangeRange);

    lua_register(lua, "addItem", lua_AddItem);
    lua_register(lua, "removeItem", lua_RemoveItem);
    lua_register(lua, "removeAllItems", lua_RemoveAllItems);
    lua_register(lua, "getItemsCount", lua_GetItemsCount);
    lua_register(lua, "createBaseItem", lua_CreateBaseItem);
    lua_register(lua, "deleteBaseItem", lua_DeleteBaseItem);
    lua_register(lua, "printItems", lua_PrintItems);

    lua_register(lua, "canTriggerEntity", lua_CanTriggerEntity);
    lua_register(lua, "spawnEntity", lua_SpawnEntity);
    lua_register(lua, "enableEntity", lua_EnableEntity);
    lua_register(lua, "disableEntity", lua_DisableEntity);

    lua_register(lua, "sameRoom", lua_SameRoom);
    lua_register(lua, "newSector", lua_NewSector);
    lua_register(lua, "similarSector", lua_SimilarSector);
    lua_register(lua, "getSectorHeight", lua_GetSectorHeight);
    lua_register(lua, "sectorTriggerClear", lua_SectorTriggerClear);
    lua_register(lua, "sectorAddTrigger", lua_SectorAddTrigger);
    lua_register(lua, "sectorAddTriggerCommand", lua_SectorAddTriggerCommand);

    lua_register(lua, "activateEntity", lua_ActivateEntity);
    lua_register(lua, "deactivateEntity", lua_DeactivateEntity);
    lua_register(lua, "noFixEntityCollision", lua_NoFixEntityCollision);

    lua_register(lua, "moveEntityGlobal", lua_MoveEntityGlobal);
    lua_register(lua, "moveEntityLocal", lua_MoveEntityLocal);
    lua_register(lua, "moveEntityToSink", lua_MoveEntityToSink);
    lua_register(lua, "moveEntityToEntity", lua_MoveEntityToEntity);
    lua_register(lua, "rotateEntity", lua_RotateEntity);

    lua_register(lua, "getEntityModelID", lua_GetEntityModelID);

    lua_register(lua, "getEntityVector", lua_GetEntityVector);
    lua_register(lua, "getEntityDirDot", lua_GetEntityDirDot);
    lua_register(lua, "getEntityDistance", lua_GetEntityDistance);
    lua_register(lua, "getEntityPos", lua_GetEntityPosition);
    lua_register(lua, "setEntityPos", lua_SetEntityPosition);
    lua_register(lua, "getEntityAngles", lua_GetEntityAngles);
    lua_register(lua, "setEntityAngles", lua_SetEntityAngles);
    lua_register(lua, "getEntityScaling", lua_GetEntityScaling);
    lua_register(lua, "setEntityScaling", lua_SetEntityScaling);
    lua_register(lua, "getEntitySpeed", lua_GetEntitySpeed);
    lua_register(lua, "setEntitySpeed", lua_SetEntitySpeed);
    lua_register(lua, "getEntitySpeedLinear", lua_GetEntitySpeedLinear);
    lua_register(lua, "setEntityCollision", lua_SetEntityCollision);
    lua_register(lua, "setEntityCollisionFlags", lua_SetEntityCollisionFlags);
    lua_register(lua, "getEntityAnim", lua_GetEntityAnim);
    lua_register(lua, "setEntityAnim", lua_SetEntityAnim);
    lua_register(lua, "setEntityAnimFlag", lua_SetEntityAnimFlag);
    lua_register(lua, "setEntityBodyPartFlag", lua_SetEntityBodyPartFlag);
    lua_register(lua, "setModelBodyPartFlag", lua_SetModelBodyPartFlag);
    lua_register(lua, "getEntityModel", lua_GetEntityModel);
    lua_register(lua, "getEntityVisibility", lua_GetEntityVisibility);
    lua_register(lua, "setEntityVisibility", lua_SetEntityVisibility);
    lua_register(lua, "getEntityActivity", lua_GetEntityActivity);
    lua_register(lua, "setEntityActivity", lua_SetEntityActivity);
    lua_register(lua, "getEntityEnability", lua_GetEntityEnability);
    lua_register(lua, "getEntityOCB", lua_GetEntityOCB);
    lua_register(lua, "setEntityOCB", lua_SetEntityOCB);
    lua_register(lua, "getEntityTimer", lua_GetEntityTimer);
    lua_register(lua, "setEntityTimer", lua_SetEntityTimer);
    lua_register(lua, "getEntityFlags", lua_GetEntityFlags);
    lua_register(lua, "setEntityFlags", lua_SetEntityFlags);
    lua_register(lua, "getEntityTypeFlag", lua_GetEntityTypeFlag);
    lua_register(lua, "setEntityTypeFlag", lua_SetEntityTypeFlag);
    lua_register(lua, "getEntityStateFlag", lua_GetEntityStateFlag);
    lua_register(lua, "setEntityStateFlag", lua_SetEntityStateFlag);
    lua_register(lua, "getEntityCallbackFlag", lua_GetEntityCallbackFlag);
    lua_register(lua, "setEntityCallbackFlag", lua_SetEntityCallbackFlag);
    lua_register(lua, "getEntityState", lua_GetEntityState);
    lua_register(lua, "setEntityState", lua_SetEntityState);
    lua_register(lua, "setEntityRoomMove", lua_SetEntityRoomMove);
    lua_register(lua, "getEntityMoveType", lua_GetEntityMoveType);
    lua_register(lua, "setEntityMoveType", lua_SetEntityMoveType);
    lua_register(lua, "getEntityResponse", lua_GetEntityResponse);
    lua_register(lua, "setEntityResponse", lua_SetEntityResponse);
    lua_register(lua, "getEntityMeshCount", lua_GetEntityMeshCount);
    lua_register(lua, "setEntityMeshswap", lua_SetEntityMeshswap);
    lua_register(lua, "setModelMeshReplaceFlag", lua_SetModelMeshReplaceFlag);
    lua_register(lua, "setModelAnimReplaceFlag", lua_SetModelAnimReplaceFlag);
    lua_register(lua, "copyMeshFromModelToModel", lua_CopyMeshFromModelToModel);

    lua_register(lua, "setEntityBodyMass", lua_SetEntityBodyMass);
    lua_register(lua, "pushEntityBody", lua_PushEntityBody);
    lua_register(lua, "lockEntityBodyLinearFactor", lua_LockEntityBodyLinearFactor);

    lua_register(lua, "getEntityTriggerLayout", lua_GetEntityTriggerLayout);
    lua_register(lua, "setEntityTriggerLayout", lua_SetEntityTriggerLayout);
    lua_register(lua, "getEntityMask", lua_GetEntityMask);
    lua_register(lua, "setEntityMask", lua_SetEntityMask);
    lua_register(lua, "getEntityEvent", lua_GetEntityEvent);
    lua_register(lua, "setEntityEvent", lua_SetEntityEvent);
    lua_register(lua, "getEntityLock", lua_GetEntityLock);
    lua_register(lua, "setEntityLock", lua_SetEntityLock);
    lua_register(lua, "getEntitySectorStatus", lua_GetEntitySectorStatus);
    lua_register(lua, "setEntitySectorStatus", lua_SetEntitySectorStatus);

    lua_register(lua, "getEntityActivationOffset", lua_GetEntityActivationOffset);
    lua_register(lua, "setEntityActivationOffset", lua_SetEntityActivationOffset);
    lua_register(lua, "getEntitySectorIndex", lua_GetEntitySectorIndex);
    lua_register(lua, "getEntitySectorFlags", lua_GetEntitySectorFlags);
    lua_register(lua, "getEntitySectorMaterial", lua_GetEntitySectorMaterial);

    lua_register(lua, "addEntityRagdoll", lua_AddEntityRagdoll);
    lua_register(lua, "removeEntityRagdoll", lua_RemoveEntityRagdoll);

    lua_register(lua, "setCharacterTarget", lua_SetCharacterTarget);
    lua_register(lua, "getCharacterParam", lua_GetCharacterParam);
    lua_register(lua, "setCharacterParam", lua_SetCharacterParam);
    lua_register(lua, "changeCharacterParam", lua_ChangeCharacterParam);
    lua_register(lua, "getCharacterCurrentWeapon", lua_GetCharacterCurrentWeapon);
    lua_register(lua, "setCharacterCurrentWeapon", lua_SetCharacterCurrentWeapon);
    lua_register(lua, "setCharacterWeaponModel", lua_SetCharacterWeaponModel);
    lua_register(lua, "getCharacterCombatMode", lua_GetCharacterCombatMode);

    lua_register(lua, "addCharacterHair", lua_AddCharacterHair);
    lua_register(lua, "resetCharacterHair", lua_ResetCharacterHair);

    lua_register(lua, "getSecretStatus", lua_GetSecretStatus);
    lua_register(lua, "setSecretStatus", lua_SetSecretStatus);

    lua_register(lua, "getActionState", lua_GetActionState);
    lua_register(lua, "getActionChange", lua_GetActionChange);

    lua_register(lua, "genUVRotateAnimation", lua_genUVRotateAnimation);

    lua_register(lua, "getGravity", lua_GetGravity);
    lua_register(lua, "setGravity", lua_SetGravity);
    lua_register(lua, "dropEntity", lua_DropEntity);
    lua_register(lua, "bind", lua_BindKey);

    lua_register(lua, "addFont", lua_AddFont);
    lua_register(lua, "removeFont", lua_RemoveFont);
    lua_register(lua, "addFontStyle", lua_AddFontStyle);
    lua_register(lua, "removeFontStyle", lua_RemoveFontStyle);
}
