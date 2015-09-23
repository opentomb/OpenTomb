
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
struct base_mesh_s;
struct room_s;
struct room_sector_s;
struct sector_tween_s;


void     Res_Sector_GenTweens(struct room_s *room, struct sector_tween_s *room_tween);
bool     Res_SetAnimTexture(struct polygon_s *polygon, uint32_t tex_index, struct anim_seq_s *anim_sequences, uint32_t anim_sequences_count);

int  Res_Sector_TranslateFloorData(struct room_s *rooms, uint32_t rooms_count, struct room_sector_s *sector, class VT_Level *tr);
int  Res_Sector_In2SideOfPortal(struct room_sector_s *s1, struct room_sector_s *s2, struct portal_s *p);
void Res_RoomSectorsCalculate(struct room_s *rooms, uint32_t rooms_count, uint32_t room_index, class VT_Level *tr);

// Functions generating native OpenTomb structs from legacy TR structs.
void TR_GenMesh(struct base_mesh_s *mesh, size_t mesh_index, struct anim_seq_s *anim_sequences, uint32_t anim_sequences_count, class bordered_texture_atlas *atlas, class VT_Level *tr);
void TR_GenRoomMesh(struct room_s *room, size_t room_index, struct anim_seq_s *anim_sequences, uint32_t anim_sequences_count, class bordered_texture_atlas *atlas, class VT_Level *tr);
void TR_GenSkeletalModel(struct skeletal_model_s *model, size_t model_id, struct base_mesh_s *base_mesh_array, int16_t *base_anim_commands_array, class VT_Level *tr);

#endif
