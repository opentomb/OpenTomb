#pragma once

#include "util/helpers.h"

#include <cstdint>
#include <memory>
#include <string>

// Max. number of game steps that are caught-up between
// rendering: This limits escalation if the system is too
// slow to keep up the logic interval.
#define MAX_SIM_SUBSTEPS (6)

namespace script
{
    class ScriptEngine;
}

namespace world
{
struct Room;
struct RoomSector;
struct World;
class Camera;
class Entity;
namespace core
{
struct Polygon;
} // namespace core
} // namespace world

namespace engine
{

class BtEngineClosestConvexResultCallback;

void Game_InitGlobals();
void Game_RegisterLuaFunctions(script::ScriptEngine &state);
int  Game_Load(const char* name);
bool Game_Save(const std::string& name);

util::Duration Game_Tick(util::Duration* game_logic_time);
void     Game_Frame(util::Duration time);

void Game_Prepare();
void Game_LevelTransition(uint16_t level_index);

void Game_ApplyControls(std::shared_ptr<world::Entity> ent);

void Game_UpdateAI();

} // namespace engine
