/* 
 * File:   engine_bullet.h
 * Author: nTesla
 *
 * Created on July 11, 2015, 1:27 PM
 */

#ifndef ENGINE_BULLET_H
#define	ENGINE_BULLET_H

#include <stdint.h>
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionWorld.h"


class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

extern btDefaultCollisionConfiguration         *bt_engine_collisionConfiguration;
extern btCollisionDispatcher                   *bt_engine_dispatcher;
extern btBroadphaseInterface                   *bt_engine_overlappingPairCache;
extern btSequentialImpulseConstraintSolver     *bt_engine_solver ;
extern btDiscreteDynamicsWorld                 *bt_engine_dynamicsWorld;

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

protected:
    bool               m_skip_ghost;
    engine_container_p m_cont;
};


class render_DebugDrawer:public btIDebugDraw
{
    uint32_t m_debugMode;
    uint32_t m_max_lines;
    uint32_t m_lines;
    bool     m_need_realloc;

    GLuint  m_gl_vbo;
    GLfloat m_color[3];
    GLfloat *m_buffer;
    
    struct obb_s *m_obb;

    public:
        // engine debug function
        render_DebugDrawer();
        ~render_DebugDrawer();
        bool IsEmpty()
        {
            return m_lines == 0;
        }
        void reset();
        void render();
        void setColor(GLfloat r, GLfloat g, GLfloat b)
        {
            m_color[0] = r;
            m_color[1] = g;
            m_color[2] = b;
        }
        void drawAxis(btScalar r, btScalar transform[16]);
        void drawPortal(struct portal_s *p);
        void drawFrustum(struct frustum_s *f);
        void drawBBox(btScalar bb_min[3], btScalar bb_max[3], btScalar *transform);
        void drawOBB(struct obb_s *obb);
        void drawMeshDebugLines(struct base_mesh_s *mesh, btScalar transform[16], const btScalar *overrideVertices, const btScalar *overrideNormals);
        void drawSkeletalModelDebugLines(struct ss_bone_frame_s *bframe, btScalar transform[16]);
        void drawEntityDebugLines(struct entity_s *entity);
        void drawSectorDebugLines(struct room_sector_s *rs);
        void drawRoomDebugLines(struct room_s *room, struct render_s *render);
        
        // bullet's debug interface
        virtual void   drawLine(const btVector3& from,const btVector3& to,const btVector3& color);
        virtual void   drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);
        virtual void   reportErrorWarning(const char* warningString);
        virtual void   draw3dText(const btVector3& location, const char* textString);
        virtual void   setDebugMode(int debugMode);
        virtual int    getDebugMode() const {return m_debugMode;}
};


extern render_DebugDrawer   debugDrawer;


void Engine_BTInit();
void Engine_BTDestroy();

void Engine_RoomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo);
void Engine_InternalTickCallback(btDynamicsWorld *world, btScalar timeStep);

#endif	/* ENGINE_BULLET_H */
