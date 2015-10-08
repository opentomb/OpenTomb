#pragma once

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

constexpr const float DegPerRad = static_cast<float>(180 / SIMD_PI);
constexpr const float RadPerDeg = static_cast<float>(SIMD_PI / 180);
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
        dot = n[3] / glm::length(glm::vec3(n));
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

inline void vec4_SetTRRotations(glm::quat& v, const btVector3& rotation)
{
    v = glm::quat(1, 0, 0, 0);
    v = glm::rotate(v, rotation[2] * RadPerDeg, { 0,0,1 });
    v = glm::rotate(v, rotation[0] * RadPerDeg, { 1,0,0 });
    v = glm::rotate(v, rotation[1] * RadPerDeg, { 0,1,0 });
}

inline glm::vec3 convert(const btVector3& v)
{
    return glm::vec3(v[0], v[1], v[2]);
}

inline btVector3 convert(const glm::vec3& v)
{
    return btVector3(v[0], v[1], v[2]);
}

inline glm::quat convert(const btQuaternion& v)
{
    return glm::quat(v.w(), v.x(), v.y(), v.z());
}

} // namespace util
