#include <cstdio>
#include <cstdlib>

#include "polygon.h"
#include "vmath.h"
#include "camera.h"
#include "portal.h"
#include "engine.h"

/*
 * POLYGONS
 */

bool Polygon::isBroken() const
{
    if(vertices.size() < 3)
    {
        return true;
    }

    btScalar dif0 = plane.length2();
    if(dif0 < 0.999 || dif0 > 1.001)
    {
        return true;
    }

    auto curr_v = &vertices.back();
    for(const auto& v : vertices)
    {
        btVector3 dif = v.position - curr_v->position;
        if(dif.length2() < 0.0001)
        {
            return 1;
        }

        curr_v = &v;
    }

    return 0;
}


void Polygon::findNormal()
{
    auto v1 = vertices[1].position - vertices[0].position;
    auto v2 = vertices[2].position - vertices[1].position;
    plane = v1.cross(v2).normalized();
    plane[3] = plane.length();
}


void Polygon::moveSelf(const btVector3& move)
{
    for(auto& v : vertices)
    {
        v.position += move;
    }

    plane[3] = -plane.dot(vertices[0].position);
}


void Polygon::move(Polygon* src, const btVector3& move)
{
    for(size_t i=0; i<src->vertices.size(); i++)
    {
        vertices[i].position = src->vertices[i].position + move;
    }

    plane = src->plane;
    plane[3] = -plane.dot(vertices[0].position);
}


void Polygon::transformSelf(const btTransform& tr)
{
    plane = tr.getBasis() * plane;
    for(auto& vp : vertices)
    {
        vp.position = tr*vp.position;
        vp.normal = tr.getBasis() * vp.normal;
    }

    plane[3] = -plane.dot(vertices[0].position);
}


void Polygon::transform(const Polygon& src, const btTransform& tr)
{
    vertices.resize(src.vertices.size());

    plane = tr.getBasis() * src.plane;
    for(size_t i=0; i<src.vertices.size(); i++)
    {
        vertices[i].position = tr * src.vertices[i].position;
        vertices[i].normal = tr.getBasis() * src.vertices[i].normal;
    }

    plane[3] = -plane.dot(vertices[0].position);
}


void Polygon::vTransform(Polygon* src, const btTransform& tr)
{
    plane = tr.getBasis() * src->plane;
    for(size_t i=0; i<src->vertices.size(); i++)
    {
        vertices[i].position = tr * src->vertices[i].position;
    }

    plane[3] = -plane.dot(vertices[0].position);
}


int Polygon::rayIntersect(const btVector3& dir, const btVector3& dot, btScalar *t) const
{
    btScalar u = plane.dot(dir);
    if(std::fabs(u) < 0.001 /*|| vec3_plane_dist(plane, dot) < -0.001*/)          // FIXME: magick
    {
        return 0;                                                               // plane is parallel to the ray - no intersection
    }
    *t = -planeDist(plane, dot) / u;

    auto vp = &vertices.front();                                                           // current polygon pointer
    btVector3 T = dot - vp[0].position;

    btVector3 E2 = vp[1].position - vp[0].position;
    for(size_t i=0; i<vertices.size()-2; i++,vp++)
    {
        btVector3 E1 = E2;                                                       // PREV
        E2 = vp[2].position - vertices[0].position;                   // NEXT

        btVector3 P = dir.cross(E2);
        btVector3 Q = T.cross(E1);

        btScalar tt = P.dot(E1);
        u = P.dot(T);
        u /= tt;
        btScalar v = Q.dot(dir);
        v /= tt;
        tt = 1.0 - u - v;
        if((u <= 1.0) && (u >= 0.0) && (v <= 1.0) && (v >= 0.0) && (tt <= 1.0) && (tt >= 0.0))
        {
            return 1;
        }
    }
    return 0;
}


bool Polygon::intersectPolygon(Polygon* p2)
{
    if(SPLIT_IN_BOTH != splitClassify(p2->plane) || (SPLIT_IN_BOTH != p2->splitClassify(plane)))
    {
        return false;                                                               // quick check
    }

    std::vector<btVector3> result_buf;

    /*
     * intersection of polygon p1 and plane p2
     */
    auto prev_v = &vertices.back();
    auto curr_v = &vertices.front();
    btScalar dist0 = planeDist(p2->plane, prev_v->position);
    for(size_t i=0; i<vertices.size(); i++)
    {
        btScalar dist1 = planeDist(p2->plane, curr_v->position);
        if(dist1 > SPLIT_EPSILON)
        {
            if(dist0 < -SPLIT_EPSILON)
            {
                auto dir = curr_v->position - prev_v->position;
                btScalar t;
                btVector3 v;
                result_buf.emplace_back();
                rayPlaneIntersect(prev_v->position, dir, p2->plane, &result_buf.back(), &t);
            }
        }
        else if(dist1 < -SPLIT_EPSILON)
        {
            if(dist0 > SPLIT_EPSILON)
            {
                auto dir = curr_v->position - prev_v->position;
                btScalar t;
                result_buf.emplace_back();
                rayPlaneIntersect(prev_v->position, dir, p2->plane, &result_buf.back(), &t);
            }
        }
        else
        {
            result_buf.emplace_back(curr_v->position);
        }

        if(result_buf.size() >= 2)
        {
            break;
        }
        dist0 = dist1;
        prev_v = curr_v;
        curr_v++;
    }

    /*
     * splitting p2 by p1 split plane
     */
    prev_v = &p2->vertices.back();
    curr_v = &p2->vertices.front();
    dist0 = planeDist(plane, prev_v->position);
    for(size_t i=0; i<p2->vertices.size(); i++)
    {
        btScalar dist1 = planeDist(plane, curr_v->position);
        if(dist1 > SPLIT_EPSILON)
        {
            if(dist0 < -SPLIT_EPSILON)
            {
                auto dir = curr_v->position - prev_v->position;
                btScalar t;
                result_buf.emplace_back();
                rayPlaneIntersect(prev_v->position, dir, plane, &result_buf.back(), &t);
            }
        }
        else if(dist1 < -SPLIT_EPSILON)
        {
            if(dist0 > SPLIT_EPSILON)
            {
                auto dir = curr_v->position - prev_v->position;
                btScalar t;
                result_buf.emplace_back();
                rayPlaneIntersect(prev_v->position, dir, plane, &result_buf.back(), &t);
            }
        }
        else
        {
            result_buf.emplace_back(curr_v->position);
        }

        if(result_buf.size() >= 4)
        {
            break;
        }
        dist0 = dist1;
        prev_v = curr_v;
        curr_v ++;
    }

    auto dir = plane.cross(p2->plane);                                      // vector of two planes intersection line
    btScalar t = std::fabs(dir[0]);
    dist0 = std::fabs(dir[1]);
    btScalar dist1 = std::fabs(dir[2]);
    btScalar dist2 = 0;
    int pn = PLANE_X;
    if(t < dist0)
    {
        t = dist0;
        pn = PLANE_Y;
    }
    if(t < dist1)
    {
        pn = PLANE_Z;
    }

    switch(pn)
    {
        case PLANE_X:
            dist0 = (result_buf[1][0] -  result_buf[0][0]) / dir[0];
            dist1 = (result_buf[2][0] -  result_buf[0][0]) / dir[0];
            dist2 = (result_buf[3][0] -  result_buf[0][0]) / dir[0];
            break;

        case PLANE_Y:
            dist0 = (result_buf[1][1] -  result_buf[0][1]) / dir[1];
            dist1 = (result_buf[2][1] -  result_buf[0][1]) / dir[1];
            dist2 = (result_buf[3][1] -  result_buf[0][1]) / dir[1];
            break;

        case PLANE_Z:
            dist0 = (result_buf[1][2] -  result_buf[0][2]) / dir[2];
            dist1 = (result_buf[2][2] -  result_buf[0][2]) / dir[2];
            dist2 = (result_buf[3][2] -  result_buf[0][2]) / dir[2];
            break;
    };

    if(dist0 > 0)
    {
        return !((dist1 < 0.0 && dist2 < 0.0) || (dist1 > dist0 && dist2 > dist0));
    }
    return !((dist1 < dist0 && dist2 < dist0) || (dist1 > 0.0 && dist2 > 0.0));
}


int Polygon::splitClassify(const btVector3& n)
{
    size_t positive=0, negative=0;
    for (const auto& v : vertices)
    {
        auto dist = planeDist(n, v.position);
        if (dist > SPLIT_EPSILON)
        {
            positive++;
        }
        else if (dist < -SPLIT_EPSILON)
        {
            negative++;
        }
    }

    if(positive > 0 && negative == 0)
    {
        return SPLIT_FRONT;
    }
    else if(positive == 0 && negative > 0)
    {
        return SPLIT_BACK;
    }
    else if (positive < 1 && negative < 1)
    {
        return SPLIT_IN_PLANE;
    }

    return SPLIT_IN_BOTH;
}

/*
 * animated textures coordinates splits too!
 */
void Polygon::split(const btVector3& n, Polygon* front, Polygon* back)
{
    front->plane = plane;
    front->anim_id = anim_id;
    front->frame_offset = frame_offset;
    front->double_side = double_side;
    front->tex_index = tex_index;
    front->transparency = transparency;

    back->plane = plane;
    back->anim_id = anim_id;
    back->frame_offset = frame_offset;
    back->double_side = double_side;
    back->tex_index = tex_index;
    back->transparency = transparency;

    auto curr_v = &vertices.front();
    auto prev_v = &vertices.back();

    auto dist0 = planeDist(n, prev_v->position);
    for(size_t i=0; i<vertices.size(); ++i)
    {
        auto dist1 = planeDist(n, curr_v->position);

        if(dist1 > SPLIT_EPSILON)
        {
            if(dist0 < -SPLIT_EPSILON)
            {
                auto dir = curr_v->position - prev_v->position;
                btScalar t;
                Vertex tv;
                rayPlaneIntersect(prev_v->position, dir, n, &tv.position, &t);

                tv.normal[0] = prev_v->normal[0] + t * (curr_v->normal[0] - prev_v->normal[0]);
                tv.normal[1] = prev_v->normal[1] + t * (curr_v->normal[1] - prev_v->normal[1]);
                tv.normal[2] = prev_v->normal[2] + t * (curr_v->normal[2] - prev_v->normal[2]);
                tv.normal.normalize();

                tv.color[0] = prev_v->color[0] + t * (curr_v->color[0] - prev_v->color[0]);
                tv.color[1] = prev_v->color[1] + t * (curr_v->color[1] - prev_v->color[1]);
                tv.color[2] = prev_v->color[2] + t * (curr_v->color[2] - prev_v->color[2]);
                tv.color[3] = prev_v->color[3] + t * (curr_v->color[3] - prev_v->color[3]);

                tv.tex_coord[0] = prev_v->tex_coord[0] + t * (curr_v->tex_coord[0] - prev_v->tex_coord[0]);
                tv.tex_coord[1] = prev_v->tex_coord[1] + t * (curr_v->tex_coord[1] - prev_v->tex_coord[1]);

                front->vertices.emplace_back(tv);
                back->vertices.emplace_back(tv);
            }
            front->vertices.emplace_back(*curr_v);
        }
        else if(dist1 < -SPLIT_EPSILON)
        {
            if(dist0 > SPLIT_EPSILON)
            {
                auto dir = curr_v->position - prev_v->position;
                btScalar t;
                Vertex tv;
                rayPlaneIntersect(prev_v->position, dir, n, &tv.position, &t);

                tv.normal[0] = prev_v->normal[0] + t * (curr_v->normal[0] - prev_v->normal[0]);
                tv.normal[1] = prev_v->normal[1] + t * (curr_v->normal[1] - prev_v->normal[1]);
                tv.normal[2] = prev_v->normal[2] + t * (curr_v->normal[2] - prev_v->normal[2]);
                tv.normal.normalize();

                tv.color[0] = prev_v->color[0] + t * (curr_v->color[0] - prev_v->color[0]);
                tv.color[1] = prev_v->color[1] + t * (curr_v->color[1] - prev_v->color[1]);
                tv.color[2] = prev_v->color[2] + t * (curr_v->color[2] - prev_v->color[2]);
                tv.color[3] = prev_v->color[3] + t * (curr_v->color[3] - prev_v->color[3]);

                tv.tex_coord[0] = prev_v->tex_coord[0] + t * (curr_v->tex_coord[0] - prev_v->tex_coord[0]);
                tv.tex_coord[1] = prev_v->tex_coord[1] + t * (curr_v->tex_coord[1] - prev_v->tex_coord[1]);

                front->vertices.emplace_back(tv);
                back->vertices.emplace_back(tv);
            }
            back->vertices.emplace_back(*curr_v);
        }
        else
        {
            front->vertices.emplace_back(*curr_v);
            back->vertices.emplace_back(*curr_v);
        }

        prev_v = curr_v;
        curr_v ++;
        dist0 = dist1;
    }
}


bool Polygon::isInsideBBox(const btVector3& bb_min, const btVector3& bb_max)
{
    for(const auto& v : vertices)
    {
        if((v.position[0] < bb_min[0]) || (v.position[0] > bb_max[0]) ||
           (v.position[1] < bb_min[1]) || (v.position[1] > bb_max[1]) ||
           (v.position[2] < bb_min[2]) || (v.position[2] > bb_max[2]))
        {
            return 0;
        }
    }

    return 1;
}


bool Polygon::isInsideBQuad(const btVector3& bb_min, const btVector3& bb_max)
{
    for(const auto& v : vertices)
    {
        if((v.position[0] < bb_min[0]) || (v.position[0] > bb_max[0]) ||
           (v.position[1] < bb_min[1]) || (v.position[1] > bb_max[1]))
        {
            return false;
        }
    }

    return true;
}
