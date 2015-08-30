#include "basemesh.h"

#include "engine/engine.h"
#include "render/shader_description.h"

namespace world
{
namespace core
{

void BaseMesh::polySortInMesh()
{
    for(Polygon& p : m_polygons)
    {
        if(p.anim_id > 0 && p.anim_id <= engine::engine_world.anim_sequences.size())
        {
            animation::AnimSeq* seq = &engine::engine_world.anim_sequences[p.anim_id - 1];
            // set tex coordinates to the first frame for correct texture transform in renderer
            engine::engine_world.tex_atlas->getCoordinates(seq->frame_list[0], false, &p, 0, seq->uvrotate);
        }

        if(p.blendMode != loader::BlendingMode::Opaque && p.blendMode != loader::BlendingMode::Transparent)
        {
            m_transparencyPolygons.emplace_back(p);
        }
    }
}

BaseMesh::~BaseMesh()
{
    clear();
}

void BaseMesh::clear()
{
    if(m_vboVertexArray)
    {
        glDeleteBuffers(1, &m_vboVertexArray);
        m_vboVertexArray = 0;
    }

    if(m_vboIndexArray)
    {
        glDeleteBuffers(1, &m_vboIndexArray);
        m_vboIndexArray = 0;
    }

    m_polygons.clear();
    m_transparencyPolygons.clear();
    m_vertices.clear();
    m_matrixIndices.clear();
    m_elementsPerTexture.clear();
    m_elements.clear();
}

/**
* Bounding box calculation
*/
void BaseMesh::updateBoundingBox()
{
    if(!m_vertices.empty())
    {
        boundingBox.min = boundingBox.max = m_vertices.front().position;
        for(const auto& v : m_vertices)
        {
            boundingBox.adjust(v.position);
        }

        m_center = boundingBox.getCenter();
    }
}

void BaseMesh::genVBO(const render::Render* /*renderer*/)
{
    if(m_vboIndexArray || m_vboVertexArray || m_vboSkinArray)
        return;

    /// now, begin VBO filling!
    glGenBuffers(1, &m_vboVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboVertexArray);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

    // Store additional skinning information
    if(!m_matrixIndices.empty())
    {
        glGenBuffers(1, &m_vboSkinArray);
        glBindBuffer(GL_ARRAY_BUFFER, m_vboSkinArray);
        glBufferData(GL_ARRAY_BUFFER, m_matrixIndices.size() * sizeof(MatrixIndex), m_matrixIndices.data(), GL_STATIC_DRAW);
    }

    // Fill indexes vbo
    glGenBuffers(1, &m_vboIndexArray);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboIndexArray);

    GLsizeiptr elementsSize = sizeof(GLuint) * m_alphaElements;
    for(uint32_t i = 0; i < m_texturePageCount; i++)
    {
        elementsSize += sizeof(GLuint) * m_elementsPerTexture[i];
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementsSize, m_elements.data(), GL_STATIC_DRAW);

    // Prepare vertex array
    render::VertexArrayAttribute attribs[] = {
        render::VertexArrayAttribute(render::LitShaderDescription::VertexAttribs::Position, 3, GL_FLOAT, false, m_vboVertexArray, sizeof(Vertex), offsetof(Vertex, position)),
        render::VertexArrayAttribute(render::LitShaderDescription::VertexAttribs::Normal, 3, GL_FLOAT, false, m_vboVertexArray, sizeof(Vertex), offsetof(Vertex, normal)),
        render::VertexArrayAttribute(render::LitShaderDescription::VertexAttribs::Color, 4, GL_FLOAT, false, m_vboVertexArray, sizeof(Vertex), offsetof(Vertex, color)),
        render::VertexArrayAttribute(render::LitShaderDescription::VertexAttribs::TexCoord, 2, GL_FLOAT, false, m_vboVertexArray, sizeof(Vertex), offsetof(Vertex, tex_coord)),
        // Only used for skinned meshes
        render::VertexArrayAttribute(render::LitShaderDescription::VertexAttribs::MatrixIndex, 2, GL_UNSIGNED_BYTE, false, m_vboSkinArray, 2, 0),
    };
    int numAttribs = !m_matrixIndices.empty() ? 5 : 4;
    m_mainVertexArray = std::make_shared<render::VertexArray>(m_vboIndexArray, numAttribs, attribs);

    // Now for animated polygons, if any
    if(!m_allAnimatedElements.empty())
    {
        // And upload.
        glGenBuffers(1, &m_animatedVboVertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, m_animatedVboVertexArray);
        glBufferData(GL_ARRAY_BUFFER, sizeof(animation::AnimatedVertex) * m_animatedVertices.size(), m_animatedVertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &m_animatedVboIndexArray);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_animatedVboIndexArray);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m_allAnimatedElements.size(), m_allAnimatedElements.data(), GL_STATIC_DRAW);

        // Prepare empty buffer for tex coords
        glGenBuffers(1, &m_animatedVboTexCoordArray);
        glBindBuffer(GL_ARRAY_BUFFER, m_animatedVboTexCoordArray);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat[2]) * m_animatedVertices.size(), nullptr, GL_STREAM_DRAW);

        // Create vertex array object.
        render::VertexArrayAttribute attribs2[] = {
            render::VertexArrayAttribute(render::LitShaderDescription::VertexAttribs::Position, 3, GL_FLOAT, false, m_animatedVboVertexArray, sizeof(animation::AnimatedVertex), offsetof(animation::AnimatedVertex, position)),
            render::VertexArrayAttribute(render::LitShaderDescription::VertexAttribs::Color, 4, GL_FLOAT, false, m_animatedVboVertexArray, sizeof(animation::AnimatedVertex), offsetof(animation::AnimatedVertex, color)),
            render::VertexArrayAttribute(render::LitShaderDescription::VertexAttribs::Normal, 3, GL_FLOAT, false, m_animatedVboVertexArray, sizeof(animation::AnimatedVertex), offsetof(animation::AnimatedVertex, normal)),

            render::VertexArrayAttribute(render::LitShaderDescription::VertexAttribs::TexCoord, 2, GL_FLOAT, false, m_animatedVboTexCoordArray, sizeof(GLfloat[2]), 0),
        };
        m_animatedVertexArray = std::make_shared<render::VertexArray>(m_animatedVboIndexArray, 4, attribs2);
    }
    else
    {
        // No animated data
        m_animatedVboVertexArray = 0;
        m_animatedVboTexCoordArray = 0;
        m_animatedVertexArray.reset();
    }

    // Update references for transparent polygons
    for(render::TransparentPolygonReference& p : m_transparentPolygons)
    {
        p.used_vertex_array = p.isAnimated ? m_animatedVertexArray : m_mainVertexArray;
    }
}

} // namespace core
} // namespace world