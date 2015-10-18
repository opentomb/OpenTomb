#pragma once

#include "loader/datatypes.h"
#include "util/helpers.h"

#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>
#include <LinearMath/btVector3.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#define PLANE_X        1
#define PLANE_Y        2
#define PLANE_Z        3

namespace util
{
constexpr const float Rad90 = static_cast<float>(0.5*SIMD_PI);
constexpr const float Rad180 = static_cast<float>(SIMD_PI);
constexpr const float Rad360 = static_cast<float>(2 * SIMD_PI);


// A simple Hesse normal form plane

struct Plane
{

    glm::vec3 normal = { 0,0,0 };   // The plane's normal
    glm::float_t dot = 0;   // The plane's distance to the origin

    // Calculates the normalized distance of an arbitrary point in terms of the normal
    // @param pos The point
    // @return The distance in multiples of the normal (if >0, @a pos is in the direction of the normal)

    glm::float_t distance(const glm::vec3& position) const
    {
        return glm::dot( normal, position ) - dot;
    }

    glm::vec3 rayIntersect(const glm::vec3& rayStart, const glm::vec3& rayDir, glm::float_t& lambda) const
    {
        lambda = dot - glm::dot( normal, rayStart );
        lambda /= glm::dot( normal, rayDir );
        return rayStart + lambda * rayDir;
    }

    glm::vec3 rayIntersect(const glm::vec3& rayStart, const glm::vec3& rayDir) const
    {
        glm::float_t t;
        return rayIntersect(rayStart, rayDir, t);
    }

    void assign(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& position)
    {
        normal = glm::normalize( glm::cross(v1, v2) );
        dot = glm::dot(normal, position);
    }

    void assign(const glm::vec3& n, const glm::vec3& position)
    {
        normal = glm::normalize(n);
        dot = glm::dot(normal, position);
    }

    void assign(const glm::vec3& n, const glm::float_t& d)
    {
        dot = d / glm::length(n);
        normal = glm::normalize(n);
    }

    void assign(const glm::vec4& n)
    {
        dot = -n[3] / glm::length(glm::vec3(n));
        normal = glm::normalize(glm::vec3(n));
    }

    void mirrorNormal()
    {
        normal = -normal;
        dot = -dot;
    }

    void moveTo(const glm::vec3& where)
    {
        ///@TODO: Project the (where--0) onto the normal before calculating the dot
        dot = glm::dot(normal, where);
    }
};

inline glm::quat vec4_SetTRRotations(const glm::vec3& rotation)
{
    glm::quat v = glm::quat(1, 0, 0, 0);
    v = glm::rotate(v, glm::radians(rotation[2]), { 0,0,1 });
    v = glm::rotate(v, glm::radians(rotation[0]), { 1,0,0 });
    v = glm::rotate(v, glm::radians(rotation[1]), { 0,1,0 });
    return v;
}

inline glm::vec3 convert(const btVector3& v)
{
    return glm::vec3(v[0], v[1], v[2]);
}

inline btVector3 convert(const glm::vec3& v)
{
    return btVector3(v[0], v[1], v[2]);
}

inline glm::vec3 convert(const loader::Vertex& tr_v)
{
    glm::vec3 v;
    v[0] = tr_v.x;
    v[1] = -tr_v.z;
    v[2] = tr_v.y;
    return v;
}

inline glm::vec4 convert(const loader::FloatColor& tr_c)
{
    glm::vec4 v;
    v[0] = tr_c.r * 2;
    v[1] = tr_c.g * 2;
    v[2] = tr_c.b * 2;
    v[3] = tr_c.a * 2;
    return v;
}

// glm has a bug in its implementation, comparing x<epsilon instead of abs(x)<epsilon
inline bool intersectRayTriangle( const glm::vec3& rayStart, const glm::vec3& rayDir, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2 )
{
    const glm::vec3 e1 = v1 - v0;
    const glm::vec3 e2 = v2 - v0;

    const glm::vec3 p = glm::cross(rayDir, e2);

    const auto a = glm::dot(e1, p);

    if(glm::abs(a) <= std::numeric_limits<decltype(a)>::epsilon())
        return false; // ray is nearly parallel to the triangle

    const glm::vec3 s = rayStart - v0;
    const auto x = glm::dot(s, p) / a;
    if( fuzzyZero(x) )
        return false;

    const glm::vec3 q = glm::cross(s, e1);
    const auto y = glm::dot(rayDir, q) / a;
    if(y < 0)
        return false;
    if(y + x > 1)
        return false;

    const auto z = glm::dot(e2, q) / a;

    return z >= 0;
}

} // namespace util
