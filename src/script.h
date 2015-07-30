
#ifndef PARSE_H
#define PARSE_H

#include <LinearMath/btScalar.h>

#include <lua.hpp>
#include "LuaState.h"

#define CVAR_NAME_SIZE 32
#define CVAR_LUA_TABLE_NAME "cvars"

struct ScreenInfo;
struct ConsoleInfo;
struct Entity;
struct AudioSettings;
struct ControlSettings;
struct RenderSettings;

namespace lua {
class State;
}

struct lua_State;
extern lua::State engine_lua;

// Simple override to register both upper- and lowercase versions of function name.

template<typename Function>
inline void lua_registerc(lua::State& state, const std::string& func_name, Function func)
{
    std::string uc, lc;
    for(char c : func_name)
    {
        lc += std::tolower(c);
        uc += std::toupper(c);
    }

    state.set(func_name.c_str(), func);
    state.set(lc.c_str(), func);
    state.set(uc.c_str(), func);
}

template<>
inline void lua_registerc(lua::State& state, const std::string& func_name, int (*func)(lua_State*))
{
    std::string uc, lc;
    for(char c : func_name)
    {
        lc += std::tolower(c);
        uc += std::toupper(c);
    }

    lua_register(state.getState(), func_name.c_str(), func);
    lua_register(state.getState(), lc.c_str(), func);
    lua_register(state.getState(), uc.c_str(), func);
}

// Source-level system routines.

void  Script_LuaInit();
void  Script_LuaClearTasks();
void  Script_LuaRegisterFuncs(lua::State &state);

// Print function override. Puts printed string into console.

int  lua_Print(lua_State *state);

// System Lua functions. Not directly called from scripts.

void lua_Clean(lua::State& state);
void lua_Prepare(lua::State& state);

void lua_LoopEntity(lua::State& state, int object_id);
void lua_ExecEntity(lua::State& state, int id_callback, int id_object, int id_activator = -1);
void lua_ExecEffect(lua::State& state, int id, int caller = -1, int operand = -1);
void lua_DoTasks(lua::State& state, btScalar time);

void lua_AddKey(lua::State& lstate, int keycode, bool state);

void lua_BindKey(int act, int primary, lua::Value secondary);

// Helper Lua functions. Not directly called from scripts.

bool lua_GetOverridedSamplesInfo(lua::State& state, int *num_samples, int *num_sounds, char *sample_name_mask);
bool lua_GetOverridedSample(lua::State& state, int sound_id, int *first_sample_number, int *samples_count);

int  lua_GetGlobalSound(lua::State& state, int global_sound_id);
int  lua_GetSecretTrackNumber(lua::State& state);
int  lua_GetNumTracks(lua::State& state);
bool lua_GetSoundtrack(lua::State& state, int track_index, char *track_path, int *load_method, int *stream_type);
bool lua_GetLoadingScreen(lua::State& state, int level_index, char *pic_path);
bool lua_GetString(lua::State& state, int string_index, size_t string_size, char *buffer);
bool lua_GetSysNotify(lua::State& state, int string_index, size_t string_size, char *buffer);

// Parsing functions - both native and Lua. Not directly called from scripts.

const char *parse_token(const char *data, char *token);

float Script_ParseFloat(const char **ch);
int   Script_ParseInt(char **ch);

void lua_ParseScreen(lua::State& state, ScreenInfo *sc);
void lua_ParseRender(lua::State& state, RenderSettings *rs);
void lua_ParseAudio(lua::State& state, AudioSettings *as);
void lua_ParseConsole(lua::State& state, ConsoleInfo *cn);
void lua_ParseControls(lua::State& state, ControlSettings *cs);

#endif
