
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


struct engine_container_s;
struct polygon_s;
struct camera_s;
struct portal_s;
struct frustum_s;
struct base_mesh_s;
struct physics_object_s;
struct trigger_header_s;


typedef struct room_zone_s
{
    int16_t GroundZone1_Normal;
    int16_t GroundZone2_Normal;
    int16_t GroundZone3_Normal;
    int16_t GroundZone4_Normal;
    int16_t FlyZone_Normal;
    int16_t GroundZone1_Alternate;
    int16_t GroundZone2_Alternate;
    int16_t GroundZone3_Alternate;
    int16_t GroundZone4_Alternate;
    int16_t FlyZone_Alternate;
}room_zone_t, *room_zone_p;


typedef struct box_overlap_s
{
    uint16_t        box : 15;
    uint16_t        end : 1;
}box_overlap_t, *box_overlap_p;


typedef struct room_box_s
{
    float                   bb_min[3];
    float                   bb_max[3];
    struct box_overlap_s   *overlaps;
    struct room_zone_s      zone;
}room_box_t, *room_box_p;


typedef struct room_sector_s
{
    uint32_t                    trig_index; // Trigger function index.

    uint32_t                    flags;      // Climbability, death etc.
    uint32_t                    material;   // Footstep sound and footsteps.

    int32_t                     floor;
    int32_t                     ceiling;

    struct trigger_header_s    *trigger;
    struct room_box_s          *box;
    struct room_s              *owner_room;    // Room that contain this sector
    struct room_s              *portal_to_room;

    struct room_s              *room_below;
    struct room_s              *room_above;
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
    uint8_t                     floor_tween_inverted;

    float                       ceiling_corners[4][3];
    uint8_t                     ceiling_tween_type;
    uint8_t                     ceiling_tween_inverted;
}sector_tween_t, *sector_tween_p;


typedef struct room_sprite_s
{
    struct sprite_s            *sprite;
    float                       pos[3];
}room_sprite_t, *room_sprite_p;


typedef struct static_mesh_s
{
    uint32_t                    object_id;                                      //
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
    uint32_t                    original_room_id;
    uint32_t                    room_flags;                                     // room's type + water, wind info
    uint32_t                    portals_count;                                  // number of room portals
    struct portal_s            *portals;                                        // room portals array
    struct room_sector_s       *sectors;
    
    uint16_t                    near_room_list_size;
    uint16_t                    overlapped_room_list_size;
    struct room_s             **near_room_list;
    struct room_s             **overlapped_room_list;
    
    uint32_t                    static_mesh_count;
    struct static_mesh_s       *static_mesh;
    uint32_t                    sprites_count;
    struct room_sprite_s       *sprites;
    struct vertex_s            *sprites_vertices;
    uint32_t                    lights_count;
    struct light_s             *lights;

    int16_t                     light_mode;                                     // (present only in TR2: 0 is normal, 1 is flickering(?), 2 and 3 are uncertain)
    uint8_t                     reverb_info;                                    // room reverb type
    uint8_t                     water_scheme;
    uint8_t                     alternate_group;

    float                       ambient_lighting[3];
    struct base_mesh_s         *mesh;                                           // room's base mesh
    struct physics_object_s    *physics_body;                                   // static physics data
    struct physics_object_s    *physics_alt_tween;                              // changable (alt room) tween physics data
}room_content_t, *room_content_p;


typedef struct room_s
{
    uint32_t                    id;                                             // room's ID
    uint32_t                    is_in_r_list : 1;                               // is room in render list
    uint32_t                    is_swapped : 1;
    struct room_s              *alternate_room_next;                            // alternative room pointer
    struct room_s              *alternate_room_prev;                            // alternative room pointer
    struct room_s              *real_room;                                      // real room, using in game
    struct frustum_s           *frustum;

    struct obb_s               *obb;
    float                       bb_min[3];                                      // room's bounding box
    float                       bb_max[3];                                      // room's bounding box
    float                       transform[16] __attribute__((packed, aligned(16))); // GL transformation matrix
    uint32_t                    sectors_count;
    uint16_t                    sectors_x;
    uint16_t                    sectors_y;
    struct engine_container_s  *containers;                                     // engine containers with moveables objects
    struct room_content_s      *content;
    struct room_content_s      *original_content;

    struct engine_container_s  *self;
}room_t, *room_p;


void Room_Clear(struct room_s *room);
void Room_Enable(struct room_s *room);
void Room_Disable(struct room_s *room);
int  Room_AddObject(struct room_s *room, struct engine_container_s *cont);
int  Room_RemoveObject(struct room_s *room, struct engine_container_s *cont);

void Room_SetActiveContent(struct room_s *room, struct room_s *room_with_content_from);
void Room_DoFlip(struct room_s *room1, struct room_s *room2);

struct room_sector_s *Room_GetSectorRaw(struct room_s *room, float pos[3]);
struct room_sector_s *Room_GetSectorXYZ(struct room_s *room, float pos[3]);

void Room_AddToNearRoomsList(struct room_s *room, struct room_s *r);
int  Room_IsJoined(struct room_s *r1, struct room_s *r2);
int  Room_IsOverlapped(struct room_s *r0, struct room_s *r1);
int  Room_IsInNearRoomsList(struct room_s *r0, struct room_s *r1);
int  Room_IsInOverlappedRoomsList(struct room_s *r0, struct room_s *r1);
void Room_MoveActiveItems(struct room_s *room_to, struct room_s *room_from);

// NOTE: Functions which take native TR level structures as argument will have
// additional _TR_ prefix. Functions which doesn't use specific TR structures
// should NOT use such prefix!
void Room_GenSpritesBuffer(struct room_s *room);

struct room_sector_s *Sector_GetPortalSectorTargetRaw(struct room_sector_s *rs);
struct room_sector_s *Sector_GetPortalSectorTargetReal(struct room_sector_s *rs);

struct room_sector_s *Sector_GetLowest(struct room_sector_s *sector);
struct room_sector_s *Sector_GetHighest(struct room_sector_s *sector);


void Sector_HighestFloorCorner(room_sector_p rs, float v[3]);
void Sector_LowestCeilingCorner(room_sector_p rs, float v[3]);

int Sectors_SimilarFloor(room_sector_p s1, room_sector_p s2, int ignore_doors);
int Sectors_SimilarCeiling(room_sector_p s1, room_sector_p s2, int ignore_doors);

#endif //ROOM_H
