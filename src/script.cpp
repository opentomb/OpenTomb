
#include <cstdio>
#include <cstdlib>
#include <cstring>

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

#include <lua.hpp>
#include "LuaState.h"
#include "luastate_extra.h"

#include <iostream>

/*
 * MISK
 */
const char *parse_token(const char *data, char *token)
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

float SC_ParseFloat(const char **ch)
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
    (*ch) = const_cast<char*>( parse_token(*ch, token) );
    if(token[0])
    {
        return atoi(token);
    }
    return 0;
}

/*
 *   Specific functions to get specific parameters from script.
 */
int lua_GetGlobalSound(lua::State& state, int global_sound_id)
{
    return state["getGlobalSound"](engine_world.version, global_sound_id);
}

int lua_GetSecretTrackNumber(lua::State& state)
{
    return state["getSecretTrackNumber"](engine_world.version);
}

int lua_GetNumTracks(lua::State& state)
{
    return state["getNumTracks"](engine_world.version);
}


bool lua_GetOverridedSamplesInfo(lua::State& state, int *num_samples, int *num_sounds, char *sample_name_mask)
{
    std::string realPath;

    lua::tie(realPath, *num_sounds, *num_samples) = state["getOverridedSamplesInfo"](engine_world.version);

    strcpy(sample_name_mask, realPath.c_str());

    return *num_sounds != -1 && *num_samples != -1 && realPath != "NONE";
}


bool lua_GetOverridedSample(lua::State& state, int sound_id, int *first_sample_number, int *samples_count)
{
    lua::tie(*first_sample_number, *samples_count) = state["getOverridedSample"](engine_world.version, int(gameflow_manager.CurrentLevelID), sound_id);
    return *first_sample_number != -1 && *samples_count != -1;
}


bool lua_GetSoundtrack(lua::State& state, int track_index, char *file_path, int *load_method, int *stream_type)
{
    std::string realPath;
    lua::tie(realPath, *stream_type, *load_method) = state["getTrackInfo"](engine_world.version, track_index);
    strcpy(file_path, realPath.c_str());
    return *stream_type != -1;
}


bool lua_GetString(lua::State& state, int string_index, size_t string_size, char *buffer)
{
    std::string str = state["getString"](string_index);
    strncpy(buffer, str.c_str(), string_size);
    return true;
}

bool lua_GetSysNotify(lua::State& state, int string_index, size_t string_size, char *buffer)
{
    std::string str = state["getSysNotify"](string_index);
    strncpy(buffer, str.c_str(), string_size);
    return true;
}


bool lua_GetLoadingScreen(lua::State& state, int level_index, char *pic_path)
{
    std::string realPath = state["getLoadingScreen"](int(gameflow_manager.CurrentGameID), int(gameflow_manager.CurrentLevelID), level_index);
    strncpy(pic_path, realPath.c_str(), MAX_ENGINE_PATH);
    return true;
}


/*
 * Gameplay functions
 */

void lua_DoTasks(lua::State& state, btScalar time)
{
    state.set( "frame_time", time );
    state["doTasks"]();
    state["clearKeys"]();
}

void lua_AddKey(lua::State& lstate, int keycode, int state)
{
    lstate["addKey"](keycode, state);
}

void lua_ExecEntity(lua::State& state, int id_callback, int id_object, int id_activator)
{
    if(id_activator>0)
        state["execEntity"](id_callback, id_object, id_activator);
    else
        state["execEntity"](id_callback, id_object);
}

void lua_LoopEntity(lua::State& state, int object_id)
{
    std::shared_ptr<Entity> ent = engine_world.getEntityByID(object_id);
    if(ent->m_active) {
        state["loopEntity"](object_id);
    }
}

/*
 * Game structures parse
 */
void lua_ParseControls(lua::State& state, struct ControlSettings *cs)
{
    state["controls"]["mouse_sensitivity"].get(cs->mouse_sensitivity);
    state["controls"]["use_joy"].get(cs->use_joy);
    state["controls"]["joy_number"].get(cs->joy_number);
    state["controls"]["joy_rumble"].get(cs->joy_rumble);
    state["controls"]["joy_look_axis_x"].get(cs->joy_axis_map[AXIS_MOVE_X]);
    state["controls"]["joy_look_axis_y"].get(cs->joy_axis_map[AXIS_MOVE_Y]);
    state["controls"]["joy_look_invert_x"].get(cs->joy_look_invert_x);
    state["controls"]["joy_look_sensitivity"].get(cs->joy_look_sensitivity);
    state["controls"]["joy_look_deadzone"].get(cs->joy_look_deadzone);
    state["controls"]["joy_move_invert_x"].get(cs->joy_move_invert_x);
    state["controls"]["joy_move_invert_y"].get(cs->joy_move_invert_y);
    state["controls"]["joy_move_sensitivity"].get(cs->joy_move_sensitivity);
    state["controls"]["joy_move_deadzone"].get(cs->joy_move_deadzone);
}

void lua_ParseScreen(lua::State& state, struct ScreenInfo *sc)
{
    state["screen"]["x"].get(sc->x);
    state["screen"]["y"].get(sc->y);
    state["screen"]["width"].get(sc->w);
    state["screen"]["height"].get(sc->h);
    state["screen"]["width"].get(sc->w);
    sc->w_unit = sc->w / GUI_SCREEN_METERING_RESOLUTION;
    state["screen"]["height"].get(sc->h);
    sc->h_unit = sc->h / GUI_SCREEN_METERING_RESOLUTION;
    state["screen"]["fullscreen"].get(sc->FS_flag);
    state["screen"]["debug_info"].get(sc->show_debuginfo);
    state["screen"]["fov"].get(sc->fov);
}

void lua_ParseRender(lua::State& state, struct RenderSettings *rs)
{
    state["render"]["mipmap_mode"].get(rs->mipmap_mode);
    state["render"]["mipmaps"].get(rs->mipmaps);
    state["render"]["lod_bias"].get(rs->lod_bias);
    state["render"]["anisotropy"].get(rs->anisotropy);
    state["render"]["antialias"].get(rs->antialias);
    state["render"]["antialias_samples"].get(rs->antialias_samples);
    state["render"]["texture_border"].get(rs->texture_border);
    state["render"]["z_depth"].get(rs->z_depth);
    state["render"]["fog_enabled"].get(rs->fog_enabled);
    state["render"]["fog_start_depth"].get(rs->fog_start_depth);
    state["render"]["fog_end_depth"].get(rs->fog_end_depth);
    state["render"]["fog_color"]["r"].get(rs->fog_color[0]);
    state["render"]["fog_color"]["g"].get(rs->fog_color[1]);
    state["render"]["fog_color"]["b"].get(rs->fog_color[2]);
    rs->fog_color[0] /= 255.0;
    rs->fog_color[1] /= 255.0;
    rs->fog_color[2] /= 255.0;
    rs->fog_color[3] = 1;
    if(rs->z_depth != 8 && rs->z_depth != 16 && rs->z_depth != 24)
    {
        rs->z_depth = 24;
    }
}

void lua_ParseAudio(lua::State& state, struct AudioSettings *as)
{
    state["audio"]["music_volume"].get(as->music_volume);
    state["audio"]["sound_volume"].get(as->sound_volume);
    state["audio"]["use_effects"].get(as->use_effects);
    state["audio"]["listener_is_player"].get(as->listener_is_player);
    state["audio"]["stream_buffer_size"].get(as->stream_buffer_size);
    as->stream_buffer_size *= 1024;
    if(as->stream_buffer_size <= 0)
        as->stream_buffer_size = 128 * 1024;
    state["audio"]["music_volume"].get(as->music_volume);
    state["audio"]["music_volume"].get(as->music_volume);
}

void lua_ParseConsole(lua::State& state, ConsoleInfo *cn)
{
    {
        float r,g,b,a;
        state["console"]["background_color"]["r"].get(r);
        state["console"]["background_color"]["g"].get(g);
        state["console"]["background_color"]["b"].get(b);
        state["console"]["background_color"]["a"].get(a);
        cn->setBackgroundColor(r/255, g/255, b/255, a/255);
    }
    float tmpF;
    int tmpI;

    state["console"]["spacing"].get(tmpF);
    if(tmpF >= CON_MIN_LINE_INTERVAL && tmpF <= CON_MAX_LINE_INTERVAL)
        cn->setSpacing( tmpF );

    state["console"]["line_size"].get(tmpI);
    if(tmpI >= CON_MIN_LINE_SIZE && tmpI <= CON_MAX_LINE_SIZE)
        cn->setLineSize( tmpI );

    state["console"]["showing_lines"].get(tmpI);
    if(tmpI >= CON_MIN_LINES && tmpI <= CON_MIN_LINES)
        cn->setVisibleLines( tmpI );

    state["console"]["log_size"].get(tmpI);
    if(tmpI >= CON_MIN_LOG && tmpI <= CON_MAX_LOG)
        cn->setHistorySize( tmpI );

    state["console"]["lines_count"].get(tmpI);
    if(tmpI >= CON_MIN_LOG && tmpI <= CON_MAX_LOG)
        cn->setBufferSize( tmpI );

    bool tmpB;
    state["console"]["show"].get(tmpB);
    cn->setVisible( tmpB );

    state["console"]["show_cursor_period"].get(tmpF);
    cn->setShowCursorPeriod( tmpF );
}

void lua_Clean(lua::State& state)
{
    state["tlist_Clear"]();
    state["entfuncs_Clear"]();
}
