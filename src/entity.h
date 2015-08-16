
#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

struct room_s;
struct room_sector_s;
struct obb_s;
struct character_s;
struct ss_animation_s;
struct ss_bone_frame_s;
struct physics_data_s;

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

typedef struct entity_s
{
    uint32_t                            id;                 // Unique entity ID
    int32_t                             OCB;                // Object code bit (since TR4)
    uint8_t                             trigger_layout;     // Mask + once + event + sector status flags
    uint8_t                             dir_flag;           // (move direction)
    uint8_t                             was_rendered;       // render once per frame trigger
    uint8_t                             was_rendered_lines; // same for debug lines
    uint8_t                             no_fix_all;
    
    float                               timer;              // Set by "timer" trigger field
    uint32_t                            callback_flags;     // information about scripts callbacks
    uint16_t                            type_flags;
    uint16_t                            state_flags;

    uint16_t                            move_type;          // on floor / free fall / swim ....    
    float                               current_speed;      // current linear speed from animation info
    float                               speed_mult;
    float                               speed[3];           // speed of the entity XYZ
    
    float                               inertia_linear;     // linear inertia
    float                               inertia_angular[2]; // angular inertia - X and Y axes
    
    uint32_t                            no_fix_skeletal_parts;
    struct ss_bone_frame_s             *bf;                 // current boneframe with full frame information
    struct physics_data_s              *physics;
    float                               scaling[3];         // entity scaling
    float                               angles[3];
    float                               transform[16] __attribute__((packed, aligned(16))); // GL transformation matrix

    struct obb_s                       *obb;                // oriented bounding box

    struct room_sector_s               *current_sector;
    struct room_sector_s               *last_sector;

    struct engine_container_s          *self;

    float                               activation_offset[4];   // where we can activate object (dx, dy, dz, r)
    
    struct character_s                 *character;
}entity_t, *entity_p;


entity_p Entity_Create();
void Entity_Clear(entity_p entity);
void Entity_Enable(entity_p ent);
void Entity_Disable(entity_p ent);
void Entity_EnableCollision(entity_p ent);
void Entity_DisableCollision(entity_p ent);
void Entity_UpdateRoomPos(entity_p ent);

struct state_change_s *Anim_FindStateChangeByAnim(struct animation_frame_s *anim, int state_change_anim);
struct state_change_s *Anim_FindStateChangeByID(struct animation_frame_s *anim, uint32_t id);
int  Entity_GetAnimDispatchCase(struct entity_s *entity, uint32_t id);
void Entity_GetNextFrame(struct ss_bone_frame_s *bf, float time, struct state_change_s *stc, int16_t *frame, int16_t *anim, uint16_t anim_flags);
int  Entity_Frame(entity_p entity, float time);  // process frame + trying to change state

void Entity_RebuildBV(entity_p ent);
void Entity_UpdateTransform(entity_p entity);
void Entity_UpdateCurrentSpeed(entity_p entity, int zeroVz = 0);
void Entity_AddOverrideAnim(struct entity_s *ent, int model_id);
void Entity_CheckActivators(struct entity_s *ent);

int  Entity_GetSubstanceState(entity_p entity);

void Entity_UpdateCurrentBoneFrame(struct ss_bone_frame_s *bf, float etr[16]);
void Entity_UpdateRigidBody(struct entity_s *ent, int force);
void Entity_GhostUpdate(struct entity_s *ent);

int  Entity_GetPenetrationFixVector(struct entity_s *ent, float reaction[3], float move_global[3]);
int  Entity_CheckNextPenetration(struct entity_s *ent, float move[3]);
void Entity_FixPenetrations(struct entity_s *ent, float move[3]);

void Entity_CheckCollisionCallbacks(entity_p ent);
void Entity_DoAnimCommands(entity_p entity, struct ss_animation_s *ss_anim, int changing);
void Entity_ProcessSector(entity_p ent);
void Entity_SetAnimation(entity_p entity, int animation, int frame = 0, int another_model = -1);
void Entity_MoveForward(entity_p ent, float dist);
void Entity_MoveStrafe(entity_p ent, float dist);
void Entity_MoveVertical(entity_p ent, float dist);

// Helper functions

float Entity_FindDistance(entity_p entity_1, entity_p entity_2);

#endif
