
#ifndef PARSE_H
#define PARSE_H

struct screen_info_s;
struct console_info_s;
struct entity_s;
struct lua_State;

#define CVAR_NAME_SIZE 32
#define CVAR_LUA_TABLE_NAME "cvars"

#include "bullet/LinearMath/btScalar.h"

extern lua_State *engine_lua;

int CVAR_Register(const char *name, const char *val);
int CVAR_Delete(const char *name);

btScalar CVAR_get_val_d(const char *key);
const char *CVAR_get_val_s(const char *key);
int CVAR_set_val_d(const char *key, btScalar val);
int CVAR_set_val_s(const char *key, const char *val);

char *parse_token(char *data, char *token);

float SC_ParseFloat(char **ch);
int SC_ParseInt(char **ch);

int lua_ParseScreen(lua_State *lua, struct screen_info_s *sc);
int lua_ParseRender(lua_State *lua, struct render_settings_s *rs);
int lua_ParseAudio(lua_State *lua, struct audio_settings_s *as);
int lua_ParseConsole(lua_State *lua, struct console_info_s *cn);
int lua_ParseControlSettings(lua_State *lua, struct control_settings_s *cs);

int SC_ParseEntity(char **ch, struct entity_s *ent);

bool lua_GetSoundtrack(lua_State *lua, int track_index, char *track_path, int *load_method, int *stream_type);

btScalar lua_GetScalarField(lua_State *lua, const char *key);
int lua_SetScalarField(lua_State *lua, const char *key, btScalar val);
const char *lua_GetStrField(lua_State *lua, const char *key);
int lua_SetStrField(lua_State *lua, const char *key, const char *val);

int lua_AclivateEntity(lua_State *lua, int id_object, int id_activator);
int lua_DoTasks(lua_State *lua, btScalar time);

#endif
