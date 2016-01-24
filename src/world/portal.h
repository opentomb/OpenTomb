#pragma once

#include "loader/datatypes.h"
#include "util/vmath.h"

#include <array>
#include <numeric>

namespace world
{
struct Room;

struct Portal
{
    Portal(const loader::Portal& portal, Room* src, Room* dest, const glm::mat4& transform)
        : vertices{ {} }
        , normal{ util::convert(portal.normal) }
        , center{}
        , source{ src }
        , destination{ dest }
    {
        for(int j = 0; j < 4; ++j)
        {
            vertices[j] = util::convert(portal.vertices[j]) + glm::vec3(transform[3]);
        }
        center = std::accumulate(vertices.begin(), vertices.end(), glm::vec3(0, 0, 0)) / static_cast<glm::float_t>(vertices.size());
    }

    std::array<glm::vec3, 4> vertices;
    glm::vec3 normal;
    glm::vec3 center = { 0,0,0 };
    Room* source = nullptr;
    Room* destination = nullptr;
};
} // namespace world
