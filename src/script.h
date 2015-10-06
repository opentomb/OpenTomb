
#ifndef PARSE_H
#define PARSE_H

struct screen_info_s;
struct entity_s;
struct lua_State;

#define CVAR_NAME_SIZE 32
#define CVAR_LUA_TABLE_NAME "cvars"


extern lua_State *engine_lua;

void Script_LoadConstants(lua_State *lua);
bool Script_LuaInit();
void Script_LuaClearTasks();
void Script_LuaRegisterFuncs(lua_State *lua);


char *parse_token(char *data, char *token);

float SC_ParseFloat(char **ch);
int   SC_ParseInt(char **ch);

void lua_Clean(lua_State *lua);

int  lua_print(lua_State * lua);
int  lua_BindKey(lua_State *lua);



#define lua_CallAndLog(L,n,r,f) lua_CallWithError(L, n, r, f, __FILE__, __LINE__)
bool  lua_CallWithError(lua_State *lua, int nargs, int nresults, int errfunc, const char *cfile, int cline);

int lua_ParseScreen(lua_State *lua, struct screen_info_s *sc);
int lua_ParseRender(lua_State *lua, struct render_settings_s *rs);
int lua_ParseAudio(lua_State *lua, struct audio_settings_s *as);
int lua_ParseConsole(lua_State *lua);
int lua_ParseControls(lua_State *lua, struct control_settings_s *cs);

bool lua_GetOverridedSamplesInfo(lua_State *lua, int *num_samples, int *num_sounds, char *sample_name_mask);
bool lua_GetOverridedSample(lua_State *lua, int sound_id, int *first_sample_number, int *samples_count);

int  lua_GetGlobalSound(lua_State *lua, int global_sound_id);
int  lua_GetSecretTrackNumber(lua_State *lua);
int  lua_GetNumTracks(lua_State *lua);
bool lua_GetSoundtrack(lua_State *lua, int track_index, char *track_path, int *load_method, int *stream_type);
bool lua_GetLoadingScreen(lua_State *lua, int level_index, char *pic_path);
bool lua_GetString(lua_State *lua, int string_index, size_t string_size, char *buffer);

void Script_LoopEntity(lua_State *lua, int object_id);
int  Script_ExecEntity(lua_State *lua, int id_callback, int id_object, int id_activator = -1);
int  Script_DoTasks(lua_State *lua, float time);
bool Script_CallVoidFunc(lua_State *lua, const char* func_name, bool destroy_after_call = false);

void Script_AddKey(lua_State *lua, int keycode, int state);

#endif
