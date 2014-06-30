
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

// Lara's character behavior constants
#define DEFAULT_MAX_MOVE_ITERATIONS             (3)                             ///@FIXME: magic
#define DEFAULT_MIN_STEP_UP_HEIGHT              (128.0)                         ///@FIXME: check original
#define DEFAULT_MAX_STEP_UP_HEIGHT              (256.0)                         ///@FIXME: check original
#define DEFAULT_FALL_DAWN_HEIGHT                (320.0)                         ///@FIXME: check original
#define DEFAULT_CLIMB_UP_HEIGHT                 (1920.0)                        ///@FIXME: check original
#define DEFAULT_CRITICAL_SLANT_Z_COMPONENT      (0.810)                         ///@FIXME: cos(alpha = 30 deg)
#define DEFAULT_CRITICAL_WALL_COMPONENT         (0.707)                         ///@FIXME: cos(alpha = 45 deg)
#define DEFAULT_CHARACTER_SPEED_MULT            (31.5)                          ///@FIXME: magic - not like in original
#define DEFAULT_CHARACTER_SLIDE_SPEED_MULT      (75.0)                          ///@FIXME: magic - not like in original
#define DEFAULT_CHARACTER_CLIMB_R               (48.0)
#define DEFAULT_CHARACTER_WADE_DEPTH            (256.0)

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


// CHARACTER PARAMETERS DEFAULTS

#define CHARACTER_OPTION_HEALTH_MAX             (1000.0)      // 30 secs of air
#define CHARACTER_OPTION_AIR_MAX                (1800.0)      // 30 secs of air
#define CHARACTER_OPTION_SPRINT_MAX             (120.0)       // 4  secs of sprint
#define CHARACTER_OPTION_FREEZE_MAX             (240.0)       // 8  secs of freeze

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
    
    btScalar                                    water_level;
    int16_t                                     water;
}height_info_t, *height_info_p;

typedef struct character_command_s
{    
    btScalar    rot[3];
    int8_t      move[3];
    
    int8_t      kill;
    int8_t      vertical_collide;     
    int8_t      horizontal_collide;
    int8_t      slide;
    
    int8_t      roll;
    int8_t      jump;
    int8_t      crouch;
    int8_t      shift;
    int8_t      action;    
    int8_t      sprint;
    
    int8_t      flags;
}character_command_t, *character_command_p;

typedef struct character_options_s
{
    float       health;
    float       air;
    float       sprint;
    float       freeze;
}character_options_t, *character_options_p;

typedef struct character_s
{
    struct entity_s             *ent;                    // actor entity
    struct character_command_s   cmd;                    // character control commands
    struct character_options_s   opt;
    
    int                        (*state_func)(struct entity_s *ent, struct character_command_s *cmd);
    int16_t                      max_move_iterations;
    int16_t                      no_fix;
    
    btScalar                     speed_mult;
    btScalar                     min_step_up_height;
    btScalar                     max_step_up_height;
    btScalar                     max_climb_height;
    btScalar                     fall_down_height;
    btScalar                     critical_slant_z_component;
    btScalar                     critical_wall_component;

    btScalar                     climb_r;                // climbing sensor radius
    btScalar                     rx;                     // base character radius X       
    btScalar                     ry;                     // base character radius Y
    btScalar                     Height;                 // base character height
    btScalar                     wade_depth;             // water depth that enable wade walk
    uint8_t                      complex_collision;      // use complex collision flag
    btCollisionShape           **shapes;
    btCollisionShape            *shapeZ;                 // running / jumping
    btCapsuleShape              *shapeY;                 // swimming / crocodile

    btBoxShape                  *shapeBox;               // simple (128, 128, 128) sized box shape
    btSphereShape               *sphere;                 // needs to height calculation
    btSphereShape               *climb_sensor;
    btPairCachingGhostObject    *ghostObject;            // like Bullet character controller for penetration resolving.
    btManifoldArray             *manifoldArray;          // keep track of the contact manifolds
    
    btCollisionObject           *platform;
    btScalar                     local_platform[16];
    struct height_info_s         height_info;
    struct climb_info_s          climb;
    
    bt_engine_ClosestRayResultCallback                  *ray_cb;
    bt_engine_ClosestConvexResultCallback               *convex_cb;
}character_t, *character_p;

void Character_Create(struct entity_s *ent, btScalar rx, btScalar ry, btScalar h);
void Character_CreateCollisionObject(struct entity_s *ent);
void Character_Clean(struct entity_s *ent);

void Character_GetHeightInfo(btScalar pos[3], struct height_info_s *fc);
int Character_CheckNextStep(struct entity_s *ent, btScalar offset[3], struct height_info_s *nfc);
int Character_HasStopSlant(struct entity_s *ent, height_info_p next_fc);
climb_info_t Character_CheckClimbability(struct entity_s *ent, btScalar offset[3], struct height_info_s *nfc, btScalar test_height);
climb_info_t Character_CheckWallsClimbability(struct entity_s *ent);
int Character_RecoverFromPenetration(btPairCachingGhostObject *ghost, btManifoldArray *manifoldArray, btScalar correction[3]);
void Character_FixPenetrations(struct entity_s *ent, character_command_p cmd, btScalar move[3]);
void Character_CheckNextPenetration(struct entity_s *ent, character_command_p cmd, btScalar move[3]);

void Character_UpdateCurrentSpeed(struct entity_s *ent, int zeroVz);
void Character_UpdateCurrentHeight(struct entity_s *ent);
void Character_UpdatePlatformPreStep(struct entity_s *ent);
void Character_UpdatePlatformPostStep(struct entity_s *ent);
void Character_UpdateCollisionObject(struct entity_s *ent, btScalar z_factor);

void Character_SetToJump(struct entity_s *ent, btScalar v_vertical, btScalar v_horizontal);
void Character_Lean(struct entity_s *ent, character_command_p cmd, btScalar max_lean);
void Character_Inertia(struct entity_s *ent, int8_t command, btScalar max_speed, btScalar in_speed, btScalar out_speed);

int Character_MoveOnFloor(struct entity_s *ent, character_command_p cmd);
int Character_FreeFalling(struct entity_s *ent, character_command_p cmd);
int Character_MonkeyClimbing(struct entity_s *ent, character_command_p cmd);
int Character_WallsClimbing(struct entity_s *ent, character_command_p cmd);
int Character_Climbing(struct entity_s *ent, character_command_p cmd);
int Character_MoveUnderWater(struct entity_s *ent, character_command_p cmd);
int Character_MoveOnWater(struct entity_s *ent, character_command_p cmd);

void Character_ApplyCommands(struct entity_s *ent, struct character_command_s *cmd);
void Character_UpdateValues(struct entity_s *ent);

bool Character_IncreaseAir(struct entity_s *ent, float value);
bool Character_DecreaseAir(struct entity_s *ent, float value);
void Character_SetAir(struct entity_s *ent, float value);

bool Character_IncreaseHealth(struct entity_s *ent, float value);
bool Character_DecreaseHealth(struct entity_s *ent, float value);
void Character_SetHealth(struct entity_s *ent, float value);

bool Character_IncreaseSprint(struct entity_s *ent, float value);
bool Character_DecreaseSprint(struct entity_s *ent, float value);
void Character_SetSprint(struct entity_s *ent, float value);



#endif  // CHARACTER_CONTROLLER_H
