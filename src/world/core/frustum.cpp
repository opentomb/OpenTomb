#include "frustum.h"

#include "engine/engine.h"
#include "orientedboundingbox.h"
#include "polygon.h"
#include "util/vmath.h"
#include "world/camera.h"
#include "world/portal.h"

namespace world
{
namespace core
{

/*
 * receiver - points to the base room frustum, which portal leads to - it's taken from the portal!
 * returns a pointer to newly generated frustum.
 */
bool Frustum::portalFrustumIntersect(const Portal& portal) const
{
    if(!portal.dest_room)
        return false;

    return isPolyVisible(portal.vertices);
}

/**
 * Check polygon visibility through the portal.
 * This method is not for realtime since check is generally more expensive than rendering ...
 */
bool Frustum::isPolyVisible(const Polygon& p, const Camera& cam) const
{
    if(!p.double_side && p.plane.distance(cam.getPosition()) < 0.0)
    {
        return false;
    }

    // iterate through all the planes of this frustum
    for(const util::Plane& plane : planes)
    {
        for(const Vertex& vec : p.vertices)
        {
            if(plane.distance(vec.position) > 0)
                return true;
        }
    }

    return false;
}

bool Frustum::isPolyVisible(const std::vector<glm::vec3>& p) const
{
    // iterate through all the planes of this frustum
    for(const util::Plane& plane : planes)
    {
        for(const glm::vec3& vec : p)
        {
            if(plane.distance(vec) > 0)
                return true;
        }
    }

    return false;
}

/**
 *
 * @param bbmin - aabb corner (x_min, y_min, z_min)
 * @param bbmax - aabb corner (x_max, y_max, z_max)
 * @return true if aabb is in frustum.
 */
bool Frustum::isAABBVisible(const BoundingBox& bb, const Camera& cam) const
{
    Polygon poly;
    poly.vertices.resize(4);
    bool ins = true;

    /* X_AXIS */

    poly.plane.normal[1] = 0.0;
    poly.plane.normal[2] = 0.0;
    if(cam.getPosition()[0] < bb.min[0])
    {
        poly.plane.normal[0] = -1.0;
        poly.plane.dot = -bb.min[0];
        poly.vertices[0].position = { bb.min[0], bb.max[1], bb.max[2] };
        poly.vertices[1].position = { bb.min[0], bb.min[1], bb.max[2] };
        poly.vertices[2].position = { bb.min[0], bb.min[1], bb.min[2] };
        poly.vertices[3].position = { bb.min[0], bb.max[1], bb.min[2] };

        if(isPolyVisible(poly, cam))
        {
            return true;
        }
        ins = false;
    }
    else if(cam.getPosition()[0] > bb.max[0])
    {
        poly.plane.normal[0] = 1.0;
        poly.plane.dot = bb.max[0];
        poly.vertices[0].position = { bb.max[0], bb.max[1], bb.max[2] };
        poly.vertices[1].position = { bb.max[0], bb.min[1], bb.max[2] };
        poly.vertices[2].position = { bb.max[0], bb.min[1], bb.min[2] };
        poly.vertices[3].position = { bb.max[0], bb.max[1], bb.min[2] };

        if(isPolyVisible(poly, cam))
        {
            return true;
        }
        ins = false;
    }

    /* Y AXIS */

    poly.plane.normal[0] = 0.0;
    poly.plane.normal[2] = 0.0;
    if(cam.getPosition()[1] < bb.min[1])
    {
        poly.plane.normal[1] = -1.0;
        poly.plane.dot = -bb.min[1];
        poly.vertices[0].position = { bb.max[0], bb.min[1], bb.max[2] };
        poly.vertices[1].position = { bb.min[0], bb.min[1], bb.max[2] };
        poly.vertices[2].position = { bb.min[0], bb.min[1], bb.min[2] };
        poly.vertices[3].position = { bb.max[0], bb.min[1], bb.min[2] };

        if(isPolyVisible(poly, cam))
        {
            return true;
        }
        ins = false;
    }
    else if(cam.getPosition()[1] > bb.max[1])
    {
        poly.plane.normal[1] = 1.0;
        poly.plane.dot = bb.max[1];
        poly.vertices[0].position = { bb.max[0], bb.max[1], bb.max[2] };
        poly.vertices[1].position = { bb.min[0], bb.max[1], bb.max[2] };
        poly.vertices[2].position = { bb.min[0], bb.max[1], bb.min[2] };
        poly.vertices[3].position = { bb.max[0], bb.max[1], bb.min[2] };

        if(isPolyVisible(poly, cam))
        {
            return true;
        }
        ins = false;
    }

    /* Z AXIS */

    poly.plane.normal[0] = 0.0;
    poly.plane.normal[1] = 0.0;
    if(cam.getPosition()[2] < bb.min[2])
    {
        poly.plane.normal[2] = -1.0;
        poly.plane.dot = -bb.min[2];
        poly.vertices[0].position = { bb.max[0], bb.max[1], bb.min[2] };
        poly.vertices[1].position = { bb.min[0], bb.max[1], bb.min[2] };
        poly.vertices[2].position = { bb.min[0], bb.min[1], bb.min[2] };
        poly.vertices[3].position = { bb.max[0], bb.min[1], bb.min[2] };

        if(isPolyVisible(poly, cam))
        {
            return true;
        }
        ins = false;
    }
    else if(cam.getPosition()[2] > bb.max[2])
    {
        poly.plane.normal[2] = 1.0;
        poly.plane.dot = bb.max[2];
        poly.vertices[0].position = { bb.max[0], bb.max[1], bb.max[2] };
        poly.vertices[1].position = { bb.min[0], bb.max[1], bb.max[2] };
        poly.vertices[2].position = { bb.min[0], bb.min[1], bb.max[2] };
        poly.vertices[3].position = { bb.max[0], bb.min[1], bb.max[2] };

        if(isPolyVisible(poly, cam))
        {
            return true;
        }
        ins = false;
    }

    return ins;
}

bool Frustum::isOBBVisible(const OrientedBoundingBox& obb, const Camera& cam) const
{
    bool ins = true;
    for(const Polygon& p : obb.polygons)
    {
        glm::float_t t = p.plane.distance(cam.getPosition());
        if(t <= 0)
            continue;

        if(isPolyVisible(p, cam))
            return true;

        ins = false;
    }

    return ins;
}

} // namespace core
} // namespace world
