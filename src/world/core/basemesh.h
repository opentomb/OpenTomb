#pragma once

#include "boundingbox.h"
#include "polygon.h"
#include "render/render.h"
#include "world/animation/animation.h"
#include "world/object.h"

#include <vector>

namespace render
{
    class VertexArray;
} // namespace render

namespace world
{
namespace core
{

struct BaseMesh
{
    ObjectId m_id;
    bool m_usesVertexColors; //!< does this mesh have prebaked vertex lighting

    std::vector<Polygon> m_polygons;

    std::vector<Polygon> m_transparencyPolygons;

    uint32_t              m_texturePageCount;
    std::vector<size_t> m_elementsPerTexture;
    std::vector<GLuint> m_elements;
    size_t m_alphaElements;

    std::vector<Vertex> m_vertices;

    size_t m_animatedElementCount;
    size_t m_alphaAnimatedElementCount;
    std::vector<GLuint> m_allAnimatedElements;
    std::vector<animation::AnimatedVertex> m_animatedVertices;

    std::vector<render::TransparentPolygonReference> m_transparentPolygons;

    glm::vec3 m_center; //!< geometry center of mesh
    BoundingBox boundingBox; //!< AABB bounding volume
    glm::float_t m_radius; //!< radius of the bounding sphere

#pragma pack(push,1)
    struct MatrixIndex
    {
        int8_t i = 0, j = 0;

        explicit MatrixIndex() = default;

        MatrixIndex(int8_t i_, int8_t j_)
            : i(i_)
            , j(j_)
        {
        }
    };
#pragma pack(pop)

    std::vector<MatrixIndex> m_matrixIndices; //!< vertices map for skin mesh

    GLuint                m_vboVertexArray = 0;
    GLuint                m_vboIndexArray = 0;
    GLuint                m_vboSkinArray = 0;
    std::shared_ptr< render::VertexArray > m_mainVertexArray;

    // Buffers for animated polygons
    // The first contains position, normal and color.
    // The second contains the texture coordinates. It gets updated every frame.
    GLuint                m_animatedVboVertexArray;
    GLuint                m_animatedVboTexCoordArray;
    GLuint                m_animatedVboIndexArray;
    std::shared_ptr< render::VertexArray > m_animatedVertexArray;

    ~BaseMesh();

    void updateBoundingBox();
    void genVBO();
    void genFaces();
    size_t addVertex(const Vertex& v);
    size_t addAnimatedVertex(const Vertex& v);
    void polySortInMesh();
    Vertex* findVertex(const glm::vec3& v);
};

btCollisionShape* BT_CSfromMesh(const std::shared_ptr<BaseMesh> &mesh, bool useCompression, bool buildBvh, bool is_static = true);

} // namespace core
} // namespace world 
