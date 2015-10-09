#include "orientedboundingbox.h"

#include <cmath>

#include "engine/engine.h"
#include "util/vmath.h"
#include "world/core/polygon.h"
#include "world/entity.h"

namespace world
{
namespace core
{

void OrientedBoundingBox::rebuild(const BoundingBox& boundingBox)
{
    extent = boundingBox.getDiameter() * 0.5f;
    base_centre = boundingBox.getCenter();
    radius = glm::length(extent);

    Polygon *p = base_polygons.data();
    // UP
    Polygon *p_up = p;
    auto v = &p->vertices.front();
    // 0 1
    // 0 0
    v->position[0] = boundingBox.max[0];
    v->position[1] = boundingBox.max[1];
    v->position[2] = boundingBox.max[2];
    v++;

    // 1 0
    // 0 0
    v->position[0] = boundingBox.min[0];
    v->position[1] = boundingBox.max[1];
    v->position[2] = boundingBox.max[2];
    v++;

    // 0 0
    // 1 0
    v->position[0] = boundingBox.min[0];
    v->position[1] = boundingBox.min[1];
    v->position[2] = boundingBox.max[2];
    v++;

    // 0 0
    // 0 1
    v->position[0] = boundingBox.max[0];
    v->position[1] = boundingBox.min[1];
    v->position[2] = boundingBox.max[2];

    //p->plane[0] = 0.0;
    //p->plane[1] = 0.0;
    //p->plane[2] = 1.0;
    //p->plane[3] = -vec3_dot(p->plane, v);
    p->updateNormal();
    p++;

    // DOWN
    struct Polygon *p_down = p;
    v = &p->vertices.front();
    // 0 1
    // 0 0
    v->position[0] = boundingBox.max[0];
    v->position[1] = boundingBox.max[1];
    v->position[2] = boundingBox.min[2];
    v++;

    // 0 0
    // 0 1
    v->position[0] = boundingBox.max[0];
    v->position[1] = boundingBox.min[1];
    v->position[2] = boundingBox.min[2];
    v++;

    // 0 0
    // 1 0
    v->position[0] = boundingBox.min[0];
    v->position[1] = boundingBox.min[1];
    v->position[2] = boundingBox.min[2];
    v++;

    // 1 0
    // 0 0
    v->position[0] = boundingBox.min[0];
    v->position[1] = boundingBox.max[1];
    v->position[2] = boundingBox.min[2];

    p->updateNormal();
    p++;

    // RIGHT: OX+
    v = &p->vertices.front();
    v[0].position = p_up->vertices[0].position;                       // 0 1  up
    v[1].position = p_up->vertices[3].position;                       // 1 0  up
    v[2].position = p_down->vertices[1].position;                     // 1 0  down
    v[3].position = p_down->vertices[0].position;                     // 0 1  down

    p->updateNormal();
    p++;

    // LEFT: OX-
    v = &p->vertices.front();
    v[0].position = p_up->vertices[1].position;                       // 0 1  up
    v[3].position = p_up->vertices[2].position;                       // 1 0  up
    v[2].position = p_down->vertices[2].position;                     // 1 0  down
    v[1].position = p_down->vertices[3].position;                     // 0 1  down

    p->updateNormal();
    p++;

    // FORWARD: OY+
    v = &p->vertices.front();
    v[0].position = p_up->vertices[0].position;                       // 0 1  up
    v[3].position = p_up->vertices[1].position;                       // 1 0  up
    v[2].position = p_down->vertices[3].position;                     // 1 0  down
    v[1].position = p_down->vertices[0].position;                     // 0 1  down

    p->updateNormal();
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
    p->updateNormal();
}

void OrientedBoundingBox::doTransform()
{
    if(transform != nullptr)
    {
        for(size_t i = 0; i < polygons.size(); i++)
        {
            polygons[i].copyTransformed(base_polygons[i], *transform);
        }
        center = glm::vec3(*transform * glm::vec4(base_centre, 1.0f));
    }
    else
    {
        for(size_t i = 0; i < polygons.size(); i++)
        {
            polygons[i] = base_polygons[i];
        }
        center = base_centre;
    }
}

/*
 * http://www.gamasutra.com/view/feature/131790/simple_intersection_tests_for_games.php?print=1
 */
bool testOverlap(const Entity& e1, const Entity& e2, glm::float_t overlap)
{
    //translation, in parent frame
    auto v = e2.m_obb.center - e1.m_obb.center;
    //translation, in A's frame
    glm::vec3 T = glm::vec3(e1.m_transform * glm::vec4(v, 1.0f));

    glm::vec3 a = e1.m_obb.extent * overlap;
    glm::vec3 b = e2.m_obb.extent * overlap;

    //B's basis with respect to A's local frame
    glm::float_t R[3][3];
    glm::float_t ra, rb, t;

    //calculate rotation matrix
    for(int i = 0; i < 3; i++)
    {
        for(int k = 0; k < 3; k++)
        {
            const glm::vec4 e1b = e1.m_transform[i];
            const glm::vec4 e2b = e2.m_transform[k];
            R[i][k] = glm::dot(e1b, e2b);
        }
    }

    /*ALGORITHM: Use the separating axis test for all 15 potential
    separating axes. If a separating axis could not be found, the two
    boxes overlap. */

    //A's basis vectors
    for(int i = 0; i < 3; i++)
    {
        ra = a[i];
        rb = b[0] * glm::abs(R[i][0]) + b[1] * glm::abs(R[i][1]) + b[2] * glm::abs(R[i][2]);
        t = glm::abs(T[i]);

        if(t > ra + rb)
        {
            return false;
        }
    }

    //B's basis vectors
    for(int k = 0; k < 3; k++)
    {
        ra = a[0] * glm::abs(R[0][k]) + a[1] * glm::abs(R[1][k]) + a[2] * glm::abs(R[2][k]);
        rb = b[k];
        t = glm::abs(T[0] * R[0][k] + T[1] * R[1][k] + T[2] * R[2][k]);
        if(t > ra + rb)
        {
            return false;
        }
    }

    //9 cross products
    //L = A0 x B0
    ra = a[1] * glm::abs(R[2][0]) + a[2] * glm::abs(R[1][0]);
    rb = b[1] * glm::abs(R[0][2]) + b[2] * glm::abs(R[0][1]);
    t = glm::abs(T[2] * R[1][0] - T[1] * R[2][0]);

    if(t > ra + rb)
    {
        return false;
    }

    //L = A0 x B1
    ra = a[1] * glm::abs(R[2][1]) + a[2] * glm::abs(R[1][1]);
    rb = b[0] * glm::abs(R[0][2]) + b[2] * glm::abs(R[0][0]);
    t = glm::abs(T[2] * R[1][1] - T[1] * R[2][1]);

    if(t > ra + rb)
    {
        return false;
    }

    //L = A0 x B2
    ra = a[1] * glm::abs(R[2][2]) + a[2] * glm::abs(R[1][2]);
    rb = b[0] * glm::abs(R[0][1]) + b[1] * glm::abs(R[0][0]);
    t = glm::abs(T[2] * R[1][2] - T[1] * R[2][2]);

    if(t > ra + rb)
    {
        return false;
    }

    //L = A1 x B0
    ra = a[0] * glm::abs(R[2][0]) + a[2] * glm::abs(R[0][0]);
    rb = b[1] * glm::abs(R[1][2]) + b[2] * glm::abs(R[1][1]);
    t = glm::abs(T[0] * R[2][0] - T[2] * R[0][0]);

    if(t > ra + rb)
    {
        return false;
    }

    //L = A1 x B1
    ra = a[0] * glm::abs(R[2][1]) + a[2] * glm::abs(R[0][1]);
    rb = b[0] * glm::abs(R[1][2]) + b[2] * glm::abs(R[1][0]);
    t = glm::abs(T[0] * R[2][1] - T[2] * R[0][1]);

    if(t > ra + rb)
    {
        return false;
    }

    //L = A1 x B2
    ra = a[0] * glm::abs(R[2][2]) + a[2] * glm::abs(R[0][2]);
    rb = b[0] * glm::abs(R[1][1]) + b[1] * glm::abs(R[1][0]);
    t = glm::abs(T[0] * R[2][2] - T[2] * R[0][2]);

    if(t > ra + rb)
    {
        return false;
    }

    //L = A2 x B0
    ra = a[0] * glm::abs(R[1][0]) + a[1] * glm::abs(R[0][0]);
    rb = b[1] * glm::abs(R[2][2]) + b[2] * glm::abs(R[2][1]);
    t = glm::abs(T[1] * R[0][0] - T[0] * R[1][0]);

    if(t > ra + rb)
    {
        return false;
    }

    //L = A2 x B1
    ra = a[0] * glm::abs(R[1][1]) + a[1] * glm::abs(R[0][1]);
    rb = b[0] * glm::abs(R[2][2]) + b[2] * glm::abs(R[2][0]);
    t = glm::abs(T[1] * R[0][1] - T[0] * R[1][1]);

    if(t > ra + rb)
    {
        return false;
    }

    //L = A2 x B2
    ra = a[0] * glm::abs(R[1][2]) + a[1] * glm::abs(R[0][2]);
    rb = b[0] * glm::abs(R[2][1]) + b[1] * glm::abs(R[2][0]);
    t = glm::abs(T[1] * R[0][2] - T[0] * R[1][2]);

    if(t > ra + rb)
    {
        return false;
    }

    /*no separating axis found, the two boxes overlap */
    return true;
}

} // namespace core
} // namespace world
