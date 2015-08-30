#include "mesh.h"

#include <cstdlib>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "engine/engine.h"
#include "orientedboundingbox.h"
#include "polygon.h"
#include "render/gl_util.h"
#include "render/render.h"
#include "render/shader_description.h"
#include "util/vmath.h"
#include "world/core/basemesh.h"
#include "world/resource.h"
#include "world/room.h"
#include "world/world.h"

namespace world
{
namespace core
{

btCollisionShape *BT_CSfromSphere(const btScalar& radius)
{
    if(radius == 0.0) return nullptr;

    btCollisionShape* ret;

    ret = new btSphereShape(radius);
    ret->setMargin(COLLISION_MARGIN_RIGIDBODY);

    return ret;
}

btCollisionShape *BT_CSfromBBox(const BoundingBox &boundingBox, bool /*useCompression*/, bool /*buildBvh*/)
{
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;
    int cnt = 0;

    OrientedBoundingBox obb;
    obb.rebuild(boundingBox);
    for(const Polygon& p : obb.base_polygons)
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

///@TODO: resolve cases with floor >> ceiling (I.E. floor - ceiling >= 2048)
btCollisionShape *BT_CSfromHeightmap(const std::vector<RoomSector>& heightmap, const std::vector<SectorTween>& tweens, bool useCompression, bool buildBvh)
{
    uint32_t cnt = 0;
    std::shared_ptr<Room> r = heightmap.front().owner_room;
    btTriangleMesh *trimesh = new btTriangleMesh;

    for(uint32_t i = 0; i < r->sectors.size(); i++)
    {
        if((heightmap[i].floor_penetration_config != PenetrationConfig::Ghost) &&
           (heightmap[i].floor_penetration_config != PenetrationConfig::Wall))
        {
            if((heightmap[i].floor_diagonal_type == DiagonalType::None) ||
               (heightmap[i].floor_diagonal_type == DiagonalType::NW))
            {
                if(heightmap[i].floor_penetration_config != PenetrationConfig::DoorVerticalA)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[2],
                                         heightmap[i].floor_corners[0],
                                         true);
                    cnt++;
                }

                if(heightmap[i].floor_penetration_config != PenetrationConfig::DoorVerticalB)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[2],
                                         heightmap[i].floor_corners[1],
                                         heightmap[i].floor_corners[0],
                                         true);
                    cnt++;
                }
            }
            else
            {
                if(heightmap[i].floor_penetration_config != PenetrationConfig::DoorVerticalA)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[2],
                                         heightmap[i].floor_corners[1],
                                         true);
                    cnt++;
                }

                if(heightmap[i].floor_penetration_config != PenetrationConfig::DoorVerticalB)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[1],
                                         heightmap[i].floor_corners[0],
                                         true);
                    cnt++;
                }
            }
        }

        if((heightmap[i].ceiling_penetration_config != PenetrationConfig::Ghost) &&
           (heightmap[i].ceiling_penetration_config != PenetrationConfig::Wall))
        {
            if((heightmap[i].ceiling_diagonal_type == DiagonalType::None) ||
               (heightmap[i].ceiling_diagonal_type == DiagonalType::NW))
            {
                if(heightmap[i].ceiling_penetration_config != PenetrationConfig::DoorVerticalA)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[0],
                                         heightmap[i].ceiling_corners[2],
                                         heightmap[i].ceiling_corners[3],
                                         true);
                    cnt++;
                }

                if(heightmap[i].ceiling_penetration_config != PenetrationConfig::DoorVerticalB)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[0],
                                         heightmap[i].ceiling_corners[1],
                                         heightmap[i].ceiling_corners[2],
                                         true);
                    cnt++;
                }
            }
            else
            {
                if(heightmap[i].ceiling_penetration_config != PenetrationConfig::DoorVerticalA)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[0],
                                         heightmap[i].ceiling_corners[1],
                                         heightmap[i].ceiling_corners[3],
                                         true);
                    cnt++;
                }

                if(heightmap[i].ceiling_penetration_config != PenetrationConfig::DoorVerticalB)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[1],
                                         heightmap[i].ceiling_corners[2],
                                         heightmap[i].ceiling_corners[3],
                                         true);
                    cnt++;
                }
            }
        }
    }

    for(const SectorTween& tween : tweens)
    {
        switch(tween.ceiling_tween_type)
        {
            case TweenType::TwoTriangles:
                {
                    btScalar t = std::abs((tween.ceiling_corners[2][2] - tween.ceiling_corners[3][2]) /
                                          (tween.ceiling_corners[0][2] - tween.ceiling_corners[1][2]));
                    t = 1.0f / (1.0f + t);
                    btVector3 o;
                    o.setInterpolate3(tween.ceiling_corners[0], tween.ceiling_corners[2], t);
                    trimesh->addTriangle(tween.ceiling_corners[0],
                                         tween.ceiling_corners[1],
                                         o, true);
                    trimesh->addTriangle(tween.ceiling_corners[3],
                                         tween.ceiling_corners[2],
                                         o, true);
                    cnt += 2;
                }
                break;

            case TweenType::TriangleLeft:
                trimesh->addTriangle(tween.ceiling_corners[0],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                cnt++;
                break;

            case TweenType::TriangleRight:
                trimesh->addTriangle(tween.ceiling_corners[2],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                cnt++;
                break;

            case TweenType::Quad:
                trimesh->addTriangle(tween.ceiling_corners[0],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                trimesh->addTriangle(tween.ceiling_corners[2],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                cnt += 2;
                break;
        };

        switch(tween.floor_tween_type)
        {
            case TweenType::TwoTriangles:
            {
                btScalar t = std::abs((tween.floor_corners[2][2] - tween.floor_corners[3][2]) /
                                      (tween.floor_corners[0][2] - tween.floor_corners[1][2]));
                t = 1.0f / (1.0f + t);
                btVector3 o;
                o.setInterpolate3(tween.floor_corners[0], tween.floor_corners[2], t);
                trimesh->addTriangle(tween.floor_corners[0],
                                     tween.floor_corners[1],
                                     o, true);
                trimesh->addTriangle(tween.floor_corners[3],
                                     tween.floor_corners[2],
                                     o, true);
                cnt += 2;
            }
            break;

            case TweenType::TriangleLeft:
                trimesh->addTriangle(tween.floor_corners[0],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                cnt++;
                break;

            case TweenType::TriangleRight:
                trimesh->addTriangle(tween.floor_corners[2],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                cnt++;
                break;

            case TweenType::Quad:
                trimesh->addTriangle(tween.floor_corners[0],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                trimesh->addTriangle(tween.floor_corners[2],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                cnt += 2;
                break;
        };
    }

    if(cnt == 0)
    {
        delete trimesh;
        return nullptr;
    }

    auto ret = new btBvhTriangleMeshShape(trimesh, useCompression, buildBvh);
    ret->setMargin(COLLISION_MARGIN_RIGIDBODY);
    return ret;
}

} // namespace core
} // namespace world
