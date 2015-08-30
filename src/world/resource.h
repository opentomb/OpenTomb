#pragma once

#include <map>
#include <memory>
#include <vector>

#include "world/entity.h"
#include "world/world.h"

namespace world
{

namespace
{
// Metering step and sector size are basic Tomb Raider world metrics.
// Use these defines at all times, when you're referencing classic TR
// dimensions and terrain manipulations.
constexpr float MeteringStep = 256.0f;
constexpr float MeteringSectorSize = 1024.0f;

// Wall height is a magical constant which specifies that sector with such
// height contains impassable wall.
constexpr int MeteringWallHeight = 32512;
}

// Penetration configuration specifies collision type for floor and ceiling
// sectors (squares).
enum class PenetrationConfig
{
    Solid,         //!< Ordinary sector.
    DoorVerticalA, //!< TR3-5 triangulated door.
    DoorVerticalB, //!< TR3-5 triangulated door.
    Wall,          //!< Wall (0x81 == TR_METERING_WALLHEIGHT)
    Ghost          //!< No collision.
};

// There are two types of diagonal splits - we call them north-east (NE) and
// north-west (NW). In case there is no diagonal in sector (TR1-2 classic sector),
// then NONE type is used.
enum class DiagonalType
{
    None,
    NE,
    NW
};

///@FIXME: Move skybox item IDs to script!

#define TR_ITEM_SKYBOX_TR2 254
#define TR_ITEM_SKYBOX_TR3 355
#define TR_ITEM_SKYBOX_TR4 459
#define TR_ITEM_SKYBOX_TR5 454

///@FIXME: Move Lara skin item IDs to script!

#define TR_ITEM_LARA_SKIN_ALTERNATE_TR1    5
#define TR_ITEM_LARA_SKIN_TR3            315
#define TR_ITEM_LARA_SKIN_TR45             8
#define TR_ITEM_LARA_SKIN_JOINTS_TR45      9

#define LOG_ANIM_DISPATCHES 0


struct SectorTween;
struct World;
struct Room;
struct RoomSector;

namespace core
{
struct BaseMesh;
struct SkeletalModel;
} // namespace core

// NOTE: Functions which take native TR level structures as argument will have
// additional _TR_ prefix. Functions which doesn't use specific TR structures
// should NOT use such prefix!

void Res_GenRBTrees(world::World *world);
void Res_GenSpritesBuffer(world::World *world);
void Res_GenRoomSpritesBuffer(std::shared_ptr<world::Room> room);
void Res_GenRoomCollision(world::World *world);
void Res_GenRoomFlipMap(world::World *world);
void Res_GenBaseItems(world::World *world);
void Res_GenVBOs(world::World *world);

uint32_t Res_Sector_BiggestCorner(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4);
void     Res_Sector_SetTweenFloorConfig(world::SectorTween *tween);
void     Res_Sector_SetTweenCeilingConfig(world::SectorTween *tween);
bool     Res_Sector_IsWall(world::RoomSector* ws, world::RoomSector* ns);
void     Res_Sector_FixHeights(world::RoomSector* sector);

bool     Res_Poly_SetAnimTexture(world::core::Polygon *polygon, uint32_t tex_index, world::World *world);

void     Res_FixRooms(world::World *world);   // Fix start-up room states.

world::core::SkeletalModel* Res_GetSkybox(world::World *world, loader::Engine engine_version);

// Create entity function from script, if exists.

void Res_SetEntityFunction(std::shared_ptr<world::Entity> ent);
void Res_CreateEntityFunc(script::ScriptEngine &lua, const std::string &func_name, int entity_id);
void Res_GenEntityFunctions(std::map<uint32_t, std::shared_ptr<world::Entity>>& entities);

// Assign pickup functions to previously created base items.

void Res_EntityToItem(std::map<uint32_t, std::shared_ptr<world::BaseItem> > &map);

// Functions setting parameters from configuration scripts.

void Res_SetEntityProperties(std::shared_ptr<world::Entity> ent);
void Res_SetStaticMeshProperties(std::shared_ptr<world::StaticMesh> r_static);

// Check if entity index was already processed (needed to remove dublicated activation calls).
// If entity is not processed, add its index into lookup table.

bool Res_IsEntityProcessed(uint16_t *lookup_table, uint16_t entity_index);

// Open autoexec.

void Res_AutoexecOpen(loader::Game engine_version);

// Functions generating native OpenTomb structs from legacy TR structs.

void TR_GenWorld(world::World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenMeshes(world::World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenMesh(world::World *world, size_t mesh_index, std::shared_ptr<world::core::BaseMesh> mesh, const std::unique_ptr<loader::Level>& tr);
void TR_GenSkeletalModels(world::World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenSkeletalModel(size_t model_id, world::core::SkeletalModel *model, const std::unique_ptr<loader::Level>& tr);
void TR_GenEntities(world::World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenSprites(world::World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenTextures(world::World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenAnimCommands(world::World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenAnimTextures(world::World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenRooms(world::World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenRoom(uint32_t room_index, std::shared_ptr<world::Room>& room, world::World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenRoomProperties(world::World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenBoxes(world::World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenCameras(world::World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenSamples(world::World *world, const std::unique_ptr<loader::Level>& tr);

// Helper functions to convert legacy TR structs to native OpenTomb structs.

void TR_vertex_to_arr(btVector3 &v, const loader::Vertex &tr_v);
void TR_color_to_arr(std::array<GLfloat, 4> &v, const loader::FloatColor &tr_c);

// Functions for getting various parameters from legacy TR structs.

void     TR_GetBFrameBB_Pos(const std::unique_ptr<loader::Level>& tr, size_t frame_offset, world::animation::BoneFrame* bone_frame);
size_t   TR_GetNumAnimationsForMoveable(const std::unique_ptr<loader::Level>& tr, size_t moveable_ind);
size_t   TR_GetNumFramesForAnimation(const std::unique_ptr<loader::Level>& tr, size_t animation_ind);
long int TR_GetOriginalAnimationFrameOffset(uint32_t offset, uint32_t anim, const std::unique_ptr<loader::Level>& tr);

// Main functions which are used to translate legacy TR floor data
// to native OpenTomb structs.

int      TR_Sector_TranslateFloorData(world::RoomSector* sector, const std::unique_ptr<loader::Level>& tr);
void     TR_Sector_Calculate(world::World *world, const std::unique_ptr<loader::Level>& tr, long int room_index);

void tr_setupRoomVertices(world::World *world, const std::unique_ptr<loader::Level>& tr, loader::Room *tr_room, const std::shared_ptr<world::core::BaseMesh>& mesh, int numCorners, const uint16_t *vertices, uint16_t masked_texture, world::core::Polygon *p);
void tr_copyNormals(world::core::Polygon *polygon, const std::shared_ptr<world::core::BaseMesh>& mesh, const uint16_t *mesh_vertex_indices);

} // namespace world
