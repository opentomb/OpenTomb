#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <LinearMath/btScalar.h>

#include <GL/glew.h>

#include "loader/datatypes.h"
#include "util/vmath.h"

namespace world
{
namespace core
{

enum class SplitType
{
    Front,
    Back,
    InPlane,
    InBoth
};

namespace
{
constexpr float SplitEpsilon = 0.02f;
}

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

    bool isBroken() const;

    void move(const btVector3 &copyMoved);
    void copyMoved(const Polygon &src, const btVector3 &copyMoved);
    void copyTransformed(const Polygon &src, const btTransform &tr, bool copyNormals = false);
    void transform(const btTransform &tr);

    void updateNormal();
    bool rayIntersect(const btVector3 &rayDir, const btVector3 &dot, btScalar *lambda) const;
    bool intersectPolygon(Polygon* p2);

    SplitType splitClassify(const util::Plane &plane);
    void split(const util::Plane &n, Polygon* front, Polygon* back);

    bool isInsideBBox(const btVector3 &bb_min, const btVector3 &bb_max);
    bool isInsideBQuad(const btVector3 &bb_min, const btVector3 &bb_max);
};

} // namespace core
} // namespace world
