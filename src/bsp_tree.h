#pragma once

#include <memory>
#include <vector>

#include "vmath.h"

class Camera;
struct Polygon;
struct Frustum;
struct TransparentPolygonReference;

struct BSPFaceRef
{
    btTransform transform;
    const TransparentPolygonReference* polygon;

    BSPFaceRef(const btTransform& matrix, const TransparentPolygonReference* polygon)
        : transform(matrix)
        , polygon(polygon)
    {
    }
};

struct BSPNode
{
    Plane plane;

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

    void addPolygon(std::unique_ptr<BSPNode> &root, const BSPFaceRef &p, const struct Polygon &transformed);

public:
    void addNewPolygonList(const std::vector<TransparentPolygonReference> &p, const btTransform &transform, const std::vector<std::shared_ptr<Frustum> > &f, const Camera& cam);

    const std::unique_ptr<BSPNode>& root() const
    {
        return m_root;
    }

    void reset()
    {
        m_root.reset(new BSPNode());
    }
};
