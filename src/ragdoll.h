#ifndef RAGDOLL_H
#define RAGDOLL_H

#include <assert.h>
#include <stdint.h>

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

#include "bullet/LinearMath/btScalar.h"
#include "bullet/LinearMath/btVector3.h"
#include "bullet/btBulletDynamicsCommon.h"

#include "engine.h"
#include "entity.h"


#define RD_CONSTRAINT_POINT 0
#define RD_CONSTRAINT_HINGE 1
#define RD_CONSTRAINT_CONE  2


// Joint setup struct is used to parse joint script entry to
// actual joint.

typedef struct rd_joint_setup_s
{
    uint32_t        body1_index;    // Primary body index
    uint32_t        body2_index;    // Secondary body index
    
    btVector3       body1_offset;   // Primary pivot point offset
    btVector3       body2_offset;   // Secondary pivot point offset
    
    btScalar        body1_angle[3]; // Primary pivot point angle
    btScalar        body2_angle[3]; // Secondary pivot point angle
    
    uint32_t        joint_type;     // See above as RD_CONSTRAINT_* definitions.
    
    btScalar        joint_limit[3]; // Only first two are used for hinge constraint.
        
}rd_joint_setup_t, *rd_joint_setup_p;


// Ragdoll body setup is used to modify body properties for ragdoll needs.

typedef struct rd_body_setup_s
{
    btScalar        mass;
    
    btScalar        damping[2];
    btScalar        restitution;
    btScalar        friction;
    
}rd_body_setup_t, *rd_body_setup_p;


// Ragdoll setup struct is an unified structure which contains settings
// for ALL joints and bodies of a given ragdoll.

typedef struct rd_setup_s
{
    uint32_t            joint_count;
    uint32_t            body_count;
    
    btScalar            joint_cfm;      // Constraint force mixing (joint softness)
    btScalar            joint_erp;      // Error reduction parameter (joint "inertia")
    
    rd_joint_setup_s   *joint_setup;
    rd_body_setup_s    *body_setup;
    
    char               *hit_func;   // Later to be implemented as hit callback function.
}rd_setup_t, *rd_setup_p;


bool Ragdoll_Create(entity_p entity, rd_setup_p setup);
bool Ragdoll_Delete(entity_p entity);

bool Ragdoll_GetSetup(int ragdoll_index, rd_setup_p setup);
void Ragdoll_ClearSetup(rd_setup_p setup);

#endif  // RAGDOLL_H
