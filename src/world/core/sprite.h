#pragma once

#include <memory>
#include <vector>

#include <GL/glew.h>
#include <LinearMath/btScalar.h>

#include "render/vertex_array.h"

namespace render
{
class VertexArray;
}

namespace world
{
namespace core
{

/*
 * base sprite structure
 */
struct Sprite
{
    uint32_t            id;
    size_t              texture;
    GLfloat             tex_coord[8];
    uint32_t            flag;
    btScalar            left;
    btScalar            right;
    btScalar            top;
    btScalar            bottom;
};

/*
 * Structure for all the sprites in a room
 */
struct SpriteBuffer
{
    //! Vertex data for the sprites
    std::unique_ptr< render::VertexArray > data{};

    //! How many sub-ranges the element_array_buffer contains. It has one for each texture listed.
    size_t num_texture_pages = 0;
    //! The element count for each sub-range.
    std::vector<size_t> element_count_per_texture{};
};

} // namespace core
} // namespace world
