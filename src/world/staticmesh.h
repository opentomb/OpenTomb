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

    uint8_t                     was_rendered = 0;                                   // 0 - was not rendered, 1 - opaque, 2 - transparency, 3 - full rendered
    uint8_t                     was_rendered_lines = 0;
    bool hide = true;
    glm::vec3 position = { 0,0,0 };
    glm::vec3 rotation = { 0,0,0 };
    std::array<float, 4> tint{{ 0,0,0,0 }};

    core::BoundingBox visibleBoundingBox;
    core::BoundingBox collisionBoundingBox;

    glm::mat4 transform{ 1.0f };
    core::OrientedBoundingBox obb;

    std::shared_ptr<core::BaseMesh> mesh = nullptr;
    btRigidBody* bt_body = nullptr;
};

} // namespace world
