#pragma once

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

namespace script
{

extern lua::State engine_lua;

// Simple override to register both upper- and lowercase versions of function name.

template<typename Function>
inline void registerCaseInsensitive(lua::State& state, const std::string& func_name, Function func)
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
inline void registerCaseInsensitive(lua::State& state, const std::string& func_name, int (*func)(lua_State*))
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

void clean(lua::State& state);
void prepare(lua::State& state);

void loopEntity(lua::State& state, int object_id);
void execEntity(lua::State& state, int id_callback, int id_object, int id_activator = -1);
void execEffect(lua::State& state, int id, int caller = -1, int operand = -1);
void doTasks(lua::State& state, btScalar time);

void addKey(lua::State& lstate, int keycode, bool state);

void lua_BindKey(int act, int primary, lua::Value secondary);

// Helper Lua functions. Not directly called from scripts.

bool getOverriddenSamplesInfo(lua::State& state, int *num_samples, int *num_sounds, char *sample_name_mask);
bool getOverriddenSample(lua::State& state, int sound_id, int *first_sample_number, int *samples_count);

int  getGlobalSound(lua::State& state, int global_sound_id);
int  getSecretTrackNumber(lua::State& state);
int  getNumTracks(lua::State& state);
bool getSoundtrack(lua::State& state, int track_index, char *track_path, int *load_method, int *stream_type);
bool getLoadingScreen(lua::State& state, int level_index, char *pic_path);
bool getString(lua::State& state, int string_index, size_t string_size, char *buffer);
bool getSysNotify(lua::State& state, int string_index, size_t string_size, char *buffer);

// Parsing functions - both native and Lua. Not directly called from scripts.

const char *parse_token(const char *data, char *token);

float Script_ParseFloat(const char **ch);
int   Script_ParseInt(char **ch);

void parseScreen(lua::State& state, ScreenInfo *sc);
void parseRender(lua::State& state, RenderSettings *rs);
void parseAudio(lua::State& state, AudioSettings *as);
void parseConsole(lua::State& state, ConsoleInfo *cn);
void parseControls(lua::State& state, ControlSettings *cs);

} // namespace script
