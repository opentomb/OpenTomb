
#ifndef ENGINE_BASE_TYPES_H
#define ENGINE_BASE_TYPES_H

#include <stdint.h>

#define OBJECT_STATIC_MESH                      (0x0001)
#define OBJECT_ROOM_BASE                        (0x0002)
#define OBJECT_ENTITY                           (0x0003)
#define OBJECT_HAIR                             (0x0004)
#define OBJECT_BULLET_MISC                      (0x7FFF)

#define COLLISION_SHAPE_BOX                     0x0001
#define COLLISION_SHAPE_BOX_BASE                0x0002     // use mesh box collision
#define COLLISION_SHAPE_SPHERE                  0x0003
#define COLLISION_SHAPE_TRIMESH                 0x0004     // for static objects and room's!
#define COLLISION_SHAPE_TRIMESH_CONVEX          0x0005     // for dynamic objects
#define COLLISION_SHAPE_SINGLE_BOX              0x0006     // use single box collision
#define COLLISION_SHAPE_SINGLE_SPHERE           0x0007

#define COLLISION_NONE                          (0x0000)
#define COLLISION_MASK_ALL                      (0x7FFF)        // bullet uses signed short int for these flags!

#define COLLISION_GROUP_ALL                     (0x7FFF)
#define COLLISION_GROUP_STATIC_ROOM             (0x0001)        // room mesh
#define COLLISION_GROUP_STATIC_OBLECT           (0x0002)        // room static object
#define COLLISION_GROUP_KINEMATIC               (0x0004)        // doors, blocks, static animated entityes
//#define COLLISION_GROUP_GHOST                   (0x0008)        // probe objects
#define COLLISION_GROUP_TRIGGERS                (0x0010)        // probe objects
#define COLLISION_GROUP_CHARACTERS              (0x0020)        // Lara, enemies, friends, creatures
#define COLLISION_GROUP_VEHICLE                 (0x0040)        // car, moto, bike
#define COLLISION_GROUP_BULLETS                 (0x0080)        // bullets, rockets, grenades, arrows...
#define COLLISION_GROUP_DYNAMICS                (0x0100)        // test balls, warious
#define COLLISION_GROUP_DYNAMICS_NI             (0x0200)        // test balls, warious


#define COLLISION_FILTER_CHARACTER              (COLLISION_GROUP_STATIC_ROOM | COLLISION_GROUP_STATIC_OBLECT | COLLISION_GROUP_KINEMATIC | \
                                                 COLLISION_GROUP_CHARACTERS | COLLISION_GROUP_VEHICLE | COLLISION_GROUP_DYNAMICS)

#define COLLISION_FILTER_HEIGHT_TEST            (COLLISION_GROUP_STATIC_ROOM | COLLISION_GROUP_STATIC_OBLECT | COLLISION_GROUP_KINEMATIC | COLLISION_GROUP_VEHICLE)

#ifdef	__cplusplus
extern "C" {
#endif

struct room_s;
struct room_sector_s;

typedef struct engine_container_s
{
    uint16_t                     object_type;
    uint16_t                     collision_shape : 8;
    uint16_t                     collision_heavy : 1;
    uint16_t                     : 7;
    int16_t                      collision_group;
    int16_t                      collision_mask;
    void                        *object;
    struct room_s               *room;
    struct room_sector_s        *sector;
    struct engine_container_s   *next;
}engine_container_t, *engine_container_p;

typedef struct engine_transform_s
{
    float                        M4x4[16] __attribute__((packed, aligned(16))); // GL transformation matrix
    float                        scaling[3];         // entity scaling
    float                        angles[3];
} engine_transform_t, *engine_transform_p;


engine_container_p Container_Create();
void Container_Delete(engine_container_p cont);

#ifdef	__cplusplus
}
#endif
#endif
