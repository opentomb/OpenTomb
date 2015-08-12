
#include <stdio.h>
#include <stdlib.h>

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "bullet/BulletCollision/CollisionShapes/btCollisionShape.h"
#include "bullet/BulletDynamics/ConstraintSolver/btTypedConstraint.h"
#include "bullet/BulletCollision/CollisionDispatch/btGhostObject.h"
#include "bullet/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"

#include "core/gl_util.h"
#include "core/gl_font.h"
#include "core/console.h"
#include "core/obb.h"
#include "engine.h"
#include "mesh.h"
#include "character_controller.h"
#include "engine_physics.h"
#include "entity.h"
#include "render.h"
#include "resource.h"
#include "world.h"

//#error MOVE ALL ENTITYES FUNCTIONS TO entity.cpp / entity.h MODULE!!!!!!!!!!!!!!

struct physics_object_s
{
    btRigidBody    *bt_body;
};

typedef struct physics_data_s
{
    // kinematic
    btRigidBody                       **bt_body;

    // dynamic
    uint32_t                            no_fix_skeletal_parts;
    int8_t                              no_fix_all;
    btPairCachingGhostObject          **ghostObjects;           // like Bullet character controller for penetration resolving.
    btManifoldArray                    *manifoldArray;          // keep track of the contact manifolds
    uint16_t                            objects_count;          // Ragdoll joints
    uint16_t                            bt_joint_count;         // Ragdoll joints
    btTypedConstraint                 **bt_joints;              // Ragdoll joints
}physics_data_t, *physics_data_p;

/*
 * INTERNAL BHYSICS CLASSES
 */
class bt_engine_ClosestRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
    bt_engine_ClosestRayResultCallback(engine_container_p cont, bool skip_ghost = false) : btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
    {
        m_cont = cont;
        m_skip_ghost = skip_ghost;
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult,bool normalInWorldSpace)
    {
        room_p r0 = NULL, r1 = NULL;
        engine_container_p c1;

        r0 = (m_cont)?(m_cont->room):(NULL);
        c1 = (engine_container_p)rayResult.m_collisionObject->getUserPointer();
        r1 = (c1)?(c1->room):(NULL);

        if(c1 && ((c1 == m_cont) || (m_skip_ghost && (c1->collision_type == COLLISION_TYPE_GHOST))))
        {
            return 1.0;
        }

        if(!r0 || !r1)
        {
            return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
        }

        if(r0 && r1)
        {
            if(Room_IsInNearRoomsList(r0, r1))
            {
                return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
            }
            else
            {
                return 1.0;
            }
        }

        return 1.0;
    }

    bool               m_skip_ghost;
    engine_container_p m_cont;
};


class bt_engine_ClosestConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
    bt_engine_ClosestConvexResultCallback(engine_container_p cont, bool skip_ghost = false) : btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
    {
        m_cont = cont;
        m_skip_ghost = skip_ghost;
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult,bool normalInWorldSpace)
    {
        room_p r0 = NULL, r1 = NULL;
        engine_container_p c1;

        r0 = (m_cont)?(m_cont->room):(NULL);
        c1 = (engine_container_p)convexResult.m_hitCollisionObject->getUserPointer();
        r1 = (c1)?(c1->room):(NULL);

        if(c1 && ((c1 == m_cont) || (m_skip_ghost && (c1->collision_type == COLLISION_TYPE_GHOST))))
        {
            return 1.0;
        }

        if(!r0 || !r1)
        {
            return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
        }

        if(r0 && r1)
        {
            if(Room_IsInNearRoomsList(r0, r1))
            {
                return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
            }
            else
            {
                return 1.0;
            }
        }

        return 1.0;
    }

private:
    bool               m_skip_ghost;
    engine_container_p m_cont;
};


class CBulletDebugDrawer : public btIDebugDraw
{
public:
    CBulletDebugDrawer(){}
   ~CBulletDebugDrawer(){}

    virtual void   drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
    {
        renderer.debug_drawer->drawLine(from.m_floats, to.m_floats, color.m_floats, color.m_floats);
    }

    virtual void   drawLine(const btVector3& from,const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
    {
        renderer.debug_drawer->drawLine(from.m_floats, to.m_floats, fromColor.m_floats, toColor.m_floats);
    }

    virtual void   drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
    {

    }

    virtual void   reportErrorWarning(const char* warningString)
    {
        Con_AddLine(warningString, FONTSTYLE_CONSOLE_WARNING);
    }

    virtual void   draw3dText(const btVector3& location, const char* textString)
    {
        //glRasterPos3f(location.x(),  location.y(),  location.z());
        //BMF_DrawString(BMF_GetFont(BMF_kHelvetica10),textString);
    }

    virtual void   setDebugMode(int debugMode)
    {
        renderer.debug_drawer->setDebugMode(debugMode);
    }

    virtual int    getDebugMode() const
    {
        return renderer.debug_drawer->getDebugMode();
    }
};

btDefaultCollisionConfiguration         *bt_engine_collisionConfiguration = NULL;
btCollisionDispatcher                   *bt_engine_dispatcher = NULL;
btGhostPairCallback                     *bt_engine_ghostPairCallback = NULL;
btBroadphaseInterface                   *bt_engine_overlappingPairCache = NULL;
btSequentialImpulseConstraintSolver     *bt_engine_solver = NULL;
btDiscreteDynamicsWorld                 *bt_engine_dynamicsWorld = NULL;

CBulletDebugDrawer                       bt_debug_drawer;

uint32_t                                 collision_nodes_pool_size = 0;
uint32_t                                 collision_nodes_pool_used = 0;
struct collision_node_s                 *collision_nodes_pool = NULL;

struct collision_node_s *Physics_GetCollisionNode();

void Physics_RoomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo);
void Physics_InternalTickCallback(btDynamicsWorld *world, btScalar timeStep);

/* bullet collision model calculation */
btCollisionShape* BT_CSfromBBox(btScalar *bb_min, btScalar *bb_max);
btCollisionShape* BT_CSfromMesh(struct base_mesh_s *mesh, bool useCompression, bool buildBvh, bool is_static = true);
btCollisionShape* BT_CSfromHeightmap(struct room_sector_s *heightmap, struct sector_tween_s *tweens, int tweens_size, bool useCompression, bool buildBvh);

// Bullet Physics initialization.
void Physics_Init()
{
    collision_nodes_pool = (struct collision_node_s*)malloc(DEFAULT_COLLSION_NODE_POOL_SIZE * sizeof(struct collision_node_s));
    collision_nodes_pool_size = DEFAULT_COLLSION_NODE_POOL_SIZE;
    collision_nodes_pool_used = 0;

    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    bt_engine_collisionConfiguration = new btDefaultCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    bt_engine_dispatcher = new btCollisionDispatcher(bt_engine_collisionConfiguration);
    bt_engine_dispatcher->setNearCallback(Physics_RoomNearCallback);

    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    bt_engine_overlappingPairCache = new btDbvtBroadphase();
    bt_engine_ghostPairCallback = new btGhostPairCallback();
    bt_engine_overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(bt_engine_ghostPairCallback);

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    bt_engine_solver = new btSequentialImpulseConstraintSolver;

    bt_engine_dynamicsWorld = new btDiscreteDynamicsWorld(bt_engine_dispatcher, bt_engine_overlappingPairCache, bt_engine_solver, bt_engine_collisionConfiguration);
    bt_engine_dynamicsWorld->setInternalTickCallback(Physics_InternalTickCallback);
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

    free(collision_nodes_pool);
    collision_nodes_pool = NULL;
    collision_nodes_pool_size = 0;
    collision_nodes_pool_used = 0;
}


void Physics_StepSimulation(btScalar time)
{
    bt_engine_dynamicsWorld->stepSimulation(time, 0);
    collision_nodes_pool_used = 0;
}

void Physics_DebugDrawWorld()
{
    bt_engine_dynamicsWorld->debugDrawWorld();
}

struct collision_node_s *Physics_GetCollisionNode()
{
    struct collision_node_s *ret = NULL;
    if(collision_nodes_pool_used < collision_nodes_pool_size)
    {
        ret = collision_nodes_pool + collision_nodes_pool_used;
        collision_nodes_pool_used++;
    }
    return ret;
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

struct physics_data_s *Physics_CreatePhysicsData()
{
    struct physics_data_s *ret = (struct physics_data_s*)malloc(sizeof(struct physics_data_s));

    ret->bt_body = NULL;
    ret->bt_joints = NULL;
    ret->objects_count = 0;
    ret->bt_joint_count = 0;
    ret->no_fix_all = 0x00;
    ret->no_fix_skeletal_parts = 0x00000000;
    ret->manifoldArray = NULL;
    ret->ghostObjects = NULL;

    return ret;
}


void Physics_DeletePhysicsData(struct physics_data_s *physics)
{
    if(physics)
    {
        if(physics->ghostObjects)
        {
            for(int i=0;i<physics->objects_count;i++)
            {
                physics->ghostObjects[i]->setUserPointer(NULL);
                if(physics->ghostObjects[i]->getCollisionShape())
                {
                    delete physics->ghostObjects[i]->getCollisionShape();
                    physics->ghostObjects[i]->setCollisionShape(NULL);
                }
                bt_engine_dynamicsWorld->removeCollisionObject(physics->ghostObjects[i]);
                delete physics->ghostObjects[i];
                physics->ghostObjects[i] = NULL;
            }
            free(physics->ghostObjects);
            physics->ghostObjects = NULL;
        }

        if(physics->manifoldArray)
        {
            physics->manifoldArray->clear();
            delete physics->manifoldArray;
            physics->manifoldArray = NULL;
        }

        if(physics->bt_body)
        {
            for(int i=0;i<physics->objects_count;i++)
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

        physics->objects_count = 0;
        free(physics);
    }
}


/**
 * overlapping room collision filter
 */
void Physics_RoomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
{
    engine_container_p c0, c1;
    room_p r0 = NULL, r1 = NULL;

    c0 = (engine_container_p)((btCollisionObject*)collisionPair.m_pProxy0->m_clientObject)->getUserPointer();
    r0 = (c0)?(c0->room):(NULL);
    c1 = (engine_container_p)((btCollisionObject*)collisionPair.m_pProxy1->m_clientObject)->getUserPointer();
    r1 = (c1)?(c1->room):(NULL);

    if(c1 && c1 == c0)
    {
        if(((btCollisionObject*)collisionPair.m_pProxy0->m_clientObject)->isStaticOrKinematicObject() ||
           ((btCollisionObject*)collisionPair.m_pProxy1->m_clientObject)->isStaticOrKinematicObject())
        {
            return;                                                             // No self interaction
        }
        dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
        return;
    }

    if(!r0 && !r1)
    {
        dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);// Both are out of rooms
        return;
    }

    if(r0 && r1)
    {
        if(Room_IsInNearRoomsList(r0, r1))
        {
            dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
            return;
        }
        else
        {
            return;
        }
    }
}

/**
 * update current room of bullet object
 */
void Physics_InternalTickCallback(btDynamicsWorld *world, btScalar timeStep)
{
    for(int i=world->getNumCollisionObjects()-1;i>=0;i--)
    {
        btCollisionObject* obj = bt_engine_dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && !body->isStaticObject() && body->getMotionState())
        {
            btTransform trans;
            body->getMotionState()->getWorldTransform(trans);
            engine_container_p cont = (engine_container_p)body->getUserPointer();
            if(cont && (cont->object_type == OBJECT_BULLET_MISC))
            {
                cont->room = Room_FindPosCogerrence(trans.getOrigin().m_floats, cont->room);
            }
        }
    }
}


/* Common physics functions */
void Physics_GetGravity(btScalar g[3])
{
    btVector3 bt_g = bt_engine_dynamicsWorld->getGravity();
    vec3_copy(g, bt_g.m_floats);
}


void Physics_SetGravity(btScalar g[3])
{
    btVector3 bt_g(g[0], g[1], g[2]);
    bt_engine_dynamicsWorld->setGravity(bt_g);
}


int  Physics_RayTest(struct collision_result_s *result, btScalar from[3], btScalar to[3], struct engine_container_s *cont)
{
    bt_engine_ClosestRayResultCallback cb(cont, true);
    btVector3 vFrom(from[0], from[1], from[2]), vTo(to[0], to[1], to[2]);

    cb.m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    if(result)
    {
        result->obj = NULL;
        bt_engine_dynamicsWorld->rayTest(vFrom, vTo, cb);
        if(cb.hasHit())
        {
            result->obj      = (struct engine_container_s *)cb.m_collisionObject->getUserPointer();
            result->bone_num = cb.m_collisionObject->getUserIndex();
            vec3_copy(result->normale, cb.m_hitNormalWorld.m_floats);
            vFrom.setInterpolate3(vFrom, vTo, cb.m_closestHitFraction);
            vec3_copy(result->point, vFrom.m_floats);
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


int  Physics_SphereTest(struct collision_result_s *result, btScalar from[3], btScalar to[3], btScalar R, struct engine_container_s *cont)
{
    bt_engine_ClosestConvexResultCallback cb(cont, true);
    btVector3 vFrom(from[0], from[1], from[2]), vTo(to[0], to[1], to[2]);
    btTransform tFrom, tTo;
    btSphereShape sphere(R);

    tFrom.setIdentity();
    tFrom.setOrigin(vFrom);
    tTo.setIdentity();
    tTo.setOrigin(vTo);

    cb.m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    if(result)
    {
        result->obj = NULL;
        bt_engine_dynamicsWorld->convexSweepTest(&sphere, tFrom, tTo, cb);
        if(cb.hasHit())
        {
            result->obj      = (struct engine_container_s *)cb.m_hitCollisionObject->getUserPointer();
            result->bone_num = cb.m_hitCollisionObject->getUserIndex();
            vec3_copy(result->normale, cb.m_hitNormalWorld.m_floats);
            vec3_copy(result->point, cb.m_hitPointWorld.m_floats);
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
    return physics && physics->ghostObjects;
}


void Physics_GetBodyWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index)
{
    physics->bt_body[index]->getWorldTransform().getOpenGLMatrix(tr);
}


void Physics_SetBodyWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index)
{
    physics->bt_body[index]->getWorldTransform().setFromOpenGLMatrix(tr);
}


void Physics_GetGhostWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index)
{
    physics->ghostObjects[index]->getWorldTransform().getOpenGLMatrix(tr);
}


void Physics_SetGhostWorldTransform(struct physics_data_s *physics, float tr[16], uint16_t index)
{
    physics->ghostObjects[index]->getWorldTransform().setFromOpenGLMatrix(tr);
}


/**
 * It is from bullet_character_controller
 */
int Physics_GetGhostPenetrationFixVector(struct physics_data_s *physics, uint16_t index, float correction[3])
{
    // Here we must refresh the overlapping paircache as the penetrating movement itself or the
    // previous recovery iteration might have used setWorldTransform and pushed us into an object
    // that is not in the previous cache contents from the last timestep, as will happen if we
    // are pushed into a new AABB overlap. Unhandled this means the next convex sweep gets stuck.
    //
    // Do this by calling the broadphase's setAabb with the moved AABB, this will update the broadphase
    // paircache and the ghostobject's internal paircache at the same time.    /BW

    int ret = 0;
    int num_pairs, manifolds_size;
    btPairCachingGhostObject *ghost = physics->ghostObjects[index];
    btBroadphasePairArray &pairArray = ghost->getOverlappingPairCache()->getOverlappingPairArray();
    btVector3 aabb_min, aabb_max, t;

    ghost->getCollisionShape()->getAabb(ghost->getWorldTransform(), aabb_min, aabb_max);
    bt_engine_dynamicsWorld->getBroadphase()->setAabb(ghost->getBroadphaseHandle(), aabb_min, aabb_max, bt_engine_dynamicsWorld->getDispatcher());
    bt_engine_dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(ghost->getOverlappingPairCache(), bt_engine_dynamicsWorld->getDispatchInfo(), bt_engine_dynamicsWorld->getDispatcher());

    vec3_set_zero(correction);
    num_pairs = ghost->getOverlappingPairCache()->getNumOverlappingPairs();
    for(int i=0;i<num_pairs;i++)
    {
        physics->manifoldArray->clear();
        // do not use commented code: it prevents to collision skips.
        //btBroadphasePair &pair = pairArray[i];
        //btBroadphasePair* collisionPair = bt_engine_dynamicsWorld->getPairCache()->findPair(pair.m_pProxy0,pair.m_pProxy1);
        btBroadphasePair *collisionPair = &pairArray[i];

        if(!collisionPair)
        {
            continue;
        }

        if(collisionPair->m_algorithm)
        {
            collisionPair->m_algorithm->getAllContactManifolds(*(physics->manifoldArray));
        }

        manifolds_size = physics->manifoldArray->size();
        for(int j=0;j<manifolds_size;j++)
        {
            btPersistentManifold* manifold = (*(physics->manifoldArray))[j];
            btScalar directionSign = manifold->getBody0() == ghost ? btScalar(-1.0) : btScalar(1.0);
            engine_container_p cont0 = (engine_container_p)manifold->getBody0()->getUserPointer();
            engine_container_p cont1 = (engine_container_p)manifold->getBody1()->getUserPointer();
            if((cont0->collision_type == COLLISION_TYPE_GHOST) || (cont1->collision_type == COLLISION_TYPE_GHOST))
            {
                continue;
            }
            for(int k=0;k<manifold->getNumContacts();k++)
            {
                const btManifoldPoint&pt = manifold->getContactPoint(k);
                btScalar dist = pt.getDistance();

                if(dist < 0.0)
                {
                    t = pt.m_normalWorldOnB * dist * directionSign;
                    vec3_add(correction, correction, t.m_floats)
                    ret++;
                }
            }
        }
    }

    return ret;
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
    for(uint16_t i=0;i<6;i++,p++)
    {
        if(Polygon_IsBroken(p))
        {
            continue;
        }
        for(uint16_t j=1;j+1<p->vertex_count;j++)
        {
            vec3_copy(v0.m_floats, p->vertices[j + 1].position);
            vec3_copy(v1.m_floats, p->vertices[j].position);
            vec3_copy(v2.m_floats, p->vertices[0].position);
            trimesh->addTriangle(v0, v1, v2, true);
        }
        cnt ++;
    }

    if(cnt == 0)                                                                // fixed: without that condition engine may easily crash
    {
        delete trimesh;
        return NULL;
    }

    OBB_Clear(obb);
    free(obb);

    ret = new btConvexTriangleMeshShape(trimesh, true);

    return ret;
}


btCollisionShape *BT_CSfromMesh(struct base_mesh_s *mesh, bool useCompression, bool buildBvh, bool is_static)
{
    uint32_t cnt = 0;
    polygon_p p;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;
    btVector3 v0, v1, v2;

    p = mesh->polygons;
    for(uint32_t i=0;i<mesh->polygons_count;i++,p++)
    {
        if(Polygon_IsBroken(p))
        {
            continue;
        }

        for(uint16_t j=1;j+1<p->vertex_count;j++)
        {
            vec3_copy(v0.m_floats, p->vertices[j + 1].position);
            vec3_copy(v1.m_floats, p->vertices[j].position);
            vec3_copy(v2.m_floats, p->vertices[0].position);
            trimesh->addTriangle(v0, v1, v2, true);
        }
        cnt ++;
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

///@TODO: resolve cases with floor >> ceiling (I.E. floor - ceiling >= 2048)
btCollisionShape *BT_CSfromHeightmap(struct room_sector_s *heightmap, struct sector_tween_s *tweens, int tweens_size, bool useCompression, bool buildBvh)
{
    uint32_t cnt = 0;
    room_p r = heightmap->owner_room;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;

    for(uint32_t i = 0; i < r->sectors_count; i++)
    {
        if( (heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_GHOST) &&
            (heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_WALL )  )
        {
            if( (heightmap[i].floor_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NONE) ||
                (heightmap[i].floor_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NW  )  )
            {
                if(heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
                {
                    btScalar *v0 = heightmap[i].floor_corners[3];
                    btScalar *v1 = heightmap[i].floor_corners[2];
                    btScalar *v2 = heightmap[i].floor_corners[0];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v2[0], v2[1], v2[2]),
                                         true);
                    cnt++;
                }

                if(heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
                {
                    btScalar *v0 = heightmap[i].floor_corners[2];
                    btScalar *v1 = heightmap[i].floor_corners[1];
                    btScalar *v2 = heightmap[i].floor_corners[0];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v2[0], v2[1], v2[2]),
                                         true);
                    cnt++;
                }
            }
            else
            {
                if(heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
                {
                    btScalar *v0 = heightmap[i].floor_corners[3];
                    btScalar *v1 = heightmap[i].floor_corners[2];
                    btScalar *v2 = heightmap[i].floor_corners[1];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v2[0], v2[1], v2[2]),
                                         true);
                    cnt++;
                }

                if(heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
                {
                    btScalar *v0 = heightmap[i].floor_corners[3];
                    btScalar *v1 = heightmap[i].floor_corners[1];
                    btScalar *v2 = heightmap[i].floor_corners[0];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v2[0], v2[1], v2[2]),
                                         true);
                    cnt++;
                }
            }
        }

        if( (heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_GHOST) &&
            (heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_WALL )  )
        {
            if( (heightmap[i].ceiling_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NONE) ||
                (heightmap[i].ceiling_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NW  )  )
            {
                if(heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
                {
                    btScalar *v0 = heightmap[i].ceiling_corners[0];
                    btScalar *v1 = heightmap[i].ceiling_corners[2];
                    btScalar *v2 = heightmap[i].ceiling_corners[3];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v2[0], v2[1], v2[2]),
                                         true);
                    cnt++;
                }

                if(heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
                {
                    btScalar *v0 = heightmap[i].ceiling_corners[0];
                    btScalar *v1 = heightmap[i].ceiling_corners[1];
                    btScalar *v2 = heightmap[i].ceiling_corners[2];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v2[0], v2[1], v2[2]),
                                         true);
                    cnt++;
                }
            }
            else
            {
                if(heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
                {
                    btScalar *v0 = heightmap[i].ceiling_corners[0];
                    btScalar *v1 = heightmap[i].ceiling_corners[1];
                    btScalar *v2 = heightmap[i].ceiling_corners[3];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v2[0], v2[1], v2[2]),
                                         true);
                    cnt++;
                }

                if(heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
                {
                    btScalar *v0 = heightmap[i].ceiling_corners[1];
                    btScalar *v1 = heightmap[i].ceiling_corners[2];
                    btScalar *v2 = heightmap[i].ceiling_corners[3];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v2[0], v2[1], v2[2]),
                                         true);
                    cnt++;
                }
            }
        }
    }

    for(int i=0; i<tweens_size; i++)
    {
        switch(tweens[i].ceiling_tween_type)
        {
            case TR_SECTOR_TWEEN_TYPE_2TRIANGLES:
                {
                    btScalar t = fabs((tweens[i].ceiling_corners[2][2] - tweens[i].ceiling_corners[3][2]) /
                                      (tweens[i].ceiling_corners[0][2] - tweens[i].ceiling_corners[1][2]));
                    t = 1.0 / (1.0 + t);
                    btScalar o[3], t1 = 1.0 - t;
                    btScalar *v0 = tweens[i].ceiling_corners[0];
                    btScalar *v1 = tweens[i].ceiling_corners[1];
                    btScalar *v2 = tweens[i].ceiling_corners[2];
                    btScalar *v3 = tweens[i].ceiling_corners[3];
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
                    btScalar *v0 = tweens[i].ceiling_corners[0];
                    btScalar *v1 = tweens[i].ceiling_corners[1];
                    btScalar *v2 = tweens[i].ceiling_corners[3];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v2[0], v2[1], v2[2]),
                                         true);
                    cnt++;
                }
                break;

            case TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT:
                {
                    btScalar *v0 = tweens[i].ceiling_corners[2];
                    btScalar *v1 = tweens[i].ceiling_corners[1];
                    btScalar *v2 = tweens[i].ceiling_corners[3];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v2[0], v2[1], v2[2]),
                                         true);
                    cnt++;
                }
                break;

            case TR_SECTOR_TWEEN_TYPE_QUAD:
                {
                    btScalar *v0 = tweens[i].ceiling_corners[0];
                    btScalar *v1 = tweens[i].ceiling_corners[1];
                    btScalar *v2 = tweens[i].ceiling_corners[2];
                    btScalar *v3 = tweens[i].ceiling_corners[3];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v3[0], v3[1], v3[2]),
                                         true);
                    trimesh->addTriangle(btVector3(v2[0], v2[1], v2[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v3[0], v3[1], v3[2]),
                                         true);
                    cnt += 2;
                }
                break;
        };

        switch(tweens[i].floor_tween_type)
        {
            case TR_SECTOR_TWEEN_TYPE_2TRIANGLES:
                {
                    btScalar t = fabs((tweens[i].floor_corners[2][2] - tweens[i].floor_corners[3][2]) /
                                      (tweens[i].floor_corners[0][2] - tweens[i].floor_corners[1][2]));
                    t = 1.0 / (1.0 + t);
                    btScalar o[3], t1 = 1.0 - t;
                    btScalar *v0 = tweens[i].floor_corners[0];
                    btScalar *v1 = tweens[i].floor_corners[1];
                    btScalar *v2 = tweens[i].floor_corners[2];
                    btScalar *v3 = tweens[i].floor_corners[3];
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
                    btScalar *v0 = tweens[i].floor_corners[0];
                    btScalar *v1 = tweens[i].floor_corners[1];
                    btScalar *v2 = tweens[i].floor_corners[3];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v2[0], v2[1], v2[2]),
                                         true);
                    cnt++;
                }
                break;

            case TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT:
                {
                    btScalar *v0 = tweens[i].floor_corners[2];
                    btScalar *v1 = tweens[i].floor_corners[1];
                    btScalar *v2 = tweens[i].floor_corners[3];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v2[0], v2[1], v2[2]),
                                         true);
                    cnt++;
                }
                break;

            case TR_SECTOR_TWEEN_TYPE_QUAD:
                {
                    btScalar *v0 = tweens[i].floor_corners[0];
                    btScalar *v1 = tweens[i].floor_corners[1];
                    btScalar *v2 = tweens[i].floor_corners[2];
                    btScalar *v3 = tweens[i].floor_corners[3];
                    trimesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v3[0], v3[1], v3[2]),
                                         true);
                    trimesh->addTriangle(btVector3(v2[0], v2[1], v2[2]),
                                         btVector3(v1[0], v1[1], v1[2]),
                                         btVector3(v3[0], v3[1], v3[2]),
                                         true);
                    cnt += 2;
                }
                break;
        };
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

void Physics_GenEntityRigidBody(struct entity_s *ent)
{
    btScalar tr[16];
    btVector3 localInertia(0, 0, 0);
    btTransform startTransform;

    if(ent->bf->animations.model == NULL)
    {
        return;
    }

    ent->physics->objects_count = ent->bf->bone_tag_count;
    ent->physics->bt_body = (btRigidBody**)malloc(ent->physics->objects_count * sizeof(btRigidBody*));

    for(uint16_t i=0;i<ent->physics->objects_count;i++)
    {
        base_mesh_p mesh = ent->bf->animations.model->mesh_tree[i].mesh_base;
        btCollisionShape *cshape = NULL;
        switch(ent->self->collision_shape)
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

        ent->physics->bt_body[i] = NULL;

        if(cshape)
        {
            cshape->calculateLocalInertia(0.0, localInertia);

            Mat4_Mat4_mul(tr, ent->transform, ent->bf->bone_tags[i].full_transform);
            startTransform.setFromOpenGLMatrix(tr);
            btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
            ent->physics->bt_body[i] = new btRigidBody(0.0, motionState, cshape, localInertia);
            //ent->physics->bt_body[i]->setCollisionFlags(ent->physics->bt_body[i]->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            bt_engine_dynamicsWorld->addRigidBody(ent->physics->bt_body[i], COLLISION_GROUP_KINEMATIC, COLLISION_MASK_ALL);
            ent->physics->bt_body[i]->setUserPointer(ent->self);
            ent->physics->bt_body[i]->setUserIndex(i);
        }
    }
}


void Physics_GenStaticMeshRigidBody(struct static_mesh_s *smesh)
{
    btCollisionShape *cshape = NULL;

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
        bt_engine_dynamicsWorld->addRigidBody(smesh->physics_body->bt_body, COLLISION_GROUP_ALL, COLLISION_MASK_ALL);
        smesh->physics_body->bt_body->setUserPointer(smesh->self);
    }
}


void Physics_GenRoomRigidBody(struct room_s *room, struct sector_tween_s *tweens, int num_tweens)
{
    btCollisionShape *cshape = BT_CSfromHeightmap(room->sectors, tweens, num_tweens, true, true);
    room->physics_body = NULL;

    if(cshape)
    {
        btVector3 localInertia(0, 0, 0);
        btTransform tr;
        tr.setFromOpenGLMatrix(room->transform);
        room->physics_body = (struct physics_object_s*)malloc(sizeof(struct physics_object_s));
        btDefaultMotionState* motionState = new btDefaultMotionState(tr);
        room->physics_body->bt_body = new btRigidBody(0.0, motionState, cshape, localInertia);
        bt_engine_dynamicsWorld->addRigidBody(room->physics_body->bt_body, COLLISION_GROUP_ALL, COLLISION_MASK_ALL);
        room->physics_body->bt_body->setUserPointer(room->self);
        room->physics_body->bt_body->setUserIndex(0);
        room->physics_body->bt_body->setRestitution(1.0);
        room->physics_body->bt_body->setFriction(1.0);
        room->self->collision_type = COLLISION_TYPE_STATIC;                     // meshtree
        room->self->collision_shape = COLLISION_SHAPE_TRIMESH;
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
    bt_engine_dynamicsWorld->addRigidBody(obj->bt_body);
}


void Physics_DisableObject(struct physics_object_s *obj)
{
    bt_engine_dynamicsWorld->removeRigidBody(obj->bt_body);
}


void Entity_CreateGhosts(struct entity_s *ent)
{
    if(ent->bf->animations.model->mesh_count > 0)
    {
        btTransform tr;
        btScalar gltr[16], v[3];

        ent->physics->manifoldArray = new btManifoldArray();
        //ent->physics->shapes = (btCollisionShape**)malloc(ent->bf->bone_tag_count * sizeof(btCollisionShape*));
        ent->physics->ghostObjects = (btPairCachingGhostObject**)malloc(ent->bf->bone_tag_count * sizeof(btPairCachingGhostObject*));
        for(uint16_t i=0;i<ent->bf->bone_tag_count;i++)
        {
            btVector3 box;
            box.m_floats[0] = 0.40 * (ent->bf->bone_tags[i].mesh_base->bb_max[0] - ent->bf->bone_tags[i].mesh_base->bb_min[0]);
            box.m_floats[1] = 0.40 * (ent->bf->bone_tags[i].mesh_base->bb_max[1] - ent->bf->bone_tags[i].mesh_base->bb_min[1]);
            box.m_floats[2] = 0.40 * (ent->bf->bone_tags[i].mesh_base->bb_max[2] - ent->bf->bone_tags[i].mesh_base->bb_min[2]);
            ent->bf->bone_tags[i].mesh_base->R = (box.m_floats[0] < box.m_floats[1])?(box.m_floats[0]):(box.m_floats[1]);
            ent->bf->bone_tags[i].mesh_base->R = (ent->bf->bone_tags[i].mesh_base->R < box.m_floats[2])?(ent->bf->bone_tags[i].mesh_base->R):(box.m_floats[2]);

            ent->physics->ghostObjects[i] = new btPairCachingGhostObject();
            ent->physics->ghostObjects[i]->setIgnoreCollisionCheck(ent->physics->bt_body[i], true);
            Mat4_Mat4_mul(gltr, ent->transform, ent->bf->bone_tags[i].full_transform);
            Mat4_vec3_mul(v, gltr, ent->bf->bone_tags[i].mesh_base->centre);
            vec3_copy(gltr+12, v);
            tr.setFromOpenGLMatrix(gltr);
            ent->physics->ghostObjects[i]->setWorldTransform(tr);
            ent->physics->ghostObjects[i]->setCollisionFlags(ent->physics->ghostObjects[i]->getCollisionFlags() | btCollisionObject::CF_CHARACTER_OBJECT);
            ent->physics->ghostObjects[i]->setUserPointer(ent->self);
            ent->physics->ghostObjects[i]->setCollisionShape(new btBoxShape(box));
            bt_engine_dynamicsWorld->addCollisionObject(ent->physics->ghostObjects[i], COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_ALL);
        }
    }
}


/**
 * This function enables collision for entity_p in all cases exept NULL models.
 * If collision models does not exists, function will create them;
 * @param ent - pointer to the entity.
 */
void Physics_EnableCollision(struct physics_data_s *physics)
{
    if(physics->bt_body != NULL)
    {
        for(uint16_t i=0;i<physics->objects_count;i++)
        {
            btRigidBody *b = physics->bt_body[i];
            if((b != NULL) && !b->isInWorld())
            {
                bt_engine_dynamicsWorld->addRigidBody(b);
            }
        }
    }
}


void Physics_DisableCollision(struct physics_data_s *physics)
{
    if(physics->bt_body != NULL)
    {
        for(uint16_t i=0;i<physics->objects_count;i++)
        {
            btRigidBody *b = physics->bt_body[i];
            if((b != NULL) && b->isInWorld())
            {
                bt_engine_dynamicsWorld->removeRigidBody(b);
            }
        }
    }
}


void Physics_SetCollisionScale(struct physics_data_s *physics, float scaling[3])
{
    for(int i=0; i<physics->objects_count; i++)
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

        btVector3 factor = (mass > 0.0)?(btVector3(1.0, 1.0, 1.0)):(btVector3(0.0, 0.0, 0.0));
        physics->bt_body[index]->setLinearFactor (factor);
        physics->bt_body[index]->setAngularFactor(factor);

        //physics_body[index]->forceActivationState(DISABLE_DEACTIVATION);

        //physics_body[index]->setCcdMotionThreshold(32.0);   // disable tunneling effect
        //physics_body[index]->setCcdSweptSphereRadius(32.0);

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

void Physics_SetNoFixBodyPartFlag(struct physics_data_s *physics, uint32_t flag)
{
    physics->no_fix_skeletal_parts = flag;
}


void Physics_SetNoFixAllFlag(struct physics_data_s *physics, uint8_t flag)
{
    physics->no_fix_all = flag;
}


uint8_t Physics_GetNoFixAllFlag(struct physics_data_s *physics)
{
    return physics->no_fix_all;
}


uint32_t Physics_GetNoFixBodyPartFlag(struct physics_data_s *physics)
{
    return physics->no_fix_skeletal_parts;
}


struct collision_node_s *Physics_GetCurrentCollisions(struct physics_data_s *physics, struct engine_container_s *cont)
{
    struct collision_node_s *ret = NULL;

    if(physics->ghostObjects != NULL)
    {
        btTransform orig_tr;
        for(uint16_t i=0;i<physics->objects_count;i++)
        {
            btPairCachingGhostObject *ghost = physics->ghostObjects[i];
            btBroadphasePairArray &pairArray = ghost->getOverlappingPairCache()->getOverlappingPairArray();
            btVector3 aabb_min, aabb_max;

            ghost->getCollisionShape()->getAabb(ghost->getWorldTransform(), aabb_min, aabb_max);
            bt_engine_dynamicsWorld->getBroadphase()->setAabb(ghost->getBroadphaseHandle(), aabb_min, aabb_max, bt_engine_dynamicsWorld->getDispatcher());
            bt_engine_dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(ghost->getOverlappingPairCache(), bt_engine_dynamicsWorld->getDispatchInfo(), bt_engine_dynamicsWorld->getDispatcher());

            int num_pairs = ghost->getOverlappingPairCache()->getNumOverlappingPairs();
            for(int j=0;j<num_pairs;j++)
            {
                physics->manifoldArray->clear();
                btBroadphasePair *collisionPair = &pairArray[j];

                if(!collisionPair)
                {
                    continue;
                }

                if(collisionPair->m_algorithm)
                {
                    collisionPair->m_algorithm->getAllContactManifolds(*physics->manifoldArray);
                }

                for(int k=0;k<physics->manifoldArray->size();k++)
                {
                    btPersistentManifold* manifold = (*physics->manifoldArray)[k];
                    for(int c=0;c<manifold->getNumContacts();c++)               // c++ in C++
                    {
                        //const btManifoldPoint &pt = manifold->getContactPoint(c);
                        if(manifold->getContactPoint(c).getDistance() < 0.0)
                        {
                            collision_node_p cn = Physics_GetCollisionNode();
                            if(cn == NULL)
                            {
                                break;
                            }
                            btCollisionObject *obj = (btCollisionObject*)(*physics->manifoldArray)[k]->getBody0();
                            cn->obj = (engine_container_p)obj->getUserPointer();
                            if(cont == cn->obj)
                            {
                                obj = (btCollisionObject*)(*physics->manifoldArray)[k]->getBody1();
                                cn->obj = (engine_container_p)obj->getUserPointer();
                            }
                            cn->part_from = obj->getUserIndex();
                            cn->part_self = i;
                            cn->next = ret;
                            ret = cn;
                            break;
                        }
                    }
                }
            }
            ghost->setWorldTransform(orig_tr);
        }
    }

    return ret;
}
