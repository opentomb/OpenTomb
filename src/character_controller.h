
#ifndef CHARACTER_CONTROLLER_H
#define CHARACTER_CONTROLLER_H

#include <stdint.h>
#include "bullet/LinearMath/btScalar.h"
#include "bullet/LinearMath/btVector3.h"
#include "bullet/BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "bullet/BulletCollision/CollisionShapes/btSphereShape.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "bullet/BulletDynamics/Dynamics/btRigidBody.h"
#include "bullet/BulletCollision/CollisionShapes/btBoxShape.h"
#include "bullet/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"
#include "bullet/BulletCollision/CollisionShapes/btMultiSphereShape.h"
#include "engine.h"


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


#define CHARACTER_BOX_HALF_SIZE (128.0)
#define CHARACTER_BASE_RADIUS   (128.0)
#define CHARACTER_BASE_HEIGHT   (512.0)


/*
 * default legs offsets
 */
#define LEFT_LEG                    (3)
#define RIGHT_LEG                   (6)
#define LEFT_HAND                   (13)
#define RIGHT_HAND                  (10)

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

#define CHARACTER_USE_COMPLEX_COLLISION         (1)

// Lara's character behavior constants
#define DEFAULT_MAX_MOVE_ITERATIONS             (3)                             ///@FIXME: magic
#define DEFAULT_MIN_STEP_UP_HEIGHT              (128.0)                         ///@FIXME: check original
#define DEFAULT_MAX_STEP_UP_HEIGHT              (256.0 + 32.0)                  ///@FIXME: check original
#define DEFAULT_FALL_DAWN_HEIGHT                (320.0)                         ///@FIXME: check original
#define DEFAULT_CLIMB_UP_HEIGHT                 (1920.0)                        ///@FIXME: check original
#define DEFAULT_CRITICAL_SLANT_Z_COMPONENT      (0.810)                         ///@FIXME: cos(alpha = 30 deg)
#define DEFAULT_CRITICAL_WALL_COMPONENT         (0.707)                         ///@FIXME: cos(alpha = 45 deg)
#define DEFAULT_CHARACTER_SPEED_MULT            (31.5)                          ///@FIXME: magic - not like in original
#define DEFAULT_CHARACTER_SLIDE_SPEED_MULT      (75.0)                          ///@FIXME: magic - not like in original
#define DEFAULT_CHARACTER_CLIMB_R               (32.0)
#define DEFAULT_CHARACTER_WADE_DEPTH            (256.0)
// If less than this much of Lara is looking out of the water, she goes from wading to swimming.
#define DEFAULT_CHARACTER_SWIM_DEPTH            (100.0) ///@FIXME: Guess

// Speed limits

#define FREE_FALL_SPEED_1        (2000)
#define FREE_FALL_SPEED_2        (4500)
#define FREE_FALL_SPEED_MAXSAFE  (5500)
#define FREE_FALL_SPEED_CRITICAL (7500)
#define FREE_FALL_SPEED_MAXIMUM  (7800)

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
    PARAM_EXTRA1,
    PARAM_EXTRA2,
    PARAM_EXTRA3,
    PARAM_EXTRA4,
    PARAM_LASTINDEX
};

// CHARACTER PARAMETERS DEFAULTS

#define PARAM_ABSOLUTE_MAX                (-1)

#define LARA_PARAM_HEALTH_MAX             (1000.0)      // 30 secs of air
#define LARA_PARAM_AIR_MAX                (1800.0)      // 30 secs of air
#define LARA_PARAM_STAMINA_MAX            (120.0)       // 4  secs of sprint
#define LARA_PARAM_WARMTH_MAX             (240.0)       // 8  secs of freeze

struct engine_container_s;
struct entity_s;
class bt_engine_ClosestConvexResultCallback;
class bt_engine_ClosestRayResultCallback;
class btPairCachingGhostObject;
class btCollisionObject;
class btConvexShape;

typedef struct climb_info_s
{
    int8_t                         height_info;
    int8_t                         can_hang;

    btScalar                       point[3];
    btScalar                       n[3];
    btScalar                       t[3];
    btScalar                       up[3];
    btScalar                       floor_limit;
    btScalar                       ceiling_limit;
    btScalar                       next_z_space;

    int8_t                         wall_hit;                                    // 0x00 - none, 0x01 hands only climb, 0x02 - 4 point wall climbing
    int8_t                         edge_hit;
    btVector3                      edge_point;
    btVector3                      edge_normale;
    btVector3                      edge_tan_xy;
    btScalar                       edge_z_ang;
    btCollisionObject             *edge_obj;
}climb_info_t, *climb_info_p;

typedef struct height_info_s
{
    bt_engine_ClosestRayResultCallback         *cb;
    bt_engine_ClosestConvexResultCallback      *ccb;
    btConvexShape                              *sp;

    int8_t                                      ceiling_climb;
    int8_t                                      walls_climb;
    int8_t                                      walls_climb_dir;

    btVector3                                   floor_normale;
    btVector3                                   floor_point;
    int16_t                                     floor_hit;
    btCollisionObject                          *floor_obj;

    btVector3                                   ceiling_normale;
    btVector3                                   ceiling_point;
    int16_t                                     ceiling_hit;
    btCollisionObject                          *ceiling_obj;

    btScalar                                    transition_level;
    int16_t                                     water;
    int16_t                                     quicksand;
}height_info_t, *height_info_p;

typedef struct character_command_s
{
    btScalar    rot[3];
    int8_t      move[3];

    int8_t      roll;
    int8_t      jump;
    int8_t      crouch;
    int8_t      shift;
    int8_t      action;
    int8_t      ready_weapon;
    int8_t      sprint;

    int8_t      flags;
}character_command_t, *character_command_p;

typedef struct character_response_s
{
    int8_t      kill;
    int8_t      vertical_collide;
    int8_t      horizontal_collide;
    int8_t      step_up;
    int8_t      slide;
}character_response_t, *character_response_p;

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

typedef struct inventory_node_s
{
    uint32_t                    id;
    int32_t                     count;
    uint32_t                    max_count;
    struct inventory_node_s    *next;
}inventory_node_t, *inventory_node_p;

typedef struct character_collision_node_s
{
    btCollisionObject          *obj;
}character_collision_node_t, *character_collision_node_p;

typedef struct character_s
{
    struct entity_s             *ent;                    // actor entity
    struct character_command_s   cmd;                    // character control commands
    struct character_response_s  resp;                   // character response info (collides, slide, next steps, drops, e.t.c.)
    struct inventory_node_s     *inventory;
    struct character_param_s     parameters;
    struct character_stats_s     statistics;
    
    int                          current_weapon;
    int                          weapon_current_state;
    
    int                        (*state_func)(struct entity_s *ent, struct ss_animation_s *ss_anim);
    int16_t                      max_move_iterations;
    
    int8_t                       no_fix;                     
    int8_t                       cam_follow_center;

    btScalar                     speed_mult;
    btScalar                     min_step_up_height;
    btScalar                     max_step_up_height;
    btScalar                     max_climb_height;
    btScalar                     fall_down_height;
    btScalar                     critical_slant_z_component;
    btScalar                     critical_wall_component;

    btScalar                     climb_r;                // climbing sensor radius
    btScalar                     forvard_size;           // offset for climbing calculation
    btScalar                     Height;                 // base character height
    btScalar                     wade_depth;             // water depth that enable wade walk
    btScalar                     swim_depth;             // depth offset for starting to swim
    int                          ghost_step_up_map_filter;
    
    btCollisionShape           **shapes;
    btSphereShape               *sphere;                 // needs to height calculation
    btSphereShape               *climb_sensor;
    btPairCachingGhostObject   **ghostObjects;           // like Bullet character controller for penetration resolving.
    btManifoldArray             *manifoldArray;          // keep track of the contact manifolds

    struct height_info_s         height_info;
    struct climb_info_s          climb;

    struct entity_s             *traversed_object;
    
    struct character_collision_node_s                   *last_collisions;
    bt_engine_ClosestRayResultCallback                  *ray_cb;
    bt_engine_ClosestConvexResultCallback               *convex_cb;
}character_t, *character_p;

void Character_Create(struct entity_s *ent);
void Character_Clean(struct entity_s *ent);

int32_t Character_AddItem(struct entity_s *ent, uint32_t item_id, int32_t count);       // returns items count after in the function's end
int32_t Character_RemoveItem(struct entity_s *ent, uint32_t item_id, int32_t count);    // returns items count after in the function's end
int32_t Character_RemoveAllItems(struct entity_s *ent);
int32_t Character_GetItemsCount(struct entity_s *ent, uint32_t item_id);                // returns items count

void Character_GetHeightInfo(btScalar pos[3], struct height_info_s *fc, btScalar v_offset = 0.0);
int Character_CheckNextStep(struct entity_s *ent, btScalar offset[3], struct height_info_s *nfc);
int Character_HasStopSlant(struct entity_s *ent, height_info_p next_fc);
climb_info_t Character_CheckClimbability(struct entity_s *ent, btScalar offset[3], struct height_info_s *nfc, btScalar test_height);
climb_info_t Character_CheckWallsClimbability(struct entity_s *ent);
int Ghost_GetPenetrationFixVector(btPairCachingGhostObject *ghost, btManifoldArray *manifoldArray, btScalar correction[3]);
void Character_GhostUpdate(struct entity_s *ent);
int Character_GetPenetrationFixVector(struct entity_s *ent, btScalar reaction[3]);
void Character_FixPenetrations(struct entity_s *ent, btScalar move[3]);
void Character_CheckNextPenetration(struct entity_s *ent, btScalar move[3]);

bool Character_WasCollisionBodyParts(struct entity_s *ent, uint32_t parts_flags);
struct engine_container_s *Character_GetRemoveCollisionBodyParts(struct entity_s *ent, uint32_t parts_flags);

void Character_UpdateCurrentHeight(struct entity_s *ent);
void Character_UpdatePlatformPreStep(struct entity_s *ent);
void Character_UpdatePlatformPostStep(struct entity_s *ent);

void Character_SetToJump(struct entity_s *ent, btScalar v_vertical, btScalar v_horizontal);
void Character_Lean(struct entity_s *ent, character_command_p cmd, btScalar max_lean);
void Character_Inertia(struct entity_s *ent, int8_t command, btScalar max_speed, btScalar in_speed, btScalar out_speed);

int Character_MoveOnFloor(struct entity_s *ent);
int Character_FreeFalling(struct entity_s *ent);
int Character_MonkeyClimbing(struct entity_s *ent);
int Character_WallsClimbing(struct entity_s *ent);
int Character_Climbing(struct entity_s *ent);
int Character_MoveUnderWater(struct entity_s *ent);
int Character_MoveOnWater(struct entity_s *ent);

int Character_FindTraverse(struct entity_s *ch);
int Sector_AllowTraverse(struct room_sector_s *rs, btScalar floor, struct engine_container_s *cont);
int Character_CheckTraverse(struct entity_s *ch, struct entity_s *obj);

void Character_ApplyCommands(struct entity_s *ent);
void Character_UpdateParams(struct entity_s *ent);

float Character_GetParam(struct entity_s *ent, int parameter);
int   Character_SetParam(struct entity_s *ent, int parameter, float value);
int   Character_ChangeParam(struct entity_s *ent, int parameter, float value);
int   Character_SetParamMaximum(struct entity_s *ent, int parameter, float max_value);

int   Character_SetWeaponModel(struct entity_s *ent, int weapon_model, int armed);

bool IsCharacter(struct entity_s *ent);

#endif  // CHARACTER_CONTROLLER_H
