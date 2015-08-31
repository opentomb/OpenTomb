#pragma once

/*
 * File:   obb.h
 * Author: nTesla
 *
 * Created on January 21, 2013, 7:11 PM
 */

#include "polygon.h"

#include <LinearMath/btScalar.h>

namespace world
{
class Camera;
struct Entity;
struct Room;

namespace core
{

struct BoundingBox;

struct OrientedBoundingBox
{
    std::array<Polygon,6> base_polygons;           // bv base surface
    std::array<Polygon,6> polygons;                // bv world coordinate surface
    const btTransform* transform = nullptr;        // Object transform matrix
    btScalar radius;

    btVector3 base_centre;
    btVector3 center;
    btVector3 extent;

    void doTransform();
    void rebuild(const BoundingBox &boundingBox);
    bool isVisibleInRoom(const Room& room, const Camera& cam) const;
};

namespace
{
constexpr btScalar DefaultTestOverlap = 1.2f;
}

bool testOverlap(const Entity &e1, const Entity &e2, btScalar overlap = DefaultTestOverlap);

} // namespace core
} // namespace world
