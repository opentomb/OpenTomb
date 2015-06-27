
#ifndef PARSE_H
#define PARSE_H

struct ScreenInfo;
struct ConsoleInfo;
struct Entity;
struct lua_State;
struct AudioSettings;
struct ControlSettings;
struct RenderSettings;

#define CVAR_NAME_SIZE 32
#define CVAR_LUA_TABLE_NAME "cvars"

#include <bullet/LinearMath/btScalar.h>

extern lua_State *engine_lua;

const char *parse_token(const char *data, char *token);

float SC_ParseFloat(const char **ch);
int   SC_ParseInt(char **ch);

void lua_Clean(lua_State *lua);

#define lua_CallAndLog(L,n,r,f) lua_CallWithError(L, n, r, f, __FILE__, __LINE__)
bool  lua_CallWithError(lua_State *lua, int nargs, int nresults, int errfunc, const char *cfile, int cline);

int lua_ParseScreen(lua_State *lua, ScreenInfo *sc);
int lua_ParseRender(lua_State *lua, RenderSettings *rs);
int lua_ParseAudio(lua_State *lua, AudioSettings *as);
int lua_ParseConsole(lua_State *lua, ConsoleInfo *cn);
int lua_ParseControls(lua_State *lua, ControlSettings *cs);

bool lua_GetOverridedSamplesInfo(lua_State *lua, int *num_samples, int *num_sounds, char *sample_name_mask);
bool lua_GetOverridedSample(lua_State *lua, int sound_id, int *first_sample_number, int *samples_count);

int  lua_GetGlobalSound(lua_State *lua, int global_sound_id);
int  lua_GetSecretTrackNumber(lua_State *lua);
int  lua_GetNumTracks(lua_State *lua);
bool lua_GetSoundtrack(lua_State *lua, int track_index, char *track_path, int *load_method, int *stream_type);
bool lua_GetLoadingScreen(lua_State *lua, int level_index, char *pic_path);
bool lua_GetString(lua_State *lua, int string_index, size_t string_size, char *buffer);
bool lua_GetSysNotify(lua_State *lua, int string_index, size_t string_size, char *buffer);

btScalar lua_GetScalarField(lua_State *lua, int index);
btScalar lua_GetScalarField(lua_State *lua, const char *key);
int lua_SetScalarField(lua_State *lua, const char *key, btScalar val);

void lua_LoopEntity(lua_State *lua, int object_id);
int  lua_ExecEntity(lua_State *lua, int id_callback, int id_object, int id_activator = -1);
int  lua_DoTasks(lua_State *lua, btScalar time);
bool lua_CallVoidFunc(lua_State *lua, const char* func_name, bool destroy_after_call = false);

void lua_AddKey(lua_State *lua, int keycode, int state);

#endif
