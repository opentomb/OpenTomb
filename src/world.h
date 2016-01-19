
#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>

/*struct room_s;
struct base_item_s;
struct entity_s;
struct skeletal_model_s;*/


void World_Prepare();
void World_Open(class VT_Level *tr);
void World_Clear();
int World_GetVersion();
uint32_t World_SpawnEntity(uint32_t model_id, uint32_t room_id, float pos[3], float ang[3], int32_t id);
struct entity_s *World_GetEntityByID(uint32_t id);
struct entity_s *World_GetPlayer();
struct RedBlackNode_s *World_GetEntityTreeRoot();
struct flyby_camera_sequence_s *World_GetFlyBySequences();
struct base_item_s *World_GetBaseItemByID(uint32_t id);
struct static_camera_sink_s *World_GetstaticCameraSink(uint32_t id);
int16_t *World_GetAnimCommands();

void World_GetRoomInfo(struct room_s **rooms, uint32_t *rooms_count);
void World_GetAnimSeqInfo(struct anim_seq_s **seq, uint32_t *seq_count);
void World_GetFlipInfo(uint8_t **flip_map, uint8_t **flip_state, uint32_t *flip_count);

int World_AddAnimSeq(struct anim_seq_s *seq);
int World_AddEntity(struct entity_s *entity);
int World_DeleteEntity(struct entity_s *entity);
int World_CreateItem(uint32_t item_id, uint32_t model_id, uint32_t world_model_id, uint16_t type, uint16_t count, const char *name);
int World_DeleteItem(uint32_t item_id);
struct sprite_s *World_GetSpriteByID(uint32_t ID);
struct skeletal_model_s *World_GetModelByID(uint32_t id);        // binary search the model by ID
struct skeletal_model_s* World_GetSkybox();

struct room_s *World_FindRoomByPos(float pos[3]);
struct room_s *World_GetRoomByID(uint32_t id);
struct room_s *World_FindRoomByPosCogerrence(float pos[3], struct room_s *old_room);
struct room_sector_s *World_GetRoomSector(int room_id, int x, int y);

void World_BuildNearRoomsList(struct room_s *room);
void World_BuildOverlappedRoomsList(struct room_s *room);

int World_SetFlipState(uint32_t flip_index, uint32_t flip_state);
int World_SetFlipMap(uint32_t flip_index, uint8_t flip_mask, uint8_t flip_operation);
uint32_t World_GetFlipMap(uint32_t flip_index);
uint32_t World_GetFlipState(uint32_t flip_index);


#endif
