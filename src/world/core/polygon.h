#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <LinearMath/btScalar.h>
#include <GL/glew.h>

#include "util/vmath.h"
#include "loader/datatypes.h"

namespace world
{
namespace core
{

#define SPLIT_FRONT    0x00
#define SPLIT_BACK     0x01
#define SPLIT_IN_PLANE 0x02
#define SPLIT_IN_BOTH  0x03

#define SPLIT_EPSILON (0.02)

/*
 * The structure taken from Cochrane. Next I realise one in my style.
 * make it aligned... is it enough good?
 */
struct Vertex
{
    btVector3 position = { 0,0,0 };
    btVector3 normal = { 0,0,0 };
    std::array<GLfloat, 4> color{ {0,0,0,0} };
    std::array<GLfloat, 2> tex_coord{ {0,0} };
};

struct Polygon
{
    std::vector<Vertex> vertices{ 4 };                                              // vertices data
    uint16_t            tex_index = 0;                                              // texture index
    uint16_t            anim_id = 0;                                                // anim texture ID
    uint16_t            frame_offset = 0;                                           // anim texture frame offset
    loader::BlendingMode blendMode = loader::BlendingMode::Opaque;                  // transparency information
    bool                double_side = false;                                        // double side flag
    util::Plane plane;                                               // polygon plane equation

    Polygon() = default;

    Polygon(const Polygon& rhs)
        : vertices(rhs.vertices)
        , tex_index(rhs.tex_index)
        , anim_id(rhs.anim_id)
        , frame_offset(rhs.frame_offset)
        , blendMode(rhs.blendMode)
        , double_side(rhs.double_side)
        , plane(rhs.plane)
    {
    }

    Polygon& operator=(const Polygon& rhs)
    {
        vertices = rhs.vertices;
        tex_index = rhs.tex_index;
        anim_id = rhs.anim_id;
        frame_offset = rhs.frame_offset;
        blendMode = rhs.blendMode;
        double_side = rhs.double_side;
        plane = rhs.plane;
        // keep next
        return *this;
    }

    bool isBroken() const;

    void moveSelf(const btVector3 &move);
    void move(Polygon* src, const btVector3 &move);
    void vTransform(Polygon* src, const btTransform &tr);
    void transform(const Polygon &src, const btTransform &tr);
    void transformSelf(const btTransform &tr);

    void findNormal();
    bool rayIntersect(const btVector3 &rayDir, const btVector3 &dot, btScalar *lambda) const;
    bool intersectPolygon(Polygon* p2);

    int  splitClassify(const util::Plane &plane);
    void split(const util::Plane &n, Polygon* front, Polygon* back);

    bool isInsideBBox(const btVector3 &bb_min, const btVector3 &bb_max);
    bool isInsideBQuad(const btVector3 &bb_min, const btVector3 &bb_max);
};

} // namespace core
} // namespace world
