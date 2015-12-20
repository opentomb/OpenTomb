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
            engine::engine_world.tex_atlas->getCoordinates(seq->frame_list[0], false, p, 0, seq->uvrotate);
        }

        if(p.blendMode != loader::BlendingMode::Opaque && p.blendMode != loader::BlendingMode::Transparent)
        {
            m_transparencyPolygons.emplace_back(p);
        }
    }
}

BaseMesh::~BaseMesh()
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

void BaseMesh::genVBO()
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

Vertex* BaseMesh::findVertex(const glm::vec3& v)
{
    for(Vertex& mv : m_vertices)
    {
        if(glm::distance(v, mv.position) < 2.0)
        {
            return &mv;
        }
    }

    return nullptr;
}

/*
* FACES FUNCTIONS
*/
size_t BaseMesh::addVertex(const Vertex& vertex)
{
    Vertex* v = m_vertices.data();

    for(size_t ind = 0; ind < m_vertices.size(); ind++, v++)
    {
        if(v->position[0] == vertex.position[0] && v->position[1] == vertex.position[1] && v->position[2] == vertex.position[2] &&
           v->tex_coord[0] == vertex.tex_coord[0] && v->tex_coord[1] == vertex.tex_coord[1])
            ///@QUESTION: color check?
        {
            return ind;
        }
    }

    m_vertices.emplace_back();

    v = &m_vertices.back();
    v->position = vertex.position;
    v->normal = vertex.normal;
    v->color = vertex.color;
    v->tex_coord[0] = vertex.tex_coord[0];
    v->tex_coord[1] = vertex.tex_coord[1];

    return m_vertices.size() - 1;
}

size_t BaseMesh::addAnimatedVertex(const Vertex& vertex)
{
    // Skip search for equal vertex; tex coords may differ but aren't stored in
    // animated_vertex_s

    m_animatedVertices.emplace_back();

    animation::AnimatedVertex& v = m_animatedVertices.back();
    v.position = vertex.position;
    v.color = vertex.color;
    v.normal = vertex.normal;

    return m_animatedVertices.size() - 1;
}

void BaseMesh::genFaces()
{
    m_elementsPerTexture.resize(m_texturePageCount);

    /*
    * Layout of the buffers:
    *
    * Normal vertex buffer:
    * - vertices of polygons in order, skipping only animated.
    * Animated vertex buffer:
    * - vertices (without tex coords) of polygons in order, skipping only
    *   non-animated.
    * Animated texture buffer:
    * - tex coords of polygons in order, skipping only non-animated.
    *   stream, initially empty.
    *
    * Normal elements:
    * - elements for texture[0]
    * ...
    * - elements for texture[n]
    * - elements for alpha
    * Animated elements:
    * - animated elements (opaque)
    * - animated elements (blended)
    */

    // Do a first pass to find the numbers of everything
    m_alphaElements = 0;
    size_t numNormalElements = 0;
    m_animatedVertices.clear();
    m_animatedElementCount = 0;
    m_alphaAnimatedElementCount = 0;

    size_t transparent = 0;
    for(const Polygon& p : m_polygons)
    {
        if(p.isBroken())
            continue;

        size_t elementCount = (p.vertices.size() - 2) * 3;
        if(p.double_side)
            elementCount *= 2;

        if(p.anim_id == 0)
        {
            if(p.blendMode == loader::BlendingMode::Opaque || p.blendMode == loader::BlendingMode::Transparent)
            {
                m_elementsPerTexture[p.tex_index] += elementCount;
                numNormalElements += elementCount;
            }
            else
            {
                m_alphaElements += elementCount;
                ++transparent;
            }
        }
        else
        {
            if(p.blendMode == loader::BlendingMode::Opaque || p.blendMode == loader::BlendingMode::Transparent)
                m_animatedElementCount += elementCount;
            else
            {
                m_alphaAnimatedElementCount += elementCount;
                ++transparent;
            }
        }
    }

    m_elements.resize(numNormalElements + m_alphaElements);
    size_t elementOffset = 0;
    std::vector<size_t> startPerTexture(m_texturePageCount, 0);
    for(uint32_t i = 0; i < m_texturePageCount; i++)
    {
        startPerTexture[i] = elementOffset;
        elementOffset += m_elementsPerTexture[i];
    }
    size_t startTransparent = elementOffset;

    m_allAnimatedElements.resize(m_animatedElementCount + m_alphaAnimatedElementCount);
    size_t animatedStart = 0;
    size_t animatedStartTransparent = m_animatedElementCount;

    m_transparentPolygons.resize(transparent);
    size_t transparentPolygonStart = 0;

    for(const struct Polygon& p : m_polygons)
    {
        if(p.isBroken())
            continue;

        size_t elementCount = (p.vertices.size() - 2) * 3;
        size_t backwardsStartOffset = elementCount;
        if(p.double_side)
        {
            elementCount *= 2;
        }

        if(p.anim_id == 0)
        {
            // Not animated
            uint32_t texture = p.tex_index;

            size_t oldStart;
            if(p.blendMode == loader::BlendingMode::Opaque || p.blendMode == loader::BlendingMode::Transparent)
            {
                oldStart = startPerTexture[texture];
                startPerTexture[texture] += elementCount;
            }
            else
            {
                oldStart = startTransparent;
                startTransparent += elementCount;
                m_transparentPolygons[transparentPolygonStart].firstIndex = oldStart;
                m_transparentPolygons[transparentPolygonStart].count = elementCount;
                m_transparentPolygons[transparentPolygonStart].polygon = &p;
                m_transparentPolygons[transparentPolygonStart].isAnimated = false;
                transparentPolygonStart += 1;
            }
            size_t backwardsStart = oldStart + backwardsStartOffset;

            // Render the polygon as a triangle fan. That is obviously correct for
            // a triangle and also correct for any quad.
            uint32_t startElement = static_cast<uint32_t>(addVertex(p.vertices[0]));
            uint32_t previousElement = static_cast<uint32_t>(addVertex(p.vertices[1]));

            for(size_t j = 2; j < p.vertices.size(); j++)
            {
                uint32_t thisElement = static_cast<uint32_t>(addVertex(p.vertices[j]));

                m_elements[oldStart + (j - 2) * 3 + 0] = startElement;
                m_elements[oldStart + (j - 2) * 3 + 1] = previousElement;
                m_elements[oldStart + (j - 2) * 3 + 2] = thisElement;

                if(p.double_side)
                {
                    m_elements[backwardsStart + (j - 2) * 3 + 0] = startElement;
                    m_elements[backwardsStart + (j - 2) * 3 + 1] = thisElement;
                    m_elements[backwardsStart + (j - 2) * 3 + 2] = previousElement;
                }

                previousElement = thisElement;
            }
        }
        else
        {
            // Animated
            size_t oldStart;
            if(p.blendMode == loader::BlendingMode::Opaque || p.blendMode == loader::BlendingMode::Transparent)
            {
                oldStart = animatedStart;
                animatedStart += elementCount;
            }
            else
            {
                oldStart = animatedStartTransparent;
                animatedStartTransparent += elementCount;
                m_transparentPolygons[transparentPolygonStart].firstIndex = oldStart;
                m_transparentPolygons[transparentPolygonStart].count = elementCount;
                m_transparentPolygons[transparentPolygonStart].polygon = &p;
                m_transparentPolygons[transparentPolygonStart].isAnimated = true;
                transparentPolygonStart += 1;
            }
            uint32_t backwardsStart = oldStart + backwardsStartOffset;

            // Render the polygon as a triangle fan. That is obviously correct for
            // a triangle and also correct for any quad.
            uint32_t startElement = static_cast<uint32_t>(addAnimatedVertex(p.vertices[0]));
            uint32_t previousElement = static_cast<uint32_t>(addAnimatedVertex(p.vertices[1]));

            for(size_t j = 2; j < p.vertices.size(); j++)
            {
                uint32_t thisElement = static_cast<uint32_t>(addAnimatedVertex(p.vertices[j]));

                m_allAnimatedElements[oldStart + (j - 2) * 3 + 0] = startElement;
                m_allAnimatedElements[oldStart + (j - 2) * 3 + 1] = previousElement;
                m_allAnimatedElements[oldStart + (j - 2) * 3 + 2] = thisElement;

                if(p.double_side)
                {
                    m_allAnimatedElements[backwardsStart + (j - 2) * 3 + 0] = startElement;
                    m_allAnimatedElements[backwardsStart + (j - 2) * 3 + 1] = thisElement;
                    m_allAnimatedElements[backwardsStart + (j - 2) * 3 + 2] = previousElement;
                }

                previousElement = thisElement;
            }
        }
    }
}

btCollisionShape *BT_CSfromMesh(const std::shared_ptr<BaseMesh>& mesh, bool useCompression, bool buildBvh, bool is_static)
{
    uint32_t cnt = 0;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;

    for(const struct Polygon &p : mesh->m_polygons)
    {
        if(p.isBroken())
        {
            continue;
        }

        for(size_t j = 1; j + 1 < p.vertices.size(); j++)
        {
            const auto& v0 = p.vertices[j + 1].position;
            const auto& v1 = p.vertices[j].position;
            const auto& v2 = p.vertices[0].position;
            trimesh->addTriangle(util::convert(v0), util::convert(v1), util::convert(v2), true);
        }
        cnt++;
    }

    if(cnt == 0)
    {
        delete trimesh;
        return nullptr;
    }

    if(is_static)
    {
        ret = new btBvhTriangleMeshShape(trimesh, useCompression, buildBvh);
    }
    else
    {
        ret = new btConvexTriangleMeshShape(trimesh, true);
    }

    ret->setMargin(COLLISION_MARGIN_RIGIDBODY);

    return ret;
}

} // namespace core
} // namespace world
