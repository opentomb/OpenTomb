
#ifndef GAME_H
#define GAME_H

#include <stdint.h>

// This is the global game logic refresh interval.
// All game logic should be refreshed at this rate, including
// enemy AI, values processing and audio update.

#define GAME_LOGIC_REFRESH_INTERVAL (1.0 / 60.0)

struct lua_State;
struct camera_s;
struct entity_s;

void Game_InitGlobals();
void Game_RegisterLuaFunctions(lua_State *lua);
int Game_Load(const char* name);
int Game_Save(const char* name);

void Game_Frame(float time);

void Game_Prepare();
void Game_LevelTransition(uint16_t level_index);

void Game_ApplyControls(struct entity_s *ent);

void Game_PlayFlyBy(uint32_t sequence_id, int once);
void Game_SetCameraTarget(uint32_t entity_id);
void Game_SetCamera(uint32_t camera_id, int once, int move, float timer);
void Game_StopFlyBy();

void Cam_PlayFlyBy(struct camera_state_s *cam_state, float time);
void Cam_FollowEntity(struct camera_s *cam, struct camera_state_s *cam_state, struct entity_s *ent);

#endif

