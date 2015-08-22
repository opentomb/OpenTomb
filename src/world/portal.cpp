#include "portal.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <LinearMath/btScalar.h>

#include "world/core/polygon.h"
#include "util/vmath.h"

/*
 * CLIP PLANES
 */

namespace world
{

void Portal::move(const btVector3& mv)
{
    centre += mv;
    for(btVector3& v : vertices)
        v += mv;
    normal.moveTo(vertices[0]);
}

/**
 * Barycentric method for determining the intersection of a ray and a triangle
 * @param ray ray directionа
 * @param dot point of ray-plane intersection
 */
bool Portal::rayIntersect(const btVector3& ray, const btVector3& rayStart)
{
    if(std::abs(normal.normal.dot(ray)) < core::SplitEpsilon)
    {
        // the plane is nearly parallel to the ray
        return false;
    }
    if(-normal.distance(rayStart) <= 0)
    {
        // plane is on the wrong side of the ray
        return false;
    }

    // The vector that does not change for the entire polygon
    const btVector3 T = rayStart - vertices[0];

    btVector3 edge = vertices[1] - vertices[0];
    // Bypass polygon fan, one of the vectors remains unchanged
    for(size_t i = 2; i < vertices.size(); i++)
    {
        // PREV
        btVector3 prevEdge = edge;
        edge = vertices[i] - vertices[0];

        btVector3 P = ray.cross(edge);
        btVector3 Q = T.cross(prevEdge);

        btScalar t = P.dot(prevEdge);
        btScalar u = P.dot(T) / t;
        btScalar v = Q.dot(ray) / t;
        t = 1.0f - u - v;
        if((u <= 1.0) && (u >= 0.0) && (v <= 1.0) && (v >= 0.0) && (t <= 1.0) && (t >= 0.0))
        {
            return true;
        }
    }

    return false;
}

/**
 * Просто генерация нормали к порталу
 */
void Portal::genNormale()
{
    assert(vertices.size() > 3);
    auto v1 = vertices[1] - vertices[0];
    auto v2 = vertices[2] - vertices[1];
    normal.assign(v1, v2, vertices[0]);
}

} // namespace world
