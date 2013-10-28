
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
struct bounding_volume_s;
struct character_s;

#define ENTITY_GHOST_COLLISION                    0                             // no one collisions
#define ENTITY_DYNAMIC_COLLISION                  1                             // hallo full physics interaction
#define ENTITY_CINEMATIC_COLLISION                2                             // doors and other moveable statics
#define ENTITY_STATIC_COLLISION                   3                             // static object - newer moved
#define ENTITY_ACTOR_COLLISION                    4                             // actor, enemies, NPC, animals
#define ENTITY_VEHICLE_COLLISION                  5                             // car, moto, bike

#define COLLISION_NONE                            (0x00000000)
#define COLLISION_TRIMESH                         (0x00000001)
#define COLLISION_BOX                             (0x00000002)

/*
 * уже конкретная игровая модель
 */

typedef struct entity_s
{
    uint32_t                            ID;                                     // ID
    uint8_t                             dir_flag;                               // (move direction)
    uint8_t                             state_flag;                             // I.E. must climb, climb action, ..., trigger action, crouch
    uint16_t                            anim_flags;                             // additional animation control param
    uint16_t                            move_type;                              // on floor / free fall / swim ....
    uint8_t                             was_rendered;                           // render once per frame trigger
    uint8_t                             was_rendered_lines;                     // same for debug lines
    uint8_t                             smooth_anim;
    
    btScalar                            current_speed;                          // current linear speed from animation info
    struct ss_bone_frame_s              bf;                                     // current boneframe with full frame information 
    struct bone_frame_s                *next_bf;
    btScalar                            angles[3];
    btScalar                            transform[16];                          // GL transformation matrix
    
    struct bounding_volume_s           *bv;                                     // oriented bounding volume - only for OCC test
    
    uint16_t                            current_stateID;
    int16_t                             current_animation;                      // 
    int16_t                             current_frame;                          // 
    
    btScalar                            period;                                 // one frame change period
    btScalar                            frame_time;                             // current time 
    btScalar                            lerp;
    
    struct skeletal_model_s            *model;                                  // 
    struct entity_s                    *next;                                   // 
    
    struct engine_container_s          *self;
    
    btVector3                           collision_offset;                       // current centre
    btRigidBody                       **bt_body;                                // FIXME - there are more complex objects
    struct character_s                 *character;
}entity_t, *entity_p;


entity_p Entity_Create();
void Entity_Clear(entity_p entity);

void Entity_UpdateRigidBody(entity_p ent);

struct state_change_s *Anim_FindStateChangeByAnim(struct animation_frame_s *anim, int state_change_anim);
struct state_change_s *Anim_FindStateChangeByID(struct animation_frame_s *anim, int id);
int Entity_GetAnimDispatchCase(struct entity_s *ent, int id);
void Entity_GetNextFrame(const entity_p entity, btScalar time, struct state_change_s *stc, int *frame, int *anim);
int Entity_Frame(entity_p entity, btScalar time, int state_id);                 // frame + trying to chabge state

void Entity_RebuildBV(entity_p ent);
void Entity_UpdateRotation(entity_p entity);

void Entity_UpdateCurrentBoneFrame(entity_p entity);
void Entity_MakeAnimCommands(entity_p entity, int changing);
void Entity_SetAnimation(entity_p entity, int animation, int frame);
void Entity_MoveForward(struct entity_s *ent, btScalar dist);
void Entity_MoveStrafe(struct entity_s *ent, btScalar dist);
void Entity_MoveVertical(struct entity_s *ent, btScalar dist);

#endif
