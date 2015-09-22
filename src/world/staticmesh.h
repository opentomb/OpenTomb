#pragma once

#include "core/basemesh.h"
#include "core/orientedboundingbox.h"
#include "object.h"

namespace world
{

struct StaticMesh : public Object
{
    explicit StaticMesh(uint32_t id, Room* room = nullptr)
        : Object(id, room)
    {
    }

    uint8_t                     was_rendered;                                   // 0 - was not rendered, 1 - opaque, 2 - transparency, 3 - full rendered
    uint8_t                     was_rendered_lines;
    bool hide;
    btVector3 position;
    btVector3 rotation;
    std::array<float, 4> tint;

    core::BoundingBox visibleBoundingBox;
    core::BoundingBox collisionBoundingBox;

    btTransform transform;
    core::OrientedBoundingBox obb;

    std::shared_ptr<core::BaseMesh> mesh;
    btRigidBody* bt_body = nullptr;
};

} // namespace world
