
#include <math.h>
#include <stdlib.h>

#include "vmath.h"
#include "bounding_volume.h"
#include "polygon.h"

#include "bullet/LinearMath/btScalar.h"
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"

bounding_volume_p BV_Create()
{
    bounding_volume_p ret;

    ret = (bounding_volume_p)malloc(sizeof(bounding_volume_t));
    ret->bv_type = BV_EMPTY;
    ret->base_polygons = NULL;
    ret->polygons = NULL;
    ret->polygons_count = 0;
    ret->transform = NULL;

    return ret;
}


void BV_Clear(bounding_volume_p bv)
{
    int i;

    if(bv)
    {
        if(bv->polygons_count > 0)
        {
            for(i=0;i<bv->polygons_count;i++)
            {
                Polygon_Clear(bv->polygons + i);
                Polygon_Clear(bv->base_polygons + i);
            }

            bv->polygons_count = 0;
            free(bv->polygons);
            free(bv->base_polygons);
        }
        bv->bv_type = BV_EMPTY;
    }
}

/**
 * cyl - rx, ry, h1, h2
 * n - sides count
 * base cylinder is Z oriented
 */
void BV_InitCylinder(bounding_volume_p bv, btScalar  cyl[4], int n)
{
    int i;
    btScalar fi, dfi;

    bv->polygons_count = n;
    bv->base_polygons = (polygon_p)calloc(n, sizeof(polygon_t));
    bv->polygons = (polygon_p)calloc(n, sizeof(polygon_t));
    bv->r = (cyl[0] > cyl[1])?(cyl[0]):(cyl[1]);
    fi = 0.0;
    dfi = 2.0 * M_PI / (btScalar)n;

    for(i=0;i<n;i++)
    {
        Polygon_Resize(bv->base_polygons+i, 4);
        Polygon_Resize(bv->polygons+i, 4);

        bv->base_polygons[i].vertices[0].position[0] = cyl[0] * cos(fi);
        bv->base_polygons[i].vertices[0].position[1] = cyl[1] * sin(fi);
        bv->base_polygons[i].vertices[0].position[2] = cyl[3];
        bv->base_polygons[i].vertices[1].position[0] = bv->base_polygons[i].vertices[0].position[0];
        bv->base_polygons[i].vertices[1].position[1] = bv->base_polygons[i].vertices[0].position[1];
        bv->base_polygons[i].vertices[1].position[2] = cyl[2];

        bv->base_polygons[i].vertices[2].position[0] = cyl[0] * cos(fi + dfi);
        bv->base_polygons[i].vertices[2].position[1] = cyl[1] * sin(fi + dfi);
        bv->base_polygons[i].vertices[2].position[2] = cyl[2];
        bv->base_polygons[i].vertices[3].position[0] = bv->base_polygons[i].vertices[2].position[0];
        bv->base_polygons[i].vertices[3].position[1] = bv->base_polygons[i].vertices[2].position[1];
        bv->base_polygons[i].vertices[3].position[2] = cyl[3];

        Polygon_FindNormale(bv->base_polygons + i);

        fi += dfi;
    }

    bv->bv_type = BV_CYLINDER;
}


void BV_InitBox(bounding_volume_p bv, btScalar bb_min[3], btScalar bb_max[3])
{
    int i;

    bv->polygons_count = 6;
    bv->base_polygons = (polygon_p)calloc(6, sizeof(polygon_t));
    bv->polygons = (polygon_p)calloc(6, sizeof(polygon_t));
    bv->bv_type = BV_BOX;

    for(i=0;i<6;i++)
    {
        Polygon_Resize(bv->base_polygons+i, 4);
        Polygon_Resize(bv->polygons+i, 4);
    }

    if(bb_min != NULL && bb_max != NULL)
    {
        BV_RebuildBox(bv, bb_min, bb_max);
    }
}


void BV_RebuildBox(bounding_volume_p bv, btScalar bb_min[3], btScalar bb_max[3])
{
    btScalar sx, sy, sz;
    polygon_p p, p_up, p_down;
    vertex_p v;

    if(bv->bv_type != BV_BOX)
    {
        return;
    }

    vec3_add(bv->base_centre, bb_min, bb_max);
    bv->base_centre[0] /= 2.0;
    bv->base_centre[1] /= 2.0;
    bv->base_centre[2] /= 2.0;

    sx = bb_max[0] - bb_min[0];
    sy = bb_max[1] - bb_min[1];
    sz = bb_max[2] - bb_min[2];
    bv->r = 0.5 * sqrt(sx*sx + sy*sy + sz*sz);

    p = bv->base_polygons;
    // UP
    p_up = p;
    v = p->vertices;
    // 0 1
    // 0 0
    v->position[0] = bb_max[0];
    v->position[1] = bb_max[1];
    v->position[2] = bb_max[2];
    v ++;

    // 1 0
    // 0 0
    v->position[0] = bb_min[0];
    v->position[1] = bb_max[1];
    v->position[2] = bb_max[2];
    v ++;

    // 0 0
    // 1 0
    v->position[0] = bb_min[0];
    v->position[1] = bb_min[1];
    v->position[2] = bb_max[2];
    v ++;

    // 0 0
    // 0 1
    v->position[0] = bb_max[0];
    v->position[1] = bb_min[1];
    v->position[2] = bb_max[2];

    //p->plane[0] = 0.0;
    //p->plane[1] = 0.0;
    //p->plane[2] = 1.0;
    //p->plane[3] = -vec3_dot(p->plane, v);
    Polygon_FindNormale(p);
    p++;

    // DOWN
    p_down = p;
    v = p->vertices;
    // 0 1
    // 0 0
    v->position[0] = bb_max[0];
    v->position[1] = bb_max[1];
    v->position[2] = bb_min[2];
    v ++;

    // 0 0
    // 0 1
    v->position[0] = bb_max[0];
    v->position[1] = bb_min[1];
    v->position[2] = bb_min[2];
    v ++;

    // 0 0
    // 1 0
    v->position[0] = bb_min[0];
    v->position[1] = bb_min[1];
    v->position[2] = bb_min[2];
    v ++;

    // 1 0
    // 0 0
    v->position[0] = bb_min[0];
    v->position[1] = bb_max[1];
    v->position[2] = bb_min[2];

    //p->plane[0] = 0.0;
    //p->plane[1] = 0.0;
    //p->plane[2] =-1.0;
    //p->plane[3] = -vec3_dot(p->plane, v);
    Polygon_FindNormale(p);
    p++;

    // RIGHT: OX+
    v = p->vertices;
    vec3_copy(v[0].position, p_up->vertices[0].position);                       // 0 1  up
    vec3_copy(v[1].position, p_up->vertices[3].position);                       // 1 0  up
    vec3_copy(v[2].position, p_down->vertices[1].position);                     // 1 0  down
    vec3_copy(v[3].position, p_down->vertices[0].position);                     // 0 1  down

    //p->plane[0] = 1.0;
    //p->plane[1] = 0.0;
    //p->plane[2] = 0.0;
    //p->plane[3] = -vec3_dot(p->plane, v);
    Polygon_FindNormale(p);
    p++;


    // LEFT: OX-
    v = p->vertices;
    vec3_copy(v[0].position, p_up->vertices[1].position);                       // 0 1  up
    vec3_copy(v[3].position, p_up->vertices[2].position);                       // 1 0  up
    vec3_copy(v[2].position, p_down->vertices[2].position);                     // 1 0  down
    vec3_copy(v[1].position, p_down->vertices[3].position);                     // 0 1  down

    //p->plane[0] =-1.0;
    //p->plane[1] = 0.0;
    //p->plane[2] = 0.0;
    //p->plane[3] = -vec3_dot(p->plane, v);
    Polygon_FindNormale(p);
    p++;


    // FORWARD: OY+
    v = p->vertices;
    vec3_copy(v[0].position, p_up->vertices[0].position);                       // 0 1  up
    vec3_copy(v[3].position, p_up->vertices[1].position);                       // 1 0  up
    vec3_copy(v[2].position, p_down->vertices[3].position);                     // 1 0  down
    vec3_copy(v[1].position, p_down->vertices[0].position);                     // 0 1  down

    //p->plane[0] = 0.0;
    //p->plane[1] = 1.0;
    //p->plane[2] = 0.0;
    //p->plane[3] = -vec3_dot(p->plane, v);
    Polygon_FindNormale(p);
    p++;

    // BACKWARD: OY-
    v = p->vertices;
    vec3_copy(v[0].position, p_up->vertices[3].position);                       // 0 1  up
    vec3_copy(v[1].position, p_up->vertices[2].position);                       // 1 0  up
    vec3_copy(v[2].position, p_down->vertices[2].position);                     // 1 0  down
    vec3_copy(v[3].position, p_down->vertices[1].position);                     // 0 1  down

    //p->plane[0] = 0.0;
    //p->plane[1] = 1.0;
    //p->plane[2] = 0.0;
    //p->plane[3] = -vec3_dot(p->plane, v);
    Polygon_FindNormale(p);
}


void BV_Transform(bounding_volume_p bv)
{
    int i;

    for(i=0;i<bv->polygons_count;i++)
    {
        Polygon_vTransform(bv->polygons+i, bv->base_polygons+i, bv->transform);
    }

    Mat4_vec3_mul_macro(bv->centre, bv->transform, bv->base_centre);
}

void BV_TransformZZ(bounding_volume_p bv, btScalar z1, btScalar z2)
{
    int i;
    polygon_p p;

    if(bv->bv_type == BV_CYLINDER)
    {
        p = bv->base_polygons;
        for(i=0;i<bv->polygons_count;i++,p++)
        {
            p->vertices[0].position[2] = z1;
            p->vertices[1].position[2] = z2;
            p->vertices[2].position[2] = z2;
            p->vertices[3].position[2] = z1;
            Polygon_vTransform(bv->polygons+i, p, bv->transform);
        }

        bv->base_centre[0] = 0.0;
        bv->base_centre[1] = 0.0;
        bv->base_centre[2] = (z1 + z2) / 2.0;
        Mat4_vec3_mul_macro(bv->centre, bv->transform, bv->base_centre);
    }
}

/**
 * Creates Z capsule - convex shape; Uses for full 3d scaling;
 * @param size = {x/2, y/2, z_cyl/2, z_capsule/2};
 * @param n = number of segments;
 * @return btCollisionShape (is convex);
 */
btCollisionShape *BV_CreateBTCapsuleZ(btScalar size[4], int n)
{
    int i;
    btScalar fi, dfi;
    btVector3 v;
    btConvexHullShape *ret;

    if(n < 3 || n > 64)
    {
        return NULL;
    }

    fi = 0.0;
    dfi = M_PI * 2.0 / (btScalar)n;
    ret = new btConvexHullShape();

    for(i=0;i<n;i++,fi+=dfi)
    {
        v.m_floats[0] = 0.0;
        v.m_floats[1] = 0.0;
        v.m_floats[2] = size[3];
        ret->addPoint(v);
        v.m_floats[0] = size[0] * cos(fi);
        v.m_floats[1] = size[1] * sin(fi);
        v.m_floats[2] = size[2];
        ret->addPoint(v);
        v.m_floats[0] = size[0] * cos(fi + dfi);
        v.m_floats[1] = size[1] * sin(fi + dfi);
        v.m_floats[2] = size[2];
        ret->addPoint(v);

        v.m_floats[2] =-size[2];
        ret->addPoint(v);
        v.m_floats[0] = size[0] * cos(fi);
        v.m_floats[1] = size[1] * sin(fi);
        ret->addPoint(v);
        v.m_floats[0] = 0.0;
        v.m_floats[1] = 0.0;
        v.m_floats[2] =-size[3];
        ret->addPoint(v);
    }

    return ret;
}
