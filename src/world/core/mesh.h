#pragma once

#include <array>
#include <memory>
#include <vector>

#include <LinearMath/btVector3.h>

#include <GL/glew.h>

#include "loader/datatypes.h"
#include "world/animation/animation.h"

class btCollisionShape;

namespace render
{
class VertexArray;
}

namespace world
{
struct RoomSector;
struct SectorTween;

namespace core
{

struct Vertex;

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

struct Light
{
    btVector3 position;
    float                       colour[4];

    float                       inner;
    float                       outer;
    float                       length;
    float                       cutoff;

    float                       falloff;

    loader::LightType           light_type;
};

btCollisionShape *BT_CSfromSphere(const btScalar& radius);
btCollisionShape* BT_CSfromBBox(const BoundingBox &boundingBox, bool useCompression, bool buildBvh);
btCollisionShape* BT_CSfromHeightmap(const std::vector<RoomSector> &heightmap, const std::vector<SectorTween> &tweens, bool useCompression, bool buildBvh);

} // namespace core
} // namespace world
