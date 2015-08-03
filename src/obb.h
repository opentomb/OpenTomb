/*
 * File:   obb.h
 * Author: nTesla
 *
 * Created on January 21, 2013, 7:11 PM
 */

#ifndef OBB_H
#define OBB_H

#include <LinearMath/btScalar.h>

#include "polygon.h"

#define DEFAULT_OBB_TEST_OVERLAP (1.2)

 /*
  * In base_edges we safe the initial shape polygons
  */

struct Entity;

struct OBB
{
    struct Polygon base_polygons[6];               // bv base surface
    struct Polygon polygons[6];                    // bv world coordinate surface
    const btTransform* transform = nullptr;        // Object transform matrix
    btScalar r;

    btVector3 base_centre;
    btVector3 centre;
    btVector3 extent;

    void doTransform();
    void rebuild(const btVector3 &bb_min, const btVector3 &bb_max);
};

int OBB_OBB_Test(const Entity &e1, const Entity &e2, btScalar overlap = DEFAULT_OBB_TEST_OVERLAP);

#endif /* OBB_H */
