
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../core/lua.h"

#include "script.h"

#include "../core/system.h"
#include "../core/console.h"
#include "../audio/audio.h"

#include "../gameflow.h"
#include "../engine.h"
#include "../world.h"


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
        as->use_effects  = lua_tointeger(lua, -1);
        lua_pop(lua, 1);

        lua_getfield(lua, -1, "listener_is_player");
        as->listener_is_player = lua_tointeger(lua, -1);
        lua_pop(lua, 1);

        lua_settop(lua, top);
        return 1;
    }

    return -1;
}

/*
 *   Specific functions to get specific parameters from script.
 */
 int Script_GetGlobalSound(lua_State *lua, int global_sound_id)
{
    int sound_id = 0;

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

    return sound_id;
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
            {
                num_tracks = lua_tointeger(lua, -1);
            }
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

        if(lua_isfunction(lua, -1))
        {
            lua_pushinteger(lua, World_GetVersion());
            if(lua_CallAndLog(lua, 1, 3, 0))
            {
                size_t string_length = 0;
                real_path   = lua_tolstring(lua, -1, &string_length);
               *num_sounds  = lua_tointeger(lua, -2);
               *num_samples = lua_tointeger(lua, -3);
                strncpy(sample_name_mask, real_path, string_length);
                result = ((*num_sounds != -1) && (*num_samples != -1) && (strcmp(real_path, "NONE") != 0));
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
            lua_pushinteger(lua, Gameflow_GetCurrentLevelID());
            lua_pushinteger(lua, sound_id);
            if(lua_CallAndLog(lua, 3, 2, 0))
            {
                *first_sample_number = (int)lua_tointeger(lua, -2);
                *samples_count       = (int)lua_tointeger(lua, -1);
                result = ((*first_sample_number != -1) && (*samples_count != -1));
            }
        }
        lua_settop(lua, top);
    }

    return result;
}


bool Script_GetSoundtrack(lua_State *lua, int track_index, char *file_path, int file_path_len, int *load_method, int *stream_type)
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

                strncpy(file_path, Engine_GetBasePath(), file_path_len);
                strncat(file_path, real_path, file_path_len);
                result = (*stream_type != -1);
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
 * General gameplay functions
 */
int lua_PlayStream(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top >= 1)
    {
        int id = lua_tointeger(lua, 1);
        if(id >= 0)
        {
            uint8_t mask = (top >= 2) ? (lua_tointeger(lua, 2)) : (0);
            Audio_StreamPlay(id, mask);
        }
        else
        {
            Con_Warning("wrong stream id");
        }
    }
    else
    {
        Con_Warning("playStream: expecting arguments (stream_id, (mask))");
    }

    return 0;
}


int lua_PlaySound(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top >= 1)
    {
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
    }
    else
    {
        Con_Warning("playSound: expecting arguments (sound_id, (entity_id))");
    }

    return 0;
}


int lua_StopSound(lua_State *lua)
{
    int top = lua_gettop(lua);

    if(top >= 1)
    {
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

        if(result < 0)
        {
            Con_Warning("audio with id = %d not played", id);
        }
    }
    else
    {
        Con_Warning("stopSound: expecting arguments (sound_id, (entity_id))");
    }

    return 0;
}


void Script_LuaRegisterAudioFuncs(lua_State *lua)
{
    lua_register(lua, "playSound", lua_PlaySound);
    lua_register(lua, "stopSound", lua_StopSound);
    lua_register(lua, "playStream", lua_PlayStream);
}
