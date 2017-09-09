
#ifndef CHARACTER_CONTROLLER_H
#define CHARACTER_CONTROLLER_H

#include <stdint.h>

#include "physics/physics.h"
#include "physics/hair.h"
#include "physics/ragdoll.h"

/*------ Lara's model-------
             .=.
            | 14|
             \ / \
         / |     | \
    11  / |   7   | \  8
       /   |     |   \
       |    =====    |
    12 |    =====    | 9
       |   /  0  \   |
    13 0  /_______\  0 10
          |  | |  |
          |1 | |4 |
          |  | |  |
          |__| |__|
          |  | |  |
          |2 | |5 |
          |  | |  |
          |__| |__|
       3  |__| |__|  6
--------------------------*/

#define BODY_PART_BODY_LOW      (0x00000001)            // 0
#define BODY_PART_BODY_UPPER    (0x00000002)            // 7
#define BODY_PART_HEAD          (0x00000004)            // 14

#define BODY_PART_LEFT_HAND_1   (0x00000008)            // 11
#define BODY_PART_LEFT_HAND_2   (0x00000010)            // 12
#define BODY_PART_LEFT_HAND_3   (0x00000020)            // 13
#define BODY_PART_RIGHT_HAND_1  (0x00000040)            // 8
#define BODY_PART_RIGHT_HAND_2  (0x00000080)            // 9
#define BODY_PART_RIGHT_HAND_3  (0x00000100)            // 10

#define BODY_PART_LEFT_LEG_1    (0x00000200)            // 1
#define BODY_PART_LEFT_LEG_2    (0x00000400)            // 2
#define BODY_PART_LEFT_LEG_3    (0x00000800)            // 3
#define BODY_PART_RIGHT_LEG_1   (0x00001000)            // 4
#define BODY_PART_RIGHT_LEG_2   (0x00002000)            // 5
#define BODY_PART_RIGHT_LEG_3   (0x00004000)            // 6

#define BODY_PART_LEGS_1        (BODY_PART_LEFT_LEG_1 | BODY_PART_RIGHT_LEG_1)
#define BODY_PART_LEGS_2        (BODY_PART_LEFT_LEG_2 | BODY_PART_RIGHT_LEG_2)
#define BODY_PART_LEGS_3        (BODY_PART_LEFT_LEG_3 | BODY_PART_RIGHT_LEG_3)

#define BODY_PART_HANDS_1        (BODY_PART_LEFT_HAND_1 | BODY_PART_RIGHT_HAND_1)
#define BODY_PART_HANDS_2        (BODY_PART_LEFT_HAND_2 | BODY_PART_RIGHT_HAND_2)
#define BODY_PART_HANDS_3        (BODY_PART_LEFT_HAND_3 | BODY_PART_RIGHT_HAND_3)

#define BODY_PART_HANDS          (BODY_PART_HANDS_1 | BODY_PART_HANDS_2 | BODY_PART_HANDS_3)
#define BODY_PART_LEGS           (BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3)

#define CHARACTER_BOX_HALF_SIZE (128.0)
#define CHARACTER_BASE_RADIUS   (128.0)
#define CHARACTER_BASE_HEIGHT   (512.0)

/*
 * ENTITY MOVEMENT TYPES
 */

#define MOVE_STATIC_POS         (0)
#define MOVE_KINEMATIC          (1)
#define MOVE_ON_FLOOR           (2)
#define MOVE_WADE               (3)
#define MOVE_QUICKSAND          (4)
#define MOVE_ON_WATER           (5)
#define MOVE_UNDERWATER         (6)
#define MOVE_FREE_FALLING       (7)
#define MOVE_CLIMBING           (8)
#define MOVE_MONKEYSWING        (9)
#define MOVE_WALLS_CLIMB        (10)
#define MOVE_DOZY               (11)
#define MOVE_FLY                (12)

#define CHARACTER_USE_COMPLEX_COLLISION         (1)

// Lara's character behavior constants
#define DEFAULT_MIN_STEP_UP_HEIGHT              (128.0)                         ///@FIXME: check original
#define DEFAULT_MAX_STEP_UP_HEIGHT              (256.0 + 32.0)                  ///@FIXME: check original
#define DEFAULT_FALL_DOWN_HEIGHT                (320.0)                         ///@FIXME: check original
#define DEFAULT_CLIMB_UP_HEIGHT                 (1920.0)                        ///@FIXME: check original
#define DEFAULT_CRITICAL_SLANT_Z_COMPONENT      (0.810)                         ///@FIXME: cos(alpha = 30 deg)
#define DEFAULT_CRITICAL_WALL_COMPONENT         (-0.707)                        ///@FIXME: cos(alpha = 45 deg)
#define DEFAULT_CHARACTER_SPEED_MULT            (31.5)                          ///@FIXME: magic - not like in original
#define DEFAULT_CHARACTER_SLIDE_SPEED_MULT      (75.0)                          ///@FIXME: magic - not like in original
#define DEFAULT_CHARACTER_CLIMB_R               (32.0)
#define DEFAULT_CHARACTER_WADE_DEPTH            (256.0)
// If less than this much of Lara is looking out of the water, she goes from wading to swimming.
#define DEFAULT_CHARACTER_SWIM_DEPTH            (100.0) ///@FIXME: Guess

// Speed limits

#define FREE_FALL_SPEED_1        (2000.0)
#define FREE_FALL_SPEED_2        (4500.0)
#define FREE_FALL_SPEED_MAXSAFE  (5500.0)
#define FREE_FALL_SPEED_CRITICAL (7500.0)
#define FREE_FALL_SPEED_MAXIMUM  (7800.0)

#define MAX_SPEED_UNDERWATER     (64.0)
#define MAX_SPEED_ONWATER        (24.0)
#define MAX_SPEED_QUICKSAND      (5.0 )

#define ROT_SPEED_UNDERWATER     (1.0)
#define ROT_SPEED_ONWATER        (1.5)
#define ROT_SPEED_LAND           (2.25)
#define ROT_SPEED_FREEFALL       (0.25)
#define ROT_SPEED_MONKEYSWING    (1.75)

#define INERTIA_SPEED_UNDERWATER (1.0)
#define INERTIA_SPEED_ONWATER    (1.5)

// flags constants
#define CHARACTER_SLIDE_FRONT                   (0x02)
#define CHARACTER_SLIDE_BACK                    (0x01)

/*
 * Next step height information
 */
#define CHARACTER_STEP_DOWN_CAN_HANG            (-0x04)                         // enough height to hang here
#define CHARACTER_STEP_DOWN_DROP                (-0x03)                         // big height, cannot walk next, drop only
#define CHARACTER_STEP_DOWN_BIG                 (-0x02)                         // enough height change, step down is needed
#define CHARACTER_STEP_DOWN_LITTLE              (-0x01)                         // too little height change, step down is not needed
#define CHARACTER_STEP_HORIZONTAL               (0x00)                          // horizontal plane
#define CHARACTER_STEP_UP_LITTLE                (0x01)                          // too little height change, step up is not needed
#define CHARACTER_STEP_UP_BIG                   (0x02)                          // enough height change, step up is needed
#define CHARACTER_STEP_UP_CLIMB                 (0x03)                          // big height, cannot walk next, climb only
#define CHARACTER_STEP_UP_IMPOSSIBLE            (0x04)                          // too big height, no one ways here, or phantom case

#define CLIMB_ABSENT                            (0x00)
#define CLIMB_HANG_ONLY                         (0x01)
#define CLIMB_ALT_HEIGHT                        (0x02)
#define CLIMB_FULL_HEIGHT                       (0x03)

// CHARACTER PARAMETERS TYPES

enum CharParameters
{
    PARAM_HEALTH,
    PARAM_AIR,
    PARAM_STAMINA,
    PARAM_WARMTH,
    PARAM_HIT_DAMAGE,
    PARAM_EXTRA1,
    PARAM_EXTRA2,
    PARAM_EXTRA3,
    PARAM_EXTRA4,
    PARAM_LASTINDEX
};

// CHARACTER PARAMETERS DEFAULTS

#define PARAM_ABSOLUTE_MAX                (-1)

#define LARA_PARAM_HEALTH_MAX             (1000.0)      // 1000 HP
#define LARA_PARAM_AIR_MAX                (3600.0)      // 60 secs of air
#define LARA_PARAM_STAMINA_MAX            (120.0)       // 4  secs of sprint
#define LARA_PARAM_WARMTH_MAX             (240.0)       // 8  secs of freeze

struct engine_container_s;
struct entity_s;

typedef struct climb_info_s
{
    int8_t                      height_info;
    int8_t                      can_hang;
    int8_t                      wall_hit;                                       // 0x00 - none, 0x01 hands only climb, 0x02 - 4 point wall climbing
    int8_t                      edge_hit;

    float                       point[3];
    float                       n[3];
    float                       t[3];
    float                       up[3];
    float                       next_z_space;

    float                       edge_point[3];
    float                       edge_normale[3];
    float                       edge_tan_xy[2];
    float                       edge_z_ang;

    struct engine_container_s  *edge_obj;
}climb_info_t, *climb_info_p;

typedef struct height_info_s
{
    int8_t                                   ceiling_climb;
    int8_t                                   walls_climb;
    int8_t                                   walls_climb_dir;
    uint8_t                                  slide; //0 - none, 1 - forward, 2 - backward
    struct engine_container_s               *self;

    struct collision_result_s                floor_hit;
    struct collision_result_s                ceiling_hit;

    float                                    transition_level;
    int16_t                                  water;
    int16_t                                  quicksand;
}height_info_t, *height_info_p;

typedef struct character_command_s
{
    int8_t      rot[3];
    int8_t      move[3];
    int8_t      flags;

    uint16_t    roll : 1;
    uint16_t    jump : 1;
    uint16_t    crouch : 1;
    uint16_t    shift : 1;
    uint16_t    action : 1;
    uint16_t    ready_weapon : 1;
    uint16_t    sprint : 1;
}character_command_t, *character_command_p;

typedef struct character_state_s
{
    uint32_t    floor_collide : 1;
    uint32_t    ceiling_collide : 1;
    uint32_t    wall_collide : 1;
    uint32_t    step_z : 2;     //0 - none, 1 - dz to step up, 2 - dz to step down;
    uint32_t    uw_current : 1;
    uint32_t    attack : 1;
    uint32_t    dead : 2;
    uint32_t    ragdoll : 1;
    uint32_t    burn : 1;
    uint32_t    crouch : 1;
    uint32_t    sprint : 1;
    uint32_t    tightrope : 1;
}character_state_t, *character_state_p;

typedef struct character_param_s
{
    float       param[PARAM_LASTINDEX];
    float       maximum[PARAM_LASTINDEX];
}character_param_t, *character_param_p;

typedef struct character_stats_s
{
    float       distance;
    uint32_t    secrets_level;         // Level amount of secrets.
    uint32_t    secrets_game;          // Overall amount of secrets.
    uint32_t    ammo_used;
    uint32_t    hits;
    uint32_t    kills;
    uint32_t    medipacks_used;
    uint32_t    saves_used;
}character_stats_t, *character_stats_p;


typedef struct character_s
{
    struct entity_s            *ent;                    // actor entity
    struct character_command_s  cmd;                    // character control commands
    struct character_state_s    state;                  // character state info (collides, slide, next steps, drops, e.t.c.)

    struct character_param_s    parameters;
    struct character_stats_s    statistics;

    uint32_t                    target_id;
    int16_t                     cam_follow_center;
    int8_t                      hair_count;
    int8_t                      path_dist;                                      // 0 .. n_path - 1
    struct hair_s             **hairs;
    struct rd_setup_s          *ragdoll;
    struct room_box_s          *path[8];
    struct room_sector_s       *path_target;
    int16_t                     ai_zone;
    uint16_t                    ai_zone_type;

    uint16_t                    bone_head;
    uint16_t                    bone_torso;
    uint16_t                    bone_l_hand_start;
    uint16_t                    bone_l_hand_end;
    uint16_t                    bone_r_hand_start;
    uint16_t                    bone_r_hand_end;
    int16_t                     current_weapon;
    int16_t                     weapon_current_state;

    int                        (*state_func)(struct entity_s *ent, struct ss_animation_s *ss_anim);
    void                       (*set_key_anim_func)(struct entity_s *ent, struct ss_animation_s *ss_anim, int key_anim);
    float                       linear_speed_mult;
    float                       rotate_speed_mult;
    float                       min_step_up_height;
    float                       max_step_up_height;
    float                       max_climb_height;
    float                       fall_down_height;
    float                       critical_slant_z_component;
    float                       critical_wall_component;

    float                       climb_r;                // climbing sensor radius
    float                       forvard_size;           // offset for climbing calculation
    float                       height;                 // base character height
    float                       wade_depth;             // water depth that enable wade walk
    float                       swim_depth;             // depth offset for starting to swim

    float                       sphere;                 // needs to height calculation
    float                       climb_sensor;

    struct height_info_s        height_info;
    struct climb_info_s         climb;

    struct entity_s            *traversed_object;
}character_t, *character_p;

void Character_Create(struct entity_s *ent);
void Character_Delete(struct entity_s *ent);
void Character_Update(struct entity_s *ent);
void Character_UpdatePath(struct entity_s *ent, struct room_sector_s *target);
void Character_GoByPathToTarget(struct entity_s *ent);
void Character_UpdateAI(struct entity_s *ent);

void Character_GetHeightInfo(struct entity_s *ent, float pos[3], struct height_info_s *fc, float v_offset = 0.0);
int  Character_CheckNextStep(struct entity_s *ent, float offset[3], struct height_info_s *nfc);
int  Character_HasStopSlant(struct entity_s *ent, height_info_p next_fc);
void Character_GetMiddleHandsPos(const struct entity_s *ent, float pos[3]);
void Character_CheckClimbability(struct entity_s *ent, struct climb_info_s *climb, float test_from[3], float test_to[3]);
void Character_CheckWallsClimbability(struct entity_s *ent, struct climb_info_s *climb);

void Character_UpdateCurrentSpeed(struct entity_s *ent, int zeroVz = 0);
void Character_UpdateCurrentHeight(struct entity_s *ent);

void Character_SetToJump(struct entity_s *ent, float v_vertical, float v_horizontal);
void Character_Lean(struct entity_s *ent, character_command_p cmd, float max_lean);
void Character_LookAt(struct entity_s *ent, float target[3]);
void Character_ClearLookAt(struct entity_s *ent);

int Character_MoveOnFloor(struct entity_s *ent);
int Character_MoveFly(struct entity_s *ent);
int Character_FreeFalling(struct entity_s *ent);
int Character_MonkeyClimbing(struct entity_s *ent);
int Character_WallsClimbing(struct entity_s *ent);
int Character_Climbing(struct entity_s *ent);
int Character_MoveUnderWater(struct entity_s *ent);
int Character_MoveOnWater(struct entity_s *ent);

int Character_FindTraverse(struct entity_s *ch);
int Sector_AllowTraverse(struct room_sector_s *rs, float floor);
int Character_CheckTraverse(struct entity_s *ch, struct entity_s *obj);

void Character_ApplyCommands(struct entity_s *ent);
void Character_UpdateParams(struct entity_s *ent);

float Character_GetParam(struct entity_s *ent, int parameter);
int   Character_SetParam(struct entity_s *ent, int parameter, float value);
int   Character_ChangeParam(struct entity_s *ent, int parameter, float value);
int   Character_SetParamMaximum(struct entity_s *ent, int parameter, float max_value);

int Character_IsTargetAccessible(struct entity_s *character, struct entity_s *target);
struct entity_s *Character_FindTarget(struct entity_s *ent);
void  Character_SetTarget(struct entity_s *ent, uint32_t target_id);
int   Character_SetWeaponModel(struct entity_s *ent, int weapon_model, int weapon_state);

/*
 * ss_animation callbacks
 */
int   Character_DoOneHandWeponFrame(struct entity_s *ent, struct  ss_animation_s *ss_anim, float time);
int   Character_DoTwoHandWeponFrame(struct entity_s *ent, struct  ss_animation_s *ss_anim, float time);

#endif  // CHARACTER_CONTROLLER_H
