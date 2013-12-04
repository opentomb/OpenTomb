
#ifndef RESOURCE_H
#define RESOURCE_H

class VT_Level;
struct base_mesh_s;
struct world_s;
struct room_s;
struct room_sector_s;
struct bordered_texture_atlas_s;

void TR_GenRoomMesh(size_t room_index, struct room_s *room, struct bordered_texture_atlas_s *atlas, class VT_Level *tr);
void TR_GenMesh(size_t mesh_index, struct base_mesh_s *mesh, struct bordered_texture_atlas_s *atlas, class VT_Level *tr);
void GenSkeletalModel(size_t model_id, struct skeletal_model_s *model, class VT_Level *tr);
void GenSkeletalModels(struct world_s *world, class VT_Level *tr);
void GenEntitys(struct world_s *world, class VT_Level *tr);
void TR_GenSprites(struct world_s *world, struct bordered_texture_atlas_s *atlas, class VT_Level *tr);
void TR_GenRoom(size_t room_index, struct room_s *room, struct world_s *world, struct bordered_texture_atlas_s *atlas, class VT_Level *tr);
void TR_GenWorld(struct world_s *world, class VT_Level *tr);

int ParseFloorData(struct room_sector_s *sector, struct world_s *world);
#endif

