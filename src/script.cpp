
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "script.h"
#include "system.h"
#include "console.h"
#include "entity.h"
#include "world.h"
#include "engine.h"
#include "controls.h"
#include "game.h"
#include "gameflow.h"
#include "anim_state_control.h"
#include "character_controller.h"
#include "vmath.h"
#include "render.h"
#include "audio.h"
#include "string.h"

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

/*
 * MISK
 */
char *parse_token(char *data, char *token)
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
    (*ch) = parse_token(*ch, token);
    if(token[0])
    {
        return atof(token);
    }
    return 0.0;
}

int SC_ParseInt(char **ch)
{
    char token[64];
    (*ch) = parse_token(*ch, token);
    if(token[0])
    {
        return atoi(token);
    }
    return 0;
}

/**
 * get tbl[key]
 * @TODO: delete functions like lua_Get[Set]xxxField! Use only native LUA functions;
 */
btScalar lua_GetScalarField(lua_State *lua, const char *key)
{
    btScalar ret = 0.0;
    int top = lua_gettop(lua);

    if (!lua_istable(lua, -1))
    {
        return 0.0;
    }

    lua_getfield(lua, -1, key);
    ret = lua_tonumber(lua, -1);
    lua_settop(lua, top);
    return ret;
}

btScalar lua_GetScalarField(lua_State *lua, int index)
{
    btScalar ret = 0.0;
    int top = lua_gettop(lua);

    if (!lua_istable(lua, -1))
    {
        return 0.0;
    }

    lua_rawgeti(lua, -1, index);
    ret = lua_tonumber(lua, -1);
    lua_settop(lua, top);
    return ret;
}


/*
 *   Specific functions to get specific parameters from script.
 */
 int lua_GetGlobalSound(lua_State *lua, int global_sound_id)
{
    lua_Integer sound_id = 0;

    if(lua)
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "getGlobalSound");

        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, engine_world.version);
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

int lua_GetSecretTrackNumber(lua_State *lua)
{
    lua_Integer track_number = 0;

    if(lua)
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "getSecretTrackNumber");

        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, engine_world.version);
            track_number = lua_tointeger(lua, -1);
            if(lua_CallAndLog(lua, 2, 1, 0))
                track_number = lua_tointeger(lua, -1);
        }
        lua_settop(lua, top);
    }

    return (int)track_number;
}

int lua_GetNumTracks(lua_State *lua)
{
    lua_Integer num_tracks = 0;

    if(lua)
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "getNumTracks");

        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, engine_world.version);
            if(lua_CallAndLog(lua, 1, 1, 0))
                num_tracks = lua_tointeger(lua, -1);
        }
        lua_settop(lua, top);
    }

    return (int)num_tracks;
}


bool lua_GetOverridedSamplesInfo(lua_State *lua, int *num_samples, int *num_sounds, char *sample_name_mask)
{
    bool result = false;

    if(lua)
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "getOverridedSamplesInfo");
        const char *real_path;

        if(lua_isfunction(lua, -1))
        {
            size_t string_length = 0;

            lua_pushinteger(lua, engine_world.version);
            if (lua_CallAndLog(lua, 1, 3, 0))
            {
                real_path   = lua_tolstring(lua, -1, &string_length);
               *num_sounds  = (int)lua_tointeger(lua, -2);
               *num_samples = (int)lua_tointeger(lua, -3);

                strcpy(sample_name_mask, real_path);

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


bool lua_GetOverridedSample(lua_State *lua, int sound_id, int *first_sample_number, int *samples_count)
{
    bool result = false;

    if(lua)
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "getOverridedSample");

        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, engine_world.version);
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


bool lua_GetSoundtrack(lua_State *lua, int track_index, char *file_path, int *load_method, int *stream_type)
{
    bool result = false;
    
    if(lua)
    {
        int top = lua_gettop(lua);
        size_t  string_length  = 0;
        const char *real_path;

        lua_getglobal(lua, "getTrackInfo");

        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, engine_world.version);
            lua_pushinteger(lua, track_index);
            if(lua_CallAndLog(lua, 2, 3, 0))
            {
                real_path   = lua_tolstring(lua, -3, &string_length);
               *stream_type = (int)lua_tointeger(lua, -2);
               *load_method = (int)lua_tointeger(lua, -1);

                // Lua returns constant string pointer, which we can't assign to
                // provided argument; so we need to straightly copy it.

                strcpy(file_path, real_path);

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


bool lua_GetString(lua_State *lua, int string_index, size_t string_size, char *buffer)
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

bool lua_GetSysNotify(lua_State *lua, int string_index, size_t string_size, char *buffer)
{
    bool result = false;

    if(lua)
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "getSysNotify");

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


bool lua_GetLoadingScreen(lua_State *lua, int level_index, char *pic_path)
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


/**
 * set tbl[key]
 */
int lua_SetScalarField(lua_State *lua, const char *key, btScalar val)
{
    int top = lua_gettop(lua);

    if (!lua_istable(lua, -1))
    {
        return 0;
    }

    lua_pushnumber(lua, val);
    lua_setfield(lua, -2, key);
    lua_settop(lua, top);
    return 1;
}

/*
 * Gameplay functions
 */

int lua_DoTasks(lua_State *lua, btScalar time)
{
    lua_pushnumber(lua, time);
    lua_setglobal(lua, "frame_time");
    
    lua_CallVoidFunc(lua, "doTasks");
    lua_CallVoidFunc(lua, "clearKeys");
    
    return 0;
}

void lua_AddKey(lua_State *lua, int keycode, int state)
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

bool lua_CallVoidFunc(lua_State *lua, const char* func_name)
{
    int top = lua_gettop(lua);
    
    lua_getglobal(lua, func_name);
    
    if(!lua_isfunction(lua, -1))
    {
        lua_settop(lua, top);
        return false;
    }
    
    lua_CallAndLog(lua, 0, 0, 0);
    lua_settop(lua, top);
    return true;
}

int lua_ExecEntity(lua_State *lua, int id_object, int id_activator, int id_callback)
{
    int top = lua_gettop(lua);

    lua_getglobal(lua, "execEntity");
    if (!lua_isfunction(lua, -1))
    {
        lua_settop(lua, top);
        //Sys_Warn("Broken \"execEntity\" script function");
        return -1;
    }

    lua_pushinteger(lua, id_object);
    lua_pushinteger(lua, id_activator);
    lua_pushinteger(lua, id_callback);
    lua_CallAndLog(lua, 3, 0, 0);

    lua_settop(lua, top);
    return 1;
}

void lua_LoopEntity(lua_State *lua, int object_id)
{
    entity_p ent = World_GetEntityByID(&engine_world, object_id);
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
int lua_ParseControls(lua_State *lua, struct control_settings_s *cs)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "controls");
        cs->mouse_sensitivity = lua_GetScalarField(lua, "mouse_sensitivity");
        cs->use_joy = lua_GetScalarField(lua, "use_joy");
        cs->joy_number = lua_GetScalarField(lua, "joy_number");
        cs->joy_rumble = lua_GetScalarField(lua, "joy_rumble");
        cs->joy_axis_map[AXIS_LOOK_X] = lua_GetScalarField(lua, "joy_look_axis_x");
        cs->joy_axis_map[AXIS_LOOK_Y] = lua_GetScalarField(lua, "joy_look_axis_y");
        cs->joy_look_invert_x = lua_GetScalarField(lua, "joy_look_invert_x");
        cs->joy_look_invert_y = lua_GetScalarField(lua, "joy_look_invert_y");
        cs->joy_look_sensitivity = lua_GetScalarField(lua, "joy_look_sensitivity");
        cs->joy_look_deadzone = lua_GetScalarField(lua, "joy_look_deadzone");
        cs->joy_axis_map[AXIS_MOVE_X] = lua_GetScalarField(lua, "joy_move_axis_x");
        cs->joy_axis_map[AXIS_MOVE_Y] = lua_GetScalarField(lua, "joy_move_axis_y");
        cs->joy_move_invert_x = lua_GetScalarField(lua, "joy_move_invert_x");
        cs->joy_move_invert_y = lua_GetScalarField(lua, "joy_move_invert_y");
        cs->joy_move_sensitivity = lua_GetScalarField(lua, "joy_move_sensitivity");
        cs->joy_move_deadzone = lua_GetScalarField(lua, "joy_move_deadzone");

        lua_settop(lua, top);
        return 1;
    }

    return -1;
}

int lua_ParseScreen(lua_State *lua, struct screen_info_s *sc)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "screen");
        sc->x = (int16_t)lua_GetScalarField(lua, "x");
        sc->y = (int16_t)lua_GetScalarField(lua, "y");
        sc->w = (int16_t)lua_GetScalarField(lua, "width");
        sc->h = (int16_t)lua_GetScalarField(lua, "height");
        sc->w_unit = (GLfloat)sc->w / GUI_SCREEN_METERING_RESOLUTION;
        sc->h_unit = (GLfloat)sc->h / GUI_SCREEN_METERING_RESOLUTION;
        sc->FS_flag = (int8_t)lua_GetScalarField(lua, "fullscreen");
        sc->show_debuginfo = (int8_t)lua_GetScalarField(lua, "debug_info");
        sc->fov = (float)lua_GetScalarField(lua, "fov");

        lua_settop(lua, top);
        return 1;
    }

    return -1;
}

int lua_ParseRender(lua_State *lua, struct render_settings_s *rs)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "render");
        rs->mipmap_mode = lua_GetScalarField(lua, "mipmap_mode");
        rs->mipmaps = lua_GetScalarField(lua, "mipmaps");
        rs->lod_bias = lua_GetScalarField(lua, "lod_bias");
        rs->anisotropy = lua_GetScalarField(lua, "anisotropy");
        rs->antialias = lua_GetScalarField(lua, "antialias");
        rs->antialias_samples = lua_GetScalarField(lua, "antialias_samples");
        rs->texture_border = lua_GetScalarField(lua, "texture_border");
        rs->z_depth = lua_GetScalarField(lua, "z_depth");
        rs->fog_enabled = lua_GetScalarField(lua, "fog_enabled");
        rs->fog_start_depth = lua_GetScalarField(lua, "fog_start_depth");
        rs->fog_end_depth = lua_GetScalarField(lua, "fog_end_depth");

        lua_getfield(lua, -1, "fog_color");
        if(lua_istable(lua, -1))
        {
            rs->fog_color[0] = (GLfloat)lua_GetScalarField(lua, "r") / 255.0;
            rs->fog_color[1] = (GLfloat)lua_GetScalarField(lua, "g") / 255.0;
            rs->fog_color[2] = (GLfloat)lua_GetScalarField(lua, "b") / 255.0;
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

int lua_ParseAudio(lua_State *lua, struct audio_settings_s *as)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "audio");
        as->music_volume = lua_GetScalarField(lua, "music_volume");
        as->sound_volume = lua_GetScalarField(lua, "sound_volume");
        as->use_effects  = lua_GetScalarField(lua, "use_effects");
        as->listener_is_player = lua_GetScalarField(lua, "listener_is_player");
        as->stream_buffer_size = (lua_GetScalarField(lua, "stream_buffer_size")) * 1024;
        if(as->stream_buffer_size <= 0)
        {
            as->stream_buffer_size = 128 * 1024;
        }

        lua_settop(lua, top);
        return 1;
    }

    return -1;
}

int lua_ParseConsole(lua_State *lua, struct console_info_s *cn)
{
    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "console");
        lua_getfield(lua, -1, "background_color");
        if(lua_istable(lua, -1))
        {
            cn->background_color[0] = (GLfloat)lua_GetScalarField(lua, "r") / 255.0;
            cn->background_color[1] = (GLfloat)lua_GetScalarField(lua, "g") / 255.0;
            cn->background_color[2] = (GLfloat)lua_GetScalarField(lua, "b") / 255.0;
            cn->background_color[3] = (GLfloat)lua_GetScalarField(lua, "a") / 255.0;
        }
        lua_pop(lua, 1);

        float tf = lua_GetScalarField(lua, "spacing");
        if(tf >= CON_MIN_LINE_INTERVAL && tf <= CON_MAX_LINE_INTERVAL)
        {
            cn->spacing = tf;
        }
        int t = lua_GetScalarField(lua, "line_size");
        if(t >= CON_MIN_LINE_SIZE && t <= CON_MAX_LINE_SIZE)
        {
            cn->line_size = t;
        }
        t = lua_GetScalarField(lua, "showing_lines");
        if(t >= CON_MIN_LINES && t <= CON_MAX_LINES)
        {
            cn->showing_lines = t;
        }
        t = lua_GetScalarField(lua, "log_size");
        if(t >= CON_MIN_LOG && t <= CON_MAX_LOG)
        {
            cn->log_lines_count = t;
        }
        t = lua_GetScalarField(lua, "lines_count");
        if(t >= CON_MIN_LOG && t <= CON_MAX_LOG)
        {
            cn->line_count = t;
        }

        cn->show = lua_GetScalarField(lua, "show");
        cn->show_cursor_period = lua_GetScalarField(lua, "show_cursor_period");

        lua_settop(lua, top);
        return 1;
    }

    return -1;
}

void lua_Clean(lua_State *lua)
{
    if(lua)
    {
        int top = lua_gettop(lua);
        lua_getglobal(lua, "tlist_Clear");
        if(lua_isfunction(lua, -1))
        {
            lua_CallAndLog(lua, 0, 1, 0);
        }

        lua_getglobal(lua, "entfuncs_Clear");
        if(lua_isfunction(lua, -1))
        {
            lua_CallAndLog(lua, 0, 1, 0);
        }
        lua_settop(lua, top);
    }
}

bool lua_CallWithError(lua_State *lua, int nargs, int nresults, int errfunc, const char *cfile, int cline)
{
    if (lua_pcall(lua, nargs, nresults, errfunc) != LUA_OK)
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
        //fprintf(stderr, "%s\n", errormessage);
        Con_AddLine(errormessage, FONTSTYLE_CONSOLE_WARNING);
        return false;
    }
    return true;
}
