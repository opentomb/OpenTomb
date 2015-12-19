#pragma once

#include "util/vmath.h"

#include <glm/gtx/string_cast.hpp>

#include <array>
#include <vector>

namespace world
{
class Camera;
struct Portal;

namespace core
{
struct OrientedBoundingBox;
struct Polygon;
struct BoundingBox;

class Frustum
{
private:
    enum
    {
        Right,
        Left,
        Top,
        Bottom,
        Near,
        Far,
        PlaneCount
    };
    static_assert(PlaneCount == 6, "Frustum plane constants wrong");

    std::array<util::Plane, PlaneCount> m_planes; //!< clip planes

public:
    void setFromMatrix(const glm::mat4& mv)
    {
        // Extract frustum planes from matrix

        const auto m = glm::transpose(mv);
        // assign in order: right, left, top, bottom, near, far
        for(int i=0; i<3; ++i)
        {
            m_planes[2*i+0].assign(m[3] - m[i]);
            m_planes[2*i+1].assign(m[3] + m[i]);
        }
    }

    bool isVisible(const Polygon &polygon, const Camera& cam) const;
    bool isVisible(const std::vector<glm::vec3>& vertices) const;
    bool isVisible(const BoundingBox& bb, const Camera& cam) const;
    bool isVisible(const OrientedBoundingBox &obb, const Camera& cam) const;
    bool isVisible(const Portal &portal) const;

    //! Check if a line intersects with the frustum
    bool intersects(const glm::vec3& a, const glm::vec3& b) const;
};

} // namespace core
} // namespace world
