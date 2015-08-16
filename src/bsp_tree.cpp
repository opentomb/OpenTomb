#include "bsp_tree.h"

#include <cassert>
#include <cstdint>

#include <LinearMath/btScalar.h>

#include "frustum.h"
#include "mesh.h"
#include "polygon.h"
#include "vmath.h"

void DynamicBSP::addPolygon(std::unique_ptr<BSPNode>& root, const BSPFaceRef& face, const struct Polygon& transformed)
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
    for(const Vertex& v : transformed.vertices)
    {
        const auto dist = root->plane.distance(v.position);
        if(dist > SPLIT_EPSILON)
            positive++;
        else if(dist < -SPLIT_EPSILON)
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

void DynamicBSP::addNewPolygonList(const std::vector<TransparentPolygonReference>& p, const btTransform& transform, const Frustum& frustum, const Camera& cam)
{
    for(const TransparentPolygonReference& pp : p)
    {
        struct Polygon transformed;
        transformed.vertices.resize(pp.polygon->vertices.size());
        transformed.transform(*pp.polygon, transform);
        transformed.double_side = pp.polygon->double_side;

        if(frustum.isPolyVisible(&transformed, cam))
        {
            this->addPolygon(m_root, BSPFaceRef(transform, &pp), transformed);
        }
    }
}
