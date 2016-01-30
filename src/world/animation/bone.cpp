#include "bone.h"

#include "world/entity.h"

namespace world
{
namespace animation
{

Bone::~Bone()
{
    if(ghostObject)
    {
        ghostObject->setUserPointer(nullptr);
        m_skeleton->getEntity()->getWorld()->m_engine->m_bullet.dynamicsWorld->removeCollisionObject(ghostObject.get());
    }

    if(bt_body)
    {
        bt_body->setUserPointer(nullptr);

        delete bt_body->getMotionState();
        bt_body->setMotionState(nullptr);

        bt_body->setCollisionShape(nullptr);

        m_skeleton->getEntity()->getWorld()->m_engine->m_bullet.dynamicsWorld->removeRigidBody(bt_body.get());
    }
}

}
}
