
#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

#include "bullet/LinearMath/btVector3.h"
#include "bullet/BulletCollision/CollisionShapes/btCollisionShape.h"
#include "bullet/BulletDynamics/ConstraintSolver/btTypedConstraint.h"
#include "bullet/BulletCollision/CollisionDispatch/btGhostObject.h"
#include "bullet/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"
#include "mesh.h"

class btCollisionShape;
class btRigidBody;

struct room_s;
struct room_sector_s;
struct obb_s;
struct character_s;
struct ss_animation_s;
struct ss_bone_frame_s;

#define ENTITY_STATE_ENABLED                        (0x0001)    // Entity is enabled.
#define ENTITY_STATE_ACTIVE                         (0x0002)    // Entity is animated.
#define ENTITY_STATE_VISIBLE                        (0x0004)    // Entity is visible.

#define ENTITY_TYPE_GENERIC                         (0x0000)    // Just an animating.
#define ENTITY_TYPE_INTERACTIVE                     (0x0001)    // Can respond to other entity's commands.
#define ENTITY_TYPE_TRIGGER_ACTIVATOR               (0x0002)    // Can activate triggers.
#define ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR          (0x0004)    // Can activate heavy triggers.
#define ENTITY_TYPE_PICKABLE                        (0x0008)    // Can be picked up.
#define ENTITY_TYPE_TRAVERSE                        (0x0010)    // Can be pushed/pulled.
#define ENTITY_TYPE_TRAVERSE_FLOOR                  (0x0020)    // Can be walked upon.
#define ENTITY_TYPE_DYNAMIC                         (0x0040)    // Acts as a physical dynamic object.
#define ENTITY_TYPE_ACTOR                           (0x0080)    // Is actor.

#define ENTITY_TYPE_SPAWNED                         (0x8000)    // Was spawned.

#define ENTITY_CALLBACK_NONE                        (0x00000000)
#define ENTITY_CALLBACK_ACTIVATE                    (0x00000001)
#define ENTITY_CALLBACK_DEACTIVATE                  (0x00000002)
#define ENTITY_CALLBACK_COLLISION                   (0x00000004)
#define ENTITY_CALLBACK_STAND                       (0x00000008)
#define ENTITY_CALLBACK_HIT                         (0x00000010)

#define ENTITY_COLLISION_GHOST                    0     // no one collisions
#define ENTITY_COLLISION_DYNAMIC                  1     // hallo full physics interaction
#define ENTITY_COLLISION_KINEMATIC                2     // doors and other moveable statics
#define ENTITY_COLLISION_STATIC                   3     // static object - never moved
#define ENTITY_COLLISION_ACTOR                    4     // actor, enemies, NPC, animals
#define ENTITY_COLLISION_VEHICLE                  5     // car, moto, bike

#define ENTITY_SUBSTANCE_NONE                     0
#define ENTITY_SUBSTANCE_WATER_SHALLOW            1
#define ENTITY_SUBSTANCE_WATER_WADE               2
#define ENTITY_SUBSTANCE_WATER_SWIM               3
#define ENTITY_SUBSTANCE_QUICKSAND_SHALLOW        4
#define ENTITY_SUBSTANCE_QUICKSAND_CONSUMED       5

#define ENTITY_TLAYOUT_MASK     0x1F    // Activation mask
#define ENTITY_TLAYOUT_EVENT    0x20    // Last trigger event
#define ENTITY_TLAYOUT_LOCK     0x40    // Activity lock
#define ENTITY_TLAYOUT_SSTATUS  0x80    // Sector status

#define WEAPON_STATE_HIDE                       (0x00)
#define WEAPON_STATE_HIDE_TO_READY              (0x01)
#define WEAPON_STATE_IDLE                       (0x02)
#define WEAPON_STATE_IDLE_TO_FIRE               (0x03)
#define WEAPON_STATE_FIRE                       (0x04)
#define WEAPON_STATE_FIRE_TO_IDLE               (0x05)
#define WEAPON_STATE_IDLE_TO_HIDE               (0x06)

// Specific in-game entity structure.

typedef struct bt_entity_data_s
{
    int8_t                              no_fix_all;
    uint32_t                            no_fix_body_parts;
    btPairCachingGhostObject          **ghostObjects;           // like Bullet character controller for penetration resolving.
    btManifoldArray                    *manifoldArray;          // keep track of the contact manifolds
    
    btCollisionShape                  **shapes;
    btRigidBody                       **bt_body;
    uint32_t                            bt_joint_count;         // Ragdoll joints
    btTypedConstraint                 **bt_joints;              // Ragdoll joints
}bt_entity_data_t, *bt_entity_data_p;

typedef struct entity_s
{
    uint32_t                            id;                 // Unique entity ID
    int32_t                             OCB;                // Object code bit (since TR4)
    uint8_t                             trigger_layout;     // Mask + once + event + sector status flags
    float                               timer;              // Set by "timer" trigger field

    uint32_t                            callback_flags;     // information about scripts callbacks
    uint16_t                            type_flags;
    uint16_t                            state_flags;

    uint8_t                             dir_flag;           // (move direction)
    uint16_t                            move_type;          // on floor / free fall / swim ....
    
    uint8_t                             was_rendered;       // render once per frame trigger
    uint8_t                             was_rendered_lines; // same for debug lines

    btScalar                            current_speed;      // current linear speed from animation info
    btVector3                           speed;              // speed of the entity XYZ
    
    btScalar                            inertia_linear;     // linear inertia
    btScalar                            inertia_angular[2]; // angular inertia - X and Y axes
    
    struct ss_bone_frame_s              bf;                 // current boneframe with full frame information
    struct bt_entity_data_s             bt;
    btScalar                            angles[3];
    btScalar                            transform[16] __attribute__((packed, aligned(16))); // GL transformation matrix

    struct obb_s                       *obb;                // oriented bounding box

    struct room_sector_s               *current_sector;
    struct room_sector_s               *last_sector;

    struct engine_container_s          *self;

    btScalar                            activation_offset[4];   // where we can activate object (dx, dy, dz, r)
    
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
int  Entity_Frame(entity_p entity, btScalar time);  // process frame + trying to change state

void Entity_RebuildBV(entity_p ent);
void Entity_UpdateRotation(entity_p entity);
void Entity_UpdateCurrentSpeed(entity_p entity, int zeroVz = 0);
void Entity_AddOverrideAnim(struct entity_s *ent, int model_id);
void Entity_CheckActivators(struct entity_s *ent);

int  Entity_GetSubstanceState(entity_p entity);

void Entity_UpdateCurrentBoneFrame(struct ss_bone_frame_s *bf, btScalar etr[16]);
void Entity_DoAnimCommands(entity_p entity, struct ss_animation_s *ss_anim, int changing);
void Entity_ProcessSector(struct entity_s *ent);
void Entity_SetAnimation(entity_p entity, int animation, int frame = 0, int another_model = -1);
void Entity_MoveForward(struct entity_s *ent, btScalar dist);
void Entity_MoveStrafe(struct entity_s *ent, btScalar dist);
void Entity_MoveVertical(struct entity_s *ent, btScalar dist);

// Helper functions

btScalar Entity_FindDistance(entity_p entity_1, entity_p entity_2);

room_sector_s* Entity_GetLowestSector(room_sector_s* sector);
room_sector_s* Entity_GetHighestSector(room_sector_s* sector);

#endif
