/*
 * File:   obb.h
 * Author: nTesla
 *
 * Created on January 21, 2013, 7:11 PM
 */

#ifndef OBB_H
#define OBB_H

#include <stdint.h>

#include "polygon.h"
#include "bullet/LinearMath/btScalar.h"

/*
 * In base_edges we safe the initial shape polygons
 */

struct entity_s;

typedef struct obb_s
{
    struct polygon_s     base_polygons[6];               // bv base surface
    struct polygon_s     polygons[6];                       // bv world coordinate surface
    btScalar            *transform;                      // Object transform matrix
    btScalar             r;

    btScalar             base_centre[3];
    btScalar             centre[3];
    btScalar             extent[3];
} obb_t, *obb_p;

obb_p OBB_Create();
void OBB_Clear(obb_p bv);

void OBB_Rebuild(obb_p obb, btScalar bb_min[3], btScalar bb_max[3]);
void OBB_Transform(obb_p obb);
int OBB_OBB_Test(struct entity_s *e1, struct entity_s *e2);

#endif /* OBB_H */

