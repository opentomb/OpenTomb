#pragma once

#include <vector>

#include "util/vmath.h"

namespace world
{
class Camera;
struct Portal;

namespace core
{
struct OrientedBoundingBox;
struct Polygon;
struct BoundingBox;

struct Frustum
{
    std::vector<util::Plane> planes; //!< clip planes

    bool isPolyVisible(const Polygon &p, const Camera& cam) const;
    bool isPolyVisible(const std::vector<glm::vec3>& p) const;
    bool isAABBVisible(const BoundingBox& bb, const Camera& cam) const;
    bool isOBBVisible(const OrientedBoundingBox &obb, const Camera& cam) const;

    //! The main function for working with portals.
    bool portalFrustumIntersect(const Portal &portal) const;
};

} // namespace core
} // namespace world
