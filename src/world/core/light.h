#pragma once

#include "loader/datatypes.h"

namespace world
{
namespace core
{

struct Light
{
    glm::vec3 position;
    float                       colour[4];

    float                       inner;
    float                       outer;
    float                       length;
    float                       cutoff;

    float                       falloff;

    loader::LightType           light_type;
};

} // namespace core
} // namespace world
