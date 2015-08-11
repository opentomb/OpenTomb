#pragma once

#include <map>
#include <memory>
#include <vector>

#include "entity.h"
#include "world.h"

// Here you can specify the way OpenTomb processes room collision -
// in a classic TR way (floor data collision) or in a modern way
// (derived from actual room mesh).

#define TR_MESH_ROOM_COLLISION 0

// Metering step and sector size are basic Tomb Raider world metrics.
// Use these defines at all times, when you're referencing classic TR
// dimensions and terrain manipulations.

namespace
{
constexpr float TR_METERING_STEP = 256.0f;
constexpr float TR_METERING_SECTORSIZE = 1024.0f;
}

// Wall height is a magical constant which specifies that sector with such
// height contains impassable wall.

#define TR_METERING_WALLHEIGHT  (32512)

// Penetration configuration specifies collision type for floor and ceiling
// sectors (squares).

#define TR_PENETRATION_CONFIG_SOLID             0   // Ordinary sector.
#define TR_PENETRATION_CONFIG_DOOR_VERTICAL_A   1   // TR3-5 triangulated door.
#define TR_PENETRATION_CONFIG_DOOR_VERTICAL_B   2   // TR3-5 triangulated door.
#define TR_PENETRATION_CONFIG_WALL              3   // Wall (0x81 == TR_METERING_WALLHEIGHT)
#define TR_PENETRATION_CONFIG_GHOST             4   // No collision.

// There are two types of diagonal splits - we call them north-east (NE) and
// north-west (NW). In case there is no diagonal in sector (TR1-2 classic sector),
// then NONE type is used.

#define TR_SECTOR_DIAGONAL_TYPE_NONE            0
#define TR_SECTOR_DIAGONAL_TYPE_NE              1
#define TR_SECTOR_DIAGONAL_TYPE_NW              2

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

struct BaseMesh;
struct World;
struct Room;
struct RoomSector;
struct SectorTween;
struct bordered_texture_atlas_s;

// NOTE: Functions which take native TR level structures as argument will have
// additional _TR_ prefix. Functions which doesn't use specific TR structures
// should NOT use such prefix!

void Res_GenRBTrees(World *world);
void Res_GenSpritesBuffer(World *world);
void Res_GenRoomSpritesBuffer(std::shared_ptr<Room> room);
void Res_GenRoomCollision(World *world);
void Res_GenRoomFlipMap(World *world);
void Res_GenBaseItems(World *world);
void Res_GenVBOs(World *world);

uint32_t Res_Sector_BiggestCorner(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4);
void     Res_Sector_SetTweenFloorConfig(SectorTween *tween);
void     Res_Sector_SetTweenCeilingConfig(SectorTween *tween);
int      Res_Sector_IsWall(RoomSector* ws, RoomSector* ns);
void     Res_Sector_FixHeights(RoomSector* sector);

bool     Res_Poly_SetAnimTexture(struct Polygon *polygon, uint32_t tex_index, World *world);

void     Res_FixRooms(World *world);   // Fix start-up room states.

struct SkeletalModel;
SkeletalModel* Res_GetSkybox(World *world, loader::Game engine_version);

// Create entity function from script, if exists.

void Res_SetEntityFunction(std::shared_ptr<Entity> ent);
void Res_CreateEntityFunc(script::ScriptEngine &lua, const std::string &func_name, int entity_id);
void Res_GenEntityFunctions(std::map<uint32_t, std::shared_ptr<Entity>>& entities);

// Assign pickup functions to previously created base items.

void Res_EntityToItem(std::map<uint32_t, std::shared_ptr<BaseItem> > &map);

// Functions setting parameters from configuration scripts.

void Res_SetEntityProperties(std::shared_ptr<Entity> ent);
void Res_SetStaticMeshProperties(std::shared_ptr<StaticMesh> r_static);

// Check if entity index was already processed (needed to remove dublicated activation calls).
// If entity is not processed, add its index into lookup table.

bool Res_IsEntityProcessed(uint16_t *lookup_table, uint16_t entity_index);

// Open/close scripts.

void Res_ScriptsOpen(int engine_version);
void Res_ScriptsClose();
void Res_AutoexecOpen(int engine_version);

// Functions generating native OpenTomb structs from legacy TR structs.

void TR_GenWorld(World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenMeshes(World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenMesh(World *world, size_t mesh_index, std::shared_ptr<BaseMesh> mesh, const std::unique_ptr<loader::Level>& tr);
void TR_GenRoomMesh(World *world, size_t room_index, std::shared_ptr<Room> room, const std::unique_ptr<loader::Level>& tr);
void TR_GenSkeletalModels(World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenSkeletalModel(size_t model_id, SkeletalModel *model, const std::unique_ptr<loader::Level>& tr);
void TR_GenEntities(World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenSprites(World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenTextures(World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenAnimCommands(World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenAnimTextures(World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenRooms(World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenRoom(size_t room_index, std::shared_ptr<Room>& room, World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenRoomProperties(World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenBoxes(World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenCameras(World *world, const std::unique_ptr<loader::Level>& tr);
void TR_GenSamples(World *world, const std::unique_ptr<loader::Level>& tr);

// Helper functions to convert legacy TR structs to native OpenTomb structs.

void TR_vertex_to_arr(btVector3 &v, const Vertex &tr_v);
void TR_color_to_arr(std::array<GLfloat, 4> &v, const loader::FloatColor &tr_c);

// Functions for getting various parameters from legacy TR structs.

struct BoneFrame;

void     TR_GetBFrameBB_Pos(const std::unique_ptr<loader::Level>& tr, size_t frame_offset, BoneFrame* bone_frame);
int      TR_GetNumAnimationsForMoveable(const std::unique_ptr<loader::Level>& tr, size_t moveable_ind);
int      TR_GetNumFramesForAnimation(const std::unique_ptr<loader::Level>& tr, size_t animation_ind);
long int TR_GetOriginalAnimationFrameOffset(uint32_t offset, uint32_t anim, const std::unique_ptr<loader::Level>& tr);

// Main functions which are used to translate legacy TR floor data
// to native OpenTomb structs.

int      TR_Sector_TranslateFloorData(RoomSector* sector, const std::unique_ptr<loader::Level>& tr);
void     TR_Sector_Calculate(World *world, const std::unique_ptr<loader::Level>& tr, long int room_index);

