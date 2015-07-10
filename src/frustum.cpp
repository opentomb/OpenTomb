#include "frustum.h"

#include <cstdio>
#include <cstdlib>

#include <bullet/LinearMath/btScalar.h>

#include "vmath.h"
#include "camera.h"
#include "polygon.h"
#include "portal.h"
#include "render.h"
#include "world.h"
#include "engine.h"
#include "obb.h"


void Frustum::splitPrepare(struct Portal *p)
{
    vertices = p->vertices;
    norm = -p->normal;
    norm[3] = -p->normal[3];
    parent.reset();
}

int Frustum::split_by_plane(const btVector3 &splitPlane)
{
    assert( !vertices.empty() );

    std::vector<btVector3> buf;

    btVector3* nextVertex = &vertices.front();
    btVector3* currentVertex = &vertices.back();

    btScalar dist[2];
    dist[0] = planeDist(splitPlane, *currentVertex);

    // run through all adjacent vertices
    for(size_t i=0; i<vertices.size(); i++)
    {
        dist[1] = planeDist(splitPlane, *nextVertex);

        if(dist[1] > SPLIT_EPSILON)
        {
            if(dist[0] < -SPLIT_EPSILON)
            {
                auto dir = *nextVertex - *currentVertex;
                btVector3 v;
                btScalar t;
                rayPlaneIntersect(*currentVertex, dir, splitPlane, &v, &t);  // Search for intersection point...
                buf.emplace_back(v);            // Shifting...
            }
            buf.emplace_back( *nextVertex );   // Adding...
        }
        else if(dist[1] < -SPLIT_EPSILON)
        {
            if(dist[0] > SPLIT_EPSILON)
            {
                auto dir = *nextVertex - *currentVertex;
                btVector3 v;
                btScalar t;
                rayPlaneIntersect(*currentVertex, dir, splitPlane, &v, &t);  // Search for intersection point...
                buf.emplace_back(v);
            }
        }
        else
        {
            buf.emplace_back(*nextVertex);   // Adding...
        }

        currentVertex = nextVertex;
        ++nextVertex;
        dist[0] = dist[1];
    }

    if(buf.size() <= 2)       // Nothing was added or degenerative.
    {
        vertices.clear();
        return SPLIT_EMPTY;
    }

#if 0
    p->vertex_count = added;
    memcpy(p->vertex, buf, added*3*sizeof(btScalar));
#else
    // filter repeating (too closest) points
    nextVertex = &buf.front();
    currentVertex = &buf.back();
    vertices.clear();
    for(size_t i=0; i<buf.size(); i++)
    {
        if(currentVertex->distance2(*nextVertex) > SPLIT_EPSILON * SPLIT_EPSILON)
        {
            vertices.emplace_back(*nextVertex);
        }
        currentVertex = nextVertex;
        ++nextVertex;
    }

    if(vertices.size() <= 2)
    {
        vertices.clear();
        return SPLIT_EMPTY;
    }
#endif

    return SPLIT_SUCCES;
}

void Frustum::genClipPlanes(Camera *cam)
{
    if(vertices.empty())
        return;

    planes.resize(vertices.size(), {0,0,0});

    auto next_v = &vertices.front();
    auto curr_v = &vertices.back();
    auto prev_v = curr_v - 1;

    //==========================================================================

    for(uint16_t i=0; i<vertices.size(); i++)
    {
        auto V1 = *prev_v - cam->m_pos;                    // POV-vertex vector
        auto V2 = *prev_v - *curr_v;                       // vector connecting neighbor vertices
        V1.normalize();
        V2.normalize();
        planes[i] = V1.cross(V2).normalized();
        planes[i][3] = -planes[i].dot(*curr_v);

        prev_v = curr_v;
        curr_v = next_v;
        ++next_v;
    }

    cam_pos = &(cam->m_pos);
}

/*
 * receiver - points to the base room frustum, which portal leads to - it's taken from the portal!
 * returns a pointer to newly generated frustum.
 */
std::shared_ptr<Frustum> Frustum::portalFrustumIntersect(Portal *portal, std::shared_ptr<Frustum> emitter, Render *render)
{
    assert( emitter );
    if(!portal->dest_room)
        return nullptr;

    if(planeDist(portal->normal, render->camera()->m_pos) < -SPLIT_EPSILON)    // non face or degenerate to the line portal
    {
        return nullptr;
    }

    if(!portal->dest_room->frustum.empty() && emitter->hasParent(portal->dest_room->frustum.front()))
    {
        return nullptr;                                                        // Abort infinite loop!
    }

    bool in_dist = false, in_face = false;
    for(const btVector3& v : portal->vertices)
    {
        if(!in_dist && planeDist(render->camera()->frustum->norm, v) < render->camera()->m_distFar)
            in_dist = true;
        if(!in_face && planeDist(emitter->norm, v) > 0.0)
            in_face = true;
        if(in_dist && in_face)
            break;
    }

    if(!in_dist || !in_face)
        return nullptr;

    /*
     * Search for the first free room's frustum
     */
    portal->dest_room->frustum.emplace_back(std::make_shared<Frustum>());
    auto current_gen = portal->dest_room->frustum.back();

    current_gen->splitPrepare(portal);                       // prepare for clipping

    if(current_gen->split_by_plane(emitter->norm))               // splitting by main frustum clip plane
    {
        for(size_t i=0; i<emitter->vertices.size(); i++)
        {
            const auto& n = emitter->planes[i];
            if(!current_gen->split_by_plane(n))
            {
                portal->dest_room->frustum.pop_back();
                return nullptr;
            }
        }

        current_gen->genClipPlanes(render->camera());                      // all is OK, let's generate clip planes

        current_gen->parent = emitter;                                      // add parent pointer
        current_gen->parents_count = emitter->parents_count + 1;
        if(portal->dest_room->max_path < current_gen->parents_count)
        {
            portal->dest_room->max_path = current_gen->parents_count;       // maximum path to the room
        }
        return current_gen;
    }

    portal->dest_room->frustum.pop_back();

    return nullptr;
}

/*
 ************************* END FRUSTUM MANAGER IMPLEMENTATION*******************
 */

/**
 * This function breaks looped recursive frustums.
 * If room has a frustum which is a parent to current frustum, function returns
 * true, and we break the loop.
 */

bool Frustum::hasParent(const std::shared_ptr<Frustum>& parent)
{
    auto frustum = this;
    while(frustum) {
        if(parent.get() == frustum)
            return true;
        frustum = frustum->parent.lock().get();
    }
    return false;
}


/**
 * Check polygon visibility through the portal.
 * This method is not for realtime since check is generally more expensive than rendering ...
 */
bool Frustum::isPolyVisible(struct Polygon *p)
{
    if(planeDist(p->plane, *cam_pos) < 0.0)
    {
        return false;
    }

    // Direction from the camera position to an arbitrary vertex frustum
    assert( !vertices.empty() );
    auto dir = vertices[0] - *cam_pos;
    btScalar t = 0;

    // Polygon fits whole frustum (shouldn't happen, but we check anyway)
    if(p->rayIntersect(dir, *cam_pos, &t))
    {
        return true;
    }

    // Generate queue order...
    btVector3* next_n = &planes.front();
    // 3 neighboring clipping planes
    btVector3* curr_n = &planes.back();
    btVector3* prev_n = curr_n - 1;
    // in case no intersection
    bool ins = true;
    // iterate through all the planes of this frustum
    for(size_t i=0; i<vertices.size(); i++)
    {
        // Queue vertices for testing
        Vertex* curr_v = &p->vertices.front();
        Vertex* prev_v = &p->vertices.back();
        // signed distance from the current point to the previous plane
        btScalar dist0 = planeDist(*curr_n, prev_v->position);
        bool outs = true;
        // iterate through all the vertices of the polygon
        for(size_t j=0; j<p->vertices.size(); j++)
        {
            btScalar dist1 = planeDist(*curr_n, curr_v->position);
            // the split point in the plane
            if(std::fabs(dist0) < SPLIT_EPSILON)
            {
                if((planeDist(*prev_n, prev_v->position) > -SPLIT_EPSILON) &&
                   (planeDist(*next_n, prev_v->position) > -SPLIT_EPSILON) &&
                   (planeDist(norm, prev_v->position) > -SPLIT_EPSILON))
                {
                    // Frustum-vertex intersection test is passed.
                    return true;
                }
            }

            // vertices from different sides of the plane (or on it)
            if((dist0 * dist1 < 0) && std::fabs(dist1) >= SPLIT_EPSILON)
            {
                // vector connecting vertices
                dir = curr_v->position - prev_v->position;
                btVector3 T;
                // We are looking for the point of intersection
                rayPlaneIntersect(prev_v->position, dir, *curr_n, &T, &t);
                if((planeDist(*prev_n, T) > -SPLIT_EPSILON) && (planeDist(*next_n, T) > -SPLIT_EPSILON))
                {
                    // Frustum-ray intersection test is passed.
                    return true;
                }
            }

            // point is outside
            if(dist1 < -SPLIT_EPSILON)
            {
                ins = false;
            }
            else
            {
                outs = false;
            }

            // We moved all the vertices of the polygon
            prev_v = curr_v;
            ++curr_v;
            // We moved all distances
            dist0 = dist1;
            // finished with all polygon vertices
        }

        if(outs)
        {
            // all points are outside of the current plane - definetly exit.
            return false;
        }
        // We moved all clipping planes
        prev_n = curr_n;
        curr_n = next_n;
        ++next_n;
        // finished with all planes of this frustum
    }
    if(ins)
    {
        // all the vertices are inside - test is passed.
        return true;
    }

    return false;
}

/**
 *
 * @param bbmin - aabb corner (x_min, y_min, z_min)
 * @param bbmax - aabb corner (x_max, y_max, z_max)
 * @param frustum - test frustum
 * @return 1 if aabb is in frustum.
 */
bool Frustum::isAABBVisible(const btVector3& bbmin, const btVector3& bbmax)
{
    Polygon poly;
    poly.vertices.resize(4);
    bool ins = true;

    /* X_AXIS */

    poly.plane[1] = 0.0;
    poly.plane[2] = 0.0;
    if((*cam_pos)[0] < bbmin[0])
    {
        poly.plane[0] = -1.0;
        poly.plane[3] = bbmin[0];
        poly.vertices[0].position[0] = bbmin[0];
        poly.vertices[0].position[1] = bbmax[1];
        poly.vertices[0].position[2] = bbmax[2];

        poly.vertices[1].position[0] = bbmin[0];
        poly.vertices[1].position[1] = bbmin[1];
        poly.vertices[1].position[2] = bbmax[2];

        poly.vertices[2].position[0] = bbmin[0];
        poly.vertices[2].position[1] = bbmin[1];
        poly.vertices[2].position[2] = bbmin[2];

        poly.vertices[3].position[0] = bbmin[0];
        poly.vertices[3].position[1] = bbmax[1];
        poly.vertices[3].position[2] = bbmin[2];

        if(isPolyVisible(&poly))
        {
            return true;
        }
        ins = false;
    }
    else if((*cam_pos)[0] > bbmax[0])
    {
        poly.plane[0] = 1.0;
        poly.plane[3] =-bbmax[0];
        poly.vertices[0].position[0] = bbmax[0];
        poly.vertices[0].position[1] = bbmax[1];
        poly.vertices[0].position[2] = bbmax[2];

        poly.vertices[1].position[0] = bbmax[0];
        poly.vertices[1].position[1] = bbmin[1];
        poly.vertices[1].position[2] = bbmax[2];

        poly.vertices[2].position[0] = bbmax[0];
        poly.vertices[2].position[1] = bbmin[1];
        poly.vertices[2].position[2] = bbmin[2];

        poly.vertices[3].position[0] = bbmax[0];
        poly.vertices[3].position[1] = bbmax[1];
        poly.vertices[3].position[2] = bbmin[2];

        if(isPolyVisible(&poly))
        {
            return true;
        }
        ins = false;
    }

    /* Y AXIS */

    poly.plane[0] = 0.0;
    poly.plane[2] = 0.0;
    if((*cam_pos)[1] < bbmin[1])
    {
        poly.plane[1] = -1.0;
        poly.plane[3] = bbmin[1];
        poly.vertices[0].position[0] = bbmax[0];
        poly.vertices[0].position[1] = bbmin[1];
        poly.vertices[0].position[2] = bbmax[2];

        poly.vertices[1].position[0] = bbmin[0];
        poly.vertices[1].position[1] = bbmin[1];
        poly.vertices[1].position[2] = bbmax[2];

        poly.vertices[2].position[0] = bbmin[0];
        poly.vertices[2].position[1] = bbmin[1];
        poly.vertices[2].position[2] = bbmin[2];

        poly.vertices[3].position[0] = bbmax[0];
        poly.vertices[3].position[1] = bbmin[1];
        poly.vertices[3].position[2] = bbmin[2];

        if(isPolyVisible(&poly))
        {
            return true;
        }
        ins = false;
    }
    else if((*cam_pos)[1] > bbmax[1])
    {
        poly.plane[1] = 1.0;
        poly.plane[3] = -bbmax[1];
        poly.vertices[0].position[0] = bbmax[0];
        poly.vertices[0].position[1] = bbmax[1];
        poly.vertices[0].position[2] = bbmax[2];

        poly.vertices[1].position[0] = bbmin[0];
        poly.vertices[1].position[1] = bbmax[1];
        poly.vertices[1].position[2] = bbmax[2];

        poly.vertices[2].position[0] = bbmin[0];
        poly.vertices[2].position[1] = bbmax[1];
        poly.vertices[2].position[2] = bbmin[2];

        poly.vertices[3].position[0] = bbmax[0];
        poly.vertices[3].position[1] = bbmax[1];
        poly.vertices[3].position[2] = bbmin[2];

        if(isPolyVisible(&poly))
        {
            return true;
        }
        ins = false;
    }

    /* Z AXIS */

    poly.plane[0] = 0.0;
    poly.plane[1] = 0.0;
    if((*cam_pos)[2] < bbmin[2])
    {
        poly.plane[2] = -1.0;
        poly.plane[3] = bbmin[2];
        poly.vertices[0].position[0] = bbmax[0];
        poly.vertices[0].position[1] = bbmax[1];
        poly.vertices[0].position[2] = bbmin[2];

        poly.vertices[1].position[0] = bbmin[0];
        poly.vertices[1].position[1] = bbmax[1];
        poly.vertices[1].position[2] = bbmin[2];

        poly.vertices[2].position[0] = bbmin[0];
        poly.vertices[2].position[1] = bbmin[1];
        poly.vertices[2].position[2] = bbmin[2];

        poly.vertices[3].position[0] = bbmax[0];
        poly.vertices[3].position[1] = bbmin[1];
        poly.vertices[3].position[2] = bbmin[2];

        if(isPolyVisible(&poly))
        {
            return true;
        }
        ins = false;
    }
    else if((*cam_pos)[2] > bbmax[2])
    {
        poly.plane[2] = 1.0;
        poly.plane[3] = -bbmax[2];
        poly.vertices[0].position[0] = bbmax[0];
        poly.vertices[0].position[1] = bbmax[1];
        poly.vertices[0].position[2] = bbmax[2];

        poly.vertices[1].position[0] = bbmin[0];
        poly.vertices[1].position[1] = bbmax[1];
        poly.vertices[1].position[2] = bbmax[2];

        poly.vertices[2].position[0] = bbmin[0];
        poly.vertices[2].position[1] = bbmin[1];
        poly.vertices[2].position[2] = bbmax[2];

        poly.vertices[3].position[0] = bbmax[0];
        poly.vertices[3].position[1] = bbmin[1];
        poly.vertices[3].position[2] = bbmax[2];

        if(isPolyVisible(&poly))
        {
            return true;
        }
        ins = false;
    }

    return ins;
}


bool Frustum::isOBBVisible(OBB *obb)
{
    bool ins = true;
    struct Polygon *p = obb->polygons;
    for(int i=0;i<6;i++,p++)
    {
        auto t = planeDist(p->plane, *cam_pos);
        if((t > 0.0) && isPolyVisible(p))
        {
            return true;
        }
        if(ins && (t > 0))
        {
            ins = false;
        }
    }

    return ins;
}

bool Frustum::isOBBVisibleInRoom(OBB *obb, const Room& room)
{
    if(!obb)
        return true;
    if(room.frustum.empty())                                                    // There's no active frustum in room, using camera frustum instead.
    {
        bool ins = true;                                                        // Let's assume camera is inside OBB.
        auto p = obb->polygons;
        for(int i=0;i<6;i++,p++)
        {
            auto t = planeDist(p->plane, engine_camera.m_pos);
            if((t > 0.0) && engine_camera.frustum->isPolyVisible(p))
            {
                return true;
            }
            if(ins && (t > 0.0))                                                // Testing if POV is inside OBB or not.
            {
                ins = false;                                                    // Even single failed test means that camera is outside OBB.
            }
        }
        return ins;                                                             // If camera is inside object's OBB, then object is visible.
    }

    for(const auto& frustum : room.frustum) {
        auto p = obb->polygons;
        for(int i=0;i<6;i++,p++) {
            auto t = planeDist(p->plane, *frustum->cam_pos);
            if((t > 0.0) && frustum->isPolyVisible(p))
            {
                return true;
            }
        }
    }

    return false;
}

