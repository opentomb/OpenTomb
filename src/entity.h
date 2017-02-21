
#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include <stdlib.h>

struct room_sector_s;
struct obb_s;
struct character_s;
struct ss_animation_s;
struct ss_bone_frame_s;
struct physics_data_s;
struct inventory_node_s;

#define ENTITY_ID_NONE                              (0xFFFFFFFF)

#define ENTITY_STATE_ENABLED                        (0x0001)    // Entity is enabled.
#define ENTITY_STATE_ACTIVE                         (0x0002)    // Entity is animated.
#define ENTITY_STATE_VISIBLE                        (0x0004)    // Entity is visible.
#define ENTITY_STATE_COLLIDABLE                     (0x0008)    // Collisions enabled.

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

/*
 * SURFACE MOVEMENT DIRECTIONS
 */
#define ENT_STAY 0x00000000
#define ENT_MOVE_FORWARD 0x00000001
#define ENT_MOVE_BACKWARD 0x00000002
#define ENT_MOVE_LEFT 0x00000004
#define ENT_MOVE_RIGHT 0x00000008
#define ENT_MOVE_JUMP 0x00000010
#define ENT_MOVE_CROUCH 0x00000020

//   ====== ANIMATION COMMANDS ======
#define TR_ANIMCOMMAND_SETPOSITION  1
#define TR_ANIMCOMMAND_JUMPDISTANCE 2
#define TR_ANIMCOMMAND_EMPTYHANDS   3
#define TR_ANIMCOMMAND_KILL         4
#define TR_ANIMCOMMAND_PLAYSOUND    5
#define TR_ANIMCOMMAND_PLAYEFFECT   6
#define TR_ANIMCOMMAND_INTERACT     7

//   ====== ANIMATION EFFECTS FLAGS ======
#define TR_ANIMCOMMAND_CONDITION_LAND  0x4000
#define TR_ANIMCOMMAND_CONDITION_WATER 0X8000

//   ====== ANIMATION EFFECTS / FLIPEFFECTS ======
#define TR_EFFECT_CHANGEDIRECTION       0
#define TR_EFFECT_SHAKESCREEN           1
#define TR_EFFECT_PLAYFLOODSOUND        2
#define TR_EFFECT_BUBBLE                3
#define TR_EFFECT_ENDLEVEL              4
#define TR_EFFECT_ACTIVATECAMERA        5
#define TR_EFFECT_ACTIVATEKEY           6
#define TR_EFFECT_ENABLEEARTHQUAKES     7
#define TR_EFFECT_GETCROWBAR            8
#define TR_EFFECT_CURTAINFX             9  // Effect 9 is empty in TR4.
#define TR_EFFECT_PLAYSOUND_TIMERFIELD  10
#define TR_EFFECT_PLAYEXPLOSIONSOUND    11
#define TR_EFFECT_DISABLEGUNS           12
#define TR_EFFECT_ENABLEGUNS            13
#define TR_EFFECT_GETRIGHTGUN           14
#define TR_EFFECT_GETLEFTGUN            15
#define TR_EFFECT_FIRERIGHTGUN          16
#define TR_EFFECT_FIRELEFTGUN           17
#define TR_EFFECT_MESHSWAP1             18
#define TR_EFFECT_MESHSWAP2             19
#define TR_EFFECT_MESHSWAP3             20
#define TR_EFFECT_INV_ON                21 // Effect 21 is unknown at offset 4376F0.
#define TR_EFFECT_INV_OFF               22 // Effect 22 is unknown at offset 437700.
#define TR_EFFECT_HIDEOBJECT            23
#define TR_EFFECT_SHOWOBJECT            24
#define TR_EFFECT_STATUEFX              25 // Effect 25 is empty in TR4.
#define TR_EFFECT_RESETHAIR             26
#define TR_EFFECT_BOILERFX              27 // Effect 27 is empty in TR4.
#define TR_EFFECT_SETFOGCOLOUR          28
#define TR_EFFECT_GHOSTTRAP             29 // Effect 29 is unknown at offset 4372F0
#define TR_EFFECT_LARALOCATION          30
#define TR_EFFECT_CLEARSCARABS          31
#define TR_EFFECT_PLAYSTEPSOUND         32 // Also called FOOTPRINT_FX in TR4 source code.

// Effects 33 - 42 are assigned to FLIP_MAP0-FLIP_MAP9 in TR4 source code,
// but are empty in TR4 binaries.
#define TR_EFFECT_GETWATERSKIN          43
#define TR_EFFECT_REMOVEWATERSKIN       44
#define TR_EFFECT_LARALOCATIONPAD       45
#define TR_EFFECT_KILLALLENEMIES        46

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
    uint32_t                            id;                     // Unique entity ID
    int32_t                             OCB;                    // Object code bit (since TR4)
    
    uint32_t                            trigger_layout : 8;     // Mask + once + event + sector status flags
    uint32_t                            dir_flag : 8;           // (move direction)
    uint32_t                            move_type : 4;          // on floor / free fall / swim ....
    uint32_t                            no_fix_all : 1;
    uint32_t                            no_fix_z : 1;
    uint32_t                            no_anim_pos_autocorrection : 1;
    
    float                               timer;              // Set by "timer" trigger field
    uint32_t                            callback_flags;     // information about scripts callbacks
    uint16_t                            type_flags;
    uint16_t                            state_flags;

    float                               linear_speed;
    float                               anim_linear_speed;  // current linear speed from animation info
    float                               speed[3];           // speed of the entity XYZ
    
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

    float                               activation_offset[4];       // where we can activate object (dx, dy, dz, r)
    float                               activation_direction[4];    // direction_xyz, cos(lim)
    struct inventory_node_s            *inventory;
    struct character_s                 *character;
}entity_t, *entity_p;


entity_p Entity_Create();
void Entity_Clear(entity_p entity);
void Entity_Enable(entity_p ent);
void Entity_Disable(entity_p ent);
void Entity_EnableCollision(entity_p ent);
void Entity_DisableCollision(entity_p ent);
void Entity_UpdateRoomPos(entity_p ent);
void Entity_MoveToRoom(entity_p entity, struct room_s *new_room);

void Entity_Frame(entity_p entity, float time);  // process frame + trying to change state

void Entity_RebuildBV(entity_p ent);
void Entity_UpdateTransform(entity_p entity);
int  Entity_CanTrigger(entity_p activator, entity_p trigger);
void Entity_RotateToTriggerZ(entity_p activator, entity_p trigger);
void Entity_RotateToTrigger(entity_p activator, entity_p trigger);
void Entity_CheckActivators(struct entity_s *ent);
int  Entity_Activate(struct entity_s *entity_object, struct entity_s *entity_activator, uint16_t trigger_mask, uint16_t trigger_op, uint16_t trigger_lock, uint16_t trigger_timer);
int  Entity_Deactivate(struct entity_s *entity_object, struct entity_s *entity_activator);

int  Entity_GetSubstanceState(entity_p entity);

void Entity_UpdateRigidBody(struct entity_s *ent, int force);
void Entity_GhostUpdate(struct entity_s *ent);

int  Entity_GetPenetrationFixVector(struct entity_s *ent, float reaction[3], float ent_move[3], int16_t filter);
int  Entity_CheckNextPenetration(struct entity_s *ent, float move[3], float reaction[3], int16_t filter);
void Entity_FixPenetrations(struct entity_s *ent, float move[3], int16_t filter);

void Entity_CheckCollisionCallbacks(entity_p ent);
void Entity_DoAnimCommands(entity_p entity, struct ss_animation_s *ss_anim);
bool Entity_DoFlipEffect(entity_p entity, uint16_t effect_id, int16_t param);
void Entity_ProcessSector(entity_p ent);
void Entity_SetAnimation(entity_p entity, int anim_type, int animation, int frame, float new_transform[16] = NULL);
void Entity_MoveToSink(entity_p entity, struct static_camera_sink_s *sink);
void Entity_MoveForward(entity_p ent, float dist);
void Entity_MoveStrafe(entity_p ent, float dist);
void Entity_MoveVertical(entity_p ent, float dist);

#endif
