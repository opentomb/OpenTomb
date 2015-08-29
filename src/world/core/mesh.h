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
 * base mesh, uses everywhere
 */
struct BaseMesh
{
    uint32_t m_id;                                                   // mesh's ID
    bool m_usesVertexColors;                                   // does this mesh have prebaked vertex lighting

    std::vector<Polygon> m_polygons;                                             // polygons data

    std::vector<Polygon> m_transparencyPolygons;                                // transparency mesh's polygons list

    uint32_t              m_texturePageCount;                                    // face without structure wrapping
    std::vector<uint32_t> m_elementsPerTexture;                            //
    std::vector<GLuint> m_elements;                                             //
    uint32_t m_alphaElements;

    std::vector<Vertex> m_vertices;

    size_t m_animatedElementCount;
    size_t m_alphaAnimatedElementCount;
    std::vector<GLuint> m_allAnimatedElements;
    std::vector<animation::AnimatedVertex> m_animatedVertices;

    std::vector<render::TransparentPolygonReference> m_transparentPolygons;

    btVector3 m_center;                                            // geometry centre of mesh
    BoundingBox boundingBox; //!< AABB bounding volume
    btScalar m_radius;                                            // radius of the bounding sphere
#pragma pack(push,1)
    struct MatrixIndex
    {
        int8_t i = 0, j = 0;
    };
#pragma pack(pop)
    std::vector<MatrixIndex> m_matrixIndices;                                       // vertices map for skin mesh

    GLuint                m_vboVertexArray = 0;
    GLuint                m_vboIndexArray = 0;
    GLuint                m_vboSkinArray = 0;
    std::shared_ptr< ::render::VertexArray > m_mainVertexArray;

    // Buffers for animated polygons
    // The first contains position, normal and color.
    // The second contains the texture coordinates. It gets updated every frame.
    GLuint                m_animatedVboVertexArray;
    GLuint                m_animatedVboTexCoordArray;
    GLuint                m_animatedVboIndexArray;
    std::shared_ptr< ::render::VertexArray > m_animatedVertexArray;

    ~BaseMesh();

    void clear();
    void updateBoundingBox();
    void genVBO(const render::Render *renderer);
    void genFaces();
    uint32_t addVertex(const Vertex& v);
    uint32_t addAnimatedVertex(const Vertex& v);
    void polySortInMesh();
    Vertex* findVertex(const btVector3& v);
};

/*
 * base sprite structure
 */
struct Sprite
{
    uint32_t            id;                                                     // object's ID
    size_t              texture;                                                // texture number
    GLfloat             tex_coord[8];                                           // texture coordinates
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
    // Vertex data for the sprites
    std::unique_ptr< ::render::VertexArray > data{};

    // How many sub-ranges the element_array_buffer contains. It has one for each texture listed.
    uint32_t              num_texture_pages = 0;
    // The element count for each sub-range.
    std::vector<uint32_t> element_count_per_texture{};
};

struct Light
{
    btVector3 pos;                                         // world position
    float                       colour[4];                                      // RGBA value

    float                       inner;
    float                       outer;
    float                       length;
    float                       cutoff;

    float                       falloff;

    loader::LightType           light_type;
};

/*
 * room static mesh.
 */
struct StaticMesh : public Object
{
    uint32_t                    object_id;                                      //
    uint8_t                     was_rendered;                                   // 0 - was not rendered, 1 - opaque, 2 - transparency, 3 - full rendered
    uint8_t                     was_rendered_lines;
    bool hide;                                           // disable static mesh rendering
    btVector3 pos;                                         // model position
    btVector3 rot;                                         // model angles
    std::array<float, 4> tint;                                        // model tint

    BoundingBox visibleBoundingBox;
    BoundingBox collisionBoundingBox;

    btTransform transform;                                  // gl transformation matrix
    OrientedBoundingBox obb;
    std::shared_ptr<engine::EngineContainer> self;

    std::shared_ptr<BaseMesh> mesh;                                           // base model
    btRigidBody                *bt_body;
};

/*
 * Animated skeletal model. Taken from openraider.
 * model -> animation -> frame -> bone
 * thanks to Terry 'Mongoose' Hendrix II
 */

struct SkeletalModel;

/*
 * mesh tree base element structure
 */
struct MeshTreeTag
{
    std::shared_ptr<BaseMesh> mesh_base;                                      // base mesh - pointer to the first mesh in array
    std::shared_ptr<BaseMesh> mesh_skin;                                      // base skinned mesh for ТР4+
    btVector3 offset;                                      // model position offset
    uint16_t                    flag;                                           // 0x0001 = POP, 0x0002 = PUSH, 0x0003 = RESET
    uint32_t                    body_part;
    uint8_t                     replace_mesh;                                   // flag for shoot / guns animations (0x00, 0x01, 0x02, 0x03)
    uint8_t                     replace_anim;
};

/*
 * skeletal model with animations data.
 */

struct SkeletalModel
{
    uint32_t                    id;
    bool                        has_transparency;

    BoundingBox boundingBox;
    btVector3                   centre;                                      // the centre of model

    std::vector<animation::AnimationFrame> animations;

    uint16_t                    mesh_count;                                     // number of model meshes
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

/* bullet collision model calculation */
btCollisionShape *BT_CSfromSphere(const btScalar& radius);
btCollisionShape* BT_CSfromBBox(const BoundingBox &boundingBox, bool useCompression, bool buildBvh);
btCollisionShape* BT_CSfromMesh(const std::shared_ptr<BaseMesh> &mesh, bool useCompression, bool buildBvh, bool is_static = true);
btCollisionShape* BT_CSfromHeightmap(const std::vector<RoomSector> &heightmap, const std::vector<SectorTween> &tweens, bool useCompression, bool buildBvh);

} // namespace core
} // namespace world
