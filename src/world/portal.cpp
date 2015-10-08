#include "portal.h"

#include <cassert>
#include <cmath>

#include "util/vmath.h"
#include "world/core/polygon.h"

/*
 * CLIP PLANES
 */

namespace world
{

void Portal::move(const glm::vec3& mv)
{
    centre += mv;
    for(glm::vec3& v : vertices)
        v += mv;
    normal.moveTo(vertices[0]);
}

/**
 * Barycentric method for determining the intersection of a ray and a triangle
 * @param ray ray directionа
 * @param dot point of ray-plane intersection
 */
bool Portal::rayIntersect(const glm::vec3& ray, const glm::vec3& rayStart)
{
    if(std::abs(glm::dot(normal.normal, ray)) < core::SplitEpsilon)
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
    const glm::vec3 T = rayStart - vertices[0];

    glm::vec3 edge = vertices[1] - vertices[0];
    // Bypass polygon fan, one of the vectors remains unchanged
    for(size_t i = 2; i < vertices.size(); i++)
    {
        // PREV
        glm::vec3 prevEdge = edge;
        edge = vertices[i] - vertices[0];

        glm::vec3 P = glm::cross(ray, edge);
        glm::vec3 Q = glm::cross(T, prevEdge);

        glm::float_t t = glm::dot(P, prevEdge);
        glm::float_t u = glm::dot(P, T) / t;
        glm::float_t v = glm::dot(Q, ray) / t;
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
void Portal::updateNormal()
{
    assert(vertices.size() > 3);
    auto v1 = vertices[1] - vertices[0];
    auto v2 = vertices[2] - vertices[1];
    normal.assign(v1, v2, vertices[0]);
}

} // namespace world
