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
    if(!polygon.isDoubleSided && glm::dot(polygon.plane.normal, cam.getPosition()) < 0.0)
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

bool Frustum::isVisible(const BoundingBox& bb) const
{
    // see https://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/, method 5
    const glm::vec3 center = bb.getCenter();
    const glm::vec3 extent = bb.getDiameter();
    for(const util::Plane& plane : m_planes)
    {
        glm::vec3 signFlipped = center;
        for(int i=0; i<3; ++i)
            signFlipped[i] += glm::sign(plane.normal[i]) * extent[i];
        if(plane.distance(signFlipped) >= 0)
            return true;
    }
    return false;
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
