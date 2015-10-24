#pragma once

#include <memory>
#include <vector>

#include "util/vmath.h"

namespace world
{
class Camera;

namespace core
{
struct Polygon;
class Frustum;
} // namespace core
} // namespace world

namespace render
{
struct TransparentPolygonReference;

struct BSPFaceRef
{
    glm::mat4 transform;
    const TransparentPolygonReference* polygon;

    BSPFaceRef(const glm::mat4& matrix, const TransparentPolygonReference* polygon)
        : transform(matrix)
        , polygon(polygon)
    {
    }
};

struct BSPNode
{
    util::Plane plane;

    std::vector<BSPFaceRef> polygons_front;
    std::vector<BSPFaceRef> polygons_back;

    std::unique_ptr<BSPNode> front;
    std::unique_ptr<BSPNode> back;
};

/**
 * Warning! that class has too primitive and rough (but fast) memory allocation space check! Maybe I will fix it in future;
 */
class DynamicBSP
{
private:
    std::unique_ptr<BSPNode> m_root{ new BSPNode() };

    void addPolygon(std::unique_ptr<BSPNode> &root, const BSPFaceRef &p, const struct world::core::Polygon &transformed);

public:
    void addNewPolygonList(const std::vector<TransparentPolygonReference> &p, const glm::mat4 &transform, const world::Camera& cam);

    const std::unique_ptr<BSPNode>& root() const
    {
        return m_root;
    }

    void reset()
    {
        m_root.reset(new BSPNode());
    }
};

} // namespace render
