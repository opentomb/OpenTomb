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
    norm = p->normal;
    norm.mirrorNormal();
    parent.reset();
}

int Frustum::split_by_plane(const Plane& splitPlane)
{
    assert( !vertices.empty() );

    std::vector<btVector3> buf;

    btVector3* nextVertex = &vertices.front();
    btVector3* currentVertex = &vertices.back();

    btScalar dist[2];
    dist[0] = splitPlane.distance(*currentVertex);

    // run through all adjacent vertices
    for(size_t i=0; i<vertices.size(); i++)
    {
        dist[1] = splitPlane.distance(*nextVertex);

        if(dist[1] > SPLIT_EPSILON)
        {
            if(dist[0] < -SPLIT_EPSILON)
            {
                buf.emplace_back( splitPlane.rayIntersect(*currentVertex, *nextVertex - *currentVertex) );            // Shifting...
            }
            buf.emplace_back( *nextVertex );   // Adding...
        }
        else if(dist[1] < -SPLIT_EPSILON)
        {
            if(dist[0] > SPLIT_EPSILON)
            {
                buf.emplace_back( splitPlane.rayIntersect(*currentVertex, *nextVertex - *currentVertex) );
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

    planes.resize(vertices.size());

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
        planes[i].assign(V1, V2, *curr_v);

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

    if(portal->normal.distance(render->camera()->m_pos) < -SPLIT_EPSILON)    // non face or degenerate to the line portal
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
        if(!in_dist && render->camera()->frustum->norm.distance(v) < render->camera()->m_distFar)
            in_dist = true;
        if(!in_face && emitter->norm.distance(v) > 0.0)
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
bool Frustum::isPolyVisible(const Polygon *p)
{
    if(p->plane.distance(*cam_pos) < 0.0)
    {
        return false;
    }

    // Direction from the camera position to an arbitrary vertex frustum
    assert( !vertices.empty() );
    auto dir = vertices[0] - *cam_pos;
    btScalar lambda = 0;

    // Polygon fits whole frustum (shouldn't happen, but we check anyway)
    if(p->rayIntersect(dir, *cam_pos, &lambda))
    {
        return true;
    }

    // Generate queue order...
    const Plane* nextPlane = &planes.front();
    // 3 neighboring clipping planes
    const Plane* currentPlane = &planes.back();
    const Plane* prevPlane = currentPlane - 1;
    // in case no intersection
    bool ins = true;
    // iterate through all the planes of this frustum
    for(size_t i=0; i<vertices.size(); i++)
    {
        // Queue vertices for testing
        const Vertex* currentVertex = &p->vertices.front();
        const Vertex* prevVertex = &p->vertices.back();
        // signed distance from the current point to the previous plane
        btScalar dist0 = currentPlane->distance(prevVertex->position);
        bool outs = true;
        // iterate through all the vertices of the polygon
        for(size_t j=0; j<p->vertices.size(); j++)
        {
            btScalar dist1 = currentPlane->distance(currentVertex->position);
            // the split point in the plane
            if(std::fabs(dist0) < SPLIT_EPSILON)
            {
                if((prevPlane->distance(prevVertex->position) > -SPLIT_EPSILON) &&
                   (nextPlane->distance(prevVertex->position) > -SPLIT_EPSILON) &&
                   (norm.distance(prevVertex->position) > -SPLIT_EPSILON))
                {
                    // Frustum-vertex intersection test is passed.
                    return true;
                }
            }

            // vertices from different sides of the plane (or on it)
            if((dist0 * dist1 < 0) && std::fabs(dist1) >= SPLIT_EPSILON)
            {
                // vector connecting vertices
                dir = currentVertex->position - prevVertex->position;
                // We are looking for the point of intersection
                const btVector3 T = currentPlane->rayIntersect(prevVertex->position, dir);
                if((prevPlane->distance(T) > -SPLIT_EPSILON) && (nextPlane->distance(T) > -SPLIT_EPSILON))
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
            prevVertex = currentVertex;
            ++currentVertex;
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
        prevPlane = currentPlane;
        currentPlane = nextPlane;
        ++nextPlane;
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
 * @return true if aabb is in frustum.
 */
bool Frustum::isAABBVisible(const btVector3& bbmin, const btVector3& bbmax)
{
    Polygon poly;
    poly.vertices.resize(4);
    bool ins = true;

    /* X_AXIS */

    poly.plane.normal[1] = 0.0;
    poly.plane.normal[2] = 0.0;
    if((*cam_pos)[0] < bbmin[0])
    {
        poly.plane.normal[0] = -1.0;
        poly.plane.dot = -bbmin[0];
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
        poly.plane.normal[0] = 1.0;
        poly.plane.dot = bbmax[0];
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

    poly.plane.normal[0] = 0.0;
    poly.plane.normal[2] = 0.0;
    if((*cam_pos)[1] < bbmin[1])
    {
        poly.plane.normal[1] = -1.0;
        poly.plane.dot = -bbmin[1];
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
        poly.plane.normal[1] = 1.0;
        poly.plane.dot = bbmax[1];
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

    poly.plane.normal[0] = 0.0;
    poly.plane.normal[1] = 0.0;
    if((*cam_pos)[2] < bbmin[2])
    {
        poly.plane.normal[2] = -1.0;
        poly.plane.dot = -bbmin[2];
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
        poly.plane.normal[2] = 1.0;
        poly.plane.dot = bbmax[2];
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
        auto t = p->plane.distance(*cam_pos);
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
            auto t = p->plane.distance(engine_camera.m_pos);
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
            auto t = p->plane.distance(*frustum->cam_pos);
            if((t > 0.0) && frustum->isPolyVisible(p))
            {
                return true;
            }
        }
    }

    return false;
}

