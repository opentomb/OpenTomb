
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
struct ss_bone_frame_s;

#define ENTITY_IS_ACTIVE                          (0x00000001)
#define ENTITY_CAN_TRIGGER                        (0x00000002)                      
#define ENTITY_IS_TRIGGER                         (0x00000004)
#define ENTITY_IS_PICKABLE                        (0x00000008)

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
#define ENTITY_SUBSTANCE_QUICKSAND                4

#define COLLISION_NONE                            (0x00000000)
#define COLLISION_TRIMESH                         (0x00000001)
#define COLLISION_BOX                             (0x00000002)

// TR floor data functions

#define TR_FD_FUNC_PORTALSECTOR                 0x01
#define TR_FD_FUNC_FLOORSLANT                   0x02
#define TR_FD_FUNC_CEILINGSLANT                 0x03
#define TR_FD_FUNC_TRIGGER                      0x04
#define TR_FD_FUNC_DEATH                        0x05
#define TR_FD_FUNC_CLIMB                        0x06
#define TR_FD_FUNC_FLOORTRIANGLE_NW             0x07    //  [_\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NE             0x08    //  [_/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW           0x09    //  [_/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE           0x0A    //  [_\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW   0x0B    //  [P\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE   0x0C    //  [_\P]
#define TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE   0x0D    //  [_/P]
#define TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW   0x0E    //  [P/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW 0x0F    //  [P\_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE 0x10    //  [_\P]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW 0x11    //  [P/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE 0x12    //  [_/P]
#define TR_FD_FUNC_MONKEY                       0x13

#define TR_FD_FUNC_MINECART_LEFT        0x14    // In TR3 only.
#define TR_FD_FUNC_MINECART_RIGHT       0x15    // In TR3 only.
#define TR_FD_FUNC_TRIGGERER_MARK       0x14    // In TR4 only.
#define TR_FD_FUNC_BEETLE_MARK          0x15    // In TR4 only.

// Trigger (TR_FD_FUNC_TRIGGER) types.

#define TR_FD_TRIGTYPE_TRIGGER          0x00    // If Lara is in sector, run (any case).
#define TR_FD_TRIGTYPE_PAD              0x01    // If Lara is in sector, run (land case).
#define TR_FD_TRIGTYPE_SWITCH           0x02    // If item is activated, run, else stop.
#define TR_FD_TRIGTYPE_KEY              0x03    // If item is activated, run.
#define TR_FD_TRIGTYPE_PICKUP           0x04    // If item is picked up, run.
#define TR_FD_TRIGTYPE_HEAVY            0x05    // If item is in sector, run, else stop.
#define TR_FD_TRIGTYPE_ANTIPAD          0x06    // If Lara is in sector, stop (land case).
#define TR_FD_TRIGTYPE_COMBAT           0x07    // If Lara is in combat state, run (any case).
#define TR_FD_TRIGTYPE_DUMMY            0x08    // If Lara is in sector, run (air case).
#define TR_FD_TRIGTYPE_ANTITRIGGER      0x09    // If Lara is in sector, stop (any case).
#define TR_FD_TRIGTYPE_HEAVYSWITCH      0x0A    // If item is activated by item, run.
#define TR_FD_TRIGTYPE_HEAVYANTITRIGGER 0x0B    // If item is activated by item, stop.
#define TR_FD_TRIGTYPE_MONKEY           0x0C    // TR3-5 only: If Lara is monkey-swinging, run.
#define TR_FD_TRIGTYPE_SKELETON         0x0D    // TR5 only: Activated by skeleton only?
#define TR_FD_TRIGTYPE_TIGHTROPE        0x0E    // TR5 only: If Lara is on tightrope, run.
#define TR_FD_TRIGTYPE_CRAWLDUCK        0x0F    // TR5 only: If Lara is crawling, run.
#define TR_FD_TRIGTYPE_CLIMB            0x10    // TR5 only: If Lara is climbing, run.

// Trigger function types.

#define TR_FD_TRIGFUNC_OBJECT           0x00
#define TR_FD_TRIGFUNC_CAMERATARGET     0x01
#define TR_FD_TRIGFUNC_UWCURRENT        0x02
#define TR_FD_TRIGFUNC_FLIPMAP          0x03
#define TR_FD_TRIGFUNC_FLIPON           0x04
#define TR_FD_TRIGFUNC_FLIPOFF          0x05
#define TR_FD_TRIGFUNC_LOOKAT           0x06
#define TR_FD_TRIGFUNC_ENDLEVEL         0x07
#define TR_FD_TRIGFUNC_PLAYTRACK        0x08
#define TR_FD_TRIGFUNC_FLIPEFFECT       0x09
#define TR_FD_TRIGFUNC_SECRET           0x0A
#define TR_FD_TRIGFUNC_BODYBAG          0x0B    // Unused in TR4
#define TR_FD_TRIGFUNC_FLYBY            0x0C
#define TR_FD_TRIGFUNC_CUTSCENE         0x0D

/*
 * уже конкретная игровая модель
 */

typedef struct entity_s
{
    uint32_t                            id;                                     // ID
    int32_t                             OCB;                                    // Object code bit (since TR4)
    uint32_t                            activation_mask;                        // 0x1F means ACTIVATE.
    
    uint32_t                            flags;
    uint8_t                             active;
    uint8_t                             hide;
    
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
    
    struct bounding_volume_s           *bv;                                     // oriented bounding volume - only for OCC test
    
    void                              (*onAnimChange)(struct entity_s *ent);
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

void Entity_UpdateRoomPos(entity_p ent);
void Entity_UpdateRigidBody(entity_p ent);

struct state_change_s *Anim_FindStateChangeByAnim(struct animation_frame_s *anim, int state_change_anim);
struct state_change_s *Anim_FindStateChangeByID(struct animation_frame_s *anim, int id);
int  Entity_GetAnimDispatchCase(struct entity_s *entity, int id);
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
