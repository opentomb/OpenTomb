
#ifndef WORLD_H
#define WORLD_H

#include <SDL2/SDL_opengl.h>
#include <stdint.h>
#include "audio.h"
#include "bordered_texture_atlas.h"
#include "bullet/LinearMath/btScalar.h"

class btCollisionShape;
class btRigidBody;

struct room_s;
struct polygon_s;
struct camera_s;
struct portal_s;
struct render_s;
struct frustum_s;
struct base_mesh_s;
struct static_mesh_s;
struct entity_s;
struct skeletal_model_s;
struct render_object_list_s;
struct RedBlackHeader_s;
struct ss_bone_frame_s;


typedef struct base_item_s
{
    uint32_t                    id;
    uint32_t                    world_model_id;
    uint16_t                    type;
    uint16_t                    count;
    char                        name[64];
    struct ss_bone_frame_s     *bf;
}base_item_t, *base_item_p;

typedef struct room_box_s
{
    int32_t     x_min;
    int32_t     x_max;
    int32_t     y_min;
    int32_t     y_max;
    int32_t     true_floor;
    int32_t     overlap_index;
}room_box_t, *room_box_p;

typedef struct room_sector_s
{
    int32_t                     box_index;
    int32_t                     floor;
    int32_t                     ceiling;

    uint16_t                    fd_index;                                       // offset to the floor data

    struct room_sector_s        *sector_below;
    struct room_sector_s        *sector_above;
    struct room_s               *owner_room;                                    // room htat contain this sector

    int16_t                     index_x;
    int16_t                     index_y;
    btScalar                    pos_x;
    btScalar                    pos_y;
}room_sector_t, *room_sector_p;


typedef struct room_sprite_s
{
    struct sprite_s             *sprite;
    btScalar                    pos[3];
    int8_t                      was_rendered;
}room_sprite_t, *room_sprite_p;


typedef struct room_s
{
    uint32_t                    id;                                             // room's ID
    uint32_t                    flags;                                          // room's type + water, wind info
    uint8_t                     reverb_info;                                    // room reverb type
    uint8_t                     extra_param;

    int8_t                      active;                                         // flag: is active
    int8_t                      is_in_r_list;                                   // is room in render list
    int8_t                      hide;                                           // do not render
    struct base_mesh_s         *mesh;                                           // room's base mesh

    uint32_t                    static_mesh_count;
    struct static_mesh_s       *static_mesh;
    uint32_t                    sprites_count;
    struct room_sprite_s       *sprites;

    struct engine_container_s  *containers;                                     // engine containers with moveables objects

    btScalar                    bb_min[3];                                      // room's bounding box
    btScalar                    bb_max[3];                                      // room's bounding box
    btScalar                    transform[16];                                  // GL transformation matrix
    btScalar                    ambient_lighting[3];

    uint32_t                    light_count;
    struct light_s             *lights;

    uint16_t                    portal_count;                                   // number of room portals
    struct portal_s            *portals;                                        // room portals array
    struct room_s              *alternate_room;                                 // alternative room pointer

    uint32_t                    sectors_count;
    uint16_t                    sectors_x;
    uint16_t                    sectors_y;
    struct room_sector_s       *sectors;

    uint16_t                    active_frustums;                                // current number of this room active frustums
    struct frustum_s           *frustum;
    struct frustum_s           *last_frustum;
    uint16_t                    max_path;                                       // maximum number of portals from camera to this room

    uint16_t                    near_room_list_size;
    struct room_s               *near_room_list[64];
    btRigidBody                 *bt_body;

    struct engine_container_s   *self;
}room_t, *room_p;


typedef struct world_s
{
    char                       *name;
    uint32_t                    id;
    uint32_t                    version;

    uint32_t                    room_count;
    struct room_s              *rooms;

    uint32_t                    room_box_count;
    struct room_box_s          *room_boxes;


    bordered_texture_atlas_p    tex_atlas;
    uint32_t                    tex_count;              // Number of textures
    GLuint                     *textures;               // OpenGL textures indexes

    uint32_t                    anim_sequences_count;   // Animated texture sequence count
    struct anim_seq_s          *anim_sequences;         // Animated textures

    uint32_t                    meshs_count;            // Base meshes count
    struct base_mesh_s         *meshes;                 // Base meshes data

    uint32_t                    sprites_count;          // Base sprites count
    struct sprite_s            *sprites;                // Base sprites data

    uint32_t                    skeletal_model_count;   // number of base skeletal models
    struct skeletal_model_s    *skeletal_models;        // base skeletal models data

    struct entity_s            *Character;              // this is an unique Lara's pointer =)
    struct skeletal_model_s    *sky_box;                // global skybox

    struct RedBlackHeader_s    *entity_tree;            // tree of world active objects
    struct RedBlackHeader_s    *items_tree;             // tree of world items [key].ss_bone_frame
    
    uint32_t                    type;

    uint32_t                    floor_data_size;
    uint16_t                   *floor_data;

    uint32_t                    anim_commands_count;
    int16_t                    *anim_commands;

    uint32_t                    audio_emitters_count;   // Amount of audio emitters in level.
    struct audio_emitter_s     *audio_emitters;         // Audio emitters.

    uint32_t                    audio_map_count;        // Amount of overall effects in engine.
    int16_t                    *audio_map;              // Effect indexes.
    uint32_t                    audio_effects_count;    // Amount of available effects in level.
    struct audio_effect_s      *audio_effects;          // Effects and their parameters.

    uint32_t                    audio_buffers_count;    // Amount of samples.
    ALuint                     *audio_buffers;          // Samples.
    uint32_t                    audio_sources_count;    // Amount of runtime channels.
    AudioSource                *audio_sources;          // Channels.

    uint32_t                    stream_tracks_count;    // Amount of stream track channels.
    StreamTrack                *stream_tracks;          // Stream tracks.

    uint32_t                    stream_track_map_count; // Stream track flag map count.
    uint8_t                    *stream_track_map;       // Stream track flag map.
}world_t, *world_p;

void World_Prepare(world_p world);
void World_Empty(world_p world);
int compEntityEQ(void *x, void *y);
int compEntityLT(void *x, void *y);
void RBEntityFree(void *x);
void RBItemFree(void *x);
struct entity_s *World_GetEntityByID(world_p world, uint32_t id);
struct base_item_s *World_GetBaseItemByID(world_p world, uint32_t id);

void Room_Empty(room_p room);
void Room_AddEntity(room_p room, struct entity_s *entity);
int Room_RemoveEntity(room_p room, struct entity_s *entity);
void Room_AddToNearRoomsList(room_p room, room_p r);

int Room_IsPointIn(room_p room, btScalar dot[3]);
room_p Room_FindPos2d(world_p w, btScalar pos[3]);

room_p Room_FindPos(world_p w, btScalar pos[3]);
room_p Room_FindPosCogerrence(world_p w, btScalar pos[3], room_p room);
room_p Room_FindPosCogerrence2d(world_p w, btScalar pos[3], room_p room);
room_p Room_GetByID(world_p w, unsigned int ID);
room_sector_p Room_GetSector(room_p room, btScalar pos[3]);
room_sector_p Room_GetSectorXYZ(room_p room, btScalar pos[3]);

void Room_Enable(room_p room);
void Room_Disable(room_p room);
void Room_SwapAlternate(room_p room);
bool Room_IsAlternate(room_p room); //checks if input room is alternate return true/false
void Room_SwapPortals(room_p room, room_p dest_room);//Swap room portals of input room to destination room
void Room_SwapItems(room_p room, room_p dest_room);//Swap room items of input room to destination room
void Room_BuildNearRoomsList(room_p room);

int Room_IsJoined(room_p r1, room_p r2);
int Room_IsOverlapped(room_p r0, room_p r1);
int Room_IsInNearRoomsList(room_p room, room_p r);

int World_AddEntity(world_p world, struct entity_s *entity);
int World_DeleteEntity(world_p world, struct entity_s *entity);
int World_CreateItem(world_p world, uint32_t item_id, uint32_t model_id, uint32_t world_model_id, uint16_t type, uint16_t count, const char *name);
int World_DeleteItem(world_p world, uint32_t item_id);
struct sprite_s* World_FindSpriteByID(unsigned int ID, world_p world);
struct skeletal_model_s* World_FindModelByID(world_p w, uint32_t id);           // binary search the model by ID


#endif
