
#include <cstdlib>

#include "mesh.h"
#include "polygon.h"
#include "world.h"
#include "vmath.h"
#include "engine.h"
#include "system.h"
#include "gl_util.h"
#include "obb.h"
#include "resource.h"
#include "render.h"
#include "shader_description.h"
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>


Vertex* FindVertexInMesh(const std::shared_ptr<BaseMesh> &mesh, const btVector3 &v);

void BaseMesh::clear()
{
    if(m_vboVertexArray)
    {
        glDeleteBuffersARB(1, &m_vboVertexArray);
        m_vboVertexArray = 0;
    }

    if(m_vboIndexArray)
    {
        glDeleteBuffersARB(1, &m_vboIndexArray);
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
void BaseMesh::findBB()
{
    if(!m_vertices.empty())
    {
        m_bbMin = m_bbMax = m_vertices.front().position;
        for(const auto& v : m_vertices) {
            for(int i=0; i<3; ++i) {
                if(m_bbMin[i] > v.position[i])
                    m_bbMin[i] = v.position[i];
                if(m_bbMax[i] < v.position[i])
                    m_bbMax[i] = v.position[i];
            }
        }

        m_center = (m_bbMin + m_bbMax) / 2.0;
    }
}


void BaseMesh::genVBO(const Render *renderer)
{
    m_vboVertexArray = 0;
    m_vboIndexArray = 0;

    /// now, begin VBO filling!
    glGenBuffersARB(1, &m_vboVertexArray);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vboVertexArray);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW_ARB);

    // Store additional skinning information
    if (!m_matrixIndices.empty())
    {
        glGenBuffersARB(1, &m_vboSkinArray);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vboSkinArray);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_matrixIndices.size(), m_matrixIndices.data(), GL_STATIC_DRAW_ARB);
    }

    // Fill indexes vbo
    glGenBuffersARB(1, &m_vboIndexArray);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_vboIndexArray);

    GLsizeiptr elementsSize = sizeof(uint32_t) * m_alphaElements;
    for (uint32_t i = 0; i < m_texturePageCount; i++)
    {
        elementsSize += sizeof(uint32_t) * m_elementsPerTexture[i];
    }
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, elementsSize, m_elements.data(), GL_STATIC_DRAW_ARB);

    // Prepare vertex array
    VertexArrayAttribute attribs[] = {
        VertexArrayAttribute(LitShaderDescription::VertexAttribs::Position, 3, GL_FLOAT, false, m_vboVertexArray, sizeof(Vertex), offsetof(Vertex, position)),
        VertexArrayAttribute(LitShaderDescription::VertexAttribs::Normal, 3, GL_FLOAT, false, m_vboVertexArray, sizeof(Vertex), offsetof(Vertex, normal)),
        VertexArrayAttribute(LitShaderDescription::VertexAttribs::Color, 4, GL_FLOAT, false, m_vboVertexArray, sizeof(Vertex), offsetof(Vertex, color)),
        VertexArrayAttribute(LitShaderDescription::VertexAttribs::TexCoord, 2, GL_FLOAT, false, m_vboVertexArray, sizeof(Vertex), offsetof(Vertex, tex_coord)),
        // Only used for skinned meshes
        VertexArrayAttribute(LitShaderDescription::VertexAttribs::MatrixIndex, 2, GL_UNSIGNED_BYTE, false, m_vboSkinArray, 2, 0),
    };
    int numAttribs = !m_matrixIndices.empty() ? 5 : 4;
    m_mainVertexArray = std::make_shared<VertexArray>(m_vboIndexArray, numAttribs, attribs);

    // Now for animated polygons, if any
    if (!m_allAnimatedElements.empty())
    {
        // And upload.
        glGenBuffersARB(1, &m_animatedVboVertexArray);
        glBindBufferARB(GL_ARRAY_BUFFER, m_animatedVboVertexArray);
        glBufferDataARB(GL_ARRAY_BUFFER, sizeof(AnimatedVertex) * m_animatedVertices.size(), m_animatedVertices.data(), GL_STATIC_DRAW);

        glGenBuffersARB(1, &m_animatedVboIndexArray);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, m_animatedVboIndexArray);
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, m_allAnimatedElements.size(), m_allAnimatedElements.data(), GL_STATIC_DRAW);

        // Prepare empty buffer for tex coords
        glGenBuffersARB(1, &m_animatedVboTexCoordArray);
        glBindBufferARB(GL_ARRAY_BUFFER, m_animatedVboTexCoordArray);
        glBufferDataARB(GL_ARRAY_BUFFER, sizeof(GLfloat [2]) * m_animatedVertices.size(), 0, GL_STREAM_DRAW);

        // Create vertex array object.
        VertexArrayAttribute attribs[] = {
            VertexArrayAttribute(LitShaderDescription::VertexAttribs::Position, 3, GL_FLOAT, false, m_animatedVboVertexArray, sizeof(AnimatedVertex), offsetof(AnimatedVertex, position)),
            VertexArrayAttribute(LitShaderDescription::VertexAttribs::Color, 4, GL_FLOAT, false, m_animatedVboVertexArray, sizeof(AnimatedVertex), offsetof(AnimatedVertex, color)),
            VertexArrayAttribute(LitShaderDescription::VertexAttribs::Normal, 3, GL_FLOAT, false, m_animatedVboVertexArray, sizeof(AnimatedVertex), offsetof(AnimatedVertex, normal)),

            VertexArrayAttribute(LitShaderDescription::VertexAttribs::TexCoord, 2, GL_FLOAT, false, m_animatedVboTexCoordArray, sizeof(GLfloat [2]), 0),
        };
        m_animatedVertexArray = std::make_shared<VertexArray>(m_animatedVboIndexArray, 4, attribs);
    }
    else
    {
        // No animated data
        m_animatedVboVertexArray = 0;
        m_animatedVboTexCoordArray = 0;
        m_animatedVertexArray.reset();
    }

    // Update references for transparent polygons
    for(TransparentPolygonReference& p : m_transparentPolygons)
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


void SSBoneFrame::fromModel(SkeletalModel* model)
{
    hasSkin = false;
    bb_min.setZero();
    bb_max.setZero();
    centre.setZero();
    pos.setZero();
    animations.anim_flags = 0x0000;
    animations.frame_time = 0.0;
    animations.period = 1.0 / 30.0;
    animations.next_state = 0;
    animations.lerp = 0.0;
    animations.current_animation = 0;
    animations.current_frame = 0;
    animations.next_animation = 0;
    animations.next_frame = 0;

    animations.next = NULL;
    animations.onFrame = NULL;
    animations.model = model;
    bone_tags.resize(model->mesh_count);

    int stack = 0;
    SSBoneTag* parents[bone_tags.size()];
    parents[0] = NULL;
    bone_tags[0].parent = NULL;                                             // root
    for(uint16_t i=0;i<bone_tags.size();i++)
    {
        bone_tags[i].index = i;
        bone_tags[i].mesh_base = model->mesh_tree[i].mesh_base;
        bone_tags[i].mesh_skin = model->mesh_tree[i].mesh_skin;
        if (bone_tags[i].mesh_skin)
            hasSkin = true;
        bone_tags[i].mesh_slot = NULL;
        bone_tags[i].body_part = model->mesh_tree[i].body_part;

        bone_tags[i].offset = model->mesh_tree[i].offset;
        bone_tags[i].qrotate = {0,0,0,0};
        bone_tags[i].transform.setIdentity();
        bone_tags[i].full_transform.setIdentity();

        if(i > 0)
        {
            bone_tags[i].parent = &bone_tags[i-1];
            if(model->mesh_tree[i].flag & 0x01)                                 // POP
            {
                if(stack > 0)
                {
                    bone_tags[i].parent = parents[stack];
                    stack--;
                }
            }
            if(model->mesh_tree[i].flag & 0x02)                                 // PUSH
            {
                if(stack + 1 < (int16_t)model->mesh_count)
                {
                    stack++;
                    parents[stack] = bone_tags[i].parent;
                }
            }
        }
    }
}


void BoneFrame_Copy(BoneFrame *dst, BoneFrame *src)
{
    dst->bone_tags.resize( src->bone_tags.size() );
    dst->pos = src->pos;
    dst->centre = src->centre;
    dst->bb_max = src->bb_max;
    dst->bb_min = src->bb_min;

    dst->command = src->command;
    dst->move = src->move;

    for(uint16_t i=0;i<dst->bone_tags.size();i++)
    {
        dst->bone_tags[i].qrotate = src->bone_tags[i].qrotate;
        dst->bone_tags[i].offset = src->bone_tags[i].offset;
    }
}

void SkeletalModel::interpolateFrames()
{
    AnimationFrame* anim = animations.data();

    for(uint16_t i=0;i<animations.size();i++,anim++)
    {
        if(anim->frames.size() > 1 && anim->original_frame_rate > 1)                      // we can't interpolate one frame or rate < 2!
        {
            std::vector<BoneFrame> new_bone_frames( anim->original_frame_rate * (anim->frames.size() - 1) + 1 );
            /*
             * the first frame does not changes
             */
            BoneFrame* bf = new_bone_frames.data();
            bf->bone_tags.resize( mesh_count );
            bf->pos.setZero();
            bf->move.setZero();
            bf->command = 0x00;
            bf->centre = anim->frames[0].centre;
            bf->pos = anim->frames[0].pos;
            bf->bb_max = anim->frames[0].bb_max;
            bf->bb_min = anim->frames[0].bb_min;
            for(uint16_t k=0;k<mesh_count;k++)
            {
                bf->bone_tags[k].offset = anim->frames[0].bone_tags[k].offset;
                bf->bone_tags[k].qrotate = anim->frames[0].bone_tags[k].qrotate;
            }
            bf++;

            for(uint16_t j=1;j<anim->frames.size();j++)
            {
                for(uint16_t l=1;l<=anim->original_frame_rate;l++)
                {
                    bf->pos.setZero();
                    bf->move.setZero();
                    bf->command = 0x00;
                    btScalar lerp = ((btScalar)l) / (btScalar)anim->original_frame_rate;
                    btScalar t = 1.0 - lerp;

                    bf->bone_tags.resize(mesh_count);

                    bf->centre[0] = t * anim->frames[j-1].centre[0] + lerp * anim->frames[j].centre[0];
                    bf->centre[1] = t * anim->frames[j-1].centre[1] + lerp * anim->frames[j].centre[1];
                    bf->centre[2] = t * anim->frames[j-1].centre[2] + lerp * anim->frames[j].centre[2];

                    bf->pos[0] = t * anim->frames[j-1].pos[0] + lerp * anim->frames[j].pos[0];
                    bf->pos[1] = t * anim->frames[j-1].pos[1] + lerp * anim->frames[j].pos[1];
                    bf->pos[2] = t * anim->frames[j-1].pos[2] + lerp * anim->frames[j].pos[2];

                    bf->bb_max[0] = t * anim->frames[j-1].bb_max[0] + lerp * anim->frames[j].bb_max[0];
                    bf->bb_max[1] = t * anim->frames[j-1].bb_max[1] + lerp * anim->frames[j].bb_max[1];
                    bf->bb_max[2] = t * anim->frames[j-1].bb_max[2] + lerp * anim->frames[j].bb_max[2];

                    bf->bb_min[0] = t * anim->frames[j-1].bb_min[0] + lerp * anim->frames[j].bb_min[0];
                    bf->bb_min[1] = t * anim->frames[j-1].bb_min[1] + lerp * anim->frames[j].bb_min[1];
                    bf->bb_min[2] = t * anim->frames[j-1].bb_min[2] + lerp * anim->frames[j].bb_min[2];

                    for(uint16_t k=0;k<mesh_count;k++) {
                        bf->bone_tags[k].offset = anim->frames[j-1].bone_tags[k].offset.lerp(anim->frames[j].bone_tags[k].offset, lerp);
                        bf->bone_tags[k].qrotate =  anim->frames[j-1].bone_tags[k].qrotate.slerp(anim->frames[j].bone_tags[k].qrotate, lerp);
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


void SkeletalModel::fillTransparency()
{
    transparency_flags = MESH_FULL_OPAQUE;
    for(uint16_t i=0;i<mesh_count;i++)
    {
        if(!mesh_tree[i].mesh_base->m_transparencyPolygons.empty())
        {
            transparency_flags = MESH_HAS_TRANSPARENCY;
            return;
        }
    }
}


MeshTreeTag *SkeletonClone(MeshTreeTag *src, int tags_count)
{
    MeshTreeTag* ret = (MeshTreeTag*)malloc(tags_count * sizeof(MeshTreeTag));

    for(int i=0;i<tags_count;i++)
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
    for(int i=0;i<tags_count;i++)
    {
        dst[i].mesh_base = src[i].mesh_base;
    }
}

void SkeletonCopyMeshes2(MeshTreeTag *dst, MeshTreeTag *src, int tags_count)
{
    for(int i=0;i<tags_count;i++)
    {
        dst[i].mesh_skin = src[i].mesh_base;
    }
}

Vertex* FindVertexInMesh(const std::shared_ptr<BaseMesh>& mesh, const btVector3& v)
{
    for(Vertex& mv : mesh->m_vertices)
    {
        if((v - mv.position).length2() < 4.0)
        {
            return &mv;
        }
    }

    return NULL;
}

void SkeletalModel::fillSkinnedMeshMap()
{
    Vertex* v, *rv;
    MeshTreeTag* tree_tag, *prev_tree_tag;

    tree_tag = mesh_tree.data();
    for(uint16_t i=0;i<mesh_count;i++,tree_tag++)
    {
        if(!tree_tag->mesh_skin)
        {
            return;
        }

        tree_tag->mesh_skin->m_matrixIndices.resize( tree_tag->mesh_skin->m_vertices.size() );
        BaseMesh::MatrixIndex* ch = tree_tag->mesh_skin->m_matrixIndices.data();
        v = tree_tag->mesh_skin->m_vertices.data();
        for(size_t k=0;k<tree_tag->mesh_skin->m_vertices.size();k++,v++, ch++)
        {
            rv = FindVertexInMesh(tree_tag->mesh_base, v->position);
            if(rv != NULL)
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
                prev_tree_tag = mesh_tree.data();
                for(uint16_t l=0;l<mesh_count;l++,prev_tree_tag++)
                {
                    rv = FindVertexInMesh(prev_tree_tag->mesh_base, tv);
                    if(rv != NULL)
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

    for(size_t ind=0; ind<m_vertices.size(); ind++, v++)
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

    return m_vertices.size()-1;
}

uint32_t BaseMesh::addAnimatedVertex(const Vertex& vertex)
{
    // Skip search for equal vertex; tex coords may differ but aren't stored in
    // animated_vertex_s

    m_animatedVertices.emplace_back();

    AnimatedVertex& v = m_animatedVertices.back();
    v.position = vertex.position;
    v.color = vertex.color;
    v.normal = vertex.normal;

    return m_animatedVertices.size()-1;
}

void BaseMesh::genFaces()
{
    m_elementsPerTexture.resize( m_texturePageCount );

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
    for (const auto& p : m_polygons) {
        if (p.isBroken())
            continue;

        uint32_t elementCount = (p.vertices.size() - 2) * 3;
        if (p.double_side) elementCount *= 2;

        if (p.anim_id == 0)
        {
            if (p.transparency < 2)
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
            if (p.transparency < 2)
                m_animatedElementCount += elementCount;
            else
            {
                m_alphaAnimatedElementCount += elementCount;
                ++transparent;
            }
        }
    }

    m_elements.resize( numNormalElements + m_alphaElements );
    uint32_t elementOffset = 0;
    uint32_t *startPerTexture = (uint32_t *) calloc(sizeof(uint32_t), m_texturePageCount);
    for (uint32_t i = 0; i < m_texturePageCount; i++)
    {
        startPerTexture[i] = elementOffset;
        elementOffset += m_elementsPerTexture[i];
    }
    uint32_t startTransparent = elementOffset;

    m_allAnimatedElements.resize( m_animatedElementCount+m_alphaAnimatedElementCount );
    size_t animatedStart = 0;
    size_t animatedStartTransparent = m_animatedElementCount;

    m_transparentPolygons.resize(transparent);
    uint32_t transparentPolygonStart = 0;

    for(const Polygon& p : m_polygons)
    {
        if (p.isBroken())
            continue;

        uint32_t elementCount = (p.vertices.size() - 2) * 3;
        uint32_t backwardsStartOffset = elementCount;
        if (p.double_side)
        {
            elementCount *= 2;
        }

        if(p.anim_id == 0)
        {
            // Not animated
            uint32_t texture = p.tex_index;

            uint32_t oldStart;
            if (p.transparency < 2)
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

                m_elements[oldStart + (j - 2)*3 + 0] = startElement;
                m_elements[oldStart + (j - 2)*3 + 1] = previousElement;
                m_elements[oldStart + (j - 2)*3 + 2] = thisElement;

                if (p.double_side)
                {
                    m_elements[backwardsStart + (j - 2)*3 + 0] = startElement;
                    m_elements[backwardsStart + (j - 2)*3 + 1] = thisElement;
                    m_elements[backwardsStart + (j - 2)*3 + 2] = previousElement;
                }

                previousElement = thisElement;
            }
        }
        else
        {
            // Animated
            uint32_t oldStart;
            if (p.transparency < 2)
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

                m_allAnimatedElements[oldStart + (j - 2)*3 + 0] = startElement;
                m_allAnimatedElements[oldStart + (j - 2)*3 + 1] = previousElement;
                m_allAnimatedElements[oldStart + (j - 2)*3 + 2] = thisElement;

                if (p.double_side)
                {
                    m_allAnimatedElements[backwardsStart + (j - 2)*3 + 0] = startElement;
                    m_allAnimatedElements[backwardsStart + (j - 2)*3 + 1] = thisElement;
                    m_allAnimatedElements[backwardsStart + (j - 2)*3 + 2] = previousElement;
                }

                previousElement = thisElement;
            }
        }
    }
    free(startPerTexture);

    // Now same for animated triangles
}


btCollisionShape *BT_CSfromBBox(const btVector3& bb_min, const btVector3& bb_max, bool useCompression, bool buildBvh)
{
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;
    int cnt = 0;

    OBB obb;
    Polygon* p = obb.base_polygons;
    obb.rebuild(bb_min, bb_max);
    for(uint16_t i=0;i<6;i++,p++)
    {
        if(p->isBroken())
        {
            continue;
        }
        for(size_t j=1; j+1<p->vertices.size(); j++)
        {
            const auto& v0 = p->vertices[j + 1].position;
            const auto& v1 = p->vertices[j].position;
            const auto& v2 = p->vertices[0].position;
            trimesh->addTriangle(v0, v1, v2, true);
        }
        cnt ++;
    }

    if(cnt == 0)                                                                // fixed: without that condition engine may easily crash
    {
        delete trimesh;
        return NULL;
    }

    ret = new btConvexTriangleMeshShape(trimesh, true);

    return ret;
}


btCollisionShape *BT_CSfromMesh(const std::shared_ptr<BaseMesh>& mesh, bool useCompression, bool buildBvh, bool is_static)
{
    uint32_t cnt = 0;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;
    btVector3 v0, v1, v2;

    for(const Polygon& p : mesh->m_polygons)
    {
        if(p.isBroken())
        {
            continue;
        }

        for(size_t j=1; j+1<p.vertices.size(); j++)
        {
            const auto& v0 = p.vertices[j + 1].position;
            const auto& v1 = p.vertices[j].position;
            const auto& v2 = p.vertices[0].position;
            trimesh->addTriangle(v0, v1, v2, true);
        }
        cnt ++;
    }

    if(cnt == 0)
    {
        delete trimesh;
        return NULL;
    }

    if(is_static)
    {
        ret = new btBvhTriangleMeshShape(trimesh, useCompression, buildBvh);
    }
    else
    {
        ret = new btConvexTriangleMeshShape(trimesh, true);
    }

    return ret;
}

///@TODO: resolve cases with floor >> ceiling (I.E. floor - ceiling >= 2048)
btCollisionShape *BT_CSfromHeightmap(const std::vector<RoomSector>& heightmap, SectorTween *tweens, int tweens_size, bool useCompression, bool buildBvh)
{
    uint32_t cnt = 0;
    std::shared_ptr<Room> r = heightmap.front().owner_room;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;

    for(uint32_t i = 0; i < r->sectors.size(); i++)
    {
        if( (heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_GHOST) &&
            (heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_WALL )  )
        {
            if( (heightmap[i].floor_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NONE) ||
                (heightmap[i].floor_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NW  )  )
            {
                if(heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[2],
                                         heightmap[i].floor_corners[0],
                                         true);
                    cnt++;
                }

                if(heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
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
                if(heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[2],
                                         heightmap[i].floor_corners[1],
                                         true);
                    cnt++;
                }

                if(heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
                {
                    trimesh->addTriangle(heightmap[i].floor_corners[3],
                                         heightmap[i].floor_corners[1],
                                         heightmap[i].floor_corners[0],
                                         true);
                    cnt++;
                }
            }
        }

        if( (heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_GHOST) &&
            (heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_WALL )  )
        {
            if( (heightmap[i].ceiling_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NONE) ||
                (heightmap[i].ceiling_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NW  )  )
            {
                if(heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[0],
                                         heightmap[i].ceiling_corners[2],
                                         heightmap[i].ceiling_corners[3],
                                         true);
                    cnt++;
                }

                if(heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
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
                if(heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_A)
                {
                    trimesh->addTriangle(heightmap[i].ceiling_corners[0],
                                         heightmap[i].ceiling_corners[1],
                                         heightmap[i].ceiling_corners[3],
                                         true);
                    cnt++;
                }

                if(heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_DOOR_VERTICAL_B)
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

    for(int i=0; i<tweens_size; i++)
    {
        switch(tweens[i].ceiling_tween_type)
        {
            case TR_SECTOR_TWEEN_TYPE_2TRIANGLES:
                {
                    btScalar t = fabs((tweens[i].ceiling_corners[2].m_floats[2] - tweens[i].ceiling_corners[3].m_floats[2]) /
                                      (tweens[i].ceiling_corners[0].m_floats[2] - tweens[i].ceiling_corners[1].m_floats[2]));
                    t = 1.0 / (1.0 + t);
                    btVector3 o;
                    o.setInterpolate3(tweens[i].ceiling_corners[0], tweens[i].ceiling_corners[2], t);
                    trimesh->addTriangle(tweens[i].ceiling_corners[0],
                                         tweens[i].ceiling_corners[1],
                                         o, true);
                    trimesh->addTriangle(tweens[i].ceiling_corners[3],
                                         tweens[i].ceiling_corners[2],
                                         o, true);
                    cnt += 2;
                }
                break;

            case TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT:
                trimesh->addTriangle(tweens[i].ceiling_corners[0],
                                     tweens[i].ceiling_corners[1],
                                     tweens[i].ceiling_corners[3],
                                     true);
                cnt++;
                break;

            case TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT:
                trimesh->addTriangle(tweens[i].ceiling_corners[2],
                                     tweens[i].ceiling_corners[1],
                                     tweens[i].ceiling_corners[3],
                                     true);
                cnt++;
                break;

            case TR_SECTOR_TWEEN_TYPE_QUAD:
                trimesh->addTriangle(tweens[i].ceiling_corners[0],
                                     tweens[i].ceiling_corners[1],
                                     tweens[i].ceiling_corners[3],
                                     true);
                trimesh->addTriangle(tweens[i].ceiling_corners[2],
                                     tweens[i].ceiling_corners[1],
                                     tweens[i].ceiling_corners[3],
                                     true);
                cnt += 2;
                break;
        };

        switch(tweens[i].floor_tween_type)
        {
            case TR_SECTOR_TWEEN_TYPE_2TRIANGLES:
                {
                    btScalar t = fabs((tweens[i].floor_corners[2].m_floats[2] - tweens[i].floor_corners[3].m_floats[2]) /
                                      (tweens[i].floor_corners[0].m_floats[2] - tweens[i].floor_corners[1].m_floats[2]));
                    t = 1.0 / (1.0 + t);
                    btVector3 o;
                    o.setInterpolate3(tweens[i].floor_corners[0], tweens[i].floor_corners[2], t);
                    trimesh->addTriangle(tweens[i].floor_corners[0],
                                         tweens[i].floor_corners[1],
                                         o, true);
                    trimesh->addTriangle(tweens[i].floor_corners[3],
                                         tweens[i].floor_corners[2],
                                         o, true);
                    cnt += 2;
                }
                break;

            case TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT:
                trimesh->addTriangle(tweens[i].floor_corners[0],
                                     tweens[i].floor_corners[1],
                                     tweens[i].floor_corners[3],
                                     true);
                cnt++;
                break;

            case TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT:
                trimesh->addTriangle(tweens[i].floor_corners[2],
                                     tweens[i].floor_corners[1],
                                     tweens[i].floor_corners[3],
                                     true);
                cnt++;
                break;

            case TR_SECTOR_TWEEN_TYPE_QUAD:
                trimesh->addTriangle(tweens[i].floor_corners[0],
                                     tweens[i].floor_corners[1],
                                     tweens[i].floor_corners[3],
                                     true);
                trimesh->addTriangle(tweens[i].floor_corners[2],
                                     tweens[i].floor_corners[1],
                                     tweens[i].floor_corners[3],
                                     true);
                cnt += 2;
                break;
        };
    }

    if(cnt == 0)
    {
        delete trimesh;
        return NULL;
    }

    ret = new btBvhTriangleMeshShape(trimesh, useCompression, buildBvh);
    return ret;
}


void BaseMesh::polySortInMesh()
{
    for(Polygon& p : m_polygons) {
        if(p.anim_id > 0 && p.anim_id <= engine_world.anim_sequences.size()) {
            AnimSeq* seq = &engine_world.anim_sequences[p.anim_id - 1];
            // set tex coordinates to the first frame for correct texture transform in renderer
            engine_world.tex_atlas->getCoordinates(seq->frame_list[0], false, &p, 0, seq->uvrotate);
        }

        if(p.transparency >= 2) {
            m_transparencyPolygons.emplace_back(p);
        }
    }
}
