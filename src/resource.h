
#ifndef RESOURCE_H
#define RESOURCE_H

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
struct RedBlackNode_s;
struct base_mesh_s;
struct world_s;
struct room_s;
struct room_sector_s;
struct sector_tween_s;

// NOTE: Functions which take native TR level structures as argument will have
// additional _TR_ prefix. Functions which doesn't use specific TR structures
// should NOT use such prefix!

void Res_GenSpritesBuffer(struct world_s *world);
void Res_GenRoomSpritesBuffer(struct room_s *room);
void Res_GenRoomCollision(struct world_s *world);
void Res_GenRoomFlipMap(struct world_s *world);
void Res_GenBaseItems(struct world_s *world);
void Res_GenVBOs(struct world_s *world);

void     Res_Sector_GenTweens(struct room_s *room, struct sector_tween_s *room_tween);
uint32_t Res_Sector_BiggestCorner(uint32_t v1,uint32_t v2,uint32_t v3,uint32_t v4);
void     Res_Sector_SetTweenFloorConfig(struct sector_tween_s *tween);
void     Res_Sector_SetTweenCeilingConfig(struct sector_tween_s *tween);
int      Res_Sector_IsWall(struct room_sector_s *wall_sector, struct room_sector_s *near_sector);

void     Res_Poly_SortInMesh(struct base_mesh_s *mesh);
void     TR_GenAnimCommands(struct world_s *world, class VT_Level *tr);
bool     Res_Poly_SetAnimTexture(struct polygon_s *polygon, uint32_t tex_index, struct world_s *world);

void     Res_FixRooms(struct world_s *world);   // Fix start-up room states.

struct   skeletal_model_s* Res_GetSkybox(struct world_s *world, uint32_t engine_version);

// Assign pickup functions to previously created base items.

void Res_EntityToItem(struct RedBlackNode_s *n);

// Functions setting parameters from configuration scripts.

void Res_GenEntityFunctions(struct RedBlackNode_s *x);
void Res_SetEntityModelProperties(struct entity_s *ent);
void Res_SetStaticMeshProperties(struct static_mesh_s *r_static);

// Check if entity index was already processed (needed to remove dublicated activation calls).
// If entity is not processed, add its index into lookup table.

bool Res_IsEntityProcessed(uint16_t *lookup_table, uint16_t entity_index);

// Open/close scripts.

void Res_ScriptsOpen(int engine_version);
void Res_ScriptsClose();
void Res_AutoexecOpen(int engine_version);


// Functions generating native OpenTomb structs from legacy TR structs.

void TR_GenMeshes(struct world_s *world, class VT_Level *tr);
void TR_GenMesh(struct world_s *world, size_t mesh_index, struct base_mesh_s *mesh, class VT_Level *tr);
void TR_GenRoomMesh(struct world_s *world, size_t room_index, struct room_s *room, class VT_Level *tr);
void TR_GenSkeletalModels(struct world_s *world, class VT_Level *tr);
void TR_GenSkeletalModel(size_t model_id, struct skeletal_model_s *model, class VT_Level *tr);
void TR_GenEntities(struct world_s *world, class VT_Level *tr);
void TR_GenSprites(struct world_s *world, class VT_Level *tr);
void TR_GenTextures(struct world_s *world, class VT_Level *tr);
void TR_GenAnimCommands(struct world_s *world, class VT_Level *tr);
void TR_GenAnimTextures(struct world_s *world, class VT_Level *tr);
void TR_GenRooms(struct world_s *world, class VT_Level *tr);
void TR_GenRoom(size_t room_index, struct room_s *room, struct world_s *world, class VT_Level *tr);
void TR_GenRoomProperties(struct world_s *world, class VT_Level *tr);
void TR_GenBoxes(struct world_s *world, class VT_Level *tr);
void TR_GenCameras(struct world_s *world, class VT_Level *tr);
void TR_GenSamples(struct world_s *world, class VT_Level *tr);

#endif
