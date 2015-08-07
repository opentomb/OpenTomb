#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <map>
#include <memory>

#include <LinearMath/btScalar.h>
#include <btBulletCollisionCommon.h>

// Original (canonical) TR frame rate.
// Needed for animation speed calculations.

#define TR_FRAME_RATE (30.0f)

// This is the global game logic refresh interval.
// All game logic should be refreshed at this rate, including
// enemy AI, values processing and audio update.

#define GAME_LOGIC_REFRESH_INTERVAL (1.0f / 60.0f)

namespace script
{
    class ScriptEngine;
}

class VT_Level;
struct Polygon;
struct BaseMesh;
struct Room;
struct World;
struct Camera;
struct Entity;
struct RoomSector;

class BtEngineClosestConvexResultCallback;

extern btVector3 cam_angles;

void Game_InitGlobals();
void Game_RegisterLuaFunctions(script::ScriptEngine &state);
int  Game_Load(const char* name);
int  Game_Save(const char* name);

btScalar Game_Tick(btScalar *game_logic_time);
void     Game_Frame(btScalar time);

void Game_Prepare();
void Game_LevelTransition(uint16_t level_index);

void Game_ApplyControls(std::shared_ptr<Entity> ent);

void Game_UpdateAllEntities(std::map<uint32_t, std::shared_ptr<Entity> >& entities);
void Game_LoopEntities(std::map<uint32_t, std::shared_ptr<Entity> >& entities);
void Game_UpdateAI();
void Game_UpdateCharacters();

void Cam_FollowEntity(struct Camera *cam, struct Entity *ent, btScalar dx, btScalar dz);
bool Cam_HasHit(BtEngineClosestConvexResultCallback *cb, btTransform &cameraFrom, btTransform &cameraTo);

#endif
