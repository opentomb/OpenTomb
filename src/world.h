
#ifndef WORLD_H
#define WORLD_H

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <stdint.h>
#include "audio.h"
#include "camera.h"
#include "bordered_texture_atlas.h"
#include "bullet/LinearMath/btScalar.h"
#include "bullet/LinearMath/btVector3.h"

// Native TR floor data functions

#define TR_FD_FUNC_PORTALSECTOR                 0x01
#define TR_FD_FUNC_FLOORSLANT                   0x02
#define TR_FD_FUNC_CEILINGSLANT                 0x03
#define TR_FD_FUNC_TRIGGER                      0x04
#define TR_FD_FUNC_DEATH                        0x05
#define TR_FD_FUNC_CLIMB                        0x06
#define TR_FD_FUNC_FLOORTRIANGLE_NW             0x07    //  [_\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NE             0x08    //  [_/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW           0x09    //  [_/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE           0x0A    //  [_\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW   0x0B    //  [P\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE   0x0C    //  [_\P]
#define TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE   0x0D    //  [_/P]
#define TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW   0x0E    //  [P/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW 0x0F    //  [P\_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE 0x10    //  [_\P]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW 0x11    //  [P/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE 0x12    //  [_/P]
#define TR_FD_FUNC_MONKEY                       0x13
#define TR_FD_FUNC_MINECART_LEFT                0x14    // In TR3 only. Function changed in TR4+.
#define TR_FD_FUNC_MINECART_RIGHT               0x15    // In TR3 only. Function changed in TR4+.

// Native TR trigger (TR_FD_FUNC_TRIGGER) types.

#define TR_FD_TRIGTYPE_TRIGGER          0x00    // If Lara is in sector, run (any case).
#define TR_FD_TRIGTYPE_PAD              0x01    // If Lara is in sector, run (land case).
#define TR_FD_TRIGTYPE_SWITCH           0x02    // If item is activated, run, else stop.
#define TR_FD_TRIGTYPE_KEY              0x03    // If item is activated, run.
#define TR_FD_TRIGTYPE_PICKUP           0x04    // If item is picked up, run.
#define TR_FD_TRIGTYPE_HEAVY            0x05    // If item is in sector, run, else stop.
#define TR_FD_TRIGTYPE_ANTIPAD          0x06    // If Lara is in sector, stop (land case).
#define TR_FD_TRIGTYPE_COMBAT           0x07    // If Lara is in combat state, run (any case).
#define TR_FD_TRIGTYPE_DUMMY            0x08    // If Lara is in sector, run (air case).
#define TR_FD_TRIGTYPE_ANTITRIGGER      0x09    // TR2-5 only: If Lara is in sector, stop (any case).
#define TR_FD_TRIGTYPE_HEAVYSWITCH      0x0A    // TR3-5 only: If item is activated by item, run.
#define TR_FD_TRIGTYPE_HEAVYANTITRIGGER 0x0B    // TR3-5 only: If item is activated by item, stop.
#define TR_FD_TRIGTYPE_MONKEY           0x0C    // TR3-5 only: If Lara is monkey-swinging, run.
#define TR_FD_TRIGTYPE_SKELETON         0x0D    // TR5 only: Activated by skeleton only?
#define TR_FD_TRIGTYPE_TIGHTROPE        0x0E    // TR5 only: If Lara is on tightrope, run.
#define TR_FD_TRIGTYPE_CRAWLDUCK        0x0F    // TR5 only: If Lara is crawling, run.
#define TR_FD_TRIGTYPE_CLIMB            0x10    // TR5 only: If Lara is climbing, run.

// Native trigger function types.

#define TR_FD_TRIGFUNC_OBJECT           0x00
#define TR_FD_TRIGFUNC_CAMERATARGET     0x01
#define TR_FD_TRIGFUNC_UWCURRENT        0x02
#define TR_FD_TRIGFUNC_FLIPMAP          0x03
#define TR_FD_TRIGFUNC_FLIPON           0x04
#define TR_FD_TRIGFUNC_FLIPOFF          0x05
#define TR_FD_TRIGFUNC_LOOKAT           0x06
#define TR_FD_TRIGFUNC_ENDLEVEL         0x07
#define TR_FD_TRIGFUNC_PLAYTRACK        0x08
#define TR_FD_TRIGFUNC_FLIPEFFECT       0x09
#define TR_FD_TRIGFUNC_SECRET           0x0A
#define TR_FD_TRIGFUNC_BODYBAG          0x0B    // Unused in TR4
#define TR_FD_TRIGFUNC_FLYBY            0x0C
#define TR_FD_TRIGFUNC_CUTSCENE         0x0D

// Action type specifies a kind of action which trigger performs. Mostly
// it's only related to item activation, as any other trigger operations
// are not affected by action type in original engines.

#define TR_ACTIONTYPE_NORMAL  0
#define TR_ACTIONTYPE_ANTI    1
#define TR_ACTIONTYPE_SWITCH  2
#define TR_ACTIONTYPE_BYPASS -1 // Used for "dummy" triggers from originals.

// Activator specifies a kind of triggering event (NOT to be confused
// with activator type mentioned below) to occur, like ordinary trigger,
// triggering by inserting a key, turning a switch or picking up item.

#define TR_ACTIVATOR_NORMAL 0
#define TR_ACTIVATOR_SWITCH 1
#define TR_ACTIVATOR_KEY    2
#define TR_ACTIVATOR_PICKUP 3

// Activator type is used to identify activator kind for specific
// trigger types (so-called HEAVY triggers). HEAVY means that trigger
// is activated by some other item, rather than Lara herself.

#define TR_ACTIVATORTYPE_LARA 0
#define TR_ACTIVATORTYPE_MISC 1

// Various room flags specify various room options. Mostly, they
// specify environment type and some additional actions which should
// be performed in such rooms.

#define TR_ROOM_FLAG_WATER          0x0001
#define TR_ROOM_FLAG_QUICKSAND      0x0002  // Moved from 0x0080 to avoid confusion with NL.
#define TR_ROOM_FLAG_SKYBOX         0x0008
#define TR_ROOM_FLAG_UNKNOWN1       0x0010
#define TR_ROOM_FLAG_WIND           0x0020
#define TR_ROOM_FLAG_UNKNOWN2       0x0040  ///@FIXME: Find what it means!!! Always set by Dxtre3d.
#define TR_ROOM_FLAG_NO_LENSFLARE   0x0080  // In TR4-5. Was quicksand in TR3.
#define TR_ROOM_FLAG_MIST           0x0100  ///@FIXME: Unknown meaning in TR1!!!
#define TR_ROOM_FLAG_CAUSTICS       0x0200
#define TR_ROOM_FLAG_UNKNOWN3       0x0400
#define TR_ROOM_FLAG_DAMAGE         0x0800  ///@FIXME: Is it really damage (D)?
#define TR_ROOM_FLAG_POISON         0x1000  ///@FIXME: Is it really poison (P)?

//Room light mode flags (TR2 ONLY)

#define TR_ROOM_LIGHTMODE_FLICKER   0x1

// Sector flags specify various unique sector properties.
// Derived from native TR floordata functions.

#define SECTOR_FLAG_CLIMB_NORTH     0x00000001  // subfunction 0x01
#define SECTOR_FLAG_CLIMB_EAST      0x00000002  // subfunction 0x02
#define SECTOR_FLAG_CLIMB_SOUTH     0x00000004  // subfunction 0x04
#define SECTOR_FLAG_CLIMB_WEST      0x00000008  // subfunction 0x08
#define SECTOR_FLAG_CLIMB_CEILING   0x00000010
#define SECTOR_FLAG_MINECART_LEFT   0x00000020
#define SECTOR_FLAG_MINECART_RIGHT  0x00000040
#define SECTOR_FLAG_TRIGGERER_MARK  0x00000080
#define SECTOR_FLAG_BEETLE_MARK     0x00000100
#define SECTOR_FLAG_DEATH           0x00000200

// Sector material specifies audio response from character footsteps, as well as
// footstep texture option, plus possible vehicle physics difference in the future.

#define SECTOR_MATERIAL_MUD         0   // Classic one, TR1-2.
#define SECTOR_MATERIAL_SNOW        1
#define SECTOR_MATERIAL_SAND        2
#define SECTOR_MATERIAL_GRAVEL      3
#define SECTOR_MATERIAL_ICE         4
#define SECTOR_MATERIAL_WATER       5
#define SECTOR_MATERIAL_STONE       6
#define SECTOR_MATERIAL_WOOD        7
#define SECTOR_MATERIAL_METAL       8
#define SECTOR_MATERIAL_MARBLE      9
#define SECTOR_MATERIAL_GRASS       10
#define SECTOR_MATERIAL_CONCRETE    11
#define SECTOR_MATERIAL_OLDWOOD     12
#define SECTOR_MATERIAL_OLDMETAL    13

// Maximum number of flipmaps specifies how many flipmap indices to store. Usually,
// TR1-3 doesn't contain flipmaps above 10, while in TR4-5 number of flipmaps could
// be as much as 14-16. To make sure flipmap array will be suitable for all game
// versions, it is set to 32.

#define FLIPMAP_MAX_NUMBER          32

// Activation mask operation can be either XOR (for switch triggers) or OR (for any
// other types of triggers).

#define AMASK_OP_OR  0
#define AMASK_OP_XOR 1

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
    uint32_t                    trig_index; // Trigger function index.
    int32_t                     box_index;
    
    uint32_t                    flags;      // Climbability, death etc.
    uint32_t                    material;   // Footstep sound and footsteps.

    int32_t                     floor;
    int32_t                     ceiling;

    struct room_sector_s        *sector_below;
    struct room_sector_s        *sector_above;
    struct room_s               *owner_room;    // Room that contain this sector

    int16_t                     index_x;
    int16_t                     index_y;
    btScalar                    pos[3];

    btVector3                   ceiling_corners[4];
    uint8_t                     ceiling_diagonal_type;
    uint8_t                     ceiling_penetration_config;

    btVector3                   floor_corners[4];
    uint8_t                     floor_diagonal_type;
    uint8_t                     floor_penetration_config;

    int32_t                     portal_to_room;
}room_sector_t, *room_sector_p;


typedef struct sector_tween_s
{
    btVector3                   floor_corners[4];
    uint8_t                     floor_tween_type;

    btVector3                   ceiling_corners[4];
    uint8_t                     ceiling_tween_type;
}sector_tween_t, *sector_tween_p;


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
    int16_t                     light_mode;                                     // (present only in TR2: 0 is normal, 1 is flickering(?), 2 and 3 are uncertain)
    uint8_t                     reverb_info;                                    // room reverb type
    uint8_t                     water_scheme;
    uint8_t                     alternate_group;

    int8_t                      active;                                         // flag: is active
    int8_t                      is_in_r_list;                                   // is room in render list
    int8_t                      hide;                                           // do not render
    struct base_mesh_s         *mesh;                                           // room's base mesh
    //struct bsp_node_s          *bsp_root;                                       // transparency polygons tree; next: add bsp_tree class as a bsp_tree header

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
    struct room_s              *base_room;                                      // base room == room->alternate_room->base_room

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
    
    uint32_t                    flip_count;             // Number of flips
    uint8_t                    *flip_map;               // Flipped room activity array.
    uint8_t                    *flip_state;             // Flipped room state array.

    bordered_texture_atlas     *tex_atlas;
    uint32_t                    tex_count;              // Number of textures
    GLuint                     *textures;               // OpenGL textures indexes

    uint32_t                    anim_sequences_count;   // Animated texture sequence count
    struct anim_seq_s          *anim_sequences;         // Animated textures

    uint32_t                    meshes_count;            // Base meshes count
    struct base_mesh_s         *meshes;                 // Base meshes data

    uint32_t                    sprites_count;          // Base sprites count
    struct sprite_s            *sprites;                // Base sprites data

    uint32_t                    skeletal_model_count;   // number of base skeletal models
    struct skeletal_model_s    *skeletal_models;        // base skeletal models data

    struct entity_s            *Character;              // this is an unique Lara's pointer =)
    struct skeletal_model_s    *sky_box;                // global skybox

    struct RedBlackHeader_s    *entity_tree;            // tree of world active objects
    struct RedBlackHeader_s    *items_tree;             // tree of world items

    uint32_t                    type;
    
    uint32_t                    cameras_sinks_count;    // Amount of cameras and sinks.
    struct stat_camera_sink_s  *cameras_sinks;          // Cameras and sinks. 

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
uint32_t World_SpawnEntity(uint32_t model_id, uint32_t room_id, btScalar pos[3], btScalar ang[3], int32_t id);
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
room_sector_p Room_GetSectorRaw(room_p room, btScalar pos[3]);
room_sector_p Room_GetSectorCheckFlip(room_p room, btScalar pos[3]);
room_sector_p Sector_CheckFlip(room_sector_p rs);
room_sector_p Room_GetSectorXYZ(room_p room, btScalar pos[3]);

void Room_Enable(room_p room);
void Room_Disable(room_p room);
void Room_SwapToAlternate(room_p room);
void Room_SwapToBase(room_p room);
room_p Room_CheckFlip(room_p r);
void Room_SwapPortals(room_p room, room_p dest_room);//Swap room portals of input room to destination room
void Room_SwapItems(room_p room, room_p dest_room);//Swap room items of input room to destination room
void Room_BuildNearRoomsList(room_p room);

int Room_IsJoined(room_p r1, room_p r2);
int Room_IsOverlapped(room_p r0, room_p r1);
int Room_IsInNearRoomsList(room_p room, room_p r);
int Room_HasSector(room_p room, int x, int y);//If this room contains a sector
room_sector_p TR_Sector_CheckBaseRoom(room_sector_p rs);
room_sector_p TR_Sector_CheckAlternateRoom(room_sector_p rs);
room_sector_p TR_Sector_CheckPortalPointerRaw(room_sector_p rs);
room_sector_p TR_Sector_CheckPortalPointer(room_sector_p rs);
int Sectors_Is2SidePortals(room_sector_p s1, room_sector_p s2);

int World_AddEntity(world_p world, struct entity_s *entity);
int World_DeleteEntity(world_p world, struct entity_s *entity);
int World_CreateItem(world_p world, uint32_t item_id, uint32_t model_id, uint32_t world_model_id, uint16_t type, uint16_t count, const char *name);
int World_DeleteItem(world_p world, uint32_t item_id);
struct sprite_s* World_GetSpriteByID(unsigned int ID, world_p world);
struct skeletal_model_s* World_GetModelByID(world_p w, uint32_t id);           // binary search the model by ID


#endif
