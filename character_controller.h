
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
#include "engine.h"

#define DEFAULT_MAX_MOVE_ITERATIONS             (3)                             ///@FIXME: magic
#define DEFAULT_MIN_STEP_UP_HEIGHT              (128.0)                         ///@FIXME: check original
#define DEFAULT_MAX_STEP_UP_HEIGHT              (320.0)                         ///@FIXME: check original
#define DEFAULT_FALL_DAWN_HEIGHT                (320.0)                         ///@FIXME: check original
#define DEFAULT_CLIMB_UP_HEIGHT                 (4096.0)                        ///@FIXME: check original
#define DEFAULT_CRITICAL_SLANT_Z_COMPONENT      (0.810)                         ///@FIXME: cos(alpha = 30 deg)
#define DEFAULT_CRITICAL_WALL_COMPONENT         (0.707)                         ///@FIXME: cos(alpha = 45 deg)
#define DEFAULT_CHARACTER_SPEED_MULT            (32.0)                          ///@FIXME: magic - not like in original
#define DEFAULT_CHARACTER_SLIDE_SPEED_MULT      (75.0)                          ///@FIXME: magic - not like in original
#define DEFAULT_CHARACTER_CLIMB_R               (72.0)

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

struct entity_s;
class bt_engine_ClosestConvexResultCallback;
class bt_engine_ClosestRayResultCallback;
class btPairCachingGhostObject;

typedef struct climb_info_s
{
    int8_t      height_info;
    int8_t      climb_flag;
}climb_info_t, *climb_info_p;

typedef struct height_info_s
{
    bt_engine_ClosestRayResultCallback         *cb;
    bt_engine_ClosestConvexResultCallback      *ccb;
    
    btVector3                                   floor_normale;
    btVector3                                   floor_point;
    int16_t                                     floor_hit;
    struct engine_container_s                  *floor_obj;
    
    btVector3                                   ceiling_normale;
    btVector3                                   ceiling_point;
    int16_t                                     ceiling_hit;
    struct engine_container_s                  *ceiling_obj;
    
    int16_t                                     edge_hit;
    btVector3                                   edge_point;
    btVector3                                   edge_normale;
    btVector3                                   edge_tan_xy;
    btScalar                                    edge_z_ang;
    
    btScalar                                    water_level;
    int16_t                                     water;
}height_info_t, *height_info_p;

typedef struct character_command_s
{    
    btScalar    rot[3];
    btScalar    climb_pos[3];
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

typedef struct character_s
{
    struct entity_s             *ent;                    // actor entity
    struct character_command_s   cmd;                    // character control commands
    int16_t                      max_move_iterations;
    int16_t                      no_fix;
    
    btVector3                    speed;                  // speed of character
    
    btScalar                     speed_mult;
    btScalar                     min_step_up_height;
    btScalar                     max_step_up_height;
    btScalar                     max_climb_height;
    btScalar                     fall_down_height;
    btScalar                     critical_slant_z_component;
    btScalar                     critical_wall_component;

    btScalar                     climb_r;                // climbing sensor radius
    btScalar                     Radius;                 // base character radius 
    btScalar                     Height;                 // base character height
    
    btCapsuleShapeZ             *shapeZ;                 // running / jumping
    btCapsuleShape              *shapeY;                 // swimming / crocodile
    btBoxShape                  *shapeBox;               // simple (128, 128, 128) sized box shape
    btSphereShape               *sphere;                 // needs to height calculation
    btSphereShape               *climb_sensor;
    btPairCachingGhostObject    *ghostObject;            // like Bullet character controller for penetration resolving.
    btManifoldArray             *manifoldArray;          // keep track of the contact manifolds
    
    struct height_info_s         height_info;
    
    bt_engine_ClosestRayResultCallback                  *ray_cb;
    bt_engine_ClosestConvexResultCallback               *convex_cb;
}character_t, *character_p;

void Character_Create(struct entity_s *ent, btScalar r, btScalar h);
void Character_Clean(struct entity_s *ent);

void Character_GetHeightInfo(btScalar pos[3], struct height_info_s *fc);
int Character_CheckNextStep(struct entity_s *ent, btScalar offset[3], struct height_info_s *nfc);
climb_info_t Character_CheckClimbability(struct entity_s *ent, btScalar offset[3], struct height_info_s *nfc, btScalar test_height);
int Character_RecoverFromPenetration(btPairCachingGhostObject *ghost, btManifoldArray *manifoldArray, btScalar react[3]);
void Character_FixPenetrations(struct entity_s *ent, character_command_p cmd, /*struct height_info_s *fc,*/ btScalar move[3]);
void Character_CheckNextPenetration(struct entity_s *ent, character_command_p cmd, btScalar move[3]);

void Character_UpdateCurrentSpeed(struct entity_s *ent, int zeroVz);
int Character_SetToJump(struct entity_s *ent, character_command_p cmd, btScalar vz);
void Character_UpdateCurrentRoom(struct entity_s *ent);
void Character_UpdateCurrentHeight(struct entity_s *ent);
void Character_UpdateCollisionObject(struct entity_s *ent, btScalar z_factor);

int Character_MoveOnFloor(struct entity_s *ent, character_command_p cmd);
int Character_FreeFalling(struct entity_s *ent, character_command_p cmd);
int Character_Climbing(struct entity_s *ent, character_command_p cmd);
int Character_MoveUnderWater(struct entity_s *ent, character_command_p cmd);
int Character_MoveOnWater(struct entity_s *ent, character_command_p cmd);

#endif  // CHARACTER_CONTROLLER_H
