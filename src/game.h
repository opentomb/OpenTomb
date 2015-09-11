
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
struct RedBlackNode_s;

void Game_InitGlobals();
void Game_RegisterLuaFunctions(lua_State *lua);
int  Game_Load(const char* name);
int  Game_Save(const char* name);

void  Game_Frame(float time);

void Game_Prepare();
void Game_LevelTransition(uint16_t level_index);

void Game_ApplyControls(struct entity_s *ent);

void Game_UpdateAllEntities(struct RedBlackNode_s *x);
void Game_LoopEntities(struct RedBlackNode_s *x);
void Game_UpdateAI();
void Game_UpdateCharacters();

void Cam_FollowEntity(struct camera_s *cam, struct entity_s *ent, float dx, float dz);

#endif

