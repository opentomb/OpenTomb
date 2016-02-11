#pragma once

#include "world/core/basemesh.h"

#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

#include <cstdint>
#include <memory>

namespace world
{
namespace animation
{
class Skeleton;

using BoneId = uint32_t;

/**
* @brief A single bone in a @c Skeleton
*
* @remark If not stated otherwise, local transformations are relative to the
*         parent bone, or the owner skeleton if the bone doesn't have a parent;
*         global transformations are in world space.
*/
struct Bone
{
    Skeleton* m_skeleton;

    Bone* parent = nullptr;
    BoneId index = 0;
    std::shared_ptr<core::BaseMesh> mesh; //!< The mesh this bone deforms
    std::shared_ptr<core::BaseMesh> mesh_skin;
    std::shared_ptr<core::BaseMesh> mesh_slot; //!< Optional additional mesh

    glm::vec3 position{ 0 };

    glm::mat4 localTransform{ 1 };
    glm::mat4 globalTransform{ 1 };

    uint32_t body_part = 0; //!< flag: BODY, LEFT_LEG_1, RIGHT_HAND_2, HEAD...

    std::shared_ptr<btRigidBody> bt_body;
    std::shared_ptr<btPairCachingGhostObject> ghostObject; // like Bullet character controller for penetration resolving.
    std::shared_ptr<btCollisionShape> shape;
    std::vector<btCollisionObject*> last_collisions{};

    explicit Bone(Skeleton* skeleton)
        : m_skeleton(skeleton)
    {
    }

    ~Bone();
};
}
}
