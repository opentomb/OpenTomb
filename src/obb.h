#pragma once

/*
 * File:   obb.h
 * Author: nTesla
 *
 * Created on January 21, 2013, 7:11 PM
 */

#include <LinearMath/btScalar.h>

#include "polygon.h"

#define DEFAULT_OBB_TEST_OVERLAP (1.2)

 /*
  * In base_edges we safe the initial shape polygons
  */

class Camera;
struct Entity;
struct Room;

struct OBB
{
    struct Polygon base_polygons[6];               // bv base surface
    struct Polygon polygons[6];                    // bv world coordinate surface
    const btTransform* transform = nullptr;        // Object transform matrix
    btScalar radius;

    btVector3 base_centre;
    btVector3 centre;
    btVector3 extent;

    void doTransform();
    void rebuild(const btVector3 &bb_min, const btVector3 &bb_max);
    bool isVisibleInRoom(const Room& room, const Camera& cam);
};

int OBB_OBB_Test(const Entity &e1, const Entity &e2, btScalar overlap = DEFAULT_OBB_TEST_OVERLAP);
