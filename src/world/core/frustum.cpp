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

bool Frustum::intersects(const glm::vec3& a, const glm::vec3& b) const
{
    uint32_t aOutside = 0, bOutside = 0;
    for(size_t i=0; i<m_planes.size(); ++i)
    {
        const auto aDist = m_planes[i].distance(a);
        if(aDist < 0)
            aOutside |= 1<<i;

        const auto bDist = m_planes[i].distance(b);
        if(bDist < 0)
            bOutside |= 1<<i;
    }

    if(aOutside==0 || bOutside==0)
        return true; // a or b or both are inside the frustum

    // if both are outside different planes, chances are high that they cross the frustum;
    // chances are low for false positives unless there are very very long edges compared to the frustum
    return aOutside != bOutside;
}

bool Frustum::isVisible(const Portal& portal) const
{
    if(!portal.destination)
        return false;

    if(intersects(portal.vertices[0], portal.vertices[1])) return true;
    if(intersects(portal.vertices[1], portal.vertices[2])) return true;
    if(intersects(portal.vertices[2], portal.vertices[3])) return true;
    if(intersects(portal.vertices[3], portal.vertices[0])) return true;
    if(intersects(portal.vertices[0], portal.vertices[2])) return true;
    if(intersects(portal.vertices[1], portal.vertices[3])) return true;

    return false;
}

bool Frustum::isVisible(const Polygon& polygon, const Camera& cam) const
{
    if(!polygon.double_side && glm::dot(polygon.plane.normal, cam.getPosition()) < 0.0)
    {
        return false;
    }

    // iterate through all the planes of this frustum
    for(const util::Plane& plane : m_planes)
    {
        for(const Vertex& vec : polygon.vertices)
        {
            if(plane.distance(vec.position) < 0)
                return false;
        }
    }

    return true;
}

bool Frustum::isVisible(const std::vector<glm::vec3>& vertices) const
{
    // iterate through all the planes of this frustum
    for(const util::Plane& plane : m_planes)
    {
        for(const glm::vec3& vec : vertices)
        {
            if(plane.distance(vec) > 0)
                return true;
        }
    }

    return false;
}

bool Frustum::isVisible(const BoundingBox& bb, const Camera& cam) const
{
#if 0
    Polygon poly;
    poly.vertices.resize(4);
    bool inside = true;

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

        if(isVisible(poly, cam))
        {
            return true;
        }
        inside = false;
    }
    else if(cam.getPosition()[0] > bb.max[0])
    {
        poly.plane.normal[0] = 1.0;
        poly.plane.dot = bb.max[0];
        poly.vertices[0].position = { bb.max[0], bb.max[1], bb.max[2] };
        poly.vertices[1].position = { bb.max[0], bb.min[1], bb.max[2] };
        poly.vertices[2].position = { bb.max[0], bb.min[1], bb.min[2] };
        poly.vertices[3].position = { bb.max[0], bb.max[1], bb.min[2] };

        if(isVisible(poly, cam))
        {
            return true;
        }
        inside = false;
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

        if(isVisible(poly, cam))
        {
            return true;
        }
        inside = false;
    }
    else if(cam.getPosition()[1] > bb.max[1])
    {
        poly.plane.normal[1] = 1.0;
        poly.plane.dot = bb.max[1];
        poly.vertices[0].position = { bb.max[0], bb.max[1], bb.max[2] };
        poly.vertices[1].position = { bb.min[0], bb.max[1], bb.max[2] };
        poly.vertices[2].position = { bb.min[0], bb.max[1], bb.min[2] };
        poly.vertices[3].position = { bb.max[0], bb.max[1], bb.min[2] };

        if(isVisible(poly, cam))
        {
            return true;
        }
        inside = false;
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

        if(isVisible(poly, cam))
        {
            return true;
        }
        inside = false;
    }
    else if(cam.getPosition()[2] > bb.max[2])
    {
        poly.plane.normal[2] = 1.0;
        poly.plane.dot = bb.max[2];
        poly.vertices[0].position = { bb.max[0], bb.max[1], bb.max[2] };
        poly.vertices[1].position = { bb.min[0], bb.max[1], bb.max[2] };
        poly.vertices[2].position = { bb.min[0], bb.min[1], bb.max[2] };
        poly.vertices[3].position = { bb.max[0], bb.min[1], bb.max[2] };

        if(isVisible(poly, cam))
        {
            return true;
        }
        inside = false;
    }

    return inside;
#else
    // see https://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/, method 5
    const glm::vec3 center = bb.getCenter() - cam.getPosition();
    const glm::vec3 extent = bb.getDiameter();
    for(const util::Plane& plane : m_planes)
    {
        glm::vec3 signFlipped;
        for(int i=0; i<3; ++i)
            signFlipped[i] = glm::sign(plane.normal[i]) * extent[i];
        if(glm::dot(center + signFlipped, plane.normal) >= plane.dot)
            return true;
    }
    return false;
#endif
}

bool Frustum::isVisible(const OrientedBoundingBox& obb, const Camera& cam) const
{
    bool inside = true;
    for(const Polygon& p : obb.polygons)
    {
        glm::float_t t = p.plane.distance(cam.getPosition());
        if(t <= 0)
            continue;

        if(isVisible(p, cam))
            return true;

        inside = false;
    }

    return inside;
}

} // namespace core
} // namespace world
