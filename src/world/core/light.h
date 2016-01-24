#pragma once

#include "loader/datatypes.h"

namespace world
{
namespace core
{
struct Light
{
    glm::vec3 position;
    glm::vec4 color;

    glm::float_t inner;
    glm::float_t outer;
    glm::float_t length;
    glm::float_t cutoff;

    glm::float_t falloff;

    loader::LightType light_type;
};
} // namespace core
} // namespace world
