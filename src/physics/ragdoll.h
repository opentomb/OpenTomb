
#ifndef PHYSICS_RAGDOLL_H
#define	PHYSICS_RAGDOLL_H

#include <stdint.h>


// Joint setup struct is used to parse joint script entry to
// actual joint.

typedef struct rd_joint_setup_s
{
    uint16_t        body_index;     // Primary body index
    uint16_t        joint_type;     // See above as RD_CONSTRAINT_* definitions.

    float           body1_offset[3];   // Primary pivot point offset
    float           body2_offset[3];   // Secondary pivot point offset

    float           body1_angle[3]; // Primary pivot point angle
    float           body2_angle[3]; // Secondary pivot point angle

    float           joint_limit[3]; // Only first two are used for hinge constraint.

}rd_joint_setup_t, *rd_joint_setup_p;


// Ragdoll body setup is used to modify body properties for ragdoll needs.

typedef struct rd_body_setup_s
{
    float        mass;

    float        damping[2];
    float        restitution;
    float        friction;

}rd_body_setup_t, *rd_body_setup_p;


// Ragdoll setup struct is an unified structure which contains settings
// for ALL joints and bodies of a given ragdoll.

typedef struct rd_setup_s
{
    uint32_t            joint_count;
    uint32_t            body_count;

    float               joint_cfm;      // Constraint force mixing (joint softness)
    float               joint_erp;      // Error reduction parameter (joint "inertia")

    rd_joint_setup_s   *joint_setup;
    rd_body_setup_s    *body_setup;

    char               *hit_func;       // Later to be implemented as hit callback function.
}rd_setup_t, *rd_setup_p;


/* Ragdoll interface */
#define RD_CONSTRAINT_POINT 0
#define RD_CONSTRAINT_HINGE 1
#define RD_CONSTRAINT_CONE  2

#define RD_DEFAULT_SLEEPING_THRESHOLD 10.0


struct rd_setup_s *Ragdoll_GetSetup(struct lua_State *lua, int stack_pos);
struct rd_setup_s *Ragdoll_AutoCreateSetup(struct skeletal_model_s *model, uint16_t anim, uint16_t frame);
void Ragdoll_DeleteSetup(struct rd_setup_s *setup);

#endif	/* PHYSICS_RAGDOLL_H */
