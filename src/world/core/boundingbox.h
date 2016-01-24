#pragma once

#include <glm/glm.hpp>
#include <algorithm>

class btCollisionShape;

namespace world
{
namespace core
{
struct BoundingBox
{
    glm::vec3 min = { 0,0,0 };
    glm::vec3 max = { 0,0,0 };

    glm::vec3 getCenter() const noexcept
    {
        return (min + max) * 0.5f;
    }

    glm::vec3 getExtent() const noexcept
    {
        return max - min;
    }

    void adjust(const glm::vec3& v, glm::float_t r = 0) noexcept
    {
        for(int i = 0; i < 3; ++i)
        {
            if(min[i] > v[i] - r)
                min[i] = v[i] - r;
            if(max[i] < v[i] + r)
                max[i] = v[i] + r;
        }
    }

    bool contains(const glm::vec3& v) const
    {
        return v[0] >= min[0] && v[0] <= max[0] &&
            v[1] >= min[1] && v[1] <= max[1] &&
            v[2] >= min[2] && v[2] <= max[2];
    }

    bool overlaps(const BoundingBox& b) const
    {
        if(min[0] >= b.max[0] || max[0] <= b.min[0] ||
           min[1] >= b.max[1] || max[1] <= b.min[1] ||
           min[2] >= b.max[2] || max[2] <= b.min[2])
        {
            return false;
        }

        return true;
    }

    glm::float_t getMinimumExtent() const
    {
        glm::vec3 d = getExtent();
        return std::min(d[0], std::min(d[1], d[2]));
    }

    glm::float_t getMaximumExtent() const
    {
        glm::vec3 d = getExtent();
        return std::max(d[0], std::max(d[1], d[2]));
    }
};

btCollisionShape* BT_CSfromBBox(const BoundingBox &boundingBox, bool useCompression, bool buildBvh);
} // namespace core
} // namespace world
