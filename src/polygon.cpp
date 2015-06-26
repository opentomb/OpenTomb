#include <cstdio>
#include <cstdlib>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include "polygon.h"
#include "vmath.h"
#include "camera.h"
#include "portal.h"
#include "engine.h"

/*
 * POLYGONS
 */

bool Polygon_IsBroken(const polygon_s *p)
{
    if(p->vertices.size() < 3)
    {
        return true;
    }

    btScalar dif0 = p->plane.length2();
    if(dif0 < 0.999 || dif0 > 1.001)
    {
        return true;
    }

    auto curr_v = &p->vertices.back();
    for(const auto& v : p->vertices)
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


void Polygon_FindNormale(polygon_p p)
{
    auto v1 = p->vertices[1].position - p->vertices[0].position;
    auto v2 = p->vertices[2].position - p->vertices[1].position;
    p->plane = v1.cross(v2);
    p->plane[3] = -p->plane.length();
    vec3_norm_plane(p->plane, p->vertices[0].position, p->plane[3]);
}


void Polygon_MoveSelf(polygon_p p, const btVector3& move)
{
    for(auto& v : p->vertices)
    {
        v.position += move;
    }

    p->plane[3] = -p->plane.dot(p->vertices[0].position);
}


void Polygon_Move(polygon_p ret, polygon_p src, const btVector3& move)
{
    for(size_t i=0; i<src->vertices.size(); i++)
    {
        ret->vertices[i].position = src->vertices[i].position + move;
    }

    ret->plane = src->plane;
    ret->plane[3] = -ret->plane.dot(ret->vertices[0].position);
}


void Polygon_TransformSelf(polygon_p p, const btTransform& tr)
{
    p->plane = tr.getBasis() * p->plane;
    for(auto& vp : p->vertices)
    {
        vp.position = tr*vp.position;
        vp.normal = tr.getBasis() * vp.normal;
    }

    p->plane[3] = -p->plane.dot(p->vertices[0].position);
}


void Polygon_Transform(polygon_p ret, const polygon_t *src, const btTransform& tr)
{
    ret->vertices.resize(src->vertices.size());

    ret->plane = tr.getBasis() * src->plane;
    for(size_t i=0; i<src->vertices.size(); i++)
    {
        ret->vertices[i].position = tr * src->vertices[i].position;
        ret->vertices[i].normal = tr.getBasis() * src->vertices[i].normal;
    }

    ret->plane[3] = -ret->plane.dot(ret->vertices[0].position);
}


void Polygon_vTransform(polygon_p ret, polygon_p src, const btTransform& tr)
{
    ret->plane = tr.getBasis() * src->plane;
    for(size_t i=0; i<src->vertices.size(); i++)
    {
        ret->vertices[i].position = tr * src->vertices[i].position;
    }

    ret->plane[3] = -ret->plane.dot(ret->vertices[0].position);
}


int Polygon_RayIntersect(const polygon_t *p, const btVector3& dir, const btVector3& dot, btScalar *t)
{
    btScalar u = p->plane.dot(dir);
    if(std::fabs(u) < 0.001 /*|| vec3_plane_dist(p->plane, dot) < -0.001*/)          // FIXME: magick
    {
        return 0;                                                               // plane is parallel to the ray - no intersection
    }
    *t = -planeDist(p->plane, dot) / u;

    auto vp = &p->vertices.front();                                                           // current polygon pointer
    btVector3 T = dot - vp[0].position;

    btVector3 E2 = vp[1].position - vp[0].position;
    for(size_t i=0; i<p->vertices.size()-2; i++,vp++)
    {
        btVector3 E1 = E2;                                                       // PREV
        E2 = vp[2].position - p->vertices[0].position;                   // NEXT

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


bool Polygon_IntersectPolygon(polygon_p p1, polygon_p p2)
{
    if(SPLIT_IN_BOTH != Polygon_SplitClassify(p1, p2->plane) || (SPLIT_IN_BOTH != Polygon_SplitClassify(p2, p1->plane)))
    {
        return false;                                                               // quick check
    }

    std::vector<btVector3> result_buf;

    /*
     * intersection of polygon p1 and plane p2
     */
    auto prev_v = &p1->vertices.back();
    auto curr_v = &p1->vertices.front();
    btScalar dist0 = planeDist(p2->plane, prev_v->position);
    for(size_t i=0; i<p1->vertices.size(); i++)
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
    dist0 = planeDist(p1->plane, prev_v->position);
    for(size_t i=0; i<p2->vertices.size(); i++)
    {
        btScalar dist1 = planeDist(p1->plane, curr_v->position);
        if(dist1 > SPLIT_EPSILON)
        {
            if(dist0 < -SPLIT_EPSILON)
            {
                auto dir = curr_v->position - prev_v->position;
                btScalar t;
                result_buf.emplace_back();
                rayPlaneIntersect(prev_v->position, dir, p1->plane, &result_buf.back(), &t);
            }
        }
        else if(dist1 < -SPLIT_EPSILON)
        {
            if(dist0 > SPLIT_EPSILON)
            {
                auto dir = curr_v->position - prev_v->position;
                btScalar t;
                result_buf.emplace_back();
                rayPlaneIntersect(prev_v->position, dir, p1->plane, &result_buf.back(), &t);
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

    auto dir = p1->plane.cross(p2->plane);                                      // vector of two planes intersection line
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


int Polygon_SplitClassify(polygon_p p, const btVector3& n)
{
    size_t positive=0, negative=0;
    for (const auto& v : p->vertices)
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
void Polygon_Split(polygon_p src, const btVector3& n, polygon_p front, polygon_p back)
{
    front->plane = src->plane;
    front->anim_id = src->anim_id;
    front->frame_offset = src->frame_offset;
    front->double_side = src->double_side;
    front->tex_index = src->tex_index;
    front->transparency = src->transparency;

    back->plane = src->plane;
    back->anim_id = src->anim_id;
    back->frame_offset = src->frame_offset;
    back->double_side = src->double_side;
    back->tex_index = src->tex_index;
    back->transparency = src->transparency;

    auto curr_v = &src->vertices.front();
    auto prev_v = &src->vertices.back();

    auto dist0 = planeDist(n, prev_v->position);
    for(size_t i=0; i<src->vertices.size(); ++i)
    {
        auto dist1 = planeDist(n, curr_v->position);

        if(dist1 > SPLIT_EPSILON)
        {
            if(dist0 < -SPLIT_EPSILON)
            {
                auto dir = curr_v->position - prev_v->position;
                btScalar t;
                vertex_s tv;
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
                vertex_s tv;
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


bool Polygon_IsInsideBBox(polygon_p p, const btVector3& bb_min, const btVector3& bb_max)
{
    for(const auto& v : p->vertices)
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


bool Polygon_IsInsideBQuad(polygon_p p, const btVector3& bb_min, const btVector3& bb_max)
{
    for(const auto& v : p->vertices)
    {
        if((v.position[0] < bb_min[0]) || (v.position[0] > bb_max[0]) ||
           (v.position[1] < bb_min[1]) || (v.position[1] > bb_max[1]))
        {
            return false;
        }
    }

    return true;
}
