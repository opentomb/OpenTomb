#include <cstdio>
#include <cstdlib>

#include "bullet/LinearMath/btScalar.h"
#include "polygon.h"
#include "portal.h"
#include "vmath.h"
#include "camera.h"
#include "world.h"
#include "render.h"
#include "frustum.h"
#include "engine.h"

/*
 * CLIP PLANES
 */


void Portal::move(const btVector3& mv)
{
    centre += mv;
    for(btVector3& v : vertices)
        v += mv;

    normal[3] = -normal.dot(vertices.front());
}


/**
 * Barycentric method for determining the intersection of a ray and a triangle
 * @param ray ray directionа
 * @param dot point of ray-plane intersection
 */
bool Portal::rayIntersect(const btVector3& ray, const btVector3& dot)
{
    btScalar u = normal.dot(ray);
    if(std::fabs(u) < SPLIT_EPSILON)
    {
        // the plane is parallel to the line
        return false;
    }
    btScalar t = -(normal[3] + normal.dot(dot)) / u;
    if(t <= 0)
    {
        // plane is on the wrong side of the ray
        return false;
    }

    // A pointer to the current triangle
    btVector3* currentVertex = &vertices.front();
    // The vector that does not change for the entire polygon
    btVector3 T = dot - *currentVertex;

    btVector3 edge = vertices[1]-vertices[0];
    // Bypass polygon fan, one of the vectors remains unchanged
    for(size_t i=0; i<vertices.size()-2; i++, currentVertex++)
    {
        // PREV
        btVector3 prevEdge = edge;
        edge = currentVertex[2] - vertices[0];

        btVector3 P = ray.cross(edge);
        btVector3 Q = T.cross(prevEdge);

        t = P.dot(prevEdge);
        u = P.dot(T) / t;
        btScalar v = Q.dot(ray) / t;
        t = 1.0 - u - v;
        if((u <= 1.0) && (u >= 0.0) && (v <= 1.0) && (v >= 0.0) && (t <= 1.0) && (t >= 0.0))
        {
            return true;
        }
    }

    return false;
}

/**
 * Просто генерация нормали к порталу
 */
void Portal::genNormale()
{
    assert( vertices.size() > 3 );
    auto v1 = vertices[0] - vertices[1];
    auto v2 = vertices[2] - vertices[1];
    normal = v1.cross(v2).normalized();
    normal[3] = -normal.dot(vertices.front());
}
