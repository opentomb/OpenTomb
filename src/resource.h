#ifndef RESOURCE_H
#define RESOURCE_H

#include <map>
#include <memory>

// Here you can specify the way OpenTomb processes room collision -
// in a classic TR way (floor data collision) or in a modern way
// (derived from actual room mesh).

#define TR_MESH_ROOM_COLLISION 0

// Metering step and sector size are basic Tomb Raider world metrics.
// Use these defines at all times, when you're referencing classic TR
// dimensions and terrain manipulations.

#define TR_METERING_STEP        (256.0)
#define TR_METERING_SECTORSIZE  (1024.0)

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

// Tween is a short word for "inbeTWEEN vertical polygon", which is needed to fill
// the gap between two sectors with different heights. If adjacent sector heights are
// similar, it means that tween is degenerated (doesn't exist physically) - in that
// case we use NONE type. If only one of two heights' pairs is similar, then tween is
// either right or left pointed triangle (where "left" or "right" is derived by viewing
// triangle from front side). If none of the heights are similar, we need quad tween.

#define TR_SECTOR_TWEEN_TYPE_NONE               0   // Degenerated vertical polygon.
#define TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT     1   // Triangle pointing right (viewed front).
#define TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT      2   // Triangle pointing left (viewed front).
#define TR_SECTOR_TWEEN_TYPE_QUAD               3   // 
#define TR_SECTOR_TWEEN_TYPE_2TRIANGLES         4   // it looks like a butterfly

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

class  VT_Level;
struct BaseMesh;
struct world_s;
struct Room;
struct room_sector_s;
struct sector_tween_s;
struct bordered_texture_atlas_s;

// NOTE: Functions which take native TR level structures as argument will have
// additional _TR_ prefix. Functions which doesn't use specific TR structures
// should NOT use such prefix!

void Res_GenRBTrees(world_s *world);
void Res_GenSpritesBuffer(world_s *world);
void Res_GenRoomSpritesBuffer(std::shared_ptr<Room> room);
void Res_GenRoomCollision(world_s *world);
void Res_GenRoomFlipMap(world_s *world);
void Res_GenBaseItems(world_s *world);
void Res_GenVBOs(world_s *world);

void     Res_Sector_GenTweens(std::shared_ptr<Room> room, sector_tween_s *room_tween);
uint32_t Res_Sector_BiggestCorner(uint32_t v1,uint32_t v2,uint32_t v3,uint32_t v4);
void     Res_Sector_SetTweenFloorConfig(sector_tween_s *tween);
void     Res_Sector_SetTweenCeilingConfig(sector_tween_s *tween);
int      Res_Sector_IsWall(room_sector_p ws, room_sector_p ns);

bool     Res_Poly_SetAnimTexture(polygon_s *polygon, uint32_t tex_index, world_s *world);

void     Res_FixRooms(world_s *world);   // Fix start-up room states.

struct SkeletalModel;
SkeletalModel* Res_GetSkybox(world_s *world, uint32_t engine_version);

// Create entity function from script, if exists.

bool Res_CreateEntityFunc(lua_State *lua, const char* func_name, int entity_id);

// Assign pickup functions to previously created base items.

void Res_EntityToItem(std::map<uint32_t, std::shared_ptr<base_item_s> > &map);

// Functions setting parameters from configuration scripts.

void Res_SetEntityModelProperties(std::shared_ptr<Entity> ent);
void Res_SetStaticMeshProperties(std::shared_ptr<StaticMesh> r_static);

// Check if entity index was already processed (needed to remove dublicated activation calls).
// If entity is not processed, add its index into lookup table.

bool Res_IsEntityProcessed(uint16_t *lookup_table, uint16_t entity_index);

// Open/close scripts.

void Res_ScriptsOpen(int engine_version);
void Res_ScriptsClose();
void Res_AutoexecOpen(int engine_version);


// Functions generating native OpenTomb structs from legacy TR structs.

void TR_GenWorld(world_s *world, class VT_Level *tr);
void TR_GenMeshes(world_s *world, class VT_Level *tr);
void TR_GenMesh(world_s *world, size_t mesh_index, std::shared_ptr<BaseMesh> mesh, class VT_Level *tr);
void TR_GenRoomMesh(world_s *world, size_t room_index, std::shared_ptr<Room> room, class VT_Level *tr);
void TR_GenSkeletalModels(world_s *world, class VT_Level *tr);
void TR_GenSkeletalModel(size_t model_id, SkeletalModel *model, class VT_Level *tr);
void TR_GenEntities(world_s *world, class VT_Level *tr);
void TR_GenSprites(world_s *world, class VT_Level *tr);
void TR_GenTextures(world_s *world, class VT_Level *tr);
void TR_GenAnimCommands(world_s *world, class VT_Level *tr);
void TR_GenAnimTextures(world_s *world, class VT_Level *tr);
void TR_GenRooms(world_s *world, class VT_Level *tr);
void TR_GenRoom(size_t room_index, std::shared_ptr<Room> room, world_s *world, class VT_Level *tr);
void TR_GenRoomProperties(world_s *world, class VT_Level *tr);
void TR_GenBoxes(world_s *world, class VT_Level *tr);
void TR_GenCameras(world_s *world, class VT_Level *tr);
void TR_GenSamples(world_s *world, class VT_Level *tr);

// Helper functions to convert legacy TR structs to native OpenTomb structs.

void TR_vertex_to_arr(btVector3 &v, const tr5_vertex_t &tr_v);
void TR_color_to_arr(std::array<GLfloat, 4> &v, const tr5_colour_t &tr_c);

// Functions for getting various parameters from legacy TR structs.

struct BoneFrame;

void     TR_GetBFrameBB_Pos(class VT_Level *tr, size_t frame_offset, BoneFrame* bone_frame);
int      TR_GetNumAnimationsForMoveable(class VT_Level *tr, size_t moveable_ind);
int      TR_GetNumFramesForAnimation(class VT_Level *tr, size_t animation_ind);
long int TR_GetOriginalAnimationFrameOffset(uint32_t offset, uint32_t anim, class VT_Level *tr);

// Main functions which are used to translate legacy TR floor data
// to native OpenTomb structs.

int      TR_Sector_TranslateFloorData(room_sector_p sector, class VT_Level *tr);
void     TR_Sector_Calculate(world_s *world, class VT_Level *tr, long int room_index);

#endif
