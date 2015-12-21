#include "polygon.h"

#include <cmath>

#include "util/vmath.h"
#include "boundingbox.h"

namespace world
{
namespace core
{

/*
 * POLYGONS
 */

bool Polygon::isBroken() const
{
    if(vertices.size() < 3)
    {
        return true;
    }

    glm::float_t dif0 = glm::dot(plane.normal, plane.normal);
    if(dif0 < 0.999 || dif0 > 1.001)
    {
        return true;
    }

    auto curr_v = &vertices.back();
    for(const auto& v : vertices)
    {
        glm::vec3 dif = v.position - curr_v->position;
        if(glm::dot(dif,dif) < 0.0001)
        {
            return true;
        }

        curr_v = &v;
    }

    return false;
}

void Polygon::updateNormal()
{
    auto v1 = vertices[0].position - vertices[1].position;
    auto v2 = vertices[2].position - vertices[1].position;
    plane.assign(v1, v2, { 0,0,0 });
}

void Polygon::move(const glm::vec3& move)
{
    for(auto& v : vertices)
    {
        v.position += move;
    }
    plane.moveTo(vertices[0].position);
}

void Polygon::copyMoved(const Polygon& src, const glm::vec3& move)
{
    for(size_t i = 0; i < src.vertices.size(); i++)
    {
        vertices[i].position = src.vertices[i].position + move;
    }

    plane = src.plane;
    plane.moveTo(vertices[0].position);
}

void Polygon::transform(const glm::mat4& tr)
{
    plane.normal = glm::mat3(tr) * plane.normal;
    for(Vertex& vp : vertices)
    {
        vp.position = glm::vec3( tr * glm::vec4(vp.position, 1.0f) );
        vp.normal = glm::mat3(tr) * vp.normal;
    }

    plane.moveTo(vertices[0].position);
}

void Polygon::copyTransformed(const Polygon& src, const glm::mat4& tr, bool copyNormals)
{
    plane.normal = glm::mat3(tr) * src.plane.normal;
    for(size_t i = 0; i < src.vertices.size(); i++)
    {
        vertices[i].position = glm::vec3( tr * glm::vec4(src.vertices[i].position, 1) );
        if(copyNormals)
            vertices[i].normal = glm::mat3(tr) * src.vertices[i].normal;
    }

    plane.moveTo(vertices[0].position);
}

bool Polygon::rayIntersect(const glm::vec3& rayDir, const glm::vec3& dot, glm::float_t& lambda) const
{
    glm::float_t u = glm::dot(plane.normal, rayDir);
    if(glm::abs(u) < 0.001 /*|| vec3_plane_dist(plane, dot) < -0.001*/)          // FIXME: magick
    {
        return false;    // plane is parallel to the ray - no intersection
    }
    lambda = -plane.distance(dot) / u;

    auto vp = &vertices.front();           // current polygon pointer
    glm::vec3 T = dot - vp[0].position;

    glm::vec3 E2 = vp[1].position - vp[0].position;
    for(size_t i = 0; i < vertices.size() - 2; i++, vp++)
    {
        glm::vec3 E1 = E2;                           // PREV
        E2 = vp[2].position - vertices[0].position;  // NEXT

        glm::vec3 P = glm::cross(rayDir, E2);
        glm::vec3 Q = glm::cross(T, E1);

        glm::float_t tt = glm::dot(P, E1);
        u = glm::dot(P, T);
        u /= tt;
        glm::float_t v = glm::dot(Q, rayDir);
        v /= tt;
        tt = 1.0f - u - v;
        if(u <= 1.0 && u >= 0.0 && v <= 1.0 && v >= 0.0 && tt <= 1.0 && tt >= 0.0)
        {
            return true;
        }
    }
    return false;
}

bool Polygon::intersectPolygon(const Polygon& p2)
{
    if(SplitType::InBoth != splitClassify(p2.plane) || SplitType::InBoth != p2.splitClassify(plane))
    {
        return false;  // quick check
    }

    std::vector<glm::vec3> result_buf;

    /*
     * intersection of polygon p1 and plane p2
     */
    const Vertex* prev_v = &vertices.back();
    const Vertex* curr_v = &vertices.front();
    glm::float_t dist0 = p2.plane.distance(prev_v->position);
    for(size_t i = 0; i < vertices.size(); i++)
    {
        glm::float_t dist1 = p2.plane.distance(curr_v->position);
        if(dist1 > SplitEpsilon)
        {
            if(dist0 < -SplitEpsilon)
            {
                result_buf.emplace_back(p2.plane.rayIntersect(prev_v->position,
                                                               curr_v->position - prev_v->position));
            }
        }
        else if(dist1 < -SplitEpsilon)
        {
            if(dist0 > SplitEpsilon)
            {
                result_buf.emplace_back(p2.plane.rayIntersect(prev_v->position,
                                                               curr_v->position - prev_v->position));
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
    prev_v = &p2.vertices.back();
    curr_v = &p2.vertices.front();
    dist0 = plane.distance(prev_v->position);
    for(size_t i = 0; i < p2.vertices.size(); i++)
    {
        glm::float_t dist1 = plane.distance(curr_v->position);
        if(dist1 > SplitEpsilon)
        {
            if(dist0 < -SplitEpsilon)
            {
                result_buf.emplace_back(plane.rayIntersect(prev_v->position,
                                                           curr_v->position - prev_v->position));
            }
        }
        else if(dist1 < -SplitEpsilon)
        {
            if(dist0 > SplitEpsilon)
            {
                result_buf.emplace_back(plane.rayIntersect(prev_v->position,
                                                           curr_v->position - prev_v->position));
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
        curr_v++;
    }

    auto dir = glm::cross(plane.normal, p2.plane.normal);  // vector of two planes intersection line
    glm::float_t t = glm::abs(dir[0]);
    dist0 = glm::abs(dir[1]);
    glm::float_t dist1 = glm::abs(dir[2]);
    glm::float_t dist2 = 0;
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
            dist0 = (result_buf[1][0] - result_buf[0][0]) / dir[0];
            dist1 = (result_buf[2][0] - result_buf[0][0]) / dir[0];
            dist2 = (result_buf[3][0] - result_buf[0][0]) / dir[0];
            break;

        case PLANE_Y:
            dist0 = (result_buf[1][1] - result_buf[0][1]) / dir[1];
            dist1 = (result_buf[2][1] - result_buf[0][1]) / dir[1];
            dist2 = (result_buf[3][1] - result_buf[0][1]) / dir[1];
            break;

        case PLANE_Z:
            dist0 = (result_buf[1][2] - result_buf[0][2]) / dir[2];
            dist1 = (result_buf[2][2] - result_buf[0][2]) / dir[2];
            dist2 = (result_buf[3][2] - result_buf[0][2]) / dir[2];
            break;
    };

    if(dist0 > 0)
    {
        return !((dist1 < 0.0 && dist2 < 0.0) || (dist1 > dist0 && dist2 > dist0));
    }
    return !((dist1 < dist0 && dist2 < dist0) || (dist1 > 0.0 && dist2 > 0.0));
}

SplitType Polygon::splitClassify(const util::Plane& plane) const
{
    size_t positive = 0, negative = 0;
    for(const auto& v : vertices)
    {
        auto dist = plane.distance(v.position);
        if(dist > SplitEpsilon)
        {
            positive++;
        }
        else if(dist < -SplitEpsilon)
        {
            negative++;
        }
    }

    if(positive > 0 && negative == 0)
    {
        return SplitType::Front;
    }
    else if(positive == 0 && negative > 0)
    {
        return SplitType::Back;
    }
    else if(positive < 1 && negative < 1)
    {
        return SplitType::InPlane;
    }

    return SplitType::InBoth;
}

/*
 * animated textures coordinates splits too!
 */
void Polygon::split(const util::Plane& n, Polygon& front, Polygon& back)
{
    front.plane = plane;
    front.anim_id = anim_id;
    front.frame_offset = frame_offset;
    front.double_side = double_side;
    front.tex_index = tex_index;
    front.blendMode = blendMode;

    back.plane = plane;
    back.anim_id = anim_id;
    back.frame_offset = frame_offset;
    back.double_side = double_side;
    back.tex_index = tex_index;
    back.blendMode = blendMode;

    auto curr_v = &vertices.front();
    auto prev_v = &vertices.back();

    auto dist0 = n.distance(prev_v->position);
    for(size_t i = 0; i < vertices.size(); ++i)
    {
        auto dist1 = n.distance(curr_v->position);

        if(dist1 > SplitEpsilon)
        {
            if(dist0 < -SplitEpsilon)
            {
                auto dir = curr_v->position - prev_v->position;
                glm::float_t t;
                Vertex tv;
                tv.position = n.rayIntersect(prev_v->position, dir, t);
                tv.normal = glm::normalize(glm::mix(prev_v->normal, curr_v->normal, t));

                tv.color[0] = prev_v->color[0] + t * (curr_v->color[0] - prev_v->color[0]);
                tv.color[1] = prev_v->color[1] + t * (curr_v->color[1] - prev_v->color[1]);
                tv.color[2] = prev_v->color[2] + t * (curr_v->color[2] - prev_v->color[2]);
                tv.color[3] = prev_v->color[3] + t * (curr_v->color[3] - prev_v->color[3]);

                tv.tex_coord[0] = prev_v->tex_coord[0] + t * (curr_v->tex_coord[0] - prev_v->tex_coord[0]);
                tv.tex_coord[1] = prev_v->tex_coord[1] + t * (curr_v->tex_coord[1] - prev_v->tex_coord[1]);

                front.vertices.emplace_back(tv);
                back.vertices.emplace_back(tv);
            }
            front.vertices.emplace_back(*curr_v);
        }
        else if(dist1 < -SplitEpsilon)
        {
            if(dist0 > SplitEpsilon)
            {
                auto dir = curr_v->position - prev_v->position;
                glm::float_t t;
                Vertex tv;
                tv.position = n.rayIntersect(prev_v->position, dir, t);
                tv.normal = glm::normalize(glm::mix(prev_v->normal, curr_v->normal, t));

                tv.color[0] = prev_v->color[0] + t * (curr_v->color[0] - prev_v->color[0]);
                tv.color[1] = prev_v->color[1] + t * (curr_v->color[1] - prev_v->color[1]);
                tv.color[2] = prev_v->color[2] + t * (curr_v->color[2] - prev_v->color[2]);
                tv.color[3] = prev_v->color[3] + t * (curr_v->color[3] - prev_v->color[3]);

                tv.tex_coord[0] = prev_v->tex_coord[0] + t * (curr_v->tex_coord[0] - prev_v->tex_coord[0]);
                tv.tex_coord[1] = prev_v->tex_coord[1] + t * (curr_v->tex_coord[1] - prev_v->tex_coord[1]);

                front.vertices.emplace_back(tv);
                back.vertices.emplace_back(tv);
            }
            back.vertices.emplace_back(*curr_v);
        }
        else
        {
            front.vertices.emplace_back(*curr_v);
            back.vertices.emplace_back(*curr_v);
        }

        prev_v = curr_v;
        curr_v++;
        dist0 = dist1;
    }
}

bool Polygon::isInsideBBox(const BoundingBox& bb) const
{
    for(const auto& v : vertices)
    {
        if(v.position[0] < bb.min[0] || v.position[0] > bb.max[0] ||
           v.position[1] < bb.min[1] || v.position[1] > bb.max[1] ||
           v.position[2] < bb.min[2] || v.position[2] > bb.max[2])
        {
            return 0;
        }
    }

    return 1;
}

bool Polygon::isInsideBQuad(const BoundingBox& bb) const
{
    for(const auto& v : vertices)
    {
        if(v.position[0] < bb.min[0] || v.position[0] > bb.max[0] ||
           v.position[1] < bb.min[1] || v.position[1] > bb.max[1])
        {
            return false;
        }
    }

    return true;
}

} // namespace core
} // namespace world
