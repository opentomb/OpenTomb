#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <LinearMath/btVector3.h>

namespace world
{

#define RD_DEFAULT_SLEEPING_THRESHOLD 10.0

// Joint setup struct is used to parse joint script entry to
// actual joint.

struct RDJointSetup
{
    enum Type
    {
        Point = 0,
        Hinge = 1,
        Cone = 2
    };

    uint16_t        body_index;     // Primary body index
    Type joint_type;     // See above as RD_CONSTRAINT_* definitions.

    btVector3       body1_offset;   // Primary pivot point offset
    btVector3       body2_offset;   // Secondary pivot point offset

    btVector3 body1_angle; // Primary pivot point angle
    btVector3 body2_angle; // Secondary pivot point angle

    btScalar        joint_limit[3]; // Only first two are used for hinge constraint.
};

// Ragdoll body setup is used to modify body properties for ragdoll needs.

struct RDBodySetup
{
    btScalar        mass;

    btScalar        damping[2];
    btScalar        restitution;
    btScalar        friction;
};

// Ragdoll setup struct is an unified structure which contains settings
// for ALL joints and bodies of a given ragdoll.

struct RDSetup
{
    btScalar            joint_cfm = 0;      // Constraint force mixing (joint softness)
    btScalar            joint_erp = 0;      // Error reduction parameter (joint "inertia")

    std::vector<RDJointSetup> joint_setup;
    std::vector<RDBodySetup> body_setup;

    std::string hit_func;   // Later to be implemented as hit callback function.

    bool getSetup(int ragdoll_index);
    void clearSetup();
};

} // namespace world
