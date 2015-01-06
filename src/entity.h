
#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

#include "bullet/LinearMath/btVector3.h"
#include "bullet/BulletCollision/CollisionShapes/btCollisionShape.h"
#include "mesh.h"

class btCollisionShape;
class btRigidBody;

struct room_s;
struct room_sector_s;
struct obb_s;
struct character_s;
struct ss_bone_frame_s;

#define ENTITY_STATE_ENABLED                        (0x0001)
#define ENTITY_STATE_ACTIVE                         (0x0002)
#define ENTITY_STATE_VISIBLE                        (0x0004)

#define ENTITY_TYPE_DECORATION                      (0x0000)
#define ENTITY_TYPE_TRIGGER                         (0x0001)
#define ENTITY_TYPE_TRIGGER_ACTIVATOR               (0x0002)
#define ENTITY_TYPE_PICKABLE                        (0x0004)
#define ENTITY_TYPE_TRAVERSE                        (0x0008)
#define ENTITY_TYPE_TRAVERSE_FLOOR                  (0x0010)
#define ENTITY_TYPE_SPAWNED                         (0x8000)

#define ENTITY_CALLBACK_NONE                        (0x00000000)
#define ENTITY_CALLBACK_ACTIVATE                    (0x00000001)
#define ENTITY_CALLBACK_COLLISION                   (0x00000002)
#define ENTITY_CALLBACK_ON_STAND                    (0x00000004)
#define ENTITY_CALLBACK_ON_HIT                      (0x00000008)

#define ENTITY_GHOST_COLLISION                    0                             // no one collisions
#define ENTITY_DYNAMIC_COLLISION                  1                             // hallo full physics interaction
#define ENTITY_CINEMATIC_COLLISION                2                             // doors and other moveable statics
#define ENTITY_STATIC_COLLISION                   3                             // static object - never moved
#define ENTITY_ACTOR_COLLISION                    4                             // actor, enemies, NPC, animals
#define ENTITY_VEHICLE_COLLISION                  5                             // car, moto, bike

#define ENTITY_SUBSTANCE_NONE                     0
#define ENTITY_SUBSTANCE_WATER_SHALLOW            1
#define ENTITY_SUBSTANCE_WATER_WADE               2
#define ENTITY_SUBSTANCE_WATER_SWIM               3
#define ENTITY_SUBSTANCE_QUICKSAND_SHALLOW        4
#define ENTITY_SUBSTANCE_QUICKSAND_CONSUMED       5

/*
 * уже конкретная игровая модель
 */

typedef struct entity_s
{
    uint32_t                            id;                                     // ID
    int32_t                             OCB;                                    // Object code bit (since TR4)
    uint16_t                            activation_mask;                        // 0x1F means ACTIVATE.

    uint32_t                            callback_flags;                         // information about scripts callbacks
    uint16_t                            type_flags;
    uint16_t                            state_flags;

    uint8_t                             dir_flag;                               // (move direction)
    uint16_t                            anim_flags;                             // additional animation control param
    uint16_t                            move_type;                              // on floor / free fall / swim ....
    uint8_t                             was_rendered;                           // render once per frame trigger
    uint8_t                             was_rendered_lines;                     // same for debug lines
    uint8_t                             smooth_anim;

    btScalar                            current_speed;                          // current linear speed from animation info
    btVector3                           speed;                                  // speed of the entity XYZ
    btScalar                            inertia;
    struct ss_bone_frame_s              bf;                                     // current boneframe with full frame information
    btScalar                            angles[3];
    btScalar                            transform[16];                          // GL transformation matrix

    struct obb_s                       *obb;                                    // oriented bounding box

    void                              (*onFrame)(struct entity_s *ent, int state);
    struct room_sector_s               *current_sector;

    struct engine_container_s          *self;

    btScalar                            activation_offset[4];                   // where we can activate object (dx, dy, dz, r)
    btRigidBody                       **bt_body;
    struct character_s                 *character;
}entity_t, *entity_p;


entity_p Entity_Create();
void Entity_Clear(entity_p entity);
void Entity_Enable(entity_p ent);
void Entity_Disable(entity_p ent);
void Entity_EnableCollision(entity_p ent);
void Entity_DisableCollision(entity_p ent);

// Bullet entity rigid body generating.
void BT_GenEntityRigidBody(entity_p ent);

void Entity_UpdateRoomPos(entity_p ent);
void Entity_UpdateRigidBody(entity_p ent, int force);

struct state_change_s *Anim_FindStateChangeByAnim(struct animation_frame_s *anim, int state_change_anim);
struct state_change_s *Anim_FindStateChangeByID(struct animation_frame_s *anim, uint32_t id);
int  Entity_GetAnimDispatchCase(struct entity_s *entity, uint32_t id);
void Entity_GetNextFrame(struct ss_bone_frame_s *bf, btScalar time, struct state_change_s *stc, int16_t *frame, int16_t *anim, uint16_t anim_flags);
int  Entity_Frame(entity_p entity, btScalar time);                 // frame + trying to chabge state

void Entity_RebuildBV(entity_p ent);
void Entity_UpdateRotation(entity_p entity);
void Entity_CheckActivators(struct entity_s *ent);

int  Entity_GetSubstanceState(entity_p entity);

void Entity_UpdateCurrentBoneFrame(struct ss_bone_frame_s *bf, btScalar etr[16]);
void Entity_DoAnimCommands(entity_p entity, int changing);
int  Entity_ParseFloorData(struct entity_s *ent, struct world_s *world);
void Entity_SetAnimation(entity_p entity, int animation, int frame);
void Entity_MoveForward(struct entity_s *ent, btScalar dist);
void Entity_MoveStrafe(struct entity_s *ent, btScalar dist);
void Entity_MoveVertical(struct entity_s *ent, btScalar dist);

// Helper functions

btScalar Entity_FindDistance(entity_p entity_1, entity_p entity_2);

#endif
