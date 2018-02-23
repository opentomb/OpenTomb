
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletDynamics/ConstraintSolver/btTypedConstraint.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>

#include "../core/gl_util.h"
#include "../core/gl_font.h"
#include "../core/gl_text.h"
#include "../core/console.h"
#include "../core/vmath.h"
#include "../core/obb.h"
#include "../render/render.h"
#include "../script/script.h"
#include "../engine.h"
#include "../mesh.h"
#include "../skeletal_model.h"
#include "../character_controller.h"
#include "../entity.h"
#include "../resource.h"
#include "../room.h"
#include "../world.h"
#include "physics.h"
#include "ragdoll.h"
#include "hair.h"


/*
 * INTERNAL BHYSICS CLASSES
 */
class bt_engine_ClosestRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
    bt_engine_ClosestRayResultCallback(engine_container_p cont, float from[3], float to[3], int16_t filter) :
        btCollisionWorld::ClosestRayResultCallback(btVector3(from[0], from[1], from[2]), btVector3(to[0], to[1], to[2])),
        m_cont(cont),
        m_filter(filter)
    {
        m_collisionFilterGroup = btBroadphaseProxy::SensorTrigger;
        m_collisionFilterMask = (filter & (COLLISION_GROUP_STATIC_OBLECT | COLLISION_GROUP_STATIC_ROOM)) ? (btBroadphaseProxy::StaticFilter) : 0x0000;
        m_collisionFilterMask |= (filter & COLLISION_GROUP_KINEMATIC) ? (btBroadphaseProxy::KinematicFilter) : 0x0000;
        m_collisionFilterMask |= (filter & (COLLISION_GROUP_CHARACTERS | COLLISION_GROUP_VEHICLE)) ? (btBroadphaseProxy::CharacterFilter) : 0x0000;
        m_collisionFilterMask |= (filter & COLLISION_GROUP_DYNAMICS) ? (btBroadphaseProxy::DefaultFilter) : 0x0000;
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override
    {
        room_p r0 = NULL, r1 = NULL;
        engine_container_p c1;

        r0 = (m_cont) ? (m_cont->room) : (NULL);
        c1 = (engine_container_p)rayResult.m_collisionObject->getUserPointer();
        r1 = (c1) ? (c1->room) : (NULL);

        if(c1 && (((c1->collision_group & m_filter) == 0x0000) || (c1 == m_cont)))
        {
            return 1.0f;
        }

        if(!r0 || !r1)
        {
            return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
        }

        if(r0 && r1)
        {
            if(Room_IsInOverlappedRoomsList(r0, r1))
            {
                return 1.0f;
            }
            if(Room_IsInNearRoomsList(r0, r1))
            {
                if(m_cont->collision_heavy && (r0 != r1) && (!m_cont->sector || ((m_cont->sector->room_above != r1) && (m_cont->sector->room_below != r1))))
                {
                    btVector3 pt = this->m_rayFromWorld.lerp(this->m_rayToWorld, rayResult.m_hitFraction);
                    room_sector_p ps0 = Room_GetSectorRaw(r1, m_cont->sector->pos);
                    room_sector_p ps1 = Room_GetSectorRaw(r0, pt.m_floats);
                    if((!ps0 || !ps1 || (ps0->portal_to_room != r0) || (ps1->portal_to_room != r1)) &&
                       (!ps0 || ((ps0->room_above != r0) && (ps0->room_below != r0))) &&
                       (!ps1 || ((ps1->room_above != r0) && (ps1->room_below != r0))))
                    {
                        return 1.0f;
                    }
                }
                return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
            }
        }

        return 1.0f;
    }

    engine_container_p m_cont;
    int16_t            m_filter;
};


class bt_engine_ClosestConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
    bt_engine_ClosestConvexResultCallback(engine_container_p cont, float from[3], float to[3], int16_t filter) :
        btCollisionWorld::ClosestConvexResultCallback(btVector3(from[0], from[1], from[2]), btVector3(to[0], to[1], to[2])),
        m_cont(cont),
        m_filter(filter)
    {
        m_collisionFilterGroup = btBroadphaseProxy::SensorTrigger;
        m_collisionFilterMask = (filter & (COLLISION_GROUP_STATIC_OBLECT | COLLISION_GROUP_STATIC_ROOM)) ? (btBroadphaseProxy::StaticFilter) : 0x0000;
        m_collisionFilterMask |= (filter & COLLISION_GROUP_KINEMATIC) ? (btBroadphaseProxy::KinematicFilter) : 0x0000;
        m_collisionFilterMask |= (filter & (COLLISION_GROUP_CHARACTERS | COLLISION_GROUP_VEHICLE)) ? (btBroadphaseProxy::CharacterFilter) : 0x0000;
        m_collisionFilterMask |= (filter & COLLISION_GROUP_DYNAMICS) ? (btBroadphaseProxy::DefaultFilter) : 0x0000;
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult &convexResult, bool normalInWorldSpace)
    {
        room_p r0 = NULL, r1 = NULL;
        engine_container_p c1;

        r0 = (m_cont)?(m_cont->room):(NULL);
        c1 = (engine_container_p)convexResult.m_hitCollisionObject->getUserPointer();
        r1 = (c1) ? (c1->room) : (NULL);

        if(c1 && (((c1->collision_group & m_filter) == 0x0000) || (c1 == m_cont)))
        {
            return 1.0f;
        }

        if(!r0 || !r1)
        {
            return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
        }

        if(r0 && r1)
        {
            if(Room_IsInOverlappedRoomsList(r0, r1))
            {
                return 1.0f;
            }
            if(Room_IsInNearRoomsList(r0, r1))
            {
                if(m_cont->collision_heavy && (r0 != r1) && (!m_cont->sector || ((m_cont->sector->room_above != r1) && (m_cont->sector->room_below != r1))))
                {
                    room_sector_p ps0 = Room_GetSectorRaw(r1, m_cont->sector->pos);
                    room_sector_p ps1 = Room_GetSectorRaw(r0, convexResult.m_hitPointLocal.m_floats);
                    if((!ps0 || !ps1 || (ps0->portal_to_room != r0) || (ps1->portal_to_room != r1)) &&
                       (!ps0 || ((ps0->room_above != r0) && (ps0->room_below != r0))) &&
                       (!ps1 || ((ps1->room_above != r0) && (ps1->room_below != r0))))
                    {
                        return 1.0f;
                    }
                }
                return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
            }
        }

        return 1.0f;
    }

private:
    engine_container_p m_cont;
    int16_t            m_filter;
};


struct bt_engine_OverlapFilterCallback : public btOverlapFilterCallback
{
    // return true when pairs need collision
    virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0,btBroadphaseProxy* proxy1) const
    {
        bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) &&
                        (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);

        if(collides)
        {
            btCollisionObject *obj0 = (btCollisionObject*)proxy0->m_clientObject;
            btCollisionObject *obj1 = (btCollisionObject*)proxy1->m_clientObject;
            engine_container_p c0 = (engine_container_p)obj0->getUserPointer();
            engine_container_p c1 = (engine_container_p)obj1->getUserPointer();;
            room_p r0 = (c0) ? (c0->room) : (NULL);
            room_p r1 = (c1) ? (c1->room) : (NULL);

            int num_ghosts = (proxy0->m_collisionFilterGroup == btBroadphaseProxy::SensorTrigger) +
                             (proxy1->m_collisionFilterGroup == btBroadphaseProxy::SensorTrigger);

            if(num_ghosts == 2)
            {
                return false;
            }

            if(c1 && (c1 == c0) && obj0->isStaticOrKinematicObject())           // No self interaction except ragdolls
            {
                return false;
            }

            if((c0 && c0->collision_group == COLLISION_GROUP_TRIGGERS && !obj1->isStaticOrKinematicObject()) ||
               (c1 && c1->collision_group == COLLISION_GROUP_TRIGGERS && !obj0->isStaticOrKinematicObject()))
            {
                return false;
            }

            collides = ((!r0 && !r1) || (Room_IsInNearRoomsList(r0, r1) && !Room_IsInOverlappedRoomsList(r0, r1) &&
                        (num_ghosts || ((c0->collision_group & c1->collision_mask) && (c1->collision_group & c0->collision_mask)))));

            if(collides && (r0 != r1) && c0 && c1 && (r0->content->overlapped_room_list || r1->content->overlapped_room_list || c0->collision_heavy || c1->collision_heavy))
            {
                if(c0->sector && c1->sector)
                {
                    room_sector_p ps0 = Room_GetSectorRaw(r1, c0->sector->pos);
                    room_sector_p ps1 = Room_GetSectorRaw(r0, c1->sector->pos);
                    collides = (ps0 && ps1 && (ps0->portal_to_room == r0) && (ps1->portal_to_room == r1)) ||
                               (ps0 && ((ps0->room_above == r0) || (ps0->room_below == r0))) ||
                               (ps1 && ((ps1->room_above == r0) || (ps1->room_below == r0)));
                }
                else if(c0->sector && !c1->sector)
                {
                    room_sector_p pc0 = Room_GetSectorRaw(r1, c0->sector->pos);
                    collides = pc0 && (pc0->portal_to_room == r0);
                }
                else if(!c0->sector && c1->sector)
                {
                    room_sector_p pc1 = Room_GetSectorRaw(r0, c1->sector->pos);
                    collides = pc1 && (pc1->portal_to_room == r1);
                }
            }
        }

        return collides;
    }
} bt_engine_overlap_filter_callback;


struct physics_object_s
{
    btRigidBody    *bt_body;
};

struct kinematic_info_s
{
    bool        has_collisions;
};

typedef struct physics_data_s
{
    // kinematic
    btRigidBody                       **bt_body;
    struct kinematic_info_s            *bt_info;

    // dynamic
    struct ghost_shape_s               *ghosts_info;
    btPairCachingGhostObject          **ghost_objects;          // like Bullet character controller for penetration resolving.
    btManifoldArray                    *manifoldArray;          // keep track of the contact manifolds
    struct collision_node_s            *collision_track;
    uint16_t                            objects_count;          // Ragdoll joints
    uint16_t                            bt_joint_count;         // Ragdoll joints
    btTypedConstraint                 **bt_joints;              // Ragdoll joints

    int16_t                             collision_group;
    int16_t                             collision_mask;
    struct engine_container_s          *cont;
}physics_data_t, *physics_data_p;


class CBulletDebugDrawer : public btIDebugDraw
{
public:
    CBulletDebugDrawer() :
    m_debugMode(0)
    {
    }

   ~CBulletDebugDrawer(){}

    virtual void   drawLine(const btVector3& from,const btVector3& to,const btVector3& color) override
    {
        renderer.debugDrawer->DrawLine(from.m_floats, to.m_floats, color.m_floats, color.m_floats);
    }

    virtual void   drawLine(const btVector3& from,const btVector3& to, const btVector3& fromColor, const btVector3& toColor) override
    {
        renderer.debugDrawer->DrawLine(from.m_floats, to.m_floats, fromColor.m_floats, toColor.m_floats);
    }

    virtual void   drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color) override
    {

    }

    virtual void   reportErrorWarning(const char* warningString) override
    {
        Con_AddLine(warningString, FONTSTYLE_CONSOLE_WARNING);
    }

    virtual void   draw3dText(const btVector3& location, const char* textString) override
    {
        renderer.OutTextXYZ(location.m_floats[0], location.m_floats[1], location.m_floats[2], textString);
    }

    virtual void   setDebugMode(int debugMode) override
    {
        m_debugMode = debugMode;
    }

    virtual int    getDebugMode() const override
    {
        return m_debugMode;
    }

private:
    int32_t m_debugMode;
};

btDefaultCollisionConfiguration         *bt_engine_collisionConfiguration = NULL;
btCollisionDispatcher                   *bt_engine_dispatcher = NULL;
btGhostPairCallback                     *bt_engine_ghostPairCallback = NULL;
btBroadphaseInterface                   *bt_engine_overlappingPairCache = NULL;
btSequentialImpulseConstraintSolver     *bt_engine_solver = NULL;
btDiscreteDynamicsWorld                 *bt_engine_dynamicsWorld = NULL;

CBulletDebugDrawer                       bt_debug_drawer;

/* bullet collision model calculation */
btCollisionShape* BT_CSfromBBox(btScalar *bb_min, btScalar *bb_max);
btCollisionShape* BT_CSfromMesh(struct base_mesh_s *mesh, bool useCompression, bool buildBvh, bool is_static = true);
btCollisionShape* BT_CSfromHeightmap(struct room_sector_s *heightmap, uint32_t sectors_count, struct sector_tween_s *tweens, uint32_t tweens_count, bool useCompression, bool buildBvh);

uint32_t BT_AddFloorAndCeilingToTrimesh(btTriangleMesh *trimesh, struct room_sector_s *sector);
uint32_t BT_AddSectorTweenToTrimesh(btTriangleMesh *trimesh, struct sector_tween_s *tween);

void Physics_DeleteRigidBody(struct physics_data_s *physics);                   // only for internal usage

btScalar getInnerBBRadius(btScalar bb_min[3], btScalar bb_max[3])
{
    btScalar r = bb_max[0] - bb_min[0];
    btScalar t = bb_max[1] - bb_min[1];
    r = ((t > r) && (r != 0.0f)) ? (r) : (t);
    t = bb_max[2] - bb_min[2];
    return ((t > r) && (r != 0.0f)) ? (0.5f * r) : (0.5f * t);
}

// Bullet Physics initialization.
void Physics_Init()
{
    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    bt_engine_collisionConfiguration = new btDefaultCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    bt_engine_dispatcher = new btCollisionDispatcher(bt_engine_collisionConfiguration);

    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    bt_engine_overlappingPairCache = new btDbvtBroadphase();
    bt_engine_ghostPairCallback = new btGhostPairCallback();
    bt_engine_overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(bt_engine_ghostPairCallback);

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    bt_engine_solver = new btSequentialImpulseConstraintSolver;

    bt_engine_dynamicsWorld = new btDiscreteDynamicsWorld(bt_engine_dispatcher, bt_engine_overlappingPairCache, bt_engine_solver, bt_engine_collisionConfiguration);
    bt_engine_dynamicsWorld->getPairCache()->setOverlapFilterCallback(&bt_engine_overlap_filter_callback);
    bt_engine_dynamicsWorld->setGravity(btVector3(0, 0, -4500.0));

    bt_debug_drawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawConstraints);
    bt_engine_dynamicsWorld->setDebugDrawer(&bt_debug_drawer);
}


void Physics_Destroy()
{
    //delete dynamics world
    delete bt_engine_dynamicsWorld;

    //delete solver
    delete bt_engine_solver;

    //delete broadphase
    delete bt_engine_overlappingPairCache;

    //delete dispatcher
    delete bt_engine_dispatcher;

    delete bt_engine_collisionConfiguration;

    delete bt_engine_ghostPairCallback;
}


void Physics_StepSimulation(float time)
{
    time = (time < 0.1f) ? (time) : (0.0f);
    bt_engine_dynamicsWorld->stepSimulation(time, 0);
}

void Physics_DebugDrawWorld()
{
    bt_engine_dynamicsWorld->debugDrawWorld();
}

void Physics_CleanUpObjects()
{
    if(bt_engine_dynamicsWorld != NULL)
    {
        int num_obj = bt_engine_dynamicsWorld->getNumCollisionObjects();
        for(int i = 0; i < num_obj; i++)
        {
            btCollisionObject* obj = bt_engine_dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if(body != NULL)
            {
                engine_container_p cont = (engine_container_p)body->getUserPointer();
                body->setUserPointer(NULL);

                if(cont && (cont->object_type == OBJECT_BULLET_MISC))
                {
                    if(body->getMotionState())
                    {
                        delete body->getMotionState();
                        body->setMotionState(NULL);
                    }

                    if(body->getCollisionShape())
                    {
                        delete body->getCollisionShape();
                        body->setCollisionShape(NULL);
                    }

                    bt_engine_dynamicsWorld->removeRigidBody(body);
                    cont->room = NULL;
                    free(cont);
                    delete body;
                }
            }
        }
    }
}

struct physics_data_s *Physics_CreatePhysicsData(struct engine_container_s *cont)
{
    struct physics_data_s *ret = (struct physics_data_s*)malloc(sizeof(struct physics_data_s));

    ret->bt_body = NULL;
    ret->bt_info = NULL;
    ret->bt_joints = NULL;
    ret->objects_count = 0;
    ret->bt_joint_count = 0;
    ret->manifoldArray = NULL;
    ret->ghosts_info = NULL;
    ret->ghost_objects = NULL;
    ret->collision_track = NULL;
    ret->collision_group = btBroadphaseProxy::KinematicFilter;
    ret->collision_mask = btBroadphaseProxy::AllFilter;
    ret->cont = cont;

    return ret;
}


void Physics_DeletePhysicsData(struct physics_data_s *physics)
{
    if(physics)
    {
        for(collision_node_p cn = physics->collision_track; cn;)
        {
            collision_node_p next = cn->next_bucket;
            free(cn);
            cn = next;
        }
        physics->collision_track = NULL;

        if(physics->bt_info)
        {
            free(physics->bt_info);
            physics->bt_info = NULL;
        }

        if(physics->bt_joints)
        {
            for(uint32_t i = 0; i < physics->bt_joint_count; i++)
            {
                if(physics->bt_joints[i])
                {
                    bt_engine_dynamicsWorld->removeConstraint(physics->bt_joints[i]);
                    delete physics->bt_joints[i];
                    physics->bt_joints[i] = NULL;
                }
            }

            free(physics->bt_joints);
            physics->bt_joints = NULL;
            physics->bt_joint_count = 0;
        }

        if(physics->ghost_objects)
        {
            for(int i = 0; i < physics->objects_count; i++)
            {
                physics->ghost_objects[i]->setUserPointer(NULL);
                if(physics->ghost_objects[i]->getCollisionShape())
                {
                    delete physics->ghost_objects[i]->getCollisionShape();
                    physics->ghost_objects[i]->setCollisionShape(NULL);
                }
                bt_engine_dynamicsWorld->removeCollisionObject(physics->ghost_objects[i]);
                delete physics->ghost_objects[i];
                physics->ghost_objects[i] = NULL;
            }
            free(physics->ghost_objects);
            physics->ghost_objects = NULL;
        }

        if(physics->ghosts_info)
        {
            free(physics->ghosts_info);
            physics->ghosts_info = NULL;
        }

        if(physics->manifoldArray)
        {
            physics->manifoldArray->clear();
            delete physics->manifoldArray;
            physics->manifoldArray = NULL;
        }

        Physics_DeleteRigidBody(physics);

        physics->objects_count = 0;
        free(physics);
    }
}


/* Common physics functions */
void Physics_GetGravity(float g[3])
{
    btVector3 bt_g = bt_engine_dynamicsWorld->getGravity();
    vec3_copy(g, bt_g.m_floats);
}


void Physics_SetGravity(float g[3])
{
    btVector3 bt_g(g[0], g[1], g[2]);
    bt_engine_dynamicsWorld->setGravity(bt_g);
}


int  Physics_RayTest(struct collision_result_s *result, float from[3], float to[3], struct engine_container_s *cont, int16_t filter)
{
    bt_engine_ClosestRayResultCallback cb(cont, from, to, filter);
    btVector3 vFrom(from[0], from[1], from[2]), vTo(to[0], to[1], to[2]);

    if(result)
    {
        result->hit = 0x00;
        result->obj = NULL;
        result->fraction = 1.0f;
        bt_engine_dynamicsWorld->rayTest(vFrom, vTo, cb);
        if(cb.hasHit())
        {
            result->obj      = (struct engine_container_s *)cb.m_collisionObject->getUserPointer();
            result->hit      = 0x01;
            result->bone_num = cb.m_collisionObject->getUserIndex();
            vec3_copy(result->normale, cb.m_hitNormalWorld.m_floats);
            vFrom.setInterpolate3(vFrom, vTo, cb.m_closestHitFraction);
            vec3_copy(result->point, vFrom.m_floats);
            result->fraction = cb.m_closestHitFraction;
            return 1;
        }
    }
    else
    {
        bt_engine_dynamicsWorld->rayTest(vFrom, vTo, cb);
        return cb.hasHit();
    }

    return 0;
}


int  Physics_RayTestFiltered(struct collision_result_s *result, float from[3], float to[3], struct engine_container_s *cont, int16_t filter)
{
    bt_engine_ClosestRayResultCallback cb(cont, from, to, filter);
    btVector3 vFrom(from[0], from[1], from[2]), vTo(to[0], to[1], to[2]);

    cb.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
    cb.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;

    if(result)
    {
        result->hit = 0x00;
        result->obj = NULL;
        result->fraction = 1.0f;
        bt_engine_dynamicsWorld->rayTest(vFrom, vTo, cb);
        if(cb.hasHit())
        {
            result->obj      = (struct engine_container_s *)cb.m_collisionObject->getUserPointer();
            result->hit      = 0x01;
            result->bone_num = cb.m_collisionObject->getUserIndex();
            vec3_copy(result->normale, cb.m_hitNormalWorld.m_floats);
            vFrom.setInterpolate3(vFrom, vTo, cb.m_closestHitFraction);
            vec3_copy(result->point, vFrom.m_floats);
            result->fraction = cb.m_closestHitFraction;
            return 1;
        }
    }
    else
    {
        bt_engine_dynamicsWorld->rayTest(vFrom, vTo, cb);
        return cb.hasHit();
    }

    return 0;
}


int  Physics_SphereTest(struct collision_result_s *result, float from[3], float to[3], float R, struct engine_container_s *cont, int16_t filter)
{
    bt_engine_ClosestConvexResultCallback cb(cont, from, to, filter);
    btVector3 vFrom(from[0], from[1], from[2]), vTo(to[0], to[1], to[2]);
    btTransform tFrom, tTo;
    btSphereShape sphere(R);

    tFrom.setIdentity();
    tFrom.setOrigin(vFrom);
    tTo.setIdentity();
    tTo.setOrigin(vTo);

    if(result)
    {
        result->obj = NULL;
        result->hit = 0x00;
        result->fraction = 1.0f;
        bt_engine_dynamicsWorld->convexSweepTest(&sphere, tFrom, tTo, cb);
        if(cb.hasHit())
        {
            result->obj      = (struct engine_container_s *)cb.m_hitCollisionObject->getUserPointer();
            result->hit      = 0x01;
            result->bone_num = cb.m_hitCollisionObject->getUserIndex();
            vec3_copy(result->normale, cb.m_hitNormalWorld.m_floats);
            vec3_copy(result->point, cb.m_hitPointWorld.m_floats);
            result->fraction = cb.m_closestHitFraction;
            return 1;
        }
    }
    else
    {
        bt_engine_dynamicsWorld->convexSweepTest(&sphere, tFrom, tTo, cb);
        return cb.hasHit();
    }

    return 0;
}


int Physics_IsBodyesInited(struct physics_data_s *physics)
{
    return physics && physics->bt_body;
}


int Physics_IsGhostsInited(struct physics_data_s *physics)
{
    return physics && physics->ghost_objects;
}

int  Physics_GetBodiesCount(struct physics_data_s *physics)
{
    return (physics) ? (physics->objects_count) : (0);
}

void Physics_GetBodyWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index)
{
    if(physics->bt_body[index])
    {
        physics->bt_body[index]->getWorldTransform().getOpenGLMatrix(tr);
    }
}


void Physics_SetBodyWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index)
{
    if(physics->bt_body[index])
    {
        physics->bt_body[index]->getWorldTransform().setFromOpenGLMatrix(tr);
    }
}


void Physics_GetGhostWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index)
{
    if(physics->ghost_objects && physics->ghost_objects[index])
    {
        float offset[3], pos[3];
        offset[0] = -physics->ghosts_info[index].offset[0];
        offset[1] = -physics->ghosts_info[index].offset[1];
        offset[2] = -physics->ghosts_info[index].offset[2];
        physics->ghost_objects[index]->getWorldTransform().getOpenGLMatrix(tr);
        Mat4_vec3_mul_macro(pos, tr, offset);
        vec3_copy(tr + 12, pos);
    }
}


void Physics_SetGhostWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index)
{
    if(physics->ghost_objects && physics->ghost_objects[index])
    {
        btVector3 origin;
        Mat4_vec3_mul_macro(origin.m_floats, tr, physics->ghosts_info[index].offset);
        physics->ghost_objects[index]->getWorldTransform().setFromOpenGLMatrix(tr);
        physics->ghost_objects[index]->getWorldTransform().setOrigin(origin);
    }
}


ghost_shape_p Physics_GetGhostShapeInfo(struct physics_data_s *physics, uint16_t index)
{
    return physics->ghosts_info + index;
}


/**
 * It is from bullet_character_controller
 */
collision_node_p Physics_GetGhostCurrentCollision(struct physics_data_s *physics, uint16_t index, int16_t filter)
{
    // Here we must refresh the overlapping paircache as the penetrating movement itself or the
    // previous recovery iteration might have used setWorldTransform and pushed us into an object
    // that is not in the previous cache contents from the last timestep, as will happen if we
    // are pushed into a new AABB overlap. Unhandled this means the next convex sweep gets stuck.
    //
    // Do this by calling the broadphase's setAabb with the moved AABB, this will update the broadphase
    // paircache and the ghostobject's internal paircache at the same time.    /BW

    collision_node_p *cn = &(physics->collision_track);
    collision_node_p *cnb = &(physics->collision_track);
    btPairCachingGhostObject *ghost = physics->ghost_objects[index];
    if(ghost && ghost->getBroadphaseHandle())
    {
        int num_pairs, manifolds_size;
        btBroadphasePairArray &pairArray = ghost->getOverlappingPairCache()->getOverlappingPairArray();
        btVector3 aabb_min, aabb_max;

        ghost->getCollisionShape()->getAabb(ghost->getWorldTransform(), aabb_min, aabb_max);
        bt_engine_dynamicsWorld->getBroadphase()->setAabb(ghost->getBroadphaseHandle(), aabb_min, aabb_max, bt_engine_dynamicsWorld->getDispatcher());
        bt_engine_dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(ghost->getOverlappingPairCache(), bt_engine_dynamicsWorld->getDispatchInfo(), bt_engine_dynamicsWorld->getDispatcher());

        num_pairs = pairArray.size();
        for(int i = 0; i < num_pairs; i++)
        {
            // do not use commented code: it prevents to collision skips.
            //btBroadphasePair &pair = pairArray[i];
            //btBroadphasePair* collisionPair = bt_engine_dynamicsWorld->getPairCache()->findPair(pair.m_pProxy0,pair.m_pProxy1);
            btBroadphasePair *collisionPair = &pairArray[i];
            if(collisionPair && collisionPair->m_algorithm)
            {
                physics->manifoldArray->clear();
                collisionPair->m_algorithm->getAllContactManifolds(*(physics->manifoldArray));
                manifolds_size = physics->manifoldArray->size();
                for(int j = 0; j < manifolds_size; j++)
                {
                    btPersistentManifold* manifold = (*(physics->manifoldArray))[j];
                    btCollisionObject *obj = (btCollisionObject*)manifold->getBody0();
                    btScalar directionSign = btScalar(1.0);
                    if(obj == ghost)
                    {
                        obj = (btCollisionObject*)manifold->getBody1();
                        directionSign = btScalar(-1.0);
                    }

                    engine_container_p cont = (engine_container_p)obj->getUserPointer();
                    if(cont && (cont->collision_group & filter))
                    {
                        for(int k = 0; k < manifold->getNumContacts(); k++)
                        {
                            const btManifoldPoint&pt = manifold->getContactPoint(k);
                            btScalar dist = pt.getDistance();

                            if(dist < 0.0)
                            {
                                if(*cn == NULL)
                                {
                                    *cn = *cnb = (collision_node_p)calloc(4, sizeof(collision_node_t));
                                    for(int bi = 0; bi < 4 - 1; ++bi)
                                    {
                                        (*cnb)[bi].next = *cnb + bi + 1;
                                    }
                                    cnb = &((*cnb)->next_bucket);
                                }

                                (*cn)->obj = cont;
                                (*cn)->part_from = obj->getUserIndex();
                                (*cn)->part_self = i;
                                (*cn)->penetration[0] = pt.m_normalWorldOnB[0];
                                (*cn)->penetration[1] = pt.m_normalWorldOnB[1];
                                (*cn)->penetration[2] = pt.m_normalWorldOnB[2];
                                (*cn)->penetration[3] = dist * directionSign;
                                (*cn)->point[0] = pt.m_positionWorldOnA[0];
                                (*cn)->point[1] = pt.m_positionWorldOnA[1];
                                (*cn)->point[2] = pt.m_positionWorldOnA[2];

                                cn = &((*cn)->next);
                            }
                        }
                    }
                }
            }
        }
        physics->manifoldArray->clear();
    }

    if(*cn)
    {
        (*cn)->obj = NULL;
    }

    return physics->collision_track;
}


btCollisionShape *BT_CSfromBBox(btScalar *bb_min, btScalar *bb_max)
{
    obb_p obb = OBB_Create();
    polygon_p p = obb->base_polygons;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btVector3 v0, v1, v2;
    btCollisionShape* ret;
    int cnt = 0;

    OBB_Rebuild(obb, bb_min, bb_max);
    for(uint32_t i = 0; i < 6; i++, p++)
    {
        if(!Polygon_IsBroken(p))
        {
            for(uint32_t j = 1; j + 1 < p->vertex_count; j++)
            {
                vec3_copy(v0.m_floats, p->vertices[j + 1].position);
                vec3_copy(v1.m_floats, p->vertices[j].position);
                vec3_copy(v2.m_floats, p->vertices[0].position);
                trimesh->addTriangle(v0, v1, v2, true);
            }
            cnt ++;
        }
    }

    OBB_Delete(obb);

    if(cnt == 0)
    {
        delete trimesh;
        return NULL;
    }

    ret = new btConvexTriangleMeshShape(trimesh, true);

    return ret;
}


btCollisionShape *BT_CSfromMesh(struct base_mesh_s *mesh, bool useCompression, bool buildBvh, bool is_static)
{
    uint32_t cnt = 0;
    polygon_p p;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret = NULL;
    btVector3 v0, v1, v2;

    p = mesh->polygons;
    for(uint32_t i = 0; i < mesh->polygons_count; i++, p++)
    {
        if(!Polygon_IsBroken(p))
        {
            for(uint32_t j = 1; j + 1 < p->vertex_count; j++)
            {
                vec3_copy(v0.m_floats, p->vertices[j + 1].position);
                vec3_copy(v1.m_floats, p->vertices[j].position);
                vec3_copy(v2.m_floats, p->vertices[0].position);
                trimesh->addTriangle(v0, v1, v2, true);
            }
            cnt ++;
        }
    }

    if(cnt == 0)
    {
        delete trimesh;
        return NULL;
    }

    if(is_static)
    {
        ret = new btBvhTriangleMeshShape(trimesh, useCompression, buildBvh);
    }
    else
    {
        ret = new btConvexTriangleMeshShape(trimesh, true);
    }

    return ret;
}


uint32_t BT_AddFloorAndCeilingToTrimesh(btTriangleMesh *trimesh, struct room_sector_s *sector)
{
    uint32_t cnt = 0;
    float *v0, *v1, *v2, *v3;

    v0 = sector->floor_corners[0];
    v1 = sector->floor_corners[1];
    v2 = sector->floor_corners[2];
    v3 = sector->floor_corners[3];
    if( (sector->floor_penetration_config != TR_PENETRATION_CONFIG_GHOST) &&
        (sector->floor_penetration_config != TR_PENETRATION_CONFIG_WALL )  )
    {
        if( (sector->floor_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NONE) ||
            (sector->floor_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NW  )  )
        {
            if(sector->floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
            {
                trimesh->addTriangle(btVector3(v3[0], v3[1], v3[2]),
                                     btVector3(v2[0], v2[1], v2[2]),
                                     btVector3(v0[0], v0[1], v0[2]),
                                     true);
                cnt++;
            }

            if(sector->floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
            {
                trimesh->addTriangle(btVector3(v2[0], v2[1], v2[2]),
                                     btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(v0[0], v0[1], v0[2]),
                                     true);
                cnt++;
            }
        }
        else
        {
            if(sector->floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
            {
                trimesh->addTriangle(btVector3(v3[0], v3[1], v3[2]),
                                     btVector3(v2[0], v2[1], v2[2]),
                                     btVector3(v1[0], v1[1], v1[2]),
                                     true);
                cnt++;
            }

            if(sector->floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
            {
                trimesh->addTriangle(btVector3(v3[0], v3[1], v3[2]),
                                     btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(v0[0], v0[1], v0[2]),
                                     true);
                cnt++;
            }
        }
    }

    v0 = sector->ceiling_corners[0];
    v1 = sector->ceiling_corners[1];
    v2 = sector->ceiling_corners[2];
    v3 = sector->ceiling_corners[3];
    if( (sector->ceiling_penetration_config != TR_PENETRATION_CONFIG_GHOST) &&
        (sector->ceiling_penetration_config != TR_PENETRATION_CONFIG_WALL )  )
    {
        if( (sector->ceiling_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NONE) ||
            (sector->ceiling_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NW  )  )
        {
            if(sector->ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
            {
                trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                     btVector3(v2[0], v2[1], v2[2]),
                                     btVector3(v3[0], v3[1], v3[2]),
                                     true);
                cnt++;
            }

            if(sector->ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
            {
                trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                     btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(v2[0], v2[1], v2[2]),
                                     true);
                cnt++;
            }
        }
        else
        {
            if(sector->ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
            {
                trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                     btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(v3[0], v3[1], v3[2]),
                                     true);
                cnt++;
            }

            if(sector->ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
            {
                trimesh->addTriangle(btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(v2[0], v2[1], v2[2]),
                                     btVector3(v3[0], v3[1], v3[2]),
                                     true);
                cnt++;
            }
        }
    }

    return cnt;
}


uint32_t BT_AddSectorTweenToTrimesh(btTriangleMesh *trimesh, struct sector_tween_s *tween)
{
    uint32_t cnt = 0;
    float *v0, *v1, *v2, *v3;

    if(tween->ceiling_tween_inverted == 0x00)
    {
        v0 = tween->ceiling_corners[0];
        v1 = tween->ceiling_corners[1];
        v2 = tween->ceiling_corners[2];
        v3 = tween->ceiling_corners[3];
    }
    else
    {
        v0 = tween->ceiling_corners[0];
        v1 = tween->ceiling_corners[3];
        v2 = tween->ceiling_corners[2];
        v3 = tween->ceiling_corners[1];
    }

    switch(tween->ceiling_tween_type)
    {
        case TR_SECTOR_TWEEN_TYPE_2TRIANGLES:
            {
                btScalar t = fabs((tween->ceiling_corners[2][2] - tween->ceiling_corners[3][2]) /
                                  (tween->ceiling_corners[0][2] - tween->ceiling_corners[1][2]));
                t = 1.0 / (1.0 + t);
                btScalar o[3], t1 = 1.0 - t;
                vec3_interpolate_macro(o, v0, v2, t, t1);

                trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                     btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(o[0], o[1], o[2]),
                                     true);
                trimesh->addTriangle(btVector3(v3[0], v3[1], v3[2]),
                                     btVector3(v2[0], v2[1], v2[2]),
                                     btVector3(o[0], o[1], o[2]),
                                     true);
                cnt += 2;
            }
            break;

        case TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT:
            {
                trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                     btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(v3[0], v3[1], v3[2]),
                                     true);
                cnt++;
            }
            break;

        case TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT:
            {
                trimesh->addTriangle(btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(v2[0], v2[1], v2[2]),
                                     btVector3(v3[0], v3[1], v3[2]),
                                     true);
                cnt++;
            }
            break;

        case TR_SECTOR_TWEEN_TYPE_QUAD:
            {
                trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                     btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(v3[0], v3[1], v3[2]),
                                     true);
                trimesh->addTriangle(btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(v2[0], v2[1], v2[2]),
                                     btVector3(v3[0], v3[1], v3[2]),
                                     true);
                cnt += 2;
            }
            break;
    };

    if(tween->floor_tween_inverted == 0x00)
    {
        v0 = tween->floor_corners[0];
        v1 = tween->floor_corners[1];
        v2 = tween->floor_corners[2];
        v3 = tween->floor_corners[3];
    }
    else
    {
        v0 = tween->floor_corners[0];
        v1 = tween->floor_corners[3];
        v2 = tween->floor_corners[2];
        v3 = tween->floor_corners[1];
    }

    switch(tween->floor_tween_type)
    {
        case TR_SECTOR_TWEEN_TYPE_2TRIANGLES:
            {
                btScalar t = fabs((tween->floor_corners[2][2] - tween->floor_corners[3][2]) /
                                  (tween->floor_corners[0][2] - tween->floor_corners[1][2]));
                t = 1.0 / (1.0 + t);
                btScalar o[3], t1 = 1.0 - t;
                vec3_interpolate_macro(o, v0, v2, t, t1);

                trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                     btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(o[0], o[1], o[2]),
                                     true);
                trimesh->addTriangle(btVector3(v3[0], v3[1], v3[2]),
                                     btVector3(v2[0], v2[1], v2[2]),
                                     btVector3(o[0], o[1], o[2]),
                                     true);
                cnt += 2;
            }
            break;

        case TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT:
            {
                trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                     btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(v3[0], v3[1], v3[2]),
                                     true);
                cnt++;
            }
            break;

        case TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT:
            {
                trimesh->addTriangle(btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(v2[0], v2[1], v2[2]),
                                     btVector3(v3[0], v3[1], v3[2]),
                                     true);
                cnt++;
            }
            break;

        case TR_SECTOR_TWEEN_TYPE_QUAD:
            {
                trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                     btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(v3[0], v3[1], v3[2]),
                                     true);
                trimesh->addTriangle(btVector3(v1[0], v1[1], v1[2]),
                                     btVector3(v2[0], v2[1], v2[2]),
                                     btVector3(v3[0], v3[1], v3[2]),
                                     true);
                cnt += 2;
            }
            break;
    };

    return cnt;
}


btCollisionShape *BT_CSfromHeightmap(struct room_sector_s *heightmap, uint32_t sectors_count, struct sector_tween_s *tweens, uint32_t tweens_count, bool useCompression, bool buildBvh)
{
    uint32_t cnt = 0;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret = NULL;

    for(uint32_t i = 0; i < sectors_count; i++)
    {
        cnt += BT_AddFloorAndCeilingToTrimesh(trimesh, heightmap + i);
    }

    for(uint32_t i = 0; i < tweens_count; i++)
    {
        cnt += BT_AddSectorTweenToTrimesh(trimesh, tweens + i);
    }

    if(cnt == 0)
    {
        delete trimesh;
        return NULL;
    }

    ret = new btBvhTriangleMeshShape(trimesh, useCompression, buildBvh);
    return ret;
}

/*
 * =============================================================================
 */

void Physics_GenRigidBody(struct physics_data_s *physics, struct ss_bone_frame_s *bf)
{
    btVector3 localInertia(0, 0, 0);
    btTransform startTransform;
    btCollisionShape *cshape = NULL;

    Physics_DeleteRigidBody(physics);
    if(physics->bt_info)
    {
        free(physics->bt_info);
        physics->bt_info = NULL;
    }

    switch(physics->cont->collision_shape)
    {
        case COLLISION_SHAPE_SINGLE_BOX:
            {
                physics->objects_count = 1;
                physics->bt_body = (btRigidBody**)malloc(physics->objects_count * sizeof(btRigidBody*));
                physics->bt_info = (struct kinematic_info_s*)malloc(physics->objects_count * sizeof(struct kinematic_info_s));
                physics->bt_info->has_collisions = true;

                float hx = (bf->bb_max[0] - bf->bb_min[0]) * 0.5f;
                float hy = (bf->bb_max[1] - bf->bb_min[1]) * 0.5f;
                float hz = (bf->bb_max[2] - bf->bb_min[2]) * 0.5f;
                cshape = new btBoxShape(btVector3(hx, hy, hz));
                cshape->calculateLocalInertia(0.0, localInertia);
                cshape->setMargin(COLLISION_MARGIN_DEFAULT);
                startTransform.setIdentity();
                btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
                physics->bt_body[0] = new btRigidBody(0.0, motionState, cshape, localInertia);
                physics->bt_body[0]->setUserPointer(physics->cont);
                physics->bt_body[0]->setUserIndex(0);
                physics->bt_body[0]->setRestitution(1.0);
                physics->bt_body[0]->setFriction(1.0);
                bt_engine_dynamicsWorld->addRigidBody(physics->bt_body[0], btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::AllFilter);
            }
            break;

        case COLLISION_SHAPE_SINGLE_SPHERE:
            {
                physics->objects_count = 1;
                physics->bt_body = (btRigidBody**)malloc(physics->objects_count * sizeof(btRigidBody*));
                physics->bt_info = (struct kinematic_info_s*)malloc(physics->objects_count * sizeof(struct kinematic_info_s));
                physics->bt_info->has_collisions = true;

                cshape = new btSphereShape(getInnerBBRadius(bf->bb_min, bf->bb_max));
                cshape->calculateLocalInertia(0.0, localInertia);
                cshape->setMargin(COLLISION_MARGIN_DEFAULT);
                startTransform.setIdentity();
                btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
                physics->bt_body[0] = new btRigidBody(0.0, motionState, cshape, localInertia);
                physics->bt_body[0]->setUserPointer(physics->cont);
                physics->bt_body[0]->setUserIndex(0);
                physics->bt_body[0]->setRestitution(1.0);
                physics->bt_body[0]->setFriction(1.0);
                bt_engine_dynamicsWorld->addRigidBody(physics->bt_body[0], btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::AllFilter);
            }
            break;

        default:
            {
                physics->objects_count = bf->bone_tag_count;
                physics->bt_body = (btRigidBody**)malloc(physics->objects_count * sizeof(btRigidBody*));
                physics->bt_info = (struct kinematic_info_s*)malloc(physics->objects_count * sizeof(struct kinematic_info_s));

                for(uint32_t i = 0; i < physics->objects_count; i++)
                {
                    base_mesh_p mesh = bf->bone_tags[i].mesh_base;
                    cshape = NULL;
                    physics->bt_info[i].has_collisions = false;
                    switch(physics->cont->collision_shape)
                    {
                        case COLLISION_SHAPE_TRIMESH_CONVEX:
                            cshape = BT_CSfromMesh(mesh, true, true, false);
                            break;

                        case COLLISION_SHAPE_TRIMESH:
                            cshape = BT_CSfromMesh(mesh, true, true, true);
                            break;

                        case COLLISION_SHAPE_BOX:
                            cshape = BT_CSfromBBox(mesh->bb_min, mesh->bb_max);
                            break;

                            ///@TODO: add other shapes implementation; may be change default;
                        default:
                             cshape = BT_CSfromMesh(mesh, true, true, true);
                             break;
                    };

                    physics->bt_body[i] = NULL;

                    if(cshape)
                    {
                        physics->bt_info[i].has_collisions = true;
                        cshape->calculateLocalInertia(0.0, localInertia);
                        cshape->setMargin(COLLISION_MARGIN_DEFAULT);

                        startTransform.setIdentity();
                        btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
                        physics->bt_body[i] = new btRigidBody(0.0, motionState, cshape, localInertia);
                        physics->bt_body[i]->setUserPointer(physics->cont);
                        physics->bt_body[i]->setUserIndex(i);
                        physics->bt_body[i]->setRestitution(1.0);
                        physics->bt_body[i]->setFriction(1.0);
                        bt_engine_dynamicsWorld->addRigidBody(physics->bt_body[i], btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::AllFilter);
                    }
                }
            }
            break;
    };
}


void Physics_DeleteRigidBody(struct physics_data_s *physics)
{
    if(physics->bt_body)
    {
        for(int i = 0; i < physics->objects_count; i++)
        {
            btRigidBody *body = physics->bt_body[i];
            if(body)
            {
                body->setUserPointer(NULL);
                if(body->getMotionState())
                {
                    delete body->getMotionState();
                    body->setMotionState(NULL);
                }
                if(body->getCollisionShape())
                {
                    delete body->getCollisionShape();
                    body->setCollisionShape(NULL);
                }

                bt_engine_dynamicsWorld->removeRigidBody(body);
                delete body;
                physics->bt_body[i] = NULL;
            }
        }
        free(physics->bt_body);
        physics->bt_body = NULL;
    }
}


/*
 * 80% boxes hack now is default, but may be rewritten;
 */
void Physics_CreateGhosts(struct physics_data_s *physics, struct ss_bone_frame_s *bf, struct ghost_shape_s *shape_info)
{
    if(physics->objects_count > 0)
    {
        btTransform tr;
        if(!physics->manifoldArray)
        {
            physics->manifoldArray = new btManifoldArray();
        }

        switch(physics->cont->collision_shape)
        {
            case COLLISION_SHAPE_SINGLE_BOX:
                {
                    physics->ghosts_info = (ghost_shape_p)malloc(sizeof(ghost_shape_t));
                    physics->ghosts_info[0].shape_id = COLLISION_SHAPE_SINGLE_BOX;
                    vec3_copy(physics->ghosts_info[0].bb_max, bf->bb_max);
                    vec3_copy(physics->ghosts_info[0].bb_min, bf->bb_min);
                    physics->ghosts_info[0].radius = getInnerBBRadius(bf->bb_min, bf->bb_max);
                    vec3_set_zero(physics->ghosts_info[0].offset);

                    physics->ghost_objects = (btPairCachingGhostObject**)malloc(bf->bone_tag_count * sizeof(btPairCachingGhostObject*));
                    physics->ghost_objects[0] = new btPairCachingGhostObject();
                    physics->ghost_objects[0]->setIgnoreCollisionCheck(physics->bt_body[0], true);
                    tr.setIdentity();
                    physics->ghost_objects[0]->setWorldTransform(tr);
                    physics->ghost_objects[0]->setUserPointer(physics->cont);
                    physics->ghost_objects[0]->setUserIndex(-1);

                    float hx = (bf->bb_max[0] - bf->bb_min[0]) * 0.5f;
                    float hy = (bf->bb_max[1] - bf->bb_min[1]) * 0.5f;
                    float hz = (bf->bb_max[2] - bf->bb_min[2]) * 0.5f;
                    physics->ghost_objects[0]->setCollisionShape(new btBoxShape(btVector3(hx, hy, hz)));
                    physics->ghost_objects[0]->getCollisionShape()->setMargin(COLLISION_MARGIN_DEFAULT);
                    bt_engine_dynamicsWorld->addCollisionObject(physics->ghost_objects[0], btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
                }
                break;

            case COLLISION_SHAPE_SINGLE_SPHERE:
                {
                    physics->ghosts_info = (ghost_shape_p)malloc(sizeof(ghost_shape_t));
                    physics->ghosts_info[0].shape_id = COLLISION_SHAPE_SINGLE_SPHERE;
                    vec3_copy(physics->ghosts_info[0].bb_max, bf->bb_max);
                    vec3_copy(physics->ghosts_info[0].bb_min, bf->bb_min);
                    physics->ghosts_info[0].radius = getInnerBBRadius(bf->bb_min, bf->bb_max);
                    vec3_set_zero(physics->ghosts_info[0].offset);

                    physics->ghost_objects = (btPairCachingGhostObject**)malloc(bf->bone_tag_count * sizeof(btPairCachingGhostObject*));
                    physics->ghost_objects[0] = new btPairCachingGhostObject();
                    physics->ghost_objects[0]->setIgnoreCollisionCheck(physics->bt_body[0], true);
                    tr.setIdentity();
                    physics->ghost_objects[0]->setWorldTransform(tr);
                    physics->ghost_objects[0]->setUserPointer(physics->cont);
                    physics->ghost_objects[0]->setUserIndex(-1);
                    physics->ghost_objects[0]->setCollisionShape(new btSphereShape(physics->ghosts_info[0].radius));
                    physics->ghost_objects[0]->getCollisionShape()->setMargin(COLLISION_MARGIN_DEFAULT);
                    bt_engine_dynamicsWorld->addCollisionObject(physics->ghost_objects[0], btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
                }
                break;

            default:
                {
                    physics->ghosts_info = (ghost_shape_p)malloc(bf->bone_tag_count * sizeof(ghost_shape_t));
                    physics->ghost_objects = (btPairCachingGhostObject**)malloc(bf->bone_tag_count * sizeof(btPairCachingGhostObject*));
                    for(uint32_t i = 0; i < physics->objects_count; i++)
                    {
                        ss_bone_tag_p b_tag = bf->bone_tags + i;

                        physics->ghost_objects[i] = new btPairCachingGhostObject();
                        physics->ghost_objects[i]->setIgnoreCollisionCheck(physics->bt_body[i], true);
                        tr.setIdentity();
                        physics->ghost_objects[i]->setWorldTransform(tr);
                        physics->ghost_objects[i]->setUserPointer(physics->cont);
                        physics->ghost_objects[i]->setUserIndex(i);
                        if(shape_info)
                        {
                            float hx = (shape_info[i].bb_max[0] - shape_info[i].bb_min[0]) * 0.5f;
                            float hy = (shape_info[i].bb_max[1] - shape_info[i].bb_min[1]) * 0.5f;
                            float hz = (shape_info[i].bb_max[2] - shape_info[i].bb_min[2]) * 0.5f;
                            physics->ghosts_info[i] = shape_info[i];
                            switch(shape_info[i].shape_id)
                            {
                                case COLLISION_SHAPE_BOX:
                                    physics->ghost_objects[i]->setCollisionShape(new btBoxShape(btVector3(hx, hy, hz)));
                                    break;

                                case COLLISION_SHAPE_SPHERE:
                                    physics->ghost_objects[i]->setCollisionShape(new btSphereShape(hx));
                                    break;

                                default:
                                    vec3_set_zero(physics->ghosts_info[i].offset);
                                    physics->ghost_objects[i]->setCollisionShape(BT_CSfromMesh(b_tag->mesh_base, true, true, false));
                                    break;
                            };
                        }
                        else
                        {
                            physics->ghosts_info[i].shape_id = COLLISION_SHAPE_TRIMESH_CONVEX;
                            vec3_copy(physics->ghosts_info[i].bb_max, b_tag->mesh_base->bb_max);
                            vec3_copy(physics->ghosts_info[i].bb_min, b_tag->mesh_base->bb_min);
                            vec3_set_zero(physics->ghosts_info[i].offset);
                            physics->ghost_objects[i]->setCollisionShape(BT_CSfromMesh(b_tag->mesh_base, true, true, false));
                        }
                        physics->ghosts_info[i].radius = getInnerBBRadius(physics->ghosts_info[i].bb_min, physics->ghosts_info[i].bb_max);
                        physics->ghost_objects[i]->getCollisionShape()->setMargin(COLLISION_MARGIN_DEFAULT);
                        bt_engine_dynamicsWorld->addCollisionObject(physics->ghost_objects[i], btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
                    }
                }
        };
    }
}


void Physics_SetGhostCollisionShape(struct physics_data_s *physics, struct ss_bone_frame_s *bf, uint16_t index, struct ghost_shape_s *shape_info)
{
    if(physics->ghost_objects && (index < physics->objects_count) && physics->ghost_objects[index])
    {
        btCollisionShape *new_shape = NULL;
        float hx = (shape_info->bb_max[0] - shape_info->bb_min[0]) * 0.5f;
        float hy = (shape_info->bb_max[1] - shape_info->bb_min[1]) * 0.5f;
        float hz = (shape_info->bb_max[2] - shape_info->bb_min[2]) * 0.5f;
        shape_info->radius = getInnerBBRadius(shape_info->bb_min, shape_info->bb_max);
        if((hx <= 0.0f) || (hy <= 0.0f) || (hz <= 0.0f))
        {
            shape_info->shape_id = COLLISION_NONE;
            physics->ghosts_info[index].shape_id = COLLISION_NONE;
        }

        switch(shape_info->shape_id)
        {
            case COLLISION_SHAPE_BOX:
                new_shape = new btBoxShape(btVector3(hx, hy, hz));
                break;

            case COLLISION_SHAPE_SPHERE:
                new_shape = new btSphereShape(hx);
                break;

            case COLLISION_SHAPE_TRIMESH:
                new_shape = BT_CSfromMesh(bf->bone_tags[index].mesh_base, true, true, false);
                break;

            case COLLISION_NONE:
                bt_engine_dynamicsWorld->removeCollisionObject(physics->ghost_objects[index]);
                break;
        };

        if(new_shape)
        {
            btCollisionShape *old_shape = physics->ghost_objects[index]->getCollisionShape();
            physics->ghosts_info[index] = *shape_info;
            physics->ghost_objects[index]->setCollisionShape(new_shape);
            physics->ghost_objects[index]->getCollisionShape()->setMargin(COLLISION_MARGIN_DEFAULT);
            if(!physics->ghost_objects[index]->getBroadphaseHandle())
            {
                bt_engine_dynamicsWorld->addCollisionObject(physics->ghost_objects[index], btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
            }
            if(old_shape)
            {
                delete old_shape;
            }
        }
    }
}


void Physics_GenStaticMeshRigidBody(struct static_mesh_s *smesh)
{
    btCollisionShape *cshape = NULL;

    if(smesh->self->collision_group == COLLISION_NONE)
    {
        return;
    }

    smesh->physics_body = NULL;
    switch(smesh->self->collision_shape)
    {
        case COLLISION_SHAPE_BOX:
            cshape = BT_CSfromBBox(smesh->cbb_min, smesh->cbb_max);
            break;

        case COLLISION_SHAPE_BOX_BASE:
            cshape = BT_CSfromBBox(smesh->mesh->bb_min, smesh->mesh->bb_max);
            break;

        case COLLISION_SHAPE_TRIMESH:
            cshape = BT_CSfromMesh(smesh->mesh, true, true, true);
            break;

        case COLLISION_SHAPE_TRIMESH_CONVEX:
            cshape = BT_CSfromMesh(smesh->mesh, true, true, false);
            break;

        default:
            cshape = NULL;
            break;
    };

    if(cshape)
    {
        btVector3 localInertia(0, 0, 0);
        btTransform startTransform;
        startTransform.setFromOpenGLMatrix(smesh->transform);
        smesh->physics_body = (struct physics_object_s*)malloc(sizeof(struct physics_object_s));
        btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
        smesh->physics_body->bt_body = new btRigidBody(0.0, motionState, cshape, localInertia);
        cshape->setMargin(COLLISION_MARGIN_DEFAULT);
        smesh->physics_body->bt_body->setRestitution(1.0);
        smesh->physics_body->bt_body->setFriction(1.0);
        bt_engine_dynamicsWorld->addRigidBody(smesh->physics_body->bt_body, btBroadphaseProxy::StaticFilter, btBroadphaseProxy::AllFilter);
        smesh->physics_body->bt_body->setUserPointer(smesh->self);
    }
}


struct physics_object_s* Physics_GenRoomRigidBody(struct room_s *room, struct room_sector_s *heightmap, uint32_t sectors_count, struct sector_tween_s *tweens, int num_tweens)
{
    btCollisionShape *cshape = BT_CSfromHeightmap(heightmap, sectors_count, tweens, num_tweens, true, true);
    struct physics_object_s *ret = NULL;

    if(cshape)
    {
        btVector3 localInertia(0, 0, 0);
        btTransform tr;
        tr.setFromOpenGLMatrix(room->transform);
        ret = (struct physics_object_s*)malloc(sizeof(struct physics_object_s));
        btDefaultMotionState* motionState = new btDefaultMotionState(tr);
        cshape->setMargin(COLLISION_MARGIN_DEFAULT);
        ret->bt_body = new btRigidBody(0.0, motionState, cshape, localInertia);
        bt_engine_dynamicsWorld->addRigidBody(ret->bt_body, btBroadphaseProxy::StaticFilter, btBroadphaseProxy::AllFilter);
        ret->bt_body->setUserPointer(room->self);
        ret->bt_body->setUserIndex(0);
        ret->bt_body->setRestitution(1.0);
        ret->bt_body->setFriction(1.0);
    }

    return ret;
}


void Physics_SetOwnerObject(struct physics_object_s *obj, struct engine_container_s *self)
{
    if(obj && obj->bt_body)
    {
        obj->bt_body->setUserPointer(self);
    }
}


void Physics_DeleteObject(struct physics_object_s *obj)
{
    if(obj)
    {
        obj->bt_body->setUserPointer(NULL);
        if(obj->bt_body->getMotionState())
        {
            delete obj->bt_body->getMotionState();
            obj->bt_body->setMotionState(NULL);
        }
        if(obj->bt_body->getCollisionShape())
        {
            delete obj->bt_body->getCollisionShape();
            obj->bt_body->setCollisionShape(NULL);
        }

        bt_engine_dynamicsWorld->removeRigidBody(obj->bt_body);
        delete obj->bt_body;
        free(obj);
    }
}


void Physics_EnableObject(struct physics_object_s *obj)
{
    if(obj->bt_body && !obj->bt_body->isInWorld())
    {
        bt_engine_dynamicsWorld->addRigidBody(obj->bt_body, btBroadphaseProxy::StaticFilter, btBroadphaseProxy::AllFilter);
    }
}


void Physics_DisableObject(struct physics_object_s *obj)
{
    if(obj->bt_body && obj->bt_body->isInWorld())
    {
        bt_engine_dynamicsWorld->removeRigidBody(obj->bt_body);
    }
}


/**
 * This function enables collision for entity_p in all cases exept NULL models.
 * If collision models does not exists, function will create them;
 * @param ent - pointer to the entity.
 */
void Physics_EnableCollision(struct physics_data_s *physics)
{
    if(physics->bt_body)
    {
        for(uint32_t i = 0; i < physics->objects_count; i++)
        {
            btRigidBody *b = physics->bt_body[i];
            if(b && physics->bt_info[i].has_collisions && !b->isInWorld())
            {
                bt_engine_dynamicsWorld->addRigidBody(b, physics->collision_group, physics->collision_mask);
            }
            if(physics->ghost_objects && physics->ghost_objects[i] &&
               (physics->ghosts_info[i].shape_id != COLLISION_NONE) &&
               !physics->ghost_objects[i]->getBroadphaseHandle())
            {
                bt_engine_dynamicsWorld->addCollisionObject(physics->ghost_objects[i], btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
            }
        }
    }
}


void Physics_DisableCollision(struct physics_data_s *physics)
{
    if(physics->bt_body != NULL)
    {
        for(uint32_t i = 0; i < physics->objects_count; i++)
        {
            btRigidBody *b = physics->bt_body[i];
            if(b && b->isInWorld())
            {
                bt_engine_dynamicsWorld->removeRigidBody(b);
            }

            if(physics->ghost_objects && physics->ghost_objects[i] &&
               physics->ghost_objects[i]->getBroadphaseHandle())
            {
                bt_engine_dynamicsWorld->removeCollisionObject(physics->ghost_objects[i]);
            }
        }
    }
}


void Physics_SetBoneCollision(struct physics_data_s *physics, int bone_index, int collision)
{
    if(physics->bt_body && (bone_index >= 0) && (bone_index < physics->objects_count))
    {
        btRigidBody *b = physics->bt_body[bone_index];
        physics->bt_info[bone_index].has_collisions = collision;

        if(b && !collision && b->isInWorld())
        {
            bt_engine_dynamicsWorld->removeRigidBody(b);
        }
    }
}


void Physics_SetCollisionGroupAndMask(struct physics_data_s *physics, int16_t group, int16_t mask)
{
    if(physics->bt_body != NULL)
    {
        physics->collision_group = (group & (COLLISION_GROUP_STATIC_OBLECT | COLLISION_GROUP_STATIC_ROOM)) ? (btBroadphaseProxy::StaticFilter) : 0x0000;
        physics->collision_group |= (group & COLLISION_GROUP_KINEMATIC) ? (btBroadphaseProxy::KinematicFilter) : 0x0000;
        physics->collision_group |= (group & (COLLISION_GROUP_CHARACTERS | COLLISION_GROUP_VEHICLE)) ? (btBroadphaseProxy::CharacterFilter) : 0x0000;
        physics->collision_group |= (group & COLLISION_GROUP_DYNAMICS) ? (btBroadphaseProxy::DefaultFilter) : 0x0000;

        physics->collision_mask = btBroadphaseProxy::SensorTrigger;
        physics->collision_mask |= (mask & (COLLISION_GROUP_STATIC_OBLECT | COLLISION_GROUP_STATIC_ROOM)) ? (btBroadphaseProxy::StaticFilter) : 0x0000;
        physics->collision_mask |= (mask & COLLISION_GROUP_KINEMATIC) ? (btBroadphaseProxy::KinematicFilter) : 0x0000;
        physics->collision_mask |= (mask & (COLLISION_GROUP_CHARACTERS | COLLISION_GROUP_VEHICLE)) ? (btBroadphaseProxy::CharacterFilter) : 0x0000;
        physics->collision_mask |= (mask & COLLISION_GROUP_DYNAMICS) ? (btBroadphaseProxy::DefaultFilter) : 0x0000;
        physics->collision_mask = (mask == COLLISION_MASK_ALL) ? (btBroadphaseProxy::AllFilter) : physics->collision_mask;

        for(uint32_t i = 0; i < physics->objects_count; i++)
        {
            btRigidBody *b = physics->bt_body[i];
            if(b && b->getBroadphaseHandle())
            {
                b->getBroadphaseHandle()->m_collisionFilterGroup = physics->collision_group;
                b->getBroadphaseHandle()->m_collisionFilterMask = physics->collision_mask;
            }
        }
    }
}


void Physics_SetCollisionScale(struct physics_data_s *physics, float scaling[3])
{
    for(int i = 0; i < physics->objects_count; i++)
    {
        bt_engine_dynamicsWorld->removeRigidBody(physics->bt_body[i]);
            physics->bt_body[i]->getCollisionShape()->setLocalScaling(btVector3(scaling[0], scaling[1], scaling[2]));
        bt_engine_dynamicsWorld->addRigidBody(physics->bt_body[i]);

        physics->bt_body[i]->activate();
    }
}


void Physics_SetBodyMass(struct physics_data_s *physics, float mass, uint16_t index)
{
    btVector3 inertia (0.0, 0.0, 0.0);
    bt_engine_dynamicsWorld->removeRigidBody(physics->bt_body[index]);

        physics->bt_body[index]->getCollisionShape()->calculateLocalInertia(mass, inertia);

        physics->bt_body[index]->setMassProps(mass, inertia);

        physics->bt_body[index]->updateInertiaTensor();
        physics->bt_body[index]->clearForces();

        btVector3 factor = (mass > 0.0) ? (btVector3(1.0, 1.0, 1.0)) : (btVector3(0.0, 0.0, 0.0));
        physics->bt_body[index]->setLinearFactor (factor);
        physics->bt_body[index]->setAngularFactor(factor);

    bt_engine_dynamicsWorld->addRigidBody(physics->bt_body[index]);

    physics->bt_body[index]->activate();
}


void Physics_PushBody(struct physics_data_s *physics, float speed[3], uint16_t index)
{
    physics->bt_body[index]->setLinearVelocity(btVector3(speed[0], speed[1], speed[2]));
}


void Physics_SetLinearFactor(struct physics_data_s *physics, float factor[3], uint16_t index)
{
    physics->bt_body[index]->setLinearFactor(btVector3(factor[0], factor[1], factor[2]));
}


/* *****************************************************************************
 * ************************  HAIR DATA  ****************************************
 * ****************************************************************************/

typedef struct hair_element_s
{
    struct base_mesh_s         *mesh;              // Pointer to rendered mesh.
    btCollisionShape           *shape;             // Pointer to collision shape.
    btRigidBody                *body;              // Pointer to dynamic body.
    btGeneric6DofConstraint    *joint;             // Array of joints.
}hair_element_t, *hair_element_p;


typedef struct hair_s
{
    engine_container_p        container;
    uint32_t                  owner_body;         // Owner entity's body ID.

    uint8_t                   root_index;         // Index of "root" element.
    uint8_t                   tail_index;         // Index of "tail" element.

    uint8_t                   element_count;      // Overall amount of elements.
    hair_element_s           *elements;           // Array of elements.

    uint8_t                   vertex_map_count;
    uint32_t                 *hair_vertex_map;    // Hair vertex indices to link
    uint32_t                 *head_vertex_map;    // Head vertex indices to link

}hair_t, *hair_p;


struct hair_s *Hair_Create(struct hair_setup_s *setup, struct physics_data_s *physics)
{
    // No setup or parent to link to - bypass function.

    if(!physics || !setup || (setup->link_body >= physics->objects_count) ||
       !physics->bt_body[setup->link_body])
    {
        return NULL;
    }

    skeletal_model_p model = World_GetModelByID(setup->model_id);
    if((!model) || (model->mesh_count == 0))
    {
        return NULL;
    }

    // Setup engine container. FIXME: DOESN'T WORK PROPERLY ATM.
    struct hair_s *hair = (struct hair_s*)calloc(1, sizeof(struct hair_s));
    hair->container = Container_Create();
    hair->container->collision_group = COLLISION_GROUP_DYNAMICS_NI;
    hair->container->collision_mask = COLLISION_GROUP_STATIC_ROOM | COLLISION_GROUP_STATIC_OBLECT | COLLISION_GROUP_KINEMATIC | COLLISION_GROUP_CHARACTERS;
    hair->container->collision_shape = COLLISION_SHAPE_TRIMESH;
    hair->container->object_type = OBJECT_HAIR;
    hair->container->object = hair;
    hair->container->room = physics->cont->room;

    // Setup initial hair parameters.
    hair->owner_body = setup->link_body;    // Entity body to refer to.

    // Setup initial position / angles.
    btTransform startTransform = physics->bt_body[hair->owner_body]->getWorldTransform();
    // Number of elements (bodies) is equal to number of hair meshes.

    hair->element_count = model->mesh_count;
    hair->elements      = (hair_element_p)calloc(hair->element_count, sizeof(hair_element_t));

    // Root index should be always zero, as it is how engine determines that it is
    // connected to head and renders it properly. Tail index should be always the
    // last element of the hair, as it indicates absence of "child" constraint.

    hair->root_index = 0;
    hair->tail_index = hair->element_count - 1;

    // Weight step is needed to determine the weight of each hair body.
    // It is derived from root body weight and tail body weight.

    btScalar weight_step = ((setup->root_weight - setup->tail_weight) / hair->element_count);
    btScalar current_weight = setup->root_weight;

    for(uint32_t i = 0; i < hair->element_count; i++)
    {
        // Point to corresponding mesh.

        hair->elements[i].mesh = model->mesh_tree[i].mesh_base;

        // Begin creating ACTUAL physical hair mesh.
        btVector3   localInertia(0, 0, 0);

        // Make collision shape out of mesh.
        hair->elements[i].shape = BT_CSfromMesh(hair->elements[i].mesh, true, true, false);
        hair->elements[i].shape->calculateLocalInertia((current_weight * setup->hair_inertia), localInertia);
        hair->elements[i].joint = NULL;

        // Decrease next body weight to weight_step parameter.
        current_weight -= weight_step;

        // Initialize motion state for body.
        btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

        // Make rigid body.
        hair->elements[i].body = new btRigidBody(current_weight, motionState, hair->elements[i].shape, localInertia);

        // Damping makes body stop in space by itself, to prevent it from continous movement.
        hair->elements[i].body->setDamping(setup->hair_damping[0], setup->hair_damping[1]);

        // Restitution and friction parameters define "bounciness" and "dullness" of hair.
        hair->elements[i].body->setRestitution(setup->hair_restitution);
        hair->elements[i].body->setFriction(setup->hair_friction);

        // Since hair is always moving with Lara, even if she's in still state (like, hanging
        // on a ledge), hair bodies shouldn't deactivate over time.
        hair->elements[i].body->forceActivationState(DISABLE_DEACTIVATION);

        // Hair bodies must not collide with each other, and also collide ONLY with kinematic
        // bodies (e. g. animated meshes), or else Lara's ghost object or anything else will be able to
        // collide with hair!
        hair->elements[i].body->setUserPointer(hair->container);
        bt_engine_dynamicsWorld->addRigidBody(hair->elements[i].body, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::DefaultFilter | btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter | btBroadphaseProxy::CharacterFilter);

        hair->elements[i].body->activate();
    }

    // GENERATE CONSTRAINTS.
    // All constraints are generic 6-DOF type, as they seem perfect fit for hair.

    // If multiple joints per body is specified, joints are placed in circular manner,
    // with obvious step of (SIMD_2_PI) / joint count. It means that all joints will form
    // circle-like figure.

    for(uint32_t i = 0; i < hair->element_count; i++)
    {
        btRigidBody* prev_body;
        btScalar     body_length;

        // Each body width and height are used to calculate position of each joint.

        //btScalar body_width = fabs(hair->elements[i].mesh->bb_max[0] - hair->elements[i].mesh->bb_min[0]);
        //btScalar body_depth = fabs(hair->elements[i].mesh->bb_max[3] - hair->elements[i].mesh->bb_min[3]);

        btTransform localA; localA.setIdentity();
        btTransform localB; localB.setIdentity();

        btScalar joint_x = 0.0;
        btScalar joint_y = 0.0;

        if(i == 0)  // First joint group
        {
            // Adjust pivot point A to parent body.
            localA.setOrigin(btVector3(setup->head_offset[0] + joint_x, setup->head_offset[1], setup->head_offset[2] + joint_y));
            localA.getBasis().setEulerZYX(setup->root_angle[0], setup->root_angle[1], setup->root_angle[2]);

            localB.setOrigin(btVector3(joint_x, 0.0, joint_y));
            localB.getBasis().setEulerZYX(0,-SIMD_HALF_PI,0);

            prev_body = physics->bt_body[hair->owner_body];                     // Previous body is parent body.
        }
        else
        {
            // Adjust pivot point A to previous mesh's length, considering mesh overlap multiplier.

            body_length = fabs(hair->elements[i-1].mesh->bb_max[1] - hair->elements[i-1].mesh->bb_min[1]) * setup->joint_overlap;

            localA.setOrigin(btVector3(joint_x, body_length, joint_y));
            localA.getBasis().setEulerZYX(0,SIMD_HALF_PI,0);

            // Pivot point B is automatically adjusted by Bullet.

            localB.setOrigin(btVector3(joint_x, 0.0, joint_y));
            localB.getBasis().setEulerZYX(0,SIMD_HALF_PI,0);

            prev_body = hair->elements[i-1].body;   // Previous body is preceiding hair mesh.
        }

        // Create 6DOF constraint.
        hair->elements[i].joint = new btGeneric6DofConstraint(*prev_body, *(hair->elements[i].body), localA, localB, true);

        // CFM and ERP parameters are critical for making joint "hard" and link
        // to Lara's head. With wrong values, constraints may become "elastic".
        for(int axis = 0; axis <= 5; axis++)
        {
            hair->elements[i].joint->setParam(BT_CONSTRAINT_STOP_CFM, setup->joint_cfm, axis);
            hair->elements[i].joint->setParam(BT_CONSTRAINT_STOP_ERP, setup->joint_erp, axis);
        }

        if(i == 0)
        {
            // First joint group should be more limited in motion, as it is connected
            // right to the head. NB: Should we make it scriptable as well?
            hair->elements[i].joint->setLinearLowerLimit(btVector3(0., 0., 0.));
            hair->elements[i].joint->setLinearUpperLimit(btVector3(0., 0., 0.));
            hair->elements[i].joint->setAngularLowerLimit(btVector3(-SIMD_HALF_PI,     0., -SIMD_HALF_PI*0.4));
            hair->elements[i].joint->setAngularUpperLimit(btVector3(-SIMD_HALF_PI*0.3, 0.,  SIMD_HALF_PI*0.4));

            // Increased solver iterations make constraint even more stable.
            hair->elements[i].joint->setOverrideNumSolverIterations(100);
        }
        else
        {
            // Normal joint with more movement freedom.
            hair->elements[i].joint->setLinearLowerLimit(btVector3(0., 0., 0.));
            hair->elements[i].joint->setLinearUpperLimit(btVector3(0., 0., 0.));
            hair->elements[i].joint->setAngularLowerLimit(btVector3(-SIMD_HALF_PI*0.5, 0., -SIMD_HALF_PI*0.5));
            hair->elements[i].joint->setAngularUpperLimit(btVector3( SIMD_HALF_PI*0.5, 0.,  SIMD_HALF_PI*0.5));
        }
        hair->elements[i].joint->setDbgDrawSize(btScalar(5.f));    // Draw constraint axes.

        // Add constraint to the world.
        bt_engine_dynamicsWorld->addConstraint(hair->elements[i].joint, true);
    }

    return hair;
}


void Hair_Delete(struct hair_s *hair)
{
    if(hair)
    {
        for(int i = 0; i < hair->element_count; i++)
        {
            if(hair->elements[i].joint)
            {
                bt_engine_dynamicsWorld->removeConstraint(hair->elements[i].joint);
                delete hair->elements[i].joint;
                hair->elements[i].joint = NULL;
            }
        }

        for(int i = 0; i < hair->element_count; i++)
        {
            if(hair->elements[i].body)
            {
                hair->elements[i].body->setUserPointer(NULL);
                bt_engine_dynamicsWorld->removeRigidBody(hair->elements[i].body);
                delete hair->elements[i].body;
                hair->elements[i].body = NULL;
            }
            if(hair->elements[i].shape)
            {
                delete hair->elements[i].shape;
                hair->elements[i].shape = NULL;
            }
        }
        free(hair->elements);
        hair->elements = NULL;
        hair->element_count = 0;

        hair->container = NULL;
        hair->owner_body = 0;

        hair->root_index = 0;
        hair->tail_index = 0;

        free(hair->container);
        hair->container = NULL;

        free(hair);
    }
}


void Hair_Update(struct hair_s *hair, struct physics_data_s *physics)
{
    if(hair && (hair->element_count > 0))
    {
        hair->container->room = physics->cont->room;
    }
}

int Hair_GetElementsCount(struct hair_s *hair)
{
    return (hair)?(hair->element_count):(0);
}

void Hair_GetElementInfo(struct hair_s *hair, int element, struct base_mesh_s **mesh, float tr[16])
{
    hair->elements[element].body->getWorldTransform().getOpenGLMatrix(tr);
    *mesh = hair->elements[element].mesh;
}


/* *****************************************************************************
 * ************************  RAGDOLL DATA  *************************************
 * ****************************************************************************/

bool Ragdoll_Create(struct physics_data_s *physics, struct ss_bone_frame_s *bf, struct rd_setup_s *setup)
{
    // No entity, setup or body count overflow - bypass function.

    if(!physics || !setup || (setup->body_count > physics->objects_count))
    {
        return false;
    }

    bool result = true;

    // If ragdoll already exists, overwrite it with new one.

    if(physics->bt_joint_count > 0)
    {
        result = Ragdoll_Delete(physics);
    }

    // Setup bodies.
    physics->bt_joint_count = 0;
    // update current character animation and full fix body to avoid starting ragdoll partially inside the wall or floor...
    for(uint32_t i = 0; i < setup->body_count; i++)
    {
        if(physics->bt_body[i] == NULL)
        {
            result = false;
            continue;   // If body is absent, return false and bypass this body setup.
        }

        btVector3 inertia (0.0, 0.0, 0.0);
        btScalar  mass = setup->body_setup[i].mass;

        if(physics->bt_body[i]->isInWorld())
        {
            bt_engine_dynamicsWorld->removeRigidBody(physics->bt_body[i]);
        }
        physics->bt_body[i]->getCollisionShape()->calculateLocalInertia(mass, inertia);
        physics->bt_body[i]->setMassProps(mass, inertia);

        physics->bt_body[i]->updateInertiaTensor();
        physics->bt_body[i]->clearForces();

        physics->bt_body[i]->setLinearFactor (btVector3(1.0, 1.0, 1.0));
        physics->bt_body[i]->setAngularFactor(btVector3(1.0, 1.0, 1.0));

        physics->bt_body[i]->setDamping(setup->body_setup[i].damping[0], setup->body_setup[i].damping[1]);
        physics->bt_body[i]->setRestitution(setup->body_setup[i].restitution);
        physics->bt_body[i]->setFriction(setup->body_setup[i].friction);
        physics->bt_body[i]->setSleepingThresholds(RD_DEFAULT_SLEEPING_THRESHOLD, RD_DEFAULT_SLEEPING_THRESHOLD);

        if(bf->bone_tags[i].parent == NULL)
        {
            btScalar r = getInnerBBRadius(bf->bone_tags[i].mesh_base->bb_min, bf->bone_tags[i].mesh_base->bb_max);
            physics->bt_body[i]->setCcdMotionThreshold(0.8 * r);
            physics->bt_body[i]->setCcdSweptSphereRadius(r);
        }
    }

    for(uint32_t i = 0; i < setup->body_count; i++)
    {
        bt_engine_dynamicsWorld->addRigidBody(physics->bt_body[i], btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::CharacterFilter | btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter);
        physics->bt_body[i]->activate();
        physics->bt_body[i]->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
    }

    // Setup constraints.
    physics->bt_joint_count = setup->joint_count;
    physics->bt_joints = (btTypedConstraint**)calloc(physics->bt_joint_count, sizeof(btTypedConstraint*));

    for(int i = 0; i < physics->bt_joint_count; i++)
    {
        if( (setup->joint_setup[i].body_index >= setup->body_count) ||
            (physics->bt_body[setup->joint_setup[i].body_index] == NULL) )
        {
            result = false;
            break;       // If body 1 or body 2 are absent, return false and bypass this joint.
        }

        btTransform localA, localB;
        ss_bone_tag_p btB = bf->bone_tags + setup->joint_setup[i].body_index;
        ss_bone_tag_p btA = btB->parent;
        if(btA == NULL)
        {
            result = false;
            break;
        }
#if 0
        localA.setFromOpenGLMatrix(btB->transform);
        localB.setIdentity();
#else
        localA.getBasis().setEulerZYX(setup->joint_setup[i].body1_angle[0], setup->joint_setup[i].body1_angle[1], setup->joint_setup[i].body1_angle[2]);
        //localA.setOrigin(setup->joint_setup[i].body1_offset);
        localA.setOrigin(btVector3(btB->transform[12 + 0], btB->transform[12 + 1], btB->transform[12 + 2]));

        localB.getBasis().setEulerZYX(setup->joint_setup[i].body2_angle[0], setup->joint_setup[i].body2_angle[1], setup->joint_setup[i].body2_angle[2]);
        //localB.setOrigin(setup->joint_setup[i].body2_offset);
        localB.setOrigin(btVector3(0.0, 0.0, 0.0));
#endif

        switch(setup->joint_setup[i].joint_type)
        {
            case RD_CONSTRAINT_POINT:
                {
                    btPoint2PointConstraint* pointC = new btPoint2PointConstraint(*physics->bt_body[btA->index], *physics->bt_body[btB->index], localA.getOrigin(), localB.getOrigin());
                    physics->bt_joints[i] = pointC;
                }
                break;

            case RD_CONSTRAINT_HINGE:
                {
                    btHingeConstraint* hingeC = new btHingeConstraint(*physics->bt_body[btA->index], *physics->bt_body[btB->index], localA, localB);
                    hingeC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], 0.9, 0.3, 0.3);
                    physics->bt_joints[i] = hingeC;
                }
                break;

            case RD_CONSTRAINT_CONE:
                {
                    btConeTwistConstraint* coneC = new btConeTwistConstraint(*physics->bt_body[btA->index], *physics->bt_body[btB->index], localA, localB);
                    coneC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], setup->joint_setup[i].joint_limit[2], 0.9, 0.3, 0.7);
                    physics->bt_joints[i] = coneC;
                }
                break;
        }

        physics->bt_joints[i]->setParam(BT_CONSTRAINT_STOP_CFM, setup->joint_cfm, -1);
        physics->bt_joints[i]->setParam(BT_CONSTRAINT_STOP_ERP, setup->joint_erp, -1);

        physics->bt_joints[i]->setDbgDrawSize(64.0);
        bt_engine_dynamicsWorld->addConstraint(physics->bt_joints[i], true);
    }

    if(result == false)
    {
        Ragdoll_Delete(physics);  // PARANOID: Clean up the mess, if something went wrong.
    }

    physics->cont->collision_group = COLLISION_GROUP_DYNAMICS_NI;

    return result;
}


bool Ragdoll_Delete(struct physics_data_s *physics)
{
    if(physics->bt_joint_count == 0)
    {
        return false;
    }

    for(uint32_t i = 0; i < physics->bt_joint_count; i++)
    {
        if(physics->bt_joints[i])
        {
            bt_engine_dynamicsWorld->removeConstraint(physics->bt_joints[i]);
            delete physics->bt_joints[i];
            physics->bt_joints[i] = NULL;
        }
    }

    for(uint32_t i = 0; i < physics->objects_count; i++)
    {
        if(physics->bt_body[i]->isInWorld())
        {
            bt_engine_dynamicsWorld->removeRigidBody(physics->bt_body[i]);
        }
        physics->bt_body[i]->setMassProps(0, btVector3(0.0, 0.0, 0.0));
        bt_engine_dynamicsWorld->addRigidBody(physics->bt_body[i], btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::AllFilter);
    }

    free(physics->bt_joints);
    physics->bt_joints = NULL;
    physics->bt_joint_count = 0;
    physics->cont->collision_group = COLLISION_GROUP_CHARACTERS;

    return true;

    // NB! Bodies remain in the same state!
    // To make them static again, additionally call setEntityBodyMass script function.
}
