#pragma once

#include "util/helpers.h"

#include <btBulletDynamicsCommon.h>

#include <memory>

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btGhostPairCallback;

namespace world
{
class Object;
}

namespace engine
{
class Engine;

struct BulletEngine
{
    TRACK_LIFETIME();

    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
    std::unique_ptr<btGhostPairCallback> ghostPairCallback;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;

    explicit BulletEngine(Engine* engine);
    ~BulletEngine() = default;
};

class BtEngineClosestRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
    explicit BtEngineClosestRayResultCallback(const world::Object* obj, bool skipGhost = false)
        : btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
        , m_object(obj)
        , m_skip_ghost(skipGhost)
    {
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override;

    const world::Object* const m_object;
    const bool m_skip_ghost;
};

class BtEngineClosestConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
    explicit BtEngineClosestConvexResultCallback(const world::Object* obj, bool skipGhost = false)
        : btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
        , m_object(obj)
        , m_skip_ghost(skipGhost)
    {
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) override;

protected:
    const world::Object* const m_object;
    const bool m_skip_ghost;
};
}
