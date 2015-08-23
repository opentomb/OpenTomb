#include "mesh.h"

#include <cstdlib>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "engine/engine.h"
#include "render/gl_util.h"
#include "orientedboundingbox.h"
#include "polygon.h"
#include "render/render.h"
#include "world/resource.h"
#include "render/shader_description.h"
#include "util/vmath.h"
#include "world/world.h"

namespace world
{
namespace core
{

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

void SkeletalModel::clear()
{
    mesh_tree.clear();
    collision_map.clear();
    animations.clear();
}

void SkeletalModel::interpolateFrames()
{
    animation::AnimationFrame* anim = animations.data();

    for(uint16_t i = 0; i < animations.size(); i++, anim++)
    {
        if(anim->frames.size() > 1 && anim->original_frame_rate > 1)                      // we can't interpolate one frame or rate < 2!
        {
            std::vector<animation::BoneFrame> new_bone_frames(anim->original_frame_rate * (anim->frames.size() - 1) + 1);
            /*
             * the first frame does not changes
             */
            animation::BoneFrame* bf = new_bone_frames.data();
            bf->bone_tags.resize(mesh_count);
            bf->pos.setZero();
            bf->move.setZero();
            bf->command = 0x00;
            bf->centre = anim->frames[0].centre;
            bf->pos = anim->frames[0].pos;
            bf->boundingBox = anim->frames[0].boundingBox;
            for(uint16_t k = 0; k < mesh_count; k++)
            {
                bf->bone_tags[k].offset = anim->frames[0].bone_tags[k].offset;
                bf->bone_tags[k].qrotate = anim->frames[0].bone_tags[k].qrotate;
            }
            bf++;

            for(uint16_t j = 1; j < anim->frames.size(); j++)
            {
                for(uint16_t l = 1; l <= anim->original_frame_rate; l++)
                {
                    bf->pos.setZero();
                    bf->move.setZero();
                    bf->command = 0x00;
                    btScalar lerp = static_cast<btScalar>(l) / static_cast<btScalar>(anim->original_frame_rate);
                    btScalar t = 1.0 - lerp;

                    bf->bone_tags.resize(mesh_count);

                    bf->centre = t * anim->frames[j - 1].centre + lerp * anim->frames[j].centre;

                    bf->pos = t * anim->frames[j - 1].pos + lerp * anim->frames[j].pos;

                    bf->boundingBox.max = t * anim->frames[j - 1].boundingBox.max + lerp * anim->frames[j].boundingBox.max;
                    bf->boundingBox.min = t * anim->frames[j - 1].boundingBox.min + lerp * anim->frames[j].boundingBox.min;

                    for(uint16_t k = 0; k < mesh_count; k++)
                    {
                        bf->bone_tags[k].offset = anim->frames[j - 1].bone_tags[k].offset.lerp(anim->frames[j].bone_tags[k].offset, lerp);
                        bf->bone_tags[k].qrotate = util::Quat_Slerp(anim->frames[j - 1].bone_tags[k].qrotate, anim->frames[j].bone_tags[k].qrotate, lerp);
                    }
                    bf++;
                }
            }

            /*
             * swap old and new animation bone brames
             * free old bone frames;
             */
            anim->frames = std::move(new_bone_frames);
        }
    }
}

void SkeletalModel::updateTransparencyFlag()
{
    has_transparency = false;
    for(uint16_t i = 0; i < mesh_count; i++)
    {
        if(!mesh_tree[i].mesh_base->m_transparencyPolygons.empty())
        {
            has_transparency = true;
            return;
        }
    }
}

MeshTreeTag *SkeletonClone(MeshTreeTag *src, int tags_count)
{
    MeshTreeTag* ret = new MeshTreeTag[tags_count];

    for(int i = 0; i < tags_count; i++)
    {
        ret[i].mesh_base = src[i].mesh_base;
        ret[i].mesh_skin = src[i].mesh_skin;
        ret[i].flag = src[i].flag;
        ret[i].offset = src[i].offset;
        ret[i].replace_anim = src[i].replace_anim;
        ret[i].replace_mesh = src[i].replace_mesh;
    }
    return ret;
}

void SkeletonCopyMeshes(MeshTreeTag *dst, MeshTreeTag *src, int tags_count)
{
    for(int i = 0; i < tags_count; i++)
    {
        dst[i].mesh_base = src[i].mesh_base;
    }
}

void SkeletonCopyMeshes2(MeshTreeTag *dst, MeshTreeTag *src, int tags_count)
{
    for(int i = 0; i < tags_count; i++)
    {
        dst[i].mesh_skin = src[i].mesh_base;
    }
}

Vertex* BaseMesh::findVertex(const btVector3& v)
{
    for(Vertex& mv : m_vertices)
    {
        if((v - mv.position).length2() < 4.0)
        {
            return &mv;
        }
    }

    return nullptr;
}

void SkeletalModel::fillSkinnedMeshMap()
{
    MeshTreeTag* tree_tag = mesh_tree.data();
    for(uint16_t i = 0; i < mesh_count; i++, tree_tag++)
    {
        if(!tree_tag->mesh_skin)
        {
            return;
        }

        tree_tag->mesh_skin->m_matrixIndices.resize(tree_tag->mesh_skin->m_vertices.size());
        BaseMesh::MatrixIndex* ch = tree_tag->mesh_skin->m_matrixIndices.data();
        Vertex* v = tree_tag->mesh_skin->m_vertices.data();
        for(size_t k = 0; k < tree_tag->mesh_skin->m_vertices.size(); k++, v++, ch++)
        {
            Vertex* rv = tree_tag->mesh_base->findVertex(v->position);
            if(rv != nullptr)
            {
                ch->i = 0;
                ch->j = 0;
                v->position = rv->position;
                v->normal = rv->normal;
            }
            else
            {
                ch->i = 0;
                ch->j = 1;
                auto tv = v->position + tree_tag->offset;
                MeshTreeTag* prev_tree_tag = mesh_tree.data();
                for(uint16_t l = 0; l < mesh_count; l++, prev_tree_tag++)
                {
                    rv = prev_tree_tag->mesh_base->findVertex(tv);
                    if(rv != nullptr)
                    {
                        ch->i = 1;
                        ch->j = 1;
                        v->position = rv->position - tree_tag->offset;
                        v->normal = rv->normal;
                        break;
                    }
                }
            }
        }
    }
}

/*
 * FACES FUNCTIONS
 */
uint32_t BaseMesh::addVertex(const Vertex& vertex)
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

uint32_t BaseMesh::addAnimatedVertex(const Vertex& vertex)
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

        uint32_t elementCount = (p.vertices.size() - 2) * 3;
        if(p.double_side) elementCount *= 2;

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
    uint32_t elementOffset = 0;
    std::vector<uint32_t> startPerTexture(m_texturePageCount, 0);
    for(uint32_t i = 0; i < m_texturePageCount; i++)
    {
        startPerTexture[i] = elementOffset;
        elementOffset += m_elementsPerTexture[i];
    }
    uint32_t startTransparent = elementOffset;

    m_allAnimatedElements.resize(m_animatedElementCount + m_alphaAnimatedElementCount);
    size_t animatedStart = 0;
    size_t animatedStartTransparent = m_animatedElementCount;

    m_transparentPolygons.resize(transparent);
    uint32_t transparentPolygonStart = 0;

    for(const struct Polygon& p : m_polygons)
    {
        if(p.isBroken())
            continue;

        uint32_t elementCount = (p.vertices.size() - 2) * 3;
        uint32_t backwardsStartOffset = elementCount;
        if(p.double_side)
        {
            elementCount *= 2;
        }

        if(p.anim_id == 0)
        {
            // Not animated
            uint32_t texture = p.tex_index;

            uint32_t oldStart;
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
            uint32_t backwardsStart = oldStart + backwardsStartOffset;

            // Render the polygon as a triangle fan. That is obviously correct for
            // a triangle and also correct for any quad.
            uint32_t startElement = addVertex(p.vertices[0]);
            uint32_t previousElement = addVertex(p.vertices[1]);

            for(size_t j = 2; j < p.vertices.size(); j++)
            {
                uint32_t thisElement = addVertex(p.vertices[j]);

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
            uint32_t oldStart;
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
            uint32_t startElement = addAnimatedVertex(p.vertices[0]);
            uint32_t previousElement = addAnimatedVertex(p.vertices[1]);

            for(size_t j = 2; j < p.vertices.size(); j++)
            {
                uint32_t thisElement = addAnimatedVertex(p.vertices[j]);

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

    // Now same for animated triangles
}

btCollisionShape *BT_CSfromSphere(const btScalar& radius)
{
    if(radius == 0.0) return nullptr;

    btCollisionShape* ret;

    ret = new btSphereShape(radius);
    ret->setMargin(COLLISION_MARGIN_RIGIDBODY);

    return ret;
}

btCollisionShape *BT_CSfromBBox(const BoundingBox &boundingBox, bool /*useCompression*/, bool /*buildBvh*/)
{
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;
    int cnt = 0;

    OrientedBoundingBox obb;
    obb.rebuild(boundingBox);
    for(const Polygon& p : obb.base_polygons)
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
            trimesh->addTriangle(v0, v1, v2, true);
        }
        cnt++;
    }

    if(cnt == 0)                                                                // fixed: without that condition engine may easily crash
    {
        delete trimesh;
        return nullptr;
    }

    ret = new btConvexTriangleMeshShape(trimesh, true);
    ret->setMargin(COLLISION_MARGIN_RIGIDBODY);

    return ret;
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
            trimesh->addTriangle(v0, v1, v2, true);
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

///@TODO: resolve cases with floor >> ceiling (I.E. floor - ceiling >= 2048)
btCollisionShape *BT_CSfromHeightmap(const std::vector<RoomSector>& heightmap, const std::vector<SectorTween>& tweens, bool useCompression, bool buildBvh)
{
    uint32_t cnt = 0;
    std::shared_ptr<Room> r = heightmap.front().owner_room;
    btTriangleMesh *trimesh = new btTriangleMesh;

    for(uint32_t i = 0; i < r->sectors.size(); i++)
    {
        if((heightmap[i].floor_penetration_config != PenetrationConfig::Ghost) &&
           (heightmap[i].floor_penetration_config != PenetrationConfig::Wall))
        {
            if((heightmap[i].floor_diagonal_type == DiagonalType::None) ||
               (heightmap[i].floor_diagonal_type == DiagonalType::NW))
            {
                if(heightmap[i].floor_penetration_config != PenetrationConfig::DoorVerticalA)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[2],
                                         heightmap[i].floor_corners[0],
                                         true);
                    cnt++;
                }

                if(heightmap[i].floor_penetration_config != PenetrationConfig::DoorVerticalB)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[2],
                                         heightmap[i].floor_corners[1],
                                         heightmap[i].floor_corners[0],
                                         true);
                    cnt++;
                }
            }
            else
            {
                if(heightmap[i].floor_penetration_config != PenetrationConfig::DoorVerticalA)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[2],
                                         heightmap[i].floor_corners[1],
                                         true);
                    cnt++;
                }

                if(heightmap[i].floor_penetration_config != PenetrationConfig::DoorVerticalB)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[1],
                                         heightmap[i].floor_corners[0],
                                         true);
                    cnt++;
                }
            }
        }

        if((heightmap[i].ceiling_penetration_config != PenetrationConfig::Ghost) &&
           (heightmap[i].ceiling_penetration_config != PenetrationConfig::Wall))
        {
            if((heightmap[i].ceiling_diagonal_type == DiagonalType::None) ||
               (heightmap[i].ceiling_diagonal_type == DiagonalType::NW))
            {
                if(heightmap[i].ceiling_penetration_config != PenetrationConfig::DoorVerticalA)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[0],
                                         heightmap[i].ceiling_corners[2],
                                         heightmap[i].ceiling_corners[3],
                                         true);
                    cnt++;
                }

                if(heightmap[i].ceiling_penetration_config != PenetrationConfig::DoorVerticalB)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[0],
                                         heightmap[i].ceiling_corners[1],
                                         heightmap[i].ceiling_corners[2],
                                         true);
                    cnt++;
                }
            }
            else
            {
                if(heightmap[i].ceiling_penetration_config != PenetrationConfig::DoorVerticalA)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[0],
                                         heightmap[i].ceiling_corners[1],
                                         heightmap[i].ceiling_corners[3],
                                         true);
                    cnt++;
                }

                if(heightmap[i].ceiling_penetration_config != PenetrationConfig::DoorVerticalB)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[1],
                                         heightmap[i].ceiling_corners[2],
                                         heightmap[i].ceiling_corners[3],
                                         true);
                    cnt++;
                }
            }
        }
    }

    for(const SectorTween& tween : tweens)
    {
        switch(tween.ceiling_tween_type)
        {
            case TweenType::TwoTriangles:
                {
                    btScalar t = std::abs((tween.ceiling_corners[2][2] - tween.ceiling_corners[3][2]) /
                                          (tween.ceiling_corners[0][2] - tween.ceiling_corners[1][2]));
                    t = 1.0f / (1.0f + t);
                    btVector3 o;
                    o.setInterpolate3(tween.ceiling_corners[0], tween.ceiling_corners[2], t);
                    trimesh->addTriangle(tween.ceiling_corners[0],
                                         tween.ceiling_corners[1],
                                         o, true);
                    trimesh->addTriangle(tween.ceiling_corners[3],
                                         tween.ceiling_corners[2],
                                         o, true);
                    cnt += 2;
                }
                break;

            case TweenType::TriangleLeft:
                trimesh->addTriangle(tween.ceiling_corners[0],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                cnt++;
                break;

            case TweenType::TriangleRight:
                trimesh->addTriangle(tween.ceiling_corners[2],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                cnt++;
                break;

            case TweenType::Quad:
                trimesh->addTriangle(tween.ceiling_corners[0],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                trimesh->addTriangle(tween.ceiling_corners[2],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                cnt += 2;
                break;
        };

        switch(tween.floor_tween_type)
        {
            case TweenType::TwoTriangles:
            {
                btScalar t = std::abs((tween.floor_corners[2][2] - tween.floor_corners[3][2]) /
                                      (tween.floor_corners[0][2] - tween.floor_corners[1][2]));
                t = 1.0 / (1.0 + t);
                btVector3 o;
                o.setInterpolate3(tween.floor_corners[0], tween.floor_corners[2], t);
                trimesh->addTriangle(tween.floor_corners[0],
                                     tween.floor_corners[1],
                                     o, true);
                trimesh->addTriangle(tween.floor_corners[3],
                                     tween.floor_corners[2],
                                     o, true);
                cnt += 2;
            }
            break;

            case TweenType::TriangleLeft:
                trimesh->addTriangle(tween.floor_corners[0],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                cnt++;
                break;

            case TweenType::TriangleRight:
                trimesh->addTriangle(tween.floor_corners[2],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                cnt++;
                break;

            case TweenType::Quad:
                trimesh->addTriangle(tween.floor_corners[0],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                trimesh->addTriangle(tween.floor_corners[2],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                cnt += 2;
                break;
        };
    }

    if(cnt == 0)
    {
        delete trimesh;
        return nullptr;
    }

    auto ret = new btBvhTriangleMeshShape(trimesh, useCompression, buildBvh);
    ret->setMargin(COLLISION_MARGIN_RIGIDBODY);
    return ret;
}

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

} // namespace core
} // namespace world
