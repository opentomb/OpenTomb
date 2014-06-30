/* 
 * File:   bounding_volume.h
 * Author: nTesla
 *
 * Created on January 21, 2013, 7:11 PM
 */

#ifndef BOUNDING_VOLUME_H
#define BOUNDING_VOLUME_H

#include <stdint.h>

struct polygon_s;
#include "bullet/LinearMath/btScalar.h"
#include "bullet/btBulletCollisionCommon.h"

/*
 * In base_edges we safe the initial shape polygons
 */

#define BV_EMPTY        0
#define BV_BOX          1
#define BV_SPHERE       2
#define BV_CYLINDER     3
#define BV_FREEFORM     4

typedef struct bounding_volume_s
{   
    uint16_t             bv_type;
    uint16_t             polygons_count;
    struct polygon_s    *base_polygons;                  // bv base surface
    struct polygon_s    *polygons;                       // bv world coordinate surface
    btScalar            *transform;                      // Object transform matrix
    btScalar             r;
    btScalar             base_centre[3];
    btScalar             centre[3];
} bounding_volume_t, *bounding_volume_p;

bounding_volume_p BV_Create();
void BV_Clear(bounding_volume_p bv);
void BV_InitCylinder(bounding_volume_p bv, btScalar cyl[3], int n);
void BV_InitBox(bounding_volume_p bv, btScalar bb_min[3], btScalar bb_max[3]);
void BV_RebuildBox(bounding_volume_p bv, btScalar bb_min[3], btScalar bb_max[3]);
void BV_Transform(bounding_volume_p bv);
void BV_TransformZZ(bounding_volume_p bv, btScalar z1, btScalar z2);

btCollisionShape *BV_CreateBTCapsuleZ(btScalar size[4], int n);

#endif /* BOUNDING_VOLUME_H */

