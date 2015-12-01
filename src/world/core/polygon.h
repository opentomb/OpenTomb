#pragma once

#include <cstdint>
#include <vector>

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

struct BoundingBox;

/*
 * The structure taken from Cochrane. Next I realise one in my style.
 * make it aligned... is it enough good?
 */
struct Vertex
{
    glm::vec3 position = { 0,0,0 };
    glm::vec3 normal = { 0,0,0 };
    glm::vec4 color = {0,0,0,0};
    glm::vec2 tex_coord = {0,0};
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

    void move(const glm::vec3 &copyMoved);
    void copyMoved(const Polygon &src, const glm::vec3 &copyMoved);
    void copyTransformed(const Polygon &src, const glm::mat4 &tr, bool copyNormals = false);
    void transform(const glm::mat4 &tr);

    void updateNormal();
    bool rayIntersect(const glm::vec3 &rayDir, const glm::vec3 &dot, glm::float_t *lambda) const;
    bool intersectPolygon(Polygon* p2);

    SplitType splitClassify(const util::Plane &plane);
    void split(const util::Plane &n, Polygon* front, Polygon* back);

    bool isInsideBBox(const BoundingBox& bb) const;
    bool isInsideBQuad(const BoundingBox& bb) const;
};

} // namespace core
} // namespace world