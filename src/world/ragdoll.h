#pragma once

#include "animation/animation.h"

#include <LinearMath/btVector3.h>

#include <vector>

namespace world
{
constexpr const btScalar RagdollDefaultSleepingThreshold = 10;

// Joint setup struct is used to parse joint script entry to
// actual joint.

struct RagdollJointSetup
{
    enum Type
    {
        Point = 0,
        Hinge = 1,
        Cone = 2
    };

    animation::BoneId body_index;     // Primary body index
    Type joint_type;     // See above as RD_CONSTRAINT_* definitions.

    btVector3       body1_offset;   // Primary pivot point offset
    btVector3       body2_offset;   // Secondary pivot point offset

    btVector3 body1_angle; // Primary pivot point angle
    btVector3 body2_angle; // Secondary pivot point angle

    btScalar        joint_limit[3]; // Only first two are used for hinge constraint.
};

// Ragdoll body setup is used to modify body properties for ragdoll needs.

struct RagdollBodySetup
{
    btScalar        mass;

    btScalar        damping[2];
    btScalar        restitution;
    btScalar        friction;
};

// Ragdoll setup struct is an unified structure which contains settings
// for ALL joints and bodies of a given ragdoll.

struct RagdollSetup
{
    btScalar            joint_cfm = 0;      // Constraint force mixing (joint softness)
    btScalar            joint_erp = 0;      // Error reduction parameter (joint "inertia")

    std::vector<RagdollJointSetup> joint_setup;
    std::vector<RagdollBodySetup> body_setup;

    std::string hit_func;   // Later to be implemented as hit callback function.

    bool getSetup(int ragdoll_index);
    void clearSetup();
};
} // namespace world
