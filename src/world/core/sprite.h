#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "render/vertex_array.h"

namespace render
{
class VertexArray;
}

namespace world
{
namespace core
{
using SpriteId = uint32_t;
/*
 * base sprite structure
 */
struct Sprite
{
    SpriteId            id;
    size_t              texture;
    glm::vec2           tex_coord[4];
    glm::float_t        left;
    glm::float_t        right;
    glm::float_t        top;
    glm::float_t        bottom;
};

/*
 * Structure for all the sprites in a room
 */
struct SpriteBuffer
{
    //! Vertex data for the sprites
    std::unique_ptr<render::VertexArray> data{};

    //! How many sub-ranges the element_array_buffer contains. It has one for each texture listed.
    size_t num_texture_pages = 0;
    //! The element count for each sub-range.
    std::vector<size_t> element_count_per_texture{};
};
} // namespace core
} // namespace world
