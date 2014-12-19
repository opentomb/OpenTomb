
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
    //FIXME: Проверять token на переполнение
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
 */
const char *lua_GetStrField(lua_State *lua, const char *key)
{
    int top;
    const char *ret;

    top = lua_gettop(lua);
    if (!lua_istable(lua, -1))
    {
        return NULL;
    }

    lua_getfield(lua, -1, key);
    ret = lua_tostring(lua, -1);
    lua_settop(lua, top);
    return ret;
}


btScalar lua_GetScalarField(lua_State *lua, const char *key)
{
    int top;
    btScalar ret = 0.0;

    top = lua_gettop(lua);
    if (!lua_istable(lua, -1))
    {
        return 0.0;
    }

    lua_getfield(lua, -1, key);
    ret = lua_tonumber(lua, -1);
    lua_settop(lua, top);
    return ret;
}


/*
 *   Specific functions to get specific parameters from script.
 */

 int lua_GetGlobalSound(lua_State *lua, int global_sound_id)
{
    int top;
    lua_Integer sound_id = 0;

    if(lua)
    {
        top = lua_gettop(lua);                                                  // save LUA stack
        lua_getglobal(lua, "GetGlobalSound");                                   // add to the up of stack LUA's function

        if(lua_isfunction(lua, -1))                                             // If function exists...
        {
            lua_pushinteger(lua, engine_world.version);                         // add to stack first argument
            lua_pushinteger(lua, global_sound_id);
            lua_pcall(lua, 2, 1, 0);                                            // call that function
            sound_id = lua_tointeger(lua, -1);                                  // get returned value 1
        }

        lua_settop(lua, top);
    }

    return (int)sound_id;
}

int lua_GetSecretTrackNumber(lua_State *lua)
{
    int top;
    lua_Integer track_number = 0;

    if(lua)
    {
        top = lua_gettop(lua);                                                  // save LUA stack
        lua_getglobal(lua, "GetSecretTrackNumber");                             // add to the up of stack LUA's function

        if(lua_isfunction(lua, -1))                                             // If function exists...
        {
            lua_pushinteger(lua, engine_world.version);                         // add to stack first argument
            lua_pcall(lua, 1, 1, 0);                                            // call that function
            track_number = lua_tointeger(lua, -1);                              // get returned value 1
        }

        lua_settop(lua, top);                                                   // restore LUA stack
    }

    return (int)track_number;
}

int lua_GetNumTracks(lua_State *lua)
{
    int top;
    lua_Integer num_tracks = 0;

    if(lua)
    {
        top = lua_gettop(lua);
        lua_getglobal(lua, "GetNumTracks");

        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, engine_world.version);
            lua_pcall(lua, 1, 1, 0);
            num_tracks = lua_tointeger(lua, -1);
        }

        lua_settop(lua, top);
    }

    return (int)num_tracks;
}


bool lua_GetOverridedSamplesInfo(lua_State *lua, int *num_samples, int *num_sounds, char *sample_name_mask)
{
    bool result = false;
    const char *real_path;

    if(lua)
    {
        int top = lua_gettop(lua);

        lua_getglobal(lua, "GetOverridedSamplesInfo");

        if(lua_isfunction(lua, -1))
        {
            size_t string_length = 0;

            lua_pushinteger(lua, engine_world.version);
            lua_pcall(lua, 1, 3, 0);

            real_path   = lua_tolstring(lua, -1, &string_length);
           *num_sounds  = (int)lua_tointeger(lua, -2);
           *num_samples = (int)lua_tointeger(lua, -3);

            strcpy(sample_name_mask, real_path);

            if((*num_sounds != -1) && (*num_samples != -1) && (strcmp(real_path, "NONE") != 0))
            {
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
        lua_getglobal(lua, "GetOverridedSample");

        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, engine_world.version);
            lua_pushinteger(lua, gameflow_manager.CurrentLevelID);
            lua_pushinteger(lua, sound_id);
            lua_pcall(lua, 3, 2, 0);

           *first_sample_number = (int)lua_tointeger(lua, -2);
           *samples_count       = (int)lua_tointeger(lua, -1);

            if((*first_sample_number != -1) && (*samples_count != -1))
                result = true;
        }
        else

        lua_settop(lua, top);
    }

    return result;
}


bool lua_GetSoundtrack(lua_State *lua, int track_index, char *file_path, int *load_method, int *stream_type)
{
    size_t  string_length  = 0;
    int     top;

    const char *real_path;

    if(lua)
    {
        top = lua_gettop(lua);                                                  // save LUA stack

        lua_getglobal(lua, "GetTrackInfo");                                     // add to the up of stack LUA's function

        if(lua_isfunction(lua, -1))                                             // If function exists...
        {
            lua_pushinteger(lua, engine_world.version);                         // add to stack first argument
            lua_pushinteger(lua, track_index);                                  // add to stack second argument

            lua_pcall(lua, 2, 3, 0);                                            // call that function

            real_path   = lua_tolstring(lua, -3, &string_length);               // get returned value 1
           *stream_type = (int)lua_tointeger(lua, -2);                          // get returned value 2
           *load_method = (int)lua_tointeger(lua, -1);                          // get returned value 3

            // For some reason, Lua returns constant string pointer, which we can't assign to
            // provided argument; so we need to straightly copy it.

            strcpy(file_path, real_path);

            lua_settop(lua, top);                                               // restore LUA stack

            if(*stream_type != -1)
                return true;                                                    // Entry extracted, success!
        }
        else
        {
            lua_settop(lua, top);                                               // restore LUA stack
        }
    }

    // If Lua wasn't able to extract file path from the script, most likely it means
    // that entry is broken or missing, or wrong track ID was specified. So we return
    // FALSE in such cases.

    return false;
}


bool lua_GetLoadingScreen(lua_State *lua, int level_index, char *pic_path)
{
    size_t  string_length  = 0;
    int     top;

    const char *real_path;

    if(lua != NULL)
    {
        top = lua_gettop(lua);                                                  // save LUA stack

        lua_getglobal(lua, "GetLoadingScreen");                                 // add to the up of stack LUA's function

        if(lua_isfunction(lua, -1))                                             // If function exists...
        {
            lua_pushinteger(lua, gameflow_manager.CurrentGameID);               // add to stack first argument
            lua_pushinteger(lua, gameflow_manager.CurrentLevelID);              // add to stack second argument
            lua_pushinteger(lua, level_index);                                  // add to stack third argument

            lua_pcall(lua, 3, 1, 0);                                            // call that function
            real_path = lua_tolstring(lua, -1, &string_length);                 // get returned value 1

            // For some reason, Lua returns constant string pointer, which we can't assign to
            // provided argument; so we need to straightly copy it.
            strcpy(pic_path, real_path);
            lua_settop(lua, top);                                               // restore LUA stack

            return true;
        }
        else
        {
            lua_settop(lua, top);                                               // restore LUA stack
        }
    }

    // If Lua wasn't able to extract file path from the script, most likely it means
    // that entry is broken or missing, or wrong track ID was specified. So we return
    // FALSE in such cases.

    return false;
}


/**
 * set tbl[key]
 */
int lua_SetScalarField(lua_State *lua, const char *key, btScalar val)
{
    int top;

    top = lua_gettop(lua);
    if (!lua_istable(lua, -1))
    {
        return 0;
    }

    lua_pushnumber(lua, val);
    lua_setfield(lua, -2, key);
    lua_settop(lua, top);
    return 1;
}


int lua_SetStrField(lua_State *lua, const char *key, const char *val)
{
    int top;

    top = lua_gettop(lua);
    if (!lua_istable(lua, -1))
    {
        return 0;
    }

    lua_pushstring(lua, val);
    lua_setfield(lua, -2, key);
    lua_settop(lua, top);
    return 1;
}

/*
 * Gameplay functions
 */

int lua_DoTasks(lua_State *lua, btScalar time)
{
    int top;

    top = lua_gettop(lua);
    lua_pushnumber(lua, time);
    lua_setglobal(lua, "frame_time");
    lua_getglobal(lua, "doTasks");
    lua_pcall(lua, 0, 0, 0);
    lua_settop(lua, top);

    return 0;
}

int lua_ActivateEntity(lua_State *lua, int id_object, int id_activator)
{
    int top;

    top = lua_gettop(lua);
    lua_getglobal(lua, "activateEntity");
    if (!lua_isfunction(lua, -1))
    {
        lua_settop(lua, top);
        //Sys_Warn("Broken \"activateEntity\" script function");
        return -1;
    }

    lua_pushinteger(lua, id_object);
    lua_pushinteger(lua, id_activator);
    lua_pcall(lua, 2, 0, 0);
    lua_settop(lua, top);
    return 1;
}


/*
 * Game structures parse
 */
int lua_ParseControlSettings(lua_State *lua, struct control_settings_s *cs)
{
    if(!lua)
    {
        return -1;
    }

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

int lua_ParseScreen(lua_State *lua, struct screen_info_s *sc)
{
    if(!lua)
    {
        return -1;
    }

    int top = lua_gettop(lua);
    lua_getglobal(lua, "screen");
    sc->x = lua_GetScalarField(lua, "x");
    sc->y = lua_GetScalarField(lua, "y");
    sc->w = lua_GetScalarField(lua, "width");
    sc->h = lua_GetScalarField(lua, "height");
    sc->FS_flag = lua_GetScalarField(lua, "fullscreen");
    sc->fov = lua_GetScalarField(lua, "fov");
    lua_settop(lua, top);

    return 1;
}

int lua_ParseRender(lua_State *lua, struct render_settings_s *rs)
{
    if(!lua)
    {
        return -1;
    }

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

int lua_ParseAudio(lua_State *lua, struct audio_settings_s *as)
{
    if(!lua)
    {
        return -1;
    }

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

int lua_ParseConsole(lua_State *lua, struct console_info_s *cn)
{
    const char *path;
    FILE *f;
    int32_t t, i;
    int top;
    float tf;

    if(!lua)
    {
        return -1;
    }

    con_base.inited = 0;

    top = lua_gettop(lua);
    lua_getglobal(lua, "console");
    path = lua_GetStrField(lua, "font_path");
    if(path && strncmp(path, cn->font_path, 255))
    {
        f = fopen(path, "rb");
        if(f)
        {
            fclose(f);
            strncpy(cn->font_path, path, 255);

            delete con_base.font;
            con_base.font = new FTGLTextureFont(con_base.font_path);
        }
        else
        {
            Sys_Warn("Console: could not find font = \"%s\"", con_base.font_path);
        }
    }
    lua_getfield(lua, -1, "font_color");
    if(lua_istable(lua, -1))
    {
        cn->font_color[0] = (GLfloat)lua_GetScalarField(lua, "r") / 255.0;
        cn->font_color[1] = (GLfloat)lua_GetScalarField(lua, "g") / 255.0;
        cn->font_color[2] = (GLfloat)lua_GetScalarField(lua, "b") / 255.0;
        cn->font_color[3] = 1.0;
    }
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "background_color");
    if(lua_istable(lua, -1))
    {
        cn->background_color[0] = (GLfloat)lua_GetScalarField(lua, "r") / 255.0;
        cn->background_color[1] = (GLfloat)lua_GetScalarField(lua, "g") / 255.0;
        cn->background_color[2] = (GLfloat)lua_GetScalarField(lua, "b") / 255.0;
        cn->background_color[3] = (GLfloat)lua_GetScalarField(lua, "a") / 255.0;
    }
    lua_pop(lua, 1);

    t = lua_GetScalarField(lua, "font_size");
    if(t >= 1 && t <= 128)
    {
        cn->font_size = t;
    }
    tf = lua_GetScalarField(lua, "spacing");
    if(tf >= CON_MIN_LINE_INTERVAL && tf <= CON_MAX_LINE_INTERVAL)
    {
        cn->spacing = tf;
    }
    t = lua_GetScalarField(lua, "line_size");
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
        if(t > cn->log_lines_count)
        {
            con_base.log_lines = (char**) realloc(con_base.log_lines, t * sizeof(char*));
            for(i=cn->log_lines_count;i<t;i++)
            {
                con_base.log_lines[i] = (char*) calloc(con_base.line_size * sizeof(char), 1);
            }
        }
        cn->log_lines_count = t;
    }
    t = lua_GetScalarField(lua, "lines_count");
    if(t >= CON_MIN_LOG && t <= CON_MAX_LOG)
    {
        if(t > cn->shown_lines_count)
        {
            con_base.shown_lines = (char**) realloc(con_base.shown_lines, t * sizeof(char*));
            for(i=cn->shown_lines_count;i<t;i++)
            {
                con_base.shown_lines[i] = (char*) calloc(con_base.line_size * sizeof(char), 1);
            }
        }
        cn->shown_lines_count = t;
    }

    cn->show = lua_GetScalarField(lua, "show");
    cn->show_cursor_period = lua_GetScalarField(lua, "show_cursor_period");
    con_base.inited = 1;
    lua_settop(lua, top);

    Con_SetFontSize(con_base.font_size);
    Con_SetLineInterval(con_base.spacing);

    return 1;
}
