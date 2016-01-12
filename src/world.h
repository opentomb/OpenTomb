
#ifndef WORLD_H
#define WORLD_H

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <stdint.h>

extern "C" {
#include "al/AL/al.h"
#include "al/AL/alc.h"
#include "al/AL/alext.h"
#include "al/AL/efx-presets.h"
}

class  AudioSource;
class  StreamTrack;
class  bordered_texture_atlas;
struct room_s;
struct base_item_s;
struct base_mesh_s;
struct entity_s;
struct skeletal_model_s;
struct RedBlackHeader_s;

struct lua_State;


typedef struct world_s
{
    char                           *name;
    uint32_t                        id;
    uint32_t                        version;

    uint32_t                        rooms_count;
    struct room_s                  *rooms;

    uint32_t                        room_boxes_count;
    struct room_box_s              *room_boxes;
    
    uint32_t                        flip_count;             // Number of flips
    uint8_t                        *flip_map;               // Flipped room activity array.
    uint8_t                        *flip_state;             // Flipped room state array.

    bordered_texture_atlas         *tex_atlas;
    uint32_t                        tex_count;              // Number of textures
    GLuint                         *textures;               // OpenGL textures indexes

    uint32_t                        anim_sequences_count;   // Animated texture sequence count
    struct anim_seq_s              *anim_sequences;         // Animated textures

    uint32_t                        meshes_count;           // Base meshes count
    struct base_mesh_s             *meshes;                 // Base meshes data

    uint32_t                        sprites_count;          // Base sprites count
    struct sprite_s                *sprites;                // Base sprites data

    uint32_t                        skeletal_models_count;  // number of base skeletal models
    struct skeletal_model_s        *skeletal_models;        // base skeletal models data

    struct entity_s                *Character;              // this is an unique Lara's pointer =)
    struct skeletal_model_s        *sky_box;                // global skybox

    struct RedBlackHeader_s        *entity_tree;            // tree of world active objects
    struct RedBlackHeader_s        *items_tree;             // tree of world items

    uint32_t                        type;
    
    uint32_t                        cameras_sinks_count;    // Amount of cameras and sinks.
    struct static_camera_sink_s    *cameras_sinks;          // Cameras and sinks. 
    uint32_t                        flyby_cameras_count;
    struct flyby_camera_state_s    *flyby_cameras;
    struct flyby_camera_sequence_s *flyby_camera_sequences;
    
    uint32_t                        anim_commands_count;
    int16_t                        *anim_commands;

    uint32_t                        audio_emitters_count;   // Amount of audio emitters in level.
    struct audio_emitter_s         *audio_emitters;         // Audio emitters.

    uint32_t                        audio_map_count;        // Amount of overall effects in engine.
    int16_t                        *audio_map;              // Effect indexes.
    uint32_t                        audio_effects_count;    // Amount of available effects in level.
    struct audio_effect_s          *audio_effects;          // Effects and their parameters.

    uint32_t                        audio_buffers_count;    // Amount of samples.
    ALuint                         *audio_buffers;          // Samples.
    uint32_t                        audio_sources_count;    // Amount of runtime channels.
    AudioSource                    *audio_sources;          // Channels.

    uint32_t                        stream_tracks_count;    // Amount of stream track channels.
    StreamTrack                    *stream_tracks;          // Stream tracks.

    uint32_t                        stream_track_map_count; // Stream track flag map count.
    uint8_t                        *stream_track_map;       // Stream track flag map.
    
    /// private:
    struct lua_State               *objects_flags_conf;
    struct lua_State               *ent_ID_override;
    struct lua_State               *level_script;
}world_t, *world_p;


void World_Prepare(world_p world);
void World_Open(world_p world, class VT_Level *tr);
void World_Clear(world_p world);
uint32_t World_SpawnEntity(world_p world, uint32_t model_id, uint32_t room_id, float pos[3], float ang[3], int32_t id);
struct entity_s *World_GetEntityByID(world_p world, uint32_t id);
struct base_item_s *World_GetBaseItemByID(world_p world, uint32_t id);

int World_AddEntity(world_p world, struct entity_s *entity);
int World_DeleteEntity(world_p world, struct entity_s *entity);
int World_CreateItem(world_p world, uint32_t item_id, uint32_t model_id, uint32_t world_model_id, uint16_t type, uint16_t count, const char *name);
int World_DeleteItem(world_p world, uint32_t item_id);
struct sprite_s *World_GetSpriteByID(world_p world, uint32_t ID);
struct skeletal_model_s *World_GetModelByID(world_p world, uint32_t id);        // binary search the model by ID
struct skeletal_model_s* World_GetSkybox(world_p world);

struct room_s *World_FindRoomByPos(world_p world, float pos[3]);
struct room_s *World_FindRoomByPosCogerrence(world_p world, float pos[3], struct room_s *old_room);
struct room_sector_s *World_GetRoomSector(world_p world, int room_id, int x, int y);

void World_BuildNearRoomsList(world_p world, struct room_s *room);
void World_BuildOverlappedRoomsList(world_p world, struct room_s *room);

int World_SetFlipState(world_p world, uint32_t flip_index, uint32_t flip_state);
int World_SetFlipMap(world_p world, uint32_t flip_index, uint8_t flip_mask, uint8_t flip_operation);
uint32_t World_GetFlipMap(world_p world, uint32_t flip_index);
uint32_t World_GetFlipState(world_p world, uint32_t flip_index);


#endif
