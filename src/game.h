#pragma once

#include <cstdint>
#include <map>
#include <memory>

#include <LinearMath/btScalar.h>
#include <btBulletCollisionCommon.h>

// Original (canonical) TR frame rate.
// Needed for animation speed calculations.
#define TR_FRAME_RATE (30.0f)

// This is the global game logic refresh interval (physics timestep)
// All game logic should be refreshed at this rate, including
// enemy AI, values processing and audio update.
// This should be a multiple of TR_FRAME_RATE (1/30,60,90,120,...)
#define GAME_LOGIC_REFRESH_INTERVAL (1.0f / 60.0f)

// Max. number of game steps that are caught-up between
// rendering: This limits escalation if the system is too
// slow to keep up the logic interval.
#define MAX_SIM_SUBSTEPS (6)

namespace script
{
    class ScriptEngine;
}

class VT_Level;
struct Polygon;
struct BaseMesh;
struct Room;
struct World;
class Camera;
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

void Game_UpdateAI();

void Cam_FollowEntity(Camera *cam, struct Entity *ent, btScalar dx, btScalar dz);
bool Cam_HasHit(BtEngineClosestConvexResultCallback *cb, btTransform &cameraFrom, btTransform &cameraTo);
