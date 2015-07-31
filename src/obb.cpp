
#include <cmath>
#include <cstdlib>

#include <LinearMath/btScalar.h>

#include "vmath.h"
#include "obb.h"
#include "polygon.h"
#include "entity.h"

#include "engine.h"

void OBB::rebuild(const btVector3& bb_min, const btVector3& bb_max)
{
    extent = (bb_max - bb_min)/2;
    base_centre = (bb_min + bb_max)/2;
    r = extent.length();

    struct Polygon *p = base_polygons;
    // UP
    struct Polygon *p_up = p;
    auto v = &p->vertices.front();
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
    p->findNormal();
    p++;

    // DOWN
    struct Polygon *p_down = p;
    v = &p->vertices.front();
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

    p->findNormal();
    p++;

    // RIGHT: OX+
    v = &p->vertices.front();
    v[0].position = p_up->vertices[0].position;                       // 0 1  up
    v[1].position = p_up->vertices[3].position;                       // 1 0  up
    v[2].position = p_down->vertices[1].position;                     // 1 0  down
    v[3].position = p_down->vertices[0].position;                     // 0 1  down

    p->findNormal();
    p++;


    // LEFT: OX-
    v = &p->vertices.front();
    v[0].position = p_up->vertices[1].position;                       // 0 1  up
    v[3].position = p_up->vertices[2].position;                       // 1 0  up
    v[2].position = p_down->vertices[2].position;                     // 1 0  down
    v[1].position = p_down->vertices[3].position;                     // 0 1  down

    p->findNormal();
    p++;


    // FORWARD: OY+
    v = &p->vertices.front();
    v[0].position = p_up->vertices[0].position;                       // 0 1  up
    v[3].position = p_up->vertices[1].position;                       // 1 0  up
    v[2].position = p_down->vertices[3].position;                     // 1 0  down
    v[1].position = p_down->vertices[0].position;                     // 0 1  down

    p->findNormal();
    p++;

    // BACKWARD: OY-
    v = &p->vertices.front();
    v[0].position = p_up->vertices[3].position;                       // 0 1  up
    v[1].position = p_up->vertices[2].position;                       // 1 0  up
    v[2].position = p_down->vertices[2].position;                     // 1 0  down
    v[3].position = p_down->vertices[1].position;                     // 0 1  down

    //p->plane[0] = 0.0;
    //p->plane[1] = 1.0;
    //p->plane[2] = 0.0;
    //p->plane[3] = -vec3_dot(p->plane, v);
    p->findNormal();
}


void OBB::doTransform()
{
    if(transform != NULL) {
        for(int i=0;i<6;i++) {
            polygons[i].vTransform(&base_polygons[i], *transform);
        }
        centre = *transform * base_centre;
    }
    else {
        for(int i=0;i<6;i++) {
            polygons[i] = base_polygons[i];
        }
        centre = base_centre;
    }
}

/*
 * http://www.gamasutra.com/view/feature/131790/simple_intersection_tests_for_games.php?print=1
 */
int OBB_OBB_Test(const Entity& e1, const Entity& e2, btScalar overlap)
{
    //translation, in parent frame
    auto v = e2.m_obb->centre - e1.m_obb->centre;
    //translation, in A's frame
    btVector3 T = e1.m_transform.getBasis() * v;

    btVector3 a = e1.m_obb->extent * overlap;
    btVector3 b = e2.m_obb->extent * overlap;

    //B's basis with respect to A's local frame
    btScalar R[3][3];
    btScalar ra, rb, t;
    int i, k;

    //calculate rotation matrix
    for(i=0 ; i<3 ; i++)
    {
        for(k=0 ; k<3 ; k++)
        {
            const btVector3 e1b = e1.m_transform.getBasis().getColumn(i);
            const btVector3 e2b = e2.m_transform.getBasis().getColumn(k);
            R[i][k] = e1b.dot(e2b);
        }
    }

    /*ALGORITHM: Use the separating axis test for all 15 potential
    separating axes. If a separating axis could not be found, the two
    boxes overlap. */

    //A's basis vectors
    for(i=0;i<3;i++)
    {
        ra = a[i];
        rb = b[0]*std::abs(R[i][0]) + b[1]*std::abs(R[i][1]) + b[2]*std::abs(R[i][2]);
        t = std::abs(T[i]);

        if(t > ra + rb)
        {
            return 0;
        }
    }

    //B's basis vectors
    for(k=0;k<3;k++)
    {
        ra = a[0]*std::abs(R[0][k]) + a[1]*std::abs(R[1][k]) + a[2]*std::abs(R[2][k]);
        rb = b[k];
        t = std::abs(T[0]*R[0][k] + T[1]*R[1][k] + T[2]*R[2][k]);
        if(t > ra + rb)
        {
            return 0;
        }
    }

    //9 cross products
    //L = A0 x B0
    ra = a[1]*std::abs(R[2][0]) + a[2]*std::abs(R[1][0]);
    rb = b[1]*std::abs(R[0][2]) + b[2]*std::abs(R[0][1]);
    t = std::abs(T[2]*R[1][0] - T[1]*R[2][0]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A0 x B1
    ra = a[1]*std::abs(R[2][1]) + a[2]*std::abs(R[1][1]);
    rb = b[0]*std::abs(R[0][2]) + b[2]*std::abs(R[0][0]);
    t = std::abs(T[2]*R[1][1] - T[1]*R[2][1]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A0 x B2
    ra = a[1]*std::abs(R[2][2]) + a[2]*std::abs(R[1][2]);
    rb = b[0]*std::abs(R[0][1]) + b[1]*std::abs(R[0][0]);
    t = std::abs(T[2]*R[1][2] - T[1]*R[2][2]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A1 x B0
    ra = a[0]*std::abs(R[2][0]) + a[2]*std::abs(R[0][0]);
    rb = b[1]*std::abs(R[1][2]) + b[2]*std::abs(R[1][1]);
    t = std::abs(T[0]*R[2][0] - T[2]*R[0][0]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A1 x B1
    ra = a[0]*std::abs(R[2][1]) + a[2]*std::abs(R[0][1]);
    rb = b[0]*std::abs(R[1][2]) + b[2]*std::abs(R[1][0]);
    t = std::abs(T[0]*R[2][1] - T[2]*R[0][1]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A1 x B2
    ra = a[0]*std::abs(R[2][2]) + a[2]*std::abs(R[0][2]);
    rb = b[0]*std::abs(R[1][1]) + b[1]*std::abs(R[1][0]);
    t = std::abs(T[0]*R[2][2] - T[2]*R[0][2]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A2 x B0
    ra = a[0]*std::abs(R[1][0]) + a[1]*std::abs(R[0][0]);
    rb = b[1]*std::abs(R[2][2]) + b[2]*std::abs(R[2][1]);
    t = std::abs(T[1]*R[0][0] - T[0]*R[1][0]);

    if(t > ra + rb)
    {
        return 0;
    }


    //L = A2 x B1
    ra = a[0]*std::abs(R[1][1]) + a[1]*std::abs(R[0][1]);
    rb = b[0] *std::abs(R[2][2]) + b[2]*std::abs(R[2][0]);
    t = std::abs(T[1]*R[0][1] - T[0]*R[1][1]);

    if(t > ra + rb)
    {
        return 0;
    }

    //L = A2 x B2
    ra = a[0]*std::abs(R[1][2]) + a[1]*std::abs(R[0][2]);
    rb = b[0]*std::abs(R[2][1]) + b[1]*std::abs(R[2][0]);
    t = std::abs(T[1]*R[0][2] - T[0]*R[1][2]);

    if(t > ra + rb)
    {
        return 0;
    }

    /*no separating axis found,
    the two boxes overlap */
    return 1;
}
