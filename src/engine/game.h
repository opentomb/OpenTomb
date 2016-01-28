#pragma once

#include "util/helpers.h"

#include <boost/optional.hpp>

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
class Engine;

void Game_InitGlobals(Engine& engine);
void Game_RegisterLuaFunctions(script::ScriptEngine &state);
bool Game_Load(Engine& engine, const std::string& name);
bool Game_Save(Engine& engine, const std::string& name);

util::Duration Game_Tick(util::Duration* game_logic_time);
void Game_Frame(Engine& engine, util::Duration time);

void Game_Prepare(Engine& engine);
void Game_LevelTransition(Engine& engine, const boost::optional<int>& level);

void Game_ApplyControls(Engine& engine);

void Game_UpdateAI();
} // namespace engine
