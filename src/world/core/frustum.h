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

    bool isVisible(const Polygon &p, const Camera& cam) const;
    bool isVisible(const std::vector<glm::vec3>& p) const;
    bool isVisible(const BoundingBox& bb, const Camera& cam) const;
    bool isVisible(const OrientedBoundingBox &obb, const Camera& cam) const;

    //! The main function for working with portals.
    bool isVisible(const Portal &portal) const;
};

} // namespace core
} // namespace world
