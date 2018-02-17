
#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>

#define FLIP_STATE_OFF      (0x00)
#define FLIP_STATE_ON       (0x01)
#define FLIP_STATE_BY_FLAG  (0x03)


void World_Prepare();
void World_Open(const char *path, int trv);
void World_Clear();
int  World_GetVersion();

uint32_t World_SpawnEntity(uint32_t model_id, uint32_t room_id, float pos[3], float ang[3], int32_t id);
struct entity_s *World_GetEntityByID(uint32_t id);
void World_SetPlayer(struct entity_s *entity);
struct entity_s *World_GetPlayer();
void World_IterateAllEntities(int (*iterator)(struct entity_s *ent, void *data), void *data);
struct flyby_camera_sequence_s *World_GetFlyBySequences();
struct base_item_s *World_GetBaseItemByID(uint32_t id);
struct base_item_s *World_GetBaseItemByWorldModelID(uint32_t id);
struct static_camera_sink_s *World_GetStaticCameraSink(uint32_t id);
struct camera_frame_s *World_GetCinematicFrame(uint32_t id);

void World_GetSkeletalModelsInfo(struct skeletal_model_s **models, uint32_t *models_count);
void World_GetRoomInfo(struct room_s **rooms, uint32_t *rooms_count);
void World_GetAnimSeqInfo(struct anim_seq_s **seq, uint32_t *seq_count);
void World_GetFlipInfo(uint8_t **flip_map, uint8_t **flip_state, uint32_t *flip_count);

int World_AddAnimSeq(struct anim_seq_s *seq);
int World_AddEntity(struct entity_s *entity);
int World_DeleteEntity(uint32_t id);
int World_CreateItem(uint32_t item_id, uint32_t model_id, uint32_t world_model_id, uint16_t type, uint16_t count, const char *name);
int World_DeleteItem(uint32_t item_id);
struct base_mesh_s *World_GetMeshByID(uint32_t ID);
struct sprite_s *World_GetSpriteByID(uint32_t ID);
struct skeletal_model_s *World_GetModelByID(uint32_t id);
struct skeletal_model_s* World_GetSkybox();

struct room_s *World_GetRoomByID(uint32_t id);
struct room_s *World_FindRoomByPos(float pos[3]);
struct room_s *World_FindRoomByPosCogerrence(float pos[3], struct room_s *old_room);
struct room_sector_s *World_GetRoomSector(int room_id, int x, int y);
uint32_t World_GetRoomBoxesCount();
struct room_box_s *World_GetRoomBoxByID(uint32_t id);

uint16_t World_GetGlobalFlipState();
void World_SetGlobalFlipState(int flip_state);
int World_SetFlipState(uint32_t flip_index, uint32_t flip_state);
int World_SetFlipMap(uint32_t flip_index, uint8_t flip_mask, uint8_t flip_operation);
void World_UpdateFlipCollisions();
uint32_t World_GetFlipMap(uint32_t flip_index);
uint32_t World_GetFlipState(uint32_t flip_index);


#endif
