#pragma once

#include "engine/engine.h"

namespace world
{
namespace core
{

inline btCollisionShape *BT_CSfromSphere(const btScalar& radius)
{
    if(radius == 0.0)
        return nullptr;

    btCollisionShape* ret;

    ret = new btSphereShape(radius);
    ret->setMargin(COLLISION_MARGIN_RIGIDBODY);

    return ret;
}

} // namespace core
} // namespace world
