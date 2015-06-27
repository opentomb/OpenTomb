/*
 * File:   obb.h
 * Author: nTesla
 *
 * Created on January 21, 2013, 7:11 PM
 */

#ifndef OBB_H
#define OBB_H

#include <cstdint>

#include "polygon.h"
#include <bullet/LinearMath/btScalar.h>

#include <memory>

/*
 * In base_edges we safe the initial shape polygons
 */

struct Entity;

struct OBB
{
    Polygon base_polygons[6];               // bv base surface
    Polygon polygons[6];                       // bv world coordinate surface
    const btTransform* transform = nullptr;                      // Object transform matrix
    btScalar r;

    btVector3 base_centre;
    btVector3 centre;
    btVector3 extent;

    void doTransform();
    void rebuild(const btVector3 &bb_min, const btVector3 &bb_max);
};

int OBB_OBB_Test(std::shared_ptr<Entity> e1, std::shared_ptr<Entity> e2);

#endif /* OBB_H */

