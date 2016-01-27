#pragma once

#include "util/helpers.h"

#include <boost/optional.hpp>

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
class Room;
struct RoomSector;
class World;
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

void Game_InitGlobals(world::World& world);
void Game_RegisterLuaFunctions(script::ScriptEngine &state);
bool Game_Load(world::World& world, const std::string& name);
bool Game_Save(world::World& world, const std::string& name);

util::Duration Game_Tick(util::Duration* game_logic_time);
void Game_Frame(world::World& world, util::Duration time);

void Game_Prepare(world::World& world);
void Game_LevelTransition(world::World& world, const boost::optional<int>& level);

void Game_ApplyControls(world::World& world);

void Game_UpdateAI();
} // namespace engine
