#include "boundingbox.h"

#include "engine/engine.h"
#include "orientedboundingbox.h"

#include <BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>

namespace world
{
namespace core
{

btCollisionShape *BT_CSfromBBox(const BoundingBox &boundingBox, bool /*useCompression*/, bool /*buildBvh*/)
{
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;
    int cnt = 0;

    OrientedBoundingBox obb;
    obb.rebuild(boundingBox);
    for(const core::Polygon& p : obb.base_polygons)
    {
        if(p.isBroken())
        {
            continue;
        }
        for(size_t j = 1; j + 1 < p.vertices.size(); j++)
        {
            const auto& v0 = p.vertices[j + 1].position;
            const auto& v1 = p.vertices[j].position;
            const auto& v2 = p.vertices[0].position;
            trimesh->addTriangle(v0, v1, v2, true);
        }
        cnt++;
    }

    if(cnt == 0)                                                                // fixed: without that condition engine may easily crash
    {
        delete trimesh;
        return nullptr;
    }

    ret = new btConvexTriangleMeshShape(trimesh, true);
    ret->setMargin(COLLISION_MARGIN_RIGIDBODY);

    return ret;
}

} // namespace core
} // namespace world
