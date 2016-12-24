
#ifndef ENGINE_SCRIPT_H
#define ENGINE_SCRIPT_H

struct screen_info_s;
struct entity_s;
struct lua_State;

#define CVAR_LUA_TABLE_NAME "cvars"

// Response constants
#define RESP_KILL           (0)
#define RESP_VERT_COLLIDE   (1)
#define RESP_HOR_COLLIDE    (2)
#define RESP_SLIDE          (3)

extern lua_State *engine_lua;

void Script_LoadConstants(lua_State *lua);
bool Script_LuaInit();
int  Script_DoLuaFile(lua_State *lua, const char *local_path);
void Script_LuaClearTasks();
void Script_LuaRegisterFuncs(lua_State *lua);

char *SC_ParseToken(char *data, char *token);
float SC_ParseFloat(char **ch);
int   SC_ParseInt(char **ch);

int  lua_print(lua_State * lua);
int  lua_BindKey(lua_State *lua);


#define lua_CallAndLog(L,n,r,f) lua_CallWithError(L, n, r, f, __FILE__, __LINE__)
bool  lua_CallWithError(lua_State *lua, int nargs, int nresults, int errfunc, const char *cfile, int cline);

int Script_ParseScreen(lua_State *lua, struct screen_info_s *sc);
int Script_ParseRender(lua_State *lua, struct render_settings_s *rs);
int Script_ParseAudio(lua_State *lua, struct audio_settings_s *as);
int Script_ParseConsole(lua_State *lua);
int Script_ParseControls(lua_State *lua, struct control_settings_s *cs);

bool Script_GetOverridedSamplesInfo(lua_State *lua, int *num_samples, int *num_sounds, char *sample_name_mask);
bool Script_GetOverridedSample(lua_State *lua, int sound_id, int *first_sample_number, int *samples_count);

int  Script_GetGlobalSound(lua_State *lua, int global_sound_id);
int  Script_GetSecretTrackNumber(lua_State *lua);
int  Script_GetNumTracks(lua_State *lua);
bool Script_GetSoundtrack(lua_State *lua, int track_index, char *track_path, int file_path_len, int *load_method, int *stream_type);
bool Script_GetLoadingScreen(lua_State *lua, int level_index, char *pic_path);
bool Script_GetString(lua_State *lua, int string_index, size_t string_size, char *buffer);

void Script_LoopEntity(lua_State *lua, int object_id);
int  Script_ExecEntity(lua_State *lua, int id_callback, int id_object, int id_activator = -1);
size_t Script_GetEntitySaveData(lua_State *lua, int id_entity, char *buf, size_t buf_size);
int  Script_DoTasks(lua_State *lua, float time);
bool Script_CallVoidFunc(lua_State *lua, const char* func_name, bool destroy_after_call = false);

void Script_AddKey(lua_State *lua, int keycode, int state);

#endif
