
#ifndef RESOURCE_H
#define RESOURCE_H

#define TR_METERING_STEP        (256.0)
#define TR_METERING_SECTORSIZE  (1024.0)

#define TR_PENETRATION_CONFIG_SOLID             0
#define TR_PENETRATION_CONFIG_DOOR_VERTICAL_A   1
#define TR_PENETRATION_CONFIG_DOOR_VERTICAL_B   2
#define TR_PENETRATION_CONFIG_WALL              3
#define TR_PENETRATION_CONFIG_GHOST             4

#define TR_SECTOR_DIAGONAL_TYPE_NONE            0
#define TR_SECTOR_DIAGONAL_TYPE_NE              1
#define TR_SECTOR_DIAGONAL_TYPE_NW              2

#define TR_SECTOR_TWEEN_TYPE_NONE               0
#define TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT     1
#define TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT      2
#define TR_SECTOR_TWEEN_TYPE_QUAD               3


#define TR_ITEM_SKYBOX_TR2 254
#define TR_ITEM_SKYBOX_TR3 355
#define TR_ITEM_SKYBOX_TR4 459
#define TR_ITEM_SKYBOX_TR5 454

#define TR_ITEM_LARA_SKIN_ALTERNATE_TR1    5
#define TR_ITEM_LARA_SKIN_TR3            315
#define TR_ITEM_LARA_SKIN_TR45             8
#define TR_ITEM_LARA_SKIN_JOINTS_TR45      9


#define LOG_ANIM_DISPATCHES 0

class VT_Level;
struct base_mesh_s;
struct world_s;
struct room_s;
struct room_sector_s;
struct sector_tween_s;
struct bordered_texture_atlas_s;

void TR_GenRoomMesh(struct world_s *world, size_t room_index, struct room_s *room, class VT_Level *tr);
void TR_GenMesh(struct world_s *world, size_t mesh_index, struct base_mesh_s *mesh, class VT_Level *tr);
void GenSkeletalModel(size_t model_id, struct skeletal_model_s *model, class VT_Level *tr);
void GenSkeletalModels(struct world_s *world, class VT_Level *tr);
void GenEntitys(struct world_s *world, class VT_Level *tr);
void TR_GenSprites(struct world_s *world, class VT_Level *tr);
void TR_GenAnimTextures(struct world_s *world, class VT_Level *tr);
void TR_GenRoom(size_t room_index, struct room_s *room, struct world_s *world, class VT_Level *tr);
void TR_GenWorld(struct world_s *world, class VT_Level *tr);

//----

btCollisionShape *MeshToBTCS(struct base_mesh_s *mesh, bool useCompression, bool buildBvh, int cflag);
btCollisionShape *HeightmapToBTCS(struct room_sector_s *heightmap, int heightmap_size, struct sector_tween_s *tweens, int tweens_size, bool useCompression, bool buildBvh);

void Gen_EntityRigidBody(entity_p ent);

void TR_vertex_to_arr(btScalar v[3], tr5_vertex_t *tr_v);
void TR_color_to_arr(btScalar v[4], tr5_colour_t *tr_c);

void SortPolygonsInMesh(struct base_mesh_s *mesh);

void GetBFrameBB_Pos(class VT_Level *tr, size_t frame_offset, bone_frame_p bone_frame);
int  GetNumAnimationsForMoveable(class VT_Level *tr, size_t moveable_ind);
int  GetNumFramesForAnimation(class VT_Level *tr, size_t animation_ind);
void RoomCalculateSectorData(struct world_s *world, class VT_Level *tr, long int room_index);


void GenerateTweens(struct room_s *room, struct sector_tween_s *room_tween);

//-----

bool SetAnimTexture(struct polygon_s *polygon, uint32_t tex_index, struct world_s *world);
int  ParseFloorData(struct room_sector_s *sector, struct world_s *world);


// Helper function to extract biggest triangulated sector corner.
// It's needed, as TRosettaStone algorhithm for ceiling height calculation is wrong.

int BiggestCorner(uint32_t v1,uint32_t v2,uint32_t v3,uint32_t v4);

#endif

