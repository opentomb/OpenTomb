#pragma once

/*
 * File:   obb.h
 * Author: nTesla
 *
 * Created on January 21, 2013, 7:11 PM
 */

#include <LinearMath/btScalar.h>

#include "polygon.h"

namespace world
{
class Camera;
struct Entity;
struct Room;

namespace core
{

struct OrientedBoundingBox
{
    Polygon base_polygons[6];               // bv base surface
    Polygon polygons[6];                    // bv world coordinate surface
    const btTransform* transform = nullptr;        // Object transform matrix
    btScalar radius;

    btVector3 base_centre;
    btVector3 centre;
    btVector3 extent;

    void doTransform();
    void rebuild(const btVector3 &bb_min, const btVector3 &bb_max);
    bool isVisibleInRoom(const Room& room, const Camera& cam) const;
};

namespace
{
constexpr btScalar DefaultTestOverlap = 1.2f;
}

bool testOverlap(const Entity &e1, const Entity &e2, btScalar overlap = DefaultTestOverlap);

} // namespace core
} // namespace world
