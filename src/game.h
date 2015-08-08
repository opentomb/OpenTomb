
#ifndef GAME_H
#define GAME_H

#include "bullet/LinearMath/btScalar.h"
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"

#include <stdint.h>
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

// This is the global game logic refresh interval.
// All game logic should be refreshed at this rate, including
// enemy AI, values processing and audio update.

#define GAME_LOGIC_REFRESH_INTERVAL (1.0 / 60.0)

class VT_Level;
struct polygon_s;
struct base_mesh_s;
struct room_s;
struct world_s;
struct camera_s;
struct entity_s;
struct room_sector_s;
struct RedBlackNode_s;

class bt_engine_ClosestConvexResultCallback;

extern btScalar cam_angles[3];

void Game_InitGlobals();
void Game_RegisterLuaFunctions(lua_State *lua);
int  Game_Load(const char* name);
int  Game_Save(const char* name);

btScalar Game_Tick(btScalar *game_logic_time);
void     Game_Frame(btScalar time);

void Game_Prepare();
void Game_LevelTransition(uint16_t level_index);

void Game_ApplyControls(struct entity_s *ent);

void Game_UpdateAllEntities(struct RedBlackNode_s *x);
void Game_LoopEntities(struct RedBlackNode_s *x);
void Game_UpdateAI();
void Game_UpdateCharacters();

void Cam_FollowEntity(struct camera_s *cam, struct entity_s *ent, btScalar dx, btScalar dz);

#endif

