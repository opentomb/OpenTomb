#pragma once

#include "engine/engine.h"

namespace world
{
namespace core
{

inline btCollisionShape *BT_CSfromSphere(glm::float_t radius)
{
    if(util::fuzzyZero(radius))
        return nullptr;

    btCollisionShape* shape = new btSphereShape(radius);
    shape->setMargin(COLLISION_MARGIN_RIGIDBODY);

    return shape;
}

} // namespace core
} // namespace world
