
#ifndef PARSE_H
#define PARSE_H

struct ScreenInfo;
struct ConsoleInfo;
struct Entity;
struct AudioSettings;
struct ControlSettings;
struct RenderSettings;

#define CVAR_NAME_SIZE 32
#define CVAR_LUA_TABLE_NAME "cvars"

#include "bullet/LinearMath/btScalar.h"

namespace lua {
class State;
}

extern lua::State engine_lua;

const char *parse_token(const char *data, char *token);

float SC_ParseFloat(const char **ch);
int   SC_ParseInt(char **ch);

void lua_Clean(lua::State& state);

void lua_ParseScreen(lua::State& state, ScreenInfo *sc);
void lua_ParseRender(lua::State& state, RenderSettings *rs);
void lua_ParseAudio(lua::State& state, AudioSettings *as);
void lua_ParseConsole(lua::State& state, ConsoleInfo *cn);
void lua_ParseControls(lua::State& state, ControlSettings *cs);

bool lua_GetOverridedSamplesInfo(lua::State& state, int *num_samples, int *num_sounds, char *sample_name_mask);
bool lua_GetOverridedSample(lua::State& state, int sound_id, int *first_sample_number, int *samples_count);

int  lua_GetGlobalSound(lua::State& state, int global_sound_id);
int  lua_GetSecretTrackNumber(lua::State& state);
int  lua_GetNumTracks(lua::State& state);
bool lua_GetSoundtrack(lua::State& state, int track_index, char *track_path, int *load_method, int *stream_type);
bool lua_GetLoadingScreen(lua::State& state, int level_index, char *pic_path);
bool lua_GetString(lua::State& state, int string_index, size_t string_size, char *buffer);
bool lua_GetSysNotify(lua::State& state, int string_index, size_t string_size, char *buffer);

void lua_LoopEntity(lua::State& state, int object_id);
void lua_ExecEntity(lua::State& state, int id_callback, int id_object, int id_activator = -1);
void lua_DoTasks(lua::State& state, btScalar time);

void lua_AddKey(lua::State& lstate, int keycode, int state);

#endif
