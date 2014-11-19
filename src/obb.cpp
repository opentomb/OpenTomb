
#include <math.h>
#include <stdlib.h>

#include "vmath.h"
#include "obb.h"
#include "polygon.h"
#include "entity.h"

#include "bullet/LinearMath/btScalar.h"
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "engine.h"

obb_p OBB_Create()
{
    obb_p ret;

    ret = (obb_p)malloc(sizeof(obb_t));
    for(int i=0;i<6;i++)
    {
        ret->base_polygons[i].vertex_count = 0;
        ret->polygons[i].vertex_count = 0;
        ret->base_polygons[i].anim_tex_frames_count = 0;
        ret->polygons[i].anim_tex_frames_count = 0;
        ret->base_polygons[i].vertices = NULL;
        ret->polygons[i].vertices = NULL;
        ret->base_polygons[i].anim_tex_frames = NULL;
        ret->polygons[i].anim_tex_frames = NULL;
        ret->base_polygons[i].anim_tex_indexes = NULL;
        ret->polygons[i].anim_tex_indexes = NULL;
        Polygon_Resize(ret->base_polygons+i, 4);
        Polygon_Resize(ret->polygons+i, 4);
    }
    ret->transform = NULL;

    return ret;
}


void OBB_Clear(obb_p obb)
{
    if(obb != NULL)
    {
        for(int i=0;i<6;i++)
        {
            Polygon_Clear(obb->polygons + i);
            Polygon_Clear(obb->base_polygons + i);
        }
    }
}


void OBB_Rebuild(obb_p obb, btScalar bb_min[3], btScalar bb_max[3])
{
    polygon_p p, p_up, p_down;
    vertex_p v;

    vec3_sub(obb->extent, bb_max, bb_min);
    vec3_mul_scalar(obb->extent, obb->extent, 0.5);

    vec3_add(obb->base_centre, bb_min, bb_max);
    vec3_mul_scalar(obb->base_centre, obb->base_centre, 0.5);
    obb->r = vec3_abs(obb->extent);

    p = obb->base_polygons;
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


void OBB_Transform(obb_p obb)
{
    if(obb->transform != NULL)
    {
        for(int i=0;i<6;i++)
        {
            Polygon_vTransform(obb->polygons+i, obb->base_polygons+i, obb->transform);
        }
        Mat4_vec3_mul_macro(obb->centre, obb->transform, obb->base_centre);
    }
    else
    {
        for(int i=0;i<6;i++)
        {
            Polygon_Copy(obb->polygons+i, obb->base_polygons+i);
        }
        vec3_copy(obb->centre, obb->base_centre);
    }
}

/*
 * http://www.gamasutra.com/view/feature/131790/simple_intersection_tests_for_games.php?print=1
 */
int OBB_OBB_Test(struct entity_s *e1, struct entity_s *e2)
{
    //translation, in parent frame
    btScalar v[3], T[3];
    vec3_sub(v, e2->obb->centre, e1->obb->centre);
    //translation, in A's frame
    T[0] = vec3_dot(v, e1->transform + 0);
    T[1] = vec3_dot(v, e1->transform + 4);
    T[2] = vec3_dot(v, e1->transform + 8);

    btScalar *a = e1->obb->extent;
    btScalar *b = e2->obb->extent;

    //B's basis with respect to A's local frame
    btScalar R[3][3];
    btScalar ra, rb, t;
    int i, k;

    //calculate rotation matrix
    for(i=0 ; i<3 ; i++)
    {
        for(k=0 ; k<3 ; k++)
        {
            btScalar *e1b = e1->transform + 4 * i;
            btScalar *e2b = e2->transform + 4 * k;
            R[i][k] = vec3_dot(e1b, e2b);
        }
    }

    /*ALGORITHM: Use the separating axis test for all 15 potential
    separating axes. If a separating axis could not be found, the two
    boxes overlap. */

    //A's basis vectors
    for(i=0;i<3;i++)
    {
        ra = a[i];
        rb = b[0]*fabs(R[i][0]) + b[1]*fabs(R[i][1]) + b[2]*fabs(R[i][2]);
        t = fabs(T[i]);

        if(t > ra + rb)
        {
            return 0;
        }
    }

    //B's basis vectors
    for(k=0;k<3;k++)
    {
        ra = a[0]*fabs(R[0][k]) + a[1]*fabs(R[1][k]) + a[2]*fabs(R[2][k]);
        rb = b[k];
        t = fabs(T[0]*R[0][k] + T[1]*R[1][k] + T[2]*R[2][k]);
        if(t > ra + rb)
        {
            return 0;
        }
    }

    //9 cross products
    //L = A0 x B0
    ra = a[1]*fabs(R[2][0]) + a[2]*fabs(R[1][0]);
    rb = b[1]*fabs(R[0][2]) + b[2]*fabs(R[0][1]);
    t = fabs(T[2]*R[1][0] - T[1]*R[2][0]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A0 x B1
    ra = a[1]*fabs(R[2][1]) + a[2]*fabs(R[1][1]);
    rb = b[0]*fabs(R[0][2]) + b[2]*fabs(R[0][0]);
    t = fabs(T[2]*R[1][1] - T[1]*R[2][1]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A0 x B2
    ra = a[1]*fabs(R[2][2]) + a[2]*fabs(R[1][2]);
    rb = b[0]*fabs(R[0][1]) + b[1]*fabs(R[0][0]);
    t = fabs(T[2]*R[1][2] - T[1]*R[2][2]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A1 x B0
    ra = a[0]*fabs(R[2][0]) + a[2]*fabs(R[0][0]);
    rb = b[1]*fabs(R[1][2]) + b[2]*fabs(R[1][1]);
    t = fabs(T[0]*R[2][0] - T[2]*R[0][0]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A1 x B1
    ra = a[0]*fabs(R[2][1]) + a[2]*fabs(R[0][1]);
    rb = b[0]*fabs(R[1][2]) + b[2]*fabs(R[1][0]);
    t = fabs(T[0]*R[2][1] - T[2]*R[0][1]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A1 x B2
    ra = a[0]*fabs(R[2][2]) + a[2]*fabs(R[0][2]);
    rb = b[0]*fabs(R[1][1]) + b[1]*fabs(R[1][0]);
    t = fabs(T[0]*R[2][2] - T[2]*R[0][2]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A2 x B0
    ra = a[0]*fabs(R[1][0]) + a[1]*fabs(R[0][0]);
    rb = b[1]*fabs(R[2][2]) + b[2]*fabs(R[2][1]);
    t = fabs(T[1]*R[0][0] - T[0]*R[1][0]);

    if(t > ra + rb)
    {
        return 0;
    }


    //L = A2 x B1
    ra = a[0]*fabs(R[1][1]) + a[1]*fabs(R[0][1]);
    rb = b[0] *fabs(R[2][2]) + b[2]*fabs(R[2][0]);
    t = fabs(T[1]*R[0][1] - T[0]*R[1][1]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A2 x B2
    ra = a[0]*fabs(R[1][2]) + a[1]*fabs(R[0][2]);
    rb = b[0]*fabs(R[2][1]) + b[1]*fabs(R[2][0]);
    t = fabs(T[1]*R[0][2] - T[0]*R[1][2]);

    if(t > ra + rb)
    {
        return 0;
    }

    /*no separating axis found,
    the two boxes overlap */
    return 1;
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
