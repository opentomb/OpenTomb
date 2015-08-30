#pragma once

#include <array>
#include <memory>
#include <vector>

#include <LinearMath/btTransform.h>
#include <LinearMath/btScalar.h>
#include <LinearMath/btVector3.h>

#include "LuaState.h"

#include "world/object.h"
#include "world/animation/animation.h"
#include "render/vertex_array.h"
#include "loader/datatypes.h"
#include "orientedboundingbox.h"

class btCollisionShape;
class btRigidBody;
class btCollisionShape;

namespace engine
{
struct EngineContainer;
} // namespace engine

namespace render
{
class Render;
struct TransparentPolygonReference;
} // namespace render

namespace world
{
struct RoomSector;
struct SectorTween;
struct Room;
struct Entity;
struct Character;

namespace animation
{
enum class AnimUpdate;
} // namespace animation

namespace core
{

#define ANIM_CMD_MOVE               0x01
#define ANIM_CMD_CHANGE_DIRECTION   0x02
#define ANIM_CMD_JUMP               0x04


struct Polygon;
struct Vertex;

/*
 * base sprite structure
 */
struct Sprite
{
    uint32_t            id;                                                     // object's ID
    size_t              texture;
    GLfloat             tex_coord[8];
    uint32_t            flag;
    btScalar            left;                                                   // world sprite's gabarites
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
    std::unique_ptr< ::render::VertexArray > data{};

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

struct StaticMesh : public Object
{
    uint32_t                    object_id;
    uint8_t                     was_rendered;                                   // 0 - was not rendered, 1 - opaque, 2 - transparency, 3 - full rendered
    uint8_t                     was_rendered_lines;
    bool hide;
    btVector3 position;
    btVector3 rotation;
    std::array<float, 4> tint;

    BoundingBox visibleBoundingBox;
    BoundingBox collisionBoundingBox;

    btTransform transform;
    OrientedBoundingBox obb;
    std::shared_ptr<engine::EngineContainer> self;

    std::shared_ptr<BaseMesh> mesh;
    btRigidBody                *bt_body;
};

/*
 * mesh tree base element structure
 */
struct MeshTreeTag
{
    std::shared_ptr<BaseMesh> mesh_base;                                      // base mesh - pointer to the first mesh in array
    std::shared_ptr<BaseMesh> mesh_skin;                                      // base skinned mesh for ТR4+
    btVector3 offset;                                      // model position offset
    uint16_t                    flag;                                           // 0x0001 = POP, 0x0002 = PUSH, 0x0003 = RESET
    uint32_t                    body_part;
    uint8_t                     replace_mesh;                                   // flag for shoot / guns animations (0x00, 0x01, 0x02, 0x03)
    uint8_t                     replace_anim;
};

 /*
 * skeletal model with animations data.
 * Animated skeletal model. Taken from openraider.
 * model -> animation -> frame -> bone
 * thanks to Terry 'Mongoose' Hendrix II
 */
struct SkeletalModel
{
    uint32_t                    id;
    bool                        has_transparency;

    BoundingBox boundingBox;
    btVector3                   centre;

    std::vector<animation::AnimationFrame> animations;

    uint16_t                    mesh_count;
    std::vector<MeshTreeTag>    mesh_tree;                                      // base mesh tree.

    std::vector<uint16_t>       collision_map;

    void clear();
    void updateTransparencyFlag();
    void interpolateFrames();
    void fillSkinnedMeshMap();
};


MeshTreeTag* SkeletonClone(MeshTreeTag* src, int tags_count);
void SkeletonCopyMeshes(MeshTreeTag* dst, MeshTreeTag* src, int tags_count);
void SkeletonCopyMeshes2(MeshTreeTag* dst, MeshTreeTag* src, int tags_count);

btCollisionShape *BT_CSfromSphere(const btScalar& radius);
btCollisionShape* BT_CSfromBBox(const BoundingBox &boundingBox, bool useCompression, bool buildBvh);
btCollisionShape* BT_CSfromMesh(const std::shared_ptr<BaseMesh> &mesh, bool useCompression, bool buildBvh, bool is_static = true);
btCollisionShape* BT_CSfromHeightmap(const std::vector<RoomSector> &heightmap, const std::vector<SectorTween> &tweens, bool useCompression, bool buildBvh);

} // namespace core
} // namespace world
