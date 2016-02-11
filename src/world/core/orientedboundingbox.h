#pragma once

/*
 * File:   obb.h
 * Author: nTesla
 *
 * Created on January 21, 2013, 7:11 PM
 */

#include "polygon.h"

#include <array>

namespace world
{
class Camera;
class Entity;
class Room;

namespace core
{
struct BoundingBox;

struct OrientedBoundingBox
{
    std::array<Polygon, 6> base_polygons;           // bv base surface
    std::array<Polygon, 6> polygons;                // bv world coordinate surface
    const glm::mat4* transform = nullptr;          // Object transform matrix
    glm::float_t radius;

    glm::vec3 base_centre;
    glm::vec3 center;
    glm::vec3 extent;

    void doTransform();
    void rebuild(const BoundingBox &boundingBox);
};

namespace
{
constexpr glm::float_t DefaultTestOverlap = 1.2f;
}

bool testOverlap(const Entity &e1, const Entity &e2, glm::float_t overlap = DefaultTestOverlap);
} // namespace core
} // namespace world
