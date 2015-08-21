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
struct Frustum;
struct TransparentPolygonReference;
} // namespace core
} // namespace world

namespace render
{

struct BSPFaceRef
{
    btTransform transform;
    const world::core::TransparentPolygonReference* polygon;

    BSPFaceRef(const btTransform& matrix, const world::core::TransparentPolygonReference* polygon)
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
    void addNewPolygonList(const std::vector<world::core::TransparentPolygonReference> &p, const btTransform &transform, const world::core::Frustum& f, const world::Camera& cam);

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
