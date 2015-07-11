
#include <stdio.h>
#include <stdlib.h>

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/CollisionDispatch/btGhostObject.h"

#include "core/gl_util.h"
#include "core/gl_font.h"
#include "core/console.h"
#include "core/obb.h"
#include "engine.h"
#include "frustum.h"
#include "portal.h"
#include "engine_bullet.h"
#include "entity.h"
#include "render.h"
#include "resource.h"
#include "world.h"


btDefaultCollisionConfiguration         *bt_engine_collisionConfiguration;
btCollisionDispatcher                   *bt_engine_dispatcher;
btGhostPairCallback                     *bt_engine_ghostPairCallback;
btBroadphaseInterface                   *bt_engine_overlappingPairCache;
btSequentialImpulseConstraintSolver     *bt_engine_solver;
btDiscreteDynamicsWorld                 *bt_engine_dynamicsWorld;
btOverlapFilterCallback                 *bt_engine_filterCallback;


render_DebugDrawer                       debugDrawer;

#define DEBUG_DRAWER_DEFAULT_BUFFER_SIZE        (128 * 1024)


// Bullet Physics initialization.
void Engine_BTInit()
{
    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    bt_engine_collisionConfiguration = new btDefaultCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    bt_engine_dispatcher = new btCollisionDispatcher(bt_engine_collisionConfiguration);
    bt_engine_dispatcher->setNearCallback(Engine_RoomNearCallback);

    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    bt_engine_overlappingPairCache = new btDbvtBroadphase();
    bt_engine_ghostPairCallback = new btGhostPairCallback();
    bt_engine_overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(bt_engine_ghostPairCallback);

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    bt_engine_solver = new btSequentialImpulseConstraintSolver;

    bt_engine_dynamicsWorld = new btDiscreteDynamicsWorld(bt_engine_dispatcher, bt_engine_overlappingPairCache, bt_engine_solver, bt_engine_collisionConfiguration);
    bt_engine_dynamicsWorld->setInternalTickCallback(Engine_InternalTickCallback);
    bt_engine_dynamicsWorld->setGravity(btVector3(0, 0, -4500.0));

    debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawConstraints);
    bt_engine_dynamicsWorld->setDebugDrawer(&debugDrawer);
    //bt_engine_dynamicsWorld->getPairCache()->setInternalGhostPairCallback(bt_engine_filterCallback);
}


void Engine_BTDestroy()
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

/**
 * overlapping room collision filter
 */
void Engine_RoomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
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
void Engine_InternalTickCallback(btDynamicsWorld *world, btScalar timeStep)
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

/**
 * DEBUG PRIMITIVES RENDERING
 */
render_DebugDrawer::render_DebugDrawer()
:m_debugMode(0)
{
    m_max_lines = DEBUG_DRAWER_DEFAULT_BUFFER_SIZE;
    m_buffer = (GLfloat*)malloc(2 * 6 * m_max_lines * sizeof(GLfloat));

    m_lines = 0;
    m_gl_vbo = 0;
    m_need_realloc = false;
    vec3_set_zero(m_color);
    m_obb = OBB_Create();
}


render_DebugDrawer::~render_DebugDrawer()
{
    free(m_buffer);
    m_buffer = NULL;
    if(m_gl_vbo != 0)
    {
        glDeleteBuffersARB(1, &m_gl_vbo);
        m_gl_vbo = 0;
    }
    OBB_Clear(m_obb);
    m_obb = NULL;
}


void render_DebugDrawer::reset()
{
    if(m_need_realloc)
    {
        uint32_t new_buffer_size = m_max_lines * 12 * 2;
        GLfloat *new_buffer = (GLfloat*)realloc(m_buffer, new_buffer_size * sizeof(GLfloat));
        if(new_buffer != NULL)
        {
            m_buffer = new_buffer;
            m_max_lines *= 2;
        }
        m_need_realloc = false;
    }
    if(m_gl_vbo == 0)
    {
        glGenBuffersARB(1, &m_gl_vbo);
    }
    m_lines = 0;
}


void render_DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    GLfloat *v;

    if(m_lines < m_max_lines - 1)
    {
        v = m_buffer + 3 * 4 * m_lines;
        m_lines++;

        vec3_copy(v, from.m_floats);
        v += 3;
        vec3_copy(v, color.m_floats);
        v += 3;
        vec3_copy(v, to.m_floats);
        v += 3;
        vec3_copy(v, color.m_floats);
    }
    else
    {
        m_need_realloc = true;
    }
}


void render_DebugDrawer::setDebugMode(int debugMode)
{
   m_debugMode = debugMode;
}


void render_DebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
   //glRasterPos3f(location.x(),  location.y(),  location.z());
   //BMF_DrawString(BMF_GetFont(BMF_kHelvetica10),textString);
}


void render_DebugDrawer::reportErrorWarning(const char* warningString)
{
   Con_AddLine(warningString, FONTSTYLE_CONSOLE_WARNING);
}


void render_DebugDrawer::drawContactPoint(const btVector3& pointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
{
    if(m_lines + 2 < m_max_lines)
    {
        GLfloat *v = m_buffer + 3 * 4 * m_lines;
        m_lines += 2;
        btVector3 to = pointOnB + normalOnB * distance;

        vec3_copy(v, pointOnB.m_floats);
        v += 3;
        vec3_copy(v, color.m_floats);
        v += 3;

        vec3_copy(v, to.m_floats);
        v += 3;
        vec3_copy(v, color.m_floats);

        //glRasterPos3f(from.x(),  from.y(),  from.z());
        //char buf[12];
        //sprintf(buf," %d",lifeTime);
        //BMF_DrawString(BMF_GetFont(BMF_kHelvetica10),buf);
    }
    else
    {
        m_need_realloc = true;
    }
}


void render_DebugDrawer::render()
{
    if((m_lines > 0) && (m_gl_vbo != 0))
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_gl_vbo);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_lines * 12 * sizeof(GLfloat), m_buffer, GL_STREAM_DRAW);
        glVertexPointer(3, GL_FLOAT, 6 * sizeof(GLfloat), (void*)0);
        glColorPointer(3, GL_FLOAT, 6 * sizeof(GLfloat),  (void*)(3 * sizeof(GLfloat)));
        glDrawArrays(GL_LINES, 0, 2 * m_lines);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }

    vec3_set_zero(m_color);
    m_lines = 0;
}


void render_DebugDrawer::drawAxis(btScalar r, btScalar transform[16])
{
    GLfloat *v0, *v;

    if(m_lines + 3 >= m_max_lines)
    {
        m_need_realloc = true;
        return;
    }

    v0 = v = m_buffer + 3 * 4 * m_lines;
    m_lines += 3;
    vec3_copy(v0, transform + 12);

    // OX
    v += 3;
    v[0] = 1.0;
    v[1] = 0.0;
    v[2] = 0.0;
    v += 3;
    vec3_add_mul(v, v0, transform + 0, r);
    v += 3;
    v[0] = 1.0;
    v[1] = 0.0;
    v[2] = 0.0;
    v += 3;

    // OY
    vec3_copy(v, v0);
    v += 3;
    v[0] = 0.0;
    v[1] = 1.0;
    v[2] = 0.0;
    v += 3;
    vec3_add_mul(v, v0, transform + 4, r);
    v += 3;
    v[0] = 0.0;
    v[1] = 1.0;
    v[2] = 0.0;
    v += 3;

    // OZ
    vec3_copy(v, v0);
    v += 3;
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 1.0;
    v += 3;
    vec3_add_mul(v, v0, transform + 8, r);
    v += 3;
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 1.0;
}


void render_DebugDrawer::drawFrustum(struct frustum_s *f)
{
    if(f != NULL)
    {
        GLfloat *v, *v0;
        btScalar *fv = f->vertex;

        if(m_lines + f->vertex_count >= m_max_lines)
        {
            m_need_realloc = true;
            return;
        }

        v = v0 = m_buffer + 3 * 4 * m_lines;
        m_lines += f->vertex_count;

        for(uint16_t i=0;i<f->vertex_count-1;i++,fv += 3)
        {
            vec3_copy(v, fv);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;

            vec3_copy(v, fv + 3);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;
        }

        vec3_copy(v, fv);
        v += 3;
        vec3_copy(v, m_color);
        v += 3;
        vec3_copy(v, v0);
        v += 3;
        vec3_copy(v, m_color);
    }
}


void render_DebugDrawer::drawPortal(struct portal_s *p)
{
    if(p != NULL)
    {
        GLfloat *v, *v0;
        btScalar *pv = p->vertex;

        if(m_lines + p->vertex_count >= m_max_lines)
        {
            m_need_realloc = true;
            return;
        }

        v = v0 = m_buffer + 3 * 4 * m_lines;
        m_lines += p->vertex_count;

        for(uint16_t i=0;i<p->vertex_count-1;i++,pv += 3)
        {
            vec3_copy(v, pv);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;

            vec3_copy(v, pv + 3);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;
        }

        vec3_copy(v, pv);
        v += 3;
        vec3_copy(v, m_color);
        v += 3;
        vec3_copy(v, v0);
        v += 3;
        vec3_copy(v, m_color);
    }
}


void render_DebugDrawer::drawBBox(btScalar bb_min[3], btScalar bb_max[3], btScalar *transform)
{
    if(m_lines + 12 < m_max_lines)
    {
        OBB_Rebuild(m_obb, bb_min, bb_max);
        m_obb->transform = transform;
        OBB_Transform(m_obb);
        drawOBB(m_obb);
    }
    else
    {
        m_need_realloc = true;
    }
}


void render_DebugDrawer::drawOBB(struct obb_s *obb)
{
    GLfloat *v, *v0;
    polygon_p p = obb->polygons;

    if(m_lines + 12 >= m_max_lines)
    {
        m_need_realloc = true;
        return;
    }

    v = v0 = m_buffer + 3 * 4 * m_lines;
    m_lines += 12;

    vec3_copy(v, p->vertices[0].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;
    vec3_copy(v, (p+1)->vertices[0].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;

    vec3_copy(v, p->vertices[1].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;
    vec3_copy(v, (p+1)->vertices[3].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;

    vec3_copy(v, p->vertices[2].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;
    vec3_copy(v, (p+1)->vertices[2].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;

    vec3_copy(v, p->vertices[3].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;
    vec3_copy(v, (p+1)->vertices[1].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;

    for(uint16_t i=0; i<2; i++,p++)
    {
        vertex_p pv = p->vertices;
        v0 = v;
        for(uint16_t j=0;j<p->vertex_count-1;j++,pv++)
        {
            vec3_copy(v, pv->position);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;

            vec3_copy(v, (pv+1)->position);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;
        }

        vec3_copy(v, pv->position);
        v += 3;
        vec3_copy(v, m_color);
        v += 3;
        vec3_copy(v, v0);
        v += 3;
        vec3_copy(v, m_color);
        v += 3;
    }
}


void render_DebugDrawer::drawMeshDebugLines(struct base_mesh_s *mesh, btScalar transform[16], const btScalar *overrideVertices, const btScalar *overrideNormals)
{
    if((!m_need_realloc) && (renderer.style & R_DRAW_NORMALS))
    {
        GLfloat *v = m_buffer + 3 * 4 * m_lines;
        btScalar n[3];

        if(m_lines + mesh->vertex_count >= m_max_lines)
        {
            m_need_realloc = true;
            return;
        }

        setColor(0.8, 0.0, 0.9);
        m_lines += mesh->vertex_count;
        if(overrideVertices)
        {
            btScalar *ov = (btScalar*)overrideVertices;
            btScalar *on = (btScalar*)overrideNormals;
            for(uint32_t i=0; i<mesh->vertex_count; i++,ov+=3,on+=3,v+=12)
            {
                Mat4_vec3_mul_macro(v, transform, ov);
                Mat4_vec3_rot_macro(n, transform, on);

                v[6 + 0] = v[0] + n[0] * 128.0;
                v[6 + 1] = v[1] + n[1] * 128.0;
                v[6 + 2] = v[2] + n[2] * 128.0;
                vec3_copy(v+3, m_color);
                vec3_copy(v+9, m_color);
            }
        }
        else
        {
            vertex_p mv = mesh->vertices;
            for (uint32_t i = 0; i < mesh->vertex_count; i++,mv++,v+=12)
            {
                Mat4_vec3_mul_macro(v, transform, mv->position);
                Mat4_vec3_rot_macro(n, transform, mv->normal);

                v[6 + 0] = v[0] + n[0] * 128.0;
                v[6 + 1] = v[1] + n[1] * 128.0;
                v[6 + 2] = v[2] + n[2] * 128.0;
                vec3_copy(v+3, m_color);
                vec3_copy(v+9, m_color);
            }
        }
    }
}


void render_DebugDrawer::drawSkeletalModelDebugLines(struct ss_bone_frame_s *bframe, btScalar transform[16])
{
    if((!m_need_realloc) && renderer.style & R_DRAW_NORMALS)
    {
        btScalar tr[16];

        ss_bone_tag_p btag = bframe->bone_tags;
        for(uint16_t i=0; i<bframe->bone_tag_count; i++,btag++)
        {
            Mat4_Mat4_mul(tr, transform, btag->full_transform);
            drawMeshDebugLines(btag->mesh_base, tr, NULL, NULL);
        }
    }
}


void render_DebugDrawer::drawEntityDebugLines(struct entity_s *entity)
{
    if(m_need_realloc || entity->was_rendered_lines || !(renderer.style & (R_DRAW_AXIS | R_DRAW_NORMALS | R_DRAW_BOXES)) ||
       !(entity->state_flags & ENTITY_STATE_VISIBLE) || (entity->bf.animations.model->hide && !(renderer.style & R_DRAW_NULLMESHES)))
    {
        return;
    }

    if(renderer.style & R_DRAW_BOXES)
    {
        debugDrawer.setColor(0.0, 0.0, 1.0);
        debugDrawer.drawOBB(entity->obb);
    }

    if(renderer.style & R_DRAW_AXIS)
    {
        // If this happens, the lines after this will get drawn with random colors. I don't care.
        debugDrawer.drawAxis(1000.0, entity->transform);
    }

    if(entity->bf.animations.model && entity->bf.animations.model->animations)
    {
        debugDrawer.drawSkeletalModelDebugLines(&entity->bf, entity->transform);
    }

    entity->was_rendered_lines = 1;
}


void render_DebugDrawer::drawSectorDebugLines(struct room_sector_s *rs)
{
    if(m_lines + 12 < m_max_lines)
    {
        btScalar bb_min[3] = {(btScalar)(rs->pos[0] - TR_METERING_SECTORSIZE / 2.0), (btScalar)(rs->pos[1] - TR_METERING_SECTORSIZE / 2.0), (btScalar)rs->floor};
        btScalar bb_max[3] = {(btScalar)(rs->pos[0] + TR_METERING_SECTORSIZE / 2.0), (btScalar)(rs->pos[1] + TR_METERING_SECTORSIZE / 2.0), (btScalar)rs->ceiling};

        drawBBox(bb_min, bb_max, NULL);
    }
    else
    {
        m_need_realloc = true;
    }
}


void render_DebugDrawer::drawRoomDebugLines(struct room_s *room, struct render_s *render)
{
    uint32_t flag;
    frustum_p frus;
    engine_container_p cont;
    entity_p ent;

    if(m_need_realloc)
    {
        return;
    }

    flag = render->style & R_DRAW_ROOMBOXES;
    if(flag)
    {
        debugDrawer.setColor(0.0, 0.1, 0.9);
        debugDrawer.drawBBox(room->bb_min, room->bb_max, NULL);
        /*for(uint32_t s=0;s<room->sectors_count;s++)
        {
            drawSectorDebugLines(room->sectors + s);
        }*/
    }

    flag = render->style & R_DRAW_PORTALS;
    if(flag)
    {
        debugDrawer.setColor(0.0, 0.0, 0.0);
        for(uint16_t i=0; i<room->portal_count; i++)
        {
            debugDrawer.drawPortal(room->portals+i);
        }
    }

    flag = render->style & R_DRAW_FRUSTUMS;
    if(flag)
    {
        debugDrawer.setColor(1.0, 0.0, 0.0);
        for(frus=room->frustum; frus; frus=frus->next)
        {
            debugDrawer.drawFrustum(frus);
        }
    }

    if(!(renderer.style & R_SKIP_ROOM) && (room->mesh != NULL))
    {
        debugDrawer.drawMeshDebugLines(room->mesh, room->transform, NULL, NULL);
    }

    flag = render->style & R_DRAW_BOXES;
    for(uint32_t i=0; i<room->static_mesh_count; i++)
    {
        if(room->static_mesh[i].was_rendered_lines || !Frustum_IsOBBVisibleInRoom(room->static_mesh[i].obb, room) ||
          ((room->static_mesh[i].hide == 1) && !(renderer.style & R_DRAW_DUMMY_STATICS)))
        {
            continue;
        }

        if(flag)
        {
            debugDrawer.setColor(0.0, 1.0, 0.1);
            debugDrawer.drawOBB(room->static_mesh[i].obb);
        }

        if(render->style & R_DRAW_AXIS)
        {
            debugDrawer.drawAxis(1000.0, room->static_mesh[i].transform);
        }

        debugDrawer.drawMeshDebugLines(room->static_mesh[i].mesh, room->static_mesh[i].transform, NULL, NULL);

        room->static_mesh[i].was_rendered_lines = 1;
    }

    for(cont=room->containers; cont; cont=cont->next)
    {
        switch(cont->object_type)
        {
        case OBJECT_ENTITY:
            ent = (entity_p)cont->object;
            if(ent->was_rendered_lines == 0)
            {
                if(Frustum_IsOBBVisibleInRoom(ent->obb, room))
                {
                    debugDrawer.drawEntityDebugLines(ent);
                }
                ent->was_rendered_lines = 1;
            }
            break;
        };
    }
}


