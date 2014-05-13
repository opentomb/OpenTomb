
#ifndef RESOURCE_H
#define RESOURCE_H

#define TR_ITEM_SKYBOX_TR2 254
#define TR_ITEM_SKYBOX_TR3 355
#define TR_ITEM_SKYBOX_TR4 459
#define TR_ITEM_SKYBOX_TR5 454

#define TR_ITEM_LARA_SKIN_HOME_TR1    5
#define TR_ITEM_LARA_SKIN_TR3         315
#define TR_ITEM_LARA_SKIN_TR45        8
#define TR_ITEM_LARA_SKIN_JOINTS_TR45 9

class VT_Level;
struct base_mesh_s;
struct world_s;
struct room_s;
struct room_sector_s;
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

bool SetAnimTexture(struct polygon_s *polygon, uint32_t tex_index, struct world_s *world);
int  ParseFloorData(struct room_sector_s *sector, struct world_s *world);

#endif

