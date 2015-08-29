#pragma once

#include <LinearMath/btVector3.h>

namespace world
{
namespace core
{

struct BoundingBox
{
    btVector3 min;
    btVector3 max;

    btVector3 getCenter() const noexcept
    {
        return (min+max)/2;
    }

    btVector3 getDiameter() const noexcept
    {
        return max-min;
    }

    void adjust(const btVector3& v) noexcept
    {
        for(int i = 0; i < 3; ++i)
        {
            if(min[i] > v[i])
                min[i] = v[i];
            if(max[i] < v[i])
                max[i] = v[i];
        }
    }

    bool contains(const btVector3& v) const
    {
        return (v[0] >= min[0]) && (v[0] < max[0]) &&
               (v[1] >= min[1]) && (v[1] < max[1]) &&
               (v[2] >= min[2]) && (v[2] < max[2]);
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

    btScalar getInnerRadius() const
    {
        btVector3 d = getDiameter();
        return btMin(d[0], btMin(d[1], d[2]));
    }
};

} // namespace core
} // namespace world
