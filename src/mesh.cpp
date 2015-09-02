#include "mesh.h"

#include <cstdlib>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "engine.h"
#include "gl_util.h"
#include "obb.h"
#include "polygon.h"
#include "render.h"
#include "resource.h"
#include "shader_description.h"
#include "vmath.h"
#include "world.h"

Vertex* FindVertexInMesh(const std::shared_ptr<BaseMesh> &mesh, const btVector3 &v);

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
void BaseMesh::findBB()
{
    if(!m_vertices.empty())
    {
        m_bbMin = m_bbMax = m_vertices.front().position;
        for(const auto& v : m_vertices)
        {
            for(int i = 0; i < 3; ++i)
            {
                if(m_bbMin[i] > v.position[i])
                    m_bbMin[i] = v.position[i];
                if(m_bbMax[i] < v.position[i])
                    m_bbMax[i] = v.position[i];
            }
        }

        m_center = (m_bbMin + m_bbMax) / 2.0;
    }
}

void BaseMesh::genVBO(const Render* /*renderer*/)
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
    if(!m_allAnimatedElements.empty())
    {
        // And upload.
        glGenBuffers(1, &m_animatedVboVertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, m_animatedVboVertexArray);
        glBufferData(GL_ARRAY_BUFFER, sizeof(AnimatedVertex) * m_animatedVertices.size(), m_animatedVertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &m_animatedVboIndexArray);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_animatedVboIndexArray);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m_allAnimatedElements.size(), m_allAnimatedElements.data(), GL_STATIC_DRAW);

        // Prepare empty buffer for tex coords
        glGenBuffers(1, &m_animatedVboTexCoordArray);
        glBindBuffer(GL_ARRAY_BUFFER, m_animatedVboTexCoordArray);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat[2]) * m_animatedVertices.size(), nullptr, GL_STREAM_DRAW);

        // Create vertex array object.
        VertexArrayAttribute attribs2[] = {
            VertexArrayAttribute(LitShaderDescription::VertexAttribs::Position, 3, GL_FLOAT, false, m_animatedVboVertexArray, sizeof(AnimatedVertex), offsetof(AnimatedVertex, position)),
            VertexArrayAttribute(LitShaderDescription::VertexAttribs::Color, 4, GL_FLOAT, false, m_animatedVboVertexArray, sizeof(AnimatedVertex), offsetof(AnimatedVertex, color)),
            VertexArrayAttribute(LitShaderDescription::VertexAttribs::Normal, 3, GL_FLOAT, false, m_animatedVboVertexArray, sizeof(AnimatedVertex), offsetof(AnimatedVertex, normal)),

            VertexArrayAttribute(LitShaderDescription::VertexAttribs::TexCoord, 2, GL_FLOAT, false, m_animatedVboTexCoordArray, sizeof(GLfloat[2]), 0),
        };
        m_animatedVertexArray = std::make_shared<VertexArray>(m_animatedVboIndexArray, 4, attribs2);
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
    animations = SSAnimation();

    animations.next = nullptr;
    animations.onFrame = nullptr;
    animations.model = model;
    bone_tags.resize(model->mesh_count);

    int stack = 0;
    std::vector<SSBoneTag*> parents(bone_tags.size());
    parents[0] = NULL;
    bone_tags[0].parent = NULL;                                             // root
    for(uint16_t i=0;i<bone_tags.size();i++)
    {
        bone_tags[i].index = i;
        bone_tags[i].mesh_base = model->mesh_tree[i].mesh_base;
        bone_tags[i].mesh_skin = model->mesh_tree[i].mesh_skin;
        if(bone_tags[i].mesh_skin)
            hasSkin = true;
        bone_tags[i].mesh_slot = nullptr;
        bone_tags[i].body_part = model->mesh_tree[i].body_part;

        bone_tags[i].offset = model->mesh_tree[i].offset;
        bone_tags[i].qrotate = { 0,0,0,0 };
        bone_tags[i].transform.setIdentity();
        bone_tags[i].full_transform.setIdentity();

        if(i > 0)
        {
            bone_tags[i].parent = &bone_tags[i - 1];
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
                if(stack + 1 < static_cast<int16_t>(model->mesh_count))
                {
                    stack++;
                    parents[stack] = bone_tags[i].parent;
                }
            }
        }
    }
}

void SSAnimation::setAnimation(int animation, int frame, int another_model)
{
    if(!model || animation >= static_cast<int>(model->animations.size()))
    {
        return;
    }
    // FIXME: is anim < 0 actually happening?
    animation = (animation < 0) ? (0) : (animation);

    if(another_model >= 0)
    {
        SkeletalModel* new_model = engine_world.getModelByID(another_model);
        if(!new_model || animation >= static_cast<int>(new_model->animations.size()))
            return;
        model = new_model;
    }

    AnimationFrame* anim = &model->animations[animation];

    if(frame < 0)
        frame = anim->frames.size() - 1 - ((-frame-1) % anim->frames.size());
    else
        frame %= anim->frames.size();

    current_animation   = animation;
    current_frame       = frame;
    last_state          = anim->state_id;
    next_state          = anim->state_id;

    period              = 1.0f / TR_FRAME_RATE;
//    m_bf.animations.lerp = 0.0f;
//    m_bf.animations.frame_time = 0.0f;
    return;
}



/**
 * Find a possible state change to \c stateid
 * \param[in]      stateid  the desired target state
 * \param[in,out]  animid   reference to anim id, receives found anim
 * \param[in,out]  frameid  reference to frame id, receives found frame
 * \return  true if state is found, false otherwise
 */
bool SSAnimation::findStateChange(uint32_t stateid, int16_t& animid_inout, int16_t& frameid_inout)
{
    const std::vector<StateChange>& stclist = model->animations[animid_inout].state_change;

    for(const StateChange& stc : stclist)
    {
        if(stc.id == stateid)
        {
            for(const AnimDispatch& dispatch : stc.anim_dispatch)
            {
                if(   frameid_inout >= dispatch.frame_low
                   && frameid_inout <= dispatch.frame_high)
                {
                    animid_inout  = dispatch.next_anim;
                    frameid_inout = dispatch.next_frame;
                    return true;
                }
            }
        }
    }
    return false;
}

/**
 * Advance animation frame
 * @param time          time step for animation
 * @param cmdEntity     optional entity for which doAnimCommand is called
 * @return  ENTITY_ANIM_NONE if frame is unchanged (time<rate), ENTITY_ANIM_NEWFRAME or ENTITY_ANIM_NEWANIM
 */
int SSAnimation::stepFrame(btScalar time, Entity* cmdEntity)
{
    int16_t frame_id, anim_id;
    const std::vector<AnimationFrame>& animlist = model->animations;
    int stepResult = ENTITY_ANIM_NEWFRAME;

    anim_id  = current_animation;
    frame_id = current_frame;

    // --------
    // FIXME: hack for reverse framesteps (weaponanims):
    if(time < 0.0f) {
        frame_time -= time;
        if(frame_time + GAME_LOGIC_REFRESH_INTERVAL/2.0f < period)
        {
            lerp = frame_time / period;
            stepResult = ENTITY_ANIM_NONE;
        }
        else
        {
            lerp_last_animation = anim_id;
            lerp_last_frame = frame_id;
            frame_time = 0.0f;
            lerp = 0.0f;
            frame_id--;
            if(frame_id < 0) {
                frame_id = animlist[anim_id].frames.size()-1;
                stepResult = ENTITY_ANIM_NEWANIM;
            } else {
                stepResult = ENTITY_ANIM_NEWFRAME;
            }
            current_frame = frame_id;
        }
        return stepResult;
    }
    // --------

    frame_time += time;
    if(frame_time + GAME_LOGIC_REFRESH_INTERVAL/2.0f < period)
    {
        lerp = frame_time / period;
        return ENTITY_ANIM_NONE;
    }

    lerp_last_animation = anim_id;
    lerp_last_frame = frame_id;
    frame_time = 0.0f;
    lerp = 0.0f;

    frame_id++;

    // check anim flags:
    if(anim_flags == ANIM_LOOP_LAST_FRAME)
    {
        if(frame_id >= static_cast<int>(animlist[anim_id].frames.size()))
        {
            current_frame = animlist[anim_id].frames.size()-1;
            return ENTITY_ANIM_NEWFRAME;    // period time has passed so it's a new frame, or should this be none?
        }
    }
    else if(anim_flags == ANIM_LOCK)
    {
        current_frame = 0;
        return ENTITY_ANIM_NEWFRAME;
    }

    // check state change:
    if(next_state != last_state)
    {
        if( findStateChange(next_state, anim_id, frame_id) )
        {
            last_state = animlist[anim_id].state_id;
            next_state = last_state;
            stepResult = ENTITY_ANIM_NEWANIM;
        }
    }

    // check end of animation:
    if( frame_id >= static_cast<int>(animlist[anim_id].frames.size()) )
    {
        if(cmdEntity)
        {
            for(AnimCommand acmd : animlist[anim_id].animCommands)  // end-of-anim cmdlist
            {
                cmdEntity->doAnimCommand(acmd);
            }
        }
        frame_id = animlist[anim_id].next_frame;
        anim_id = animlist[anim_id].next_anim->id;

        last_state = animlist[anim_id].state_id;
        next_state = last_state;
        stepResult = ENTITY_ANIM_NEWANIM;
    }

    current_animation = anim_id;
    current_frame = frame_id;

    if(cmdEntity)
    {
        for(AnimCommand acmd : animlist[anim_id].frames[frame_id].animCommands)  // frame cmdlist
        {
            cmdEntity->doAnimCommand(acmd);
        }
    }
    return stepResult;
}


void BoneFrame_Copy(BoneFrame *dst, BoneFrame *src)
{
    dst->bone_tags.resize(src->bone_tags.size());
    dst->pos = src->pos;
    dst->centre = src->centre;
    dst->bb_max = src->bb_max;
    dst->bb_min = src->bb_min;

    dst->command = src->command;
    dst->move = src->move;

    for(uint16_t i = 0; i < dst->bone_tags.size(); i++)
    {
        dst->bone_tags[i].qrotate = src->bone_tags[i].qrotate;
        dst->bone_tags[i].offset = src->bone_tags[i].offset;
    }
}

/**
 * Expand compressed frames
 * For animations with rate>1, interpolate frames over
 * the real frame range.
 */
void SkeletalModel::interpolateFrames()
{
    AnimationFrame* anim = animations.data();

    for(uint16_t i = 0; i < animations.size(); i++, anim++)
    {
        int length = anim->frames.size();
        int rate = anim->original_frame_rate;
        if(length > 1 && rate > 1)
        {
            int destIdx = int((length-1) / rate) * rate;
            int srcIdx = 0;

            while(destIdx >= 0)
            {
                srcIdx = destIdx / rate;
                for(int j=rate-1; j>0; j--) {
                    if(destIdx+j >= length) continue;

                    btScalar lerp = static_cast<btScalar>(j) / static_cast<btScalar>(rate);

                    anim->frames[destIdx + j].move.setZero();
                    anim->frames[destIdx + j].command = 0;

                    anim->frames[destIdx + j].centre = anim->frames[srcIdx].centre.lerp(anim->frames[srcIdx+1].centre, lerp);
                    anim->frames[destIdx + j].pos    = anim->frames[srcIdx].pos.lerp(   anim->frames[srcIdx+1].pos,    lerp);
                    anim->frames[destIdx + j].bb_max = anim->frames[srcIdx].bb_max.lerp(anim->frames[srcIdx+1].bb_max, lerp);
                    anim->frames[destIdx + j].bb_min = anim->frames[srcIdx].bb_min.lerp(anim->frames[srcIdx+1].bb_min, lerp);


                    for(uint16_t k = 0; k < mesh_count; k++)
                    {
                        anim->frames[destIdx + j].bone_tags[k].offset  = anim->frames[srcIdx  ].bone_tags[k].offset.lerp(
                                                                         anim->frames[srcIdx+1].bone_tags[k].offset, lerp);
                        anim->frames[destIdx + j].bone_tags[k].qrotate = Quat_Slerp(anim->frames[srcIdx  ].bone_tags[k].qrotate,
                                                                                    anim->frames[srcIdx+1].bone_tags[k].qrotate, lerp);
                    }
                }
                if(destIdx > 0)
                {
                    anim->frames[destIdx] = anim->frames[srcIdx];
                }

                destIdx -= rate;
            }
        }
    }
}

void SkeletalModel::fillTransparency()
{
    transparency_flags = MESH_FULL_OPAQUE;
    for(uint16_t i = 0; i < mesh_count; i++)
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

Vertex* FindVertexInMesh(const std::shared_ptr<BaseMesh>& mesh, const btVector3& v)
{
    for(Vertex& mv : mesh->m_vertices)
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
    Vertex* v, *rv;
    MeshTreeTag* tree_tag, *prev_tree_tag;

    tree_tag = mesh_tree.data();
    for(uint16_t i = 0; i < mesh_count; i++, tree_tag++)
    {
        if(!tree_tag->mesh_skin)
        {
            return;
        }

        tree_tag->mesh_skin->m_matrixIndices.resize(tree_tag->mesh_skin->m_vertices.size());
        BaseMesh::MatrixIndex* ch = tree_tag->mesh_skin->m_matrixIndices.data();
        v = tree_tag->mesh_skin->m_vertices.data();
        for(size_t k = 0; k < tree_tag->mesh_skin->m_vertices.size(); k++, v++, ch++)
        {
            rv = FindVertexInMesh(tree_tag->mesh_base, v->position);
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
                prev_tree_tag = mesh_tree.data();
                for(uint16_t l = 0; l < mesh_count; l++, prev_tree_tag++)
                {
                    rv = FindVertexInMesh(prev_tree_tag->mesh_base, tv);
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

    AnimatedVertex& v = m_animatedVertices.back();
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

btCollisionShape *BT_CSfromBBox(const btVector3& bb_min, const btVector3& bb_max, bool /*useCompression*/, bool /*buildBvh*/)
{
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;
    int cnt = 0;

    OBB obb;
    struct Polygon *p = obb.base_polygons;
    obb.rebuild(bb_min, bb_max);
    for(uint16_t i = 0; i < 6; i++, p++)
    {
        if(p->isBroken())
        {
            continue;
        }
        for(size_t j = 1; j + 1 < p->vertices.size(); j++)
        {
            const auto& v0 = p->vertices[j + 1].position;
            const auto& v1 = p->vertices[j].position;
            const auto& v2 = p->vertices[0].position;
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
        if((heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_GHOST) &&
           (heightmap[i].floor_penetration_config != TR_PENETRATION_CONFIG_WALL))
        {
            if((heightmap[i].floor_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NONE) ||
               (heightmap[i].floor_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NW))
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

        if((heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_GHOST) &&
           (heightmap[i].ceiling_penetration_config != TR_PENETRATION_CONFIG_WALL))
        {
            if((heightmap[i].ceiling_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NONE) ||
               (heightmap[i].ceiling_diagonal_type == TR_SECTOR_DIAGONAL_TYPE_NW))
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

    for(const SectorTween& tween : tweens)
    {
        switch(tween.ceiling_tween_type)
        {
            case TR_SECTOR_TWEEN_TYPE_2TRIANGLES:
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

            case TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT:
                trimesh->addTriangle(tween.ceiling_corners[0],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                cnt++;
                break;

            case TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT:
                trimesh->addTriangle(tween.ceiling_corners[2],
                                     tween.ceiling_corners[1],
                                     tween.ceiling_corners[3],
                                     true);
                cnt++;
                break;

            case TR_SECTOR_TWEEN_TYPE_QUAD:
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
            case TR_SECTOR_TWEEN_TYPE_2TRIANGLES:
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

            case TR_SECTOR_TWEEN_TYPE_TRIANGLE_LEFT:
                trimesh->addTriangle(tween.floor_corners[0],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                cnt++;
                break;

            case TR_SECTOR_TWEEN_TYPE_TRIANGLE_RIGHT:
                trimesh->addTriangle(tween.floor_corners[2],
                                     tween.floor_corners[1],
                                     tween.floor_corners[3],
                                     true);
                cnt++;
                break;

            case TR_SECTOR_TWEEN_TYPE_QUAD:
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
    for(struct Polygon &p : m_polygons)
    {
        if(p.anim_id > 0 && p.anim_id <= engine_world.anim_sequences.size())
        {
            AnimSeq* seq = &engine_world.anim_sequences[p.anim_id - 1];
            // set tex coordinates to the first frame for correct texture transform in renderer
            engine_world.tex_atlas->getCoordinates(seq->frame_list[0], false, &p, 0, seq->uvrotate);
        }

        if(p.blendMode != loader::BlendingMode::Opaque && p.blendMode != loader::BlendingMode::Transparent)
        {
            m_transparencyPolygons.emplace_back(p);
        }
    }
}
