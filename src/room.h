
#ifndef ROOM_H
#define ROOM_H

#include <stdint.h>

// Here you can specify the way OpenTomb processes room collision -
// in a classic TR way (floor data collision) or in a modern way
// (derived from actual room mesh).

#define TR_MESH_ROOM_COLLISION 0

// Metering step and sector size are basic Tomb Raider world metrics.
// Use these defines at all times, when you're referencing classic TR
// dimensions and terrain manipulations.

#define TR_METERING_STEP        (256.0)
#define TR_METERING_SECTORSIZE  (1024.0)

// Wall height is a magical constant which specifies that sector with such
// height contains impassable wall.

#define TR_METERING_WALLHEIGHT  (32512)

// Penetration configuration specifies collision type for floor and ceiling
// sectors (squares).

#define TR_PENETRATION_CONFIG_SOLID             0   // Ordinary sector.
#define TR_PENETRATION_CONFIG_DOOR_VERTICAL_A   1   // TR3-5 triangulated door.
#define TR_PENETRATION_CONFIG_DOOR_VERTICAL_B   2   // TR3-5 triangulated door.
#define TR_PENETRATION_CONFIG_WALL              3   // Wall (0x81 == TR_METERING_WALLHEIGHT)
#define TR_PENETRATION_CONFIG_GHOST             4   // No collision.

// There are two types of diagonal splits - we call them north-east (NE) and
// north-west (NW). In case there is no diagonal in sector (TR1-2 classic sector),
// then NONE type is used.

#define TR_SECTOR_DIAGONAL_TYPE_NONE            0
#define TR_SECTOR_DIAGONAL_TYPE_NE              1
#define TR_SECTOR_DIAGONAL_TYPE_NW              2

// Tween is a short word for "inbeTWEEN vertical polygon", which is needed to fill
// the gap between two sectors with different heights. If adjacent sector heights are
// similar, it means that tween is degenerated (doesn't exist physically) - in that
// case we use NONE type. If only one of two heights' pairs is similar, then tween is
// either right or left pointed triangle (where "left" or "right" is derived by viewing
// triangle from front side). If none of the heights are similar, we need quad tween.

#define TR_SECTOR_TWEEN_TYPE_NONE               0   // Degenerated vertical polygon.
#define TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT     1   // Triangle pointing right (viewed front).
#define TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT      2   // Triangle pointing left (viewed front).
#define TR_SECTOR_TWEEN_TYPE_QUAD               3   // 
#define TR_SECTOR_TWEEN_TYPE_2TRIANGLES         4   // it looks like a butterfly


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
#define TR_FD_TRIGFUNC_CLEARBODIES      0x0B    // Unused in TR4
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

struct engine_container_s;
struct polygon_s;
struct camera_s;
struct portal_s;
struct frustum_s;
struct base_mesh_s;
struct physics_object_s;
struct sprite_buffer_s;

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

    struct room_sector_s       *sector_below;
    struct room_sector_s       *sector_above;
    struct room_s              *owner_room;    // Room that contain this sector
    struct room_s              *portal_to_room;
    
    int16_t                     index_x;
    int16_t                     index_y;
    float                       pos[3];

    float                       ceiling_corners[4][3];
    uint8_t                     ceiling_diagonal_type;
    uint8_t                     ceiling_penetration_config;

    float                       floor_corners[4][3];
    uint8_t                     floor_diagonal_type;
    uint8_t                     floor_penetration_config;
}room_sector_t, *room_sector_p;


typedef struct sector_tween_s
{
    float                       floor_corners[4][3];
    uint8_t                     floor_tween_type;

    float                       ceiling_corners[4][3];
    uint8_t                     ceiling_tween_type;
}sector_tween_t, *sector_tween_p;


typedef struct room_sprite_s
{
    struct sprite_s            *sprite;
    float                       pos[3];
    int8_t                      was_rendered;
}room_sprite_t, *room_sprite_p;


typedef struct static_mesh_s
{
    uint32_t                    object_id;                                      //
    uint8_t                     was_rendered;                                   // 0 - was not rendered, 1 - opaque, 2 - transparency, 3 - full rendered
    uint8_t                     was_rendered_lines;
    uint8_t                     hide;                                           // disable static mesh rendering
    float                       pos[3];                                         // model position
    float                       rot[3];                                         // model angles
    GLfloat                     tint[4];                                        // model tint

    float                       vbb_min[3];                                     // visible bounding box
    float                       vbb_max[3];
    float                       cbb_min[3];                                     // collision bounding box
    float                       cbb_max[3];

    float                       transform[16]   __attribute__((packed, aligned(16)));   // gl transformation matrix
    struct obb_s               *obb;
    struct engine_container_s  *self;

    struct base_mesh_s         *mesh;                                           // base model
    struct physics_object_s    *physics_body;
}static_mesh_t, *static_mesh_p;


typedef struct room_content_s
{
    struct engine_container_s  *containers;                                     // engine containers with moveables objects
    uint32_t                    static_mesh_count;
    struct static_mesh_s       *static_mesh;
    uint32_t                    sprites_count;
    struct room_sprite_s       *sprites;
    uint32_t                    light_count;
    struct light_s             *lights;
    
    int16_t                     light_mode;                                     // (present only in TR2: 0 is normal, 1 is flickering(?), 2 and 3 are uncertain)
    uint8_t                     reverb_info;                                    // room reverb type
    uint8_t                     water_scheme;
    uint8_t                     alternate_group;
    
    float                       ambient_lighting[3];
    struct base_mesh_s         *mesh;                                           // room's base mesh
    struct sprite_buffer_s     *sprite_buffer;                                  // Render data for sprites
    struct physics_object_s    *physics_body;
}room_content_t, *room_content_p;


typedef struct room_s
{
    uint32_t                    id;                                             // room's ID
    uint32_t                    flags;                                          // room's type + water, wind info
    
    int8_t                      is_in_r_list;                                   // is room in render list
    int8_t                      active;
    uint16_t                    portals_count;                                  // number of room portals
    struct portal_s            *portals;                                        // room portals array
    struct room_s              *alternate_room;                                 // alternative room pointer
    struct room_s              *base_room;                                      // base room == room->alternate_room->base_room
    struct frustum_s           *frustum;
    
    float                       bb_min[3];                                      // room's bounding box
    float                       bb_max[3];                                      // room's bounding box
    float                       transform[16] __attribute__((packed, aligned(16))); // GL transformation matrix
    uint32_t                    sectors_count;
    uint16_t                    sectors_x;
    uint16_t                    sectors_y;
    struct room_sector_s       *sectors;

    uint16_t                    near_room_list_size;
    struct room_s              *near_room_list[32];
    uint16_t                    overlapped_room_list_size;
    struct room_s              *overlapped_room_list[32];
    struct room_content_s      *content;
    
    struct engine_container_s  *self;
}room_t, *room_p;


void Room_Clear(struct room_s *room);
void Room_Enable(struct room_s *room);
void Room_Disable(struct room_s *room);
int  Room_AddObject(struct room_s *room, struct engine_container_s *cont);
int  Room_RemoveObject(struct room_s *room, struct engine_container_s *cont);

struct room_sector_s *Room_GetSectorRaw(struct room_s *room, float pos[3]);
struct room_sector_s *Room_GetSectorCheckFlip(struct room_s *room, float pos[3]);
struct room_sector_s *Room_GetSectorXYZ(struct room_s *room, float pos[3]);

void Room_AddToNearRoomsList(struct room_s *room, struct room_s *r);
int  Room_IsJoined(struct room_s *r1, struct room_s *r2);
int  Room_IsOverlapped(struct room_s *r0, struct room_s *r1);
int  Room_IsInNearRoomsList(struct room_s *r0, struct room_s *r1);
void Room_SwapItems(struct room_s *room, struct room_s *dest_room);

struct room_s *Room_CheckFlip(struct room_s *r);

// NOTE: Functions which take native TR level structures as argument will have
// additional _TR_ prefix. Functions which doesn't use specific TR structures
// should NOT use such prefix!
void Room_GenSpritesBuffer(struct room_s *room);

struct room_sector_s *Sector_CheckBaseRoom(struct room_sector_s *rs);
struct room_sector_s *Sector_CheckAlternateRoom(struct room_sector_s *rs);
struct room_sector_s *Sector_GetPortalSectorTargetRaw(struct room_sector_s *rs);
struct room_sector_s *Sector_GetPortalSectorTarget(struct room_sector_s *rs);
int Sectors_Is2SidePortals(struct room_sector_s *s1, struct room_sector_s *s2);

struct room_sector_s *Sector_CheckFlip(struct room_sector_s *rs);
struct room_sector_s *Sector_GetLowest(struct room_sector_s *sector);
struct room_sector_s *Sector_GetHighest(struct room_sector_s *sector);


void Sector_HighestFloorCorner(room_sector_p rs, float v[3]);
void Sector_LowestCeilingCorner(room_sector_p rs, float v[3]);

int Sectors_SimilarFloor(room_sector_p s1, room_sector_p s2, int ignore_doors);
int Sectors_SimilarCeiling(room_sector_p s1, room_sector_p s2, int ignore_doors);

#endif //ROOM_H
