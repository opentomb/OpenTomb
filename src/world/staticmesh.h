#pragma once

#include "core/basemesh.h"
#include "core/orientedboundingbox.h"
#include "object.h"

namespace world
{
struct StaticMesh : public Object
{
    explicit StaticMesh(ObjectId id, Room* room = nullptr)
        : Object(id, room)
    {
    }

    bool was_rendered = false;
    bool was_rendered_lines = false;
    bool hide = true;
    glm::vec3 position = { 0,0,0 };
    glm::vec3 rotation = { 0,0,0 };
    glm::vec4 tint = { 0,0,0,0 };

    core::BoundingBox visibleBoundingBox;
    core::BoundingBox collisionBoundingBox;

    glm::mat4 transform{ 1.0f };
    core::OrientedBoundingBox obb;

    std::shared_ptr<core::BaseMesh> mesh = nullptr;
    btRigidBody* bt_body = nullptr;
};
} // namespace world
