#include "bsp_tree.h"

#include <cassert>
#include <cstdint>

#include <LinearMath/btScalar.h>

#include "world/core/frustum.h"
#include "world/core/mesh.h"
#include "world/core/polygon.h"
#include "util/vmath.h"

namespace render
{

void DynamicBSP::addPolygon(std::unique_ptr<BSPNode>& root, const BSPFaceRef& face, const world::core::Polygon& transformed)
{
    if(!root)
        root.reset(new BSPNode());

    if(root->polygons_front.empty())
    {
        // we though root->front == NULL and root->back == NULL
        root->plane = transformed.plane;
        root->polygons_front = { face };
        return;
    }

    size_t positive = 0;
    size_t negative = 0;
    size_t in_plane = 0;
    for(const world::core::Vertex& v : transformed.vertices)
    {
        const auto dist = root->plane.distance(v.position);
        if(dist > world::core::SplitEpsilon)
            positive++;
        else if(dist < -world::core::SplitEpsilon)
            negative++;
        else
            in_plane++;
    }

    if(positive > 0 && negative == 0)                   // SPLIT_FRONT
    {
        addPolygon(root->front, face, transformed);
    }
    else if((positive == 0) && (negative > 0))              // SPLIT_BACK
    {
        addPolygon(root->back, face, transformed);
    }
    else //((positive == 0) && (negative == 0))             // SPLIT_IN_PLANE
    {
        if(transformed.plane.normal.dot(root->plane.normal) > 0.9)
        {
            root->polygons_front.insert(root->polygons_front.end(), face);
        }
        else
        {
            root->polygons_back.insert(root->polygons_back.end(), face);
        }
    }
}

void DynamicBSP::addNewPolygonList(const std::vector<world::core::TransparentPolygonReference>& p, const btTransform& transform, const world::core::Frustum& frustum, const world::Camera& cam)
{
    for(const world::core::TransparentPolygonReference& pp : p)
    {
        world::core::Polygon transformed;
        transformed.vertices.resize(pp.polygon->vertices.size());
        transformed.copyTransformed(*pp.polygon, transform, true);
        transformed.double_side = pp.polygon->double_side;

        if(frustum.isPolyVisible(transformed, cam))
        {
            this->addPolygon(m_root, BSPFaceRef(transform, &pp), transformed);
        }
    }
}

} // namespace render
