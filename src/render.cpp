
#include <cmath>
#include <cstdlib>
#include <algorithm>

#include <LinearMath/btScalar.h>

#include "gl_util.h"
#include "render.h"
#include "console.h"
#include "world.h"
#include "portal.h"
#include "frustum.h"
#include "polygon.h"
#include "camera.h"
#include "script.h"
#include "vmath.h"
#include "mesh.h"
#include "hair.h"
#include "entity.h"
#include "engine.h"
#include "obb.h"
#include "bsp_tree.h"
#include "resource.h"
#include "shader_description.h"
#include "shader_manager.h"

Render renderer;
DynamicBSP render_dBSP;
extern RenderDebugDrawer debugDrawer;

void Render::initGlobals()
{
    m_settings = RenderSettings();
}

void Render::doShaders()
{
    m_shaderManager.reset( new ShaderManager() );
}


void Render::init()
{
    m_blocked = true;
    m_cam = nullptr;

    m_renderList.clear();

    m_world = nullptr;

    m_drawWire = false;
    m_drawRoomBoxes = false;
    m_drawBoxes = false;
    m_drawPortals = false;
    m_drawFrustums = false;
    m_drawNormals = false;
    m_drawAxis = false;
    m_skipRoom = false;
    m_skipStatic = false;
    m_skipEntities = false;
    m_drawNullMeshes = false;
    m_drawDummyStatics = false;
    m_drawColl = false;
    m_drawSkybox = false;
    m_drawPoints = false;
}


void Render::empty()
{
    m_world = nullptr;

    m_renderList.clear();

    m_shaderManager.reset();
}


void Render::renderSkyBox(const matrix4& modelViewProjectionMatrix)
{
    if(m_drawSkybox && (m_world != NULL) && (m_world->sky_box != NULL))
    {
        glDepthMask(GL_FALSE);
        btTransform tr;
        tr.getOrigin() = m_cam->m_pos + m_world->sky_box->animations.front().frames.front().bone_tags.front().offset;
        tr.setRotation( m_world->sky_box->animations.front().frames.front().bone_tags.front().qrotate );
        matrix4 fullView = modelViewProjectionMatrix * tr;

        UnlitTintedShaderDescription *shader = m_shaderManager->getStaticMeshShader();
        glUseProgram(shader->program);
        glUniformMatrix4fv(shader->model_view_projection, 1, false, fullView.c_ptr());
        glUniform1i(shader->sampler, 0);
        GLfloat tint[] = { 1, 1, 1, 1 };
        glUniform4fv(shader->tint_mult, 1, tint);

        renderMesh(m_world->sky_box->mesh_tree.front().mesh_base);
        glDepthMask(GL_TRUE);
    }
}

/**
 * Opaque meshes drawing
 */
void Render::renderMesh(const std::shared_ptr<BaseMesh>& mesh)
{
    if(!mesh->m_allAnimatedElements.empty())
    {
        // Respecify the tex coord buffer
        glBindBuffer(GL_ARRAY_BUFFER, mesh->m_animatedVboTexCoordArray);
        // Tell OpenGL to discard the old values
        glBufferData(GL_ARRAY_BUFFER, mesh->m_animatedVertices.size() * sizeof(GLfloat [2]), 0, GL_STREAM_DRAW);
        // Get writable data (to avoid copy)
        GLfloat *data = (GLfloat *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

        size_t offset = 0;
        for(const struct Polygon& p : mesh->m_polygons)
        {
            if (p.anim_id == 0 || p.isBroken())
            {
                continue;
            }

            AnimSeq* seq = &engine_world.anim_sequences[ p.anim_id - 1 ];

            uint16_t frame = (seq->current_frame + p.frame_offset) % seq->frames.size();
            TexFrame* tf = &seq->frames[frame];
            for(const Vertex& vert : p.vertices)
            {
                const auto& v = vert.tex_coord;
                data[offset + 0] = tf->mat[0+0*2] * v[0] + tf->mat[0+1*2] * v[1] + tf->move[0];
                data[offset + 1] = tf->mat[1+0*2] * v[0] + tf->mat[1+1*2] * v[1] + tf->move[1];

                offset += 2;
            }
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);

        if (mesh->m_animatedElementCount > 0)
        {
            mesh->m_animatedVertexArray->bind();

            //! @bug textures[0] only works if all animated textures are on the first page
            glBindTexture(GL_TEXTURE_2D, m_world->textures[0]);
            glDrawElements(GL_TRIANGLES, mesh->m_animatedElementCount, GL_UNSIGNED_INT, 0);
        }
    }

    if(!mesh->m_vertices.empty())
    {
        mesh->m_mainVertexArray->bind();

        const uint32_t* const elementsbase = nullptr;

        size_t offset = 0;
        for(uint32_t texture = 0; texture < mesh->m_texturePageCount; texture++)
        {
            if(mesh->m_elementsPerTexture[texture] == 0)
            {
                continue;
            }

            glBindTexture(GL_TEXTURE_2D, m_world->textures[texture]);
            glDrawElements(GL_TRIANGLES, mesh->m_elementsPerTexture[texture], GL_UNSIGNED_INT, elementsbase + offset);
            offset += mesh->m_elementsPerTexture[texture];
        }
    }
}


/**
 * draw transparency polygons
 */
void Render::renderPolygonTransparency(uint16_t &currentTransparency, const BSPFaceRef& bsp_ref, const UnlitTintedShaderDescription *shader)
{
    // Blending mode switcher.
    // Note that modes above 2 aren't explicitly used in TR textures, only for
    // internal particle processing. Theoretically it's still possible to use
    // them if you will force type via TRTextur utility.
    const TransparentPolygonReference* ref = bsp_ref.polygon;
    const struct Polygon *p = ref->polygon;
    if (currentTransparency != p->transparency)
    {
        currentTransparency = p->transparency;
        switch(p->transparency)
        {
            case BM_MULTIPLY:                                    // Classic PC alpha
                glBlendFunc(GL_ONE, GL_ONE);
                break;

            case BM_INVERT_SRC:                                  // Inversion by src (PS darkness) - SAME AS IN TR3-TR5
                glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
                break;

            case BM_INVERT_DEST:                                 // Inversion by dest
                glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
                break;

            case BM_SCREEN:                                      // Screen (smoke, etc.)
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
                break;

            case BM_ANIMATED_TEX:
                glBlendFunc(GL_ONE, GL_ZERO);
                break;

            default:                                             // opaque animated textures case
                break;
        };
    }

    matrix4 mvp = m_cam->m_glViewProjMat * bsp_ref.transform;

    glUniformMatrix4fv(shader->model_view_projection, 1, false, mvp.c_ptr());

    ref->used_vertex_array->bind();
    glBindTexture(GL_TEXTURE_2D, m_world->textures[p->tex_index]);

    glDrawElements(GL_TRIANGLES, ref->count, GL_UNSIGNED_INT, (GLvoid *) (sizeof(GLuint) * ref->firstIndex));
}


void Render::renderBSPFrontToBack(uint16_t &currentTransparency, const std::unique_ptr<BSPNode>& root, const UnlitTintedShaderDescription *shader)
{
    btScalar d = root->plane.distance(engine_camera.m_pos);

    if(d >= 0)
    {
        if(root->front != NULL)
        {
            renderBSPFrontToBack(currentTransparency, root->front, shader);
        }

        for(const BSPFaceRef& p : root->polygons_front)
        {
            renderPolygonTransparency(currentTransparency, p, shader);
        }
        for(const BSPFaceRef& p : root->polygons_back)
        {
            renderPolygonTransparency(currentTransparency, p, shader);
        }

        if(root->back != NULL)
        {
            renderBSPFrontToBack(currentTransparency, root->back, shader);
        }
    }
    else
    {
        if(root->back != NULL)
        {
            renderBSPFrontToBack(currentTransparency, root->back, shader);
        }

        for(const BSPFaceRef& p : root->polygons_back)
        {
            renderPolygonTransparency(currentTransparency, p, shader);
        }
        for(const BSPFaceRef& p : root->polygons_front)
        {
            renderPolygonTransparency(currentTransparency, p, shader);
        }

        if(root->front != NULL)
        {
            renderBSPFrontToBack(currentTransparency, root->front, shader);
        }
    }
}

void Render::renderBSPBackToFront(uint16_t &currentTransparency, const std::unique_ptr<BSPNode>& root, const UnlitTintedShaderDescription *shader)
{
    btScalar d = root->plane.distance(engine_camera.m_pos);

    if(d >= 0)
    {
        if(root->back != NULL)
        {
            renderBSPBackToFront(currentTransparency, root->back, shader);
        }

        for(const BSPFaceRef& p : root->polygons_back)
        {
            renderPolygonTransparency(currentTransparency, p, shader);
        }
        for(const BSPFaceRef& p : root->polygons_front)
        {
            renderPolygonTransparency(currentTransparency, p, shader);
        }

        if(root->front != NULL)
        {
            renderBSPBackToFront(currentTransparency, root->front, shader);
        }
    }
    else
    {
        if(root->front != NULL)
        {
            renderBSPBackToFront(currentTransparency, root->front, shader);
        }

        for(const BSPFaceRef& p : root->polygons_front)
        {
            renderPolygonTransparency(currentTransparency, p, shader);
        }
        for(const BSPFaceRef& p : root->polygons_back)
        {
            renderPolygonTransparency(currentTransparency, p, shader);
        }

        if(root->back != NULL)
        {
            renderBSPBackToFront(currentTransparency, root->back, shader);
        }
    }
}

/**
 * skeletal model drawing
 */
void Render::renderSkeletalModel(const LitShaderDescription *shader, SSBoneFrame *bframe, const matrix4& mvMatrix, const matrix4& mvpMatrix)
{
    for(const SSBoneTag& btag : bframe->bone_tags) {
        matrix4 mvTransform = mvMatrix * btag.full_transform;
        glUniformMatrix4fv(shader->model_view, 1, false, mvTransform.c_ptr());

        matrix4 mvpTransform = mvpMatrix * btag.full_transform;
        glUniformMatrix4fv(shader->model_view_projection, 1, false, mvpTransform.c_ptr());

        renderMesh(btag.mesh_base);
        if(btag.mesh_slot) {
            renderMesh(btag.mesh_slot);
        }
    }
}

void Render::renderSkeletalModelSkin(const LitShaderDescription *shader, Entity* ent, const matrix4& mvMatrix, const matrix4& pMatrix)
{
    SSBoneTag* btag = ent->m_bf.bone_tags.data();

    glUniformMatrix4fv(shader->projection, 1, false, pMatrix.c_ptr());

    for(uint16_t i=0; i<ent->m_bf.bone_tags.size(); i++,btag++)
    {
        GLfloat transforms[32];
        matrix4 mvTransforms = mvMatrix * btag->full_transform;
        std::copy_n(mvTransforms.c_ptr(), 16, &transforms[0]);

        // Calculate parent transform
        const btTransform* parentTransform = btag->parent ? &btag->parent->full_transform : &ent->m_transform;

        btTransform translate;
        translate.setIdentity();
        translate.getOrigin() += btag->offset;

        btTransform secondTransform = *parentTransform * translate;

        matrix4 mvTransforms2 = mvMatrix * secondTransform;
        std::copy_n(mvTransforms2.c_ptr(), 16, &transforms[16]);
        glUniformMatrix4fv(shader->model_view, 2, false, transforms);

        if(btag->mesh_skin)
        {
            renderMesh(btag->mesh_skin);
        }
    }
}

void Render::renderDynamicEntitySkin(const LitShaderDescription *shader, Entity* ent, const matrix4& mvMatrix, const matrix4& pMatrix)
{
    glUniformMatrix4fv(shader->projection, 1, false, pMatrix.c_ptr());

    for(uint16_t i=0; i<ent->m_bf.bone_tags.size(); i++)
    {
        matrix4 mvTransforms[2];

        matrix4 tr0( ent->m_bt.bt_body[i]->getWorldTransform() );
        matrix4 tr1;

        mvTransforms[0] = mvMatrix * tr0;

        // Calculate parent transform
        SSBoneTag &btag = ent->m_bf.bone_tags[i];
        bool foundParentTransform = false;
        for (size_t j = 0; j < ent->m_bf.bone_tags.size(); j++) {
            if (&(ent->m_bf.bone_tags[j]) == btag.parent) {
                tr1 = matrix4(ent->m_bt.bt_body[j]->getWorldTransform());
                foundParentTransform = true;
                break;
            }
        }
        if (!foundParentTransform)
            tr1 = matrix4(ent->m_transform);

        btTransform translate;
        translate.setIdentity();
        translate.getOrigin() += btag.offset;

        matrix4 secondTransform = tr1 * translate;
        mvTransforms[1] = mvMatrix * secondTransform;

        GLfloat transforms[32];
        std::copy_n(mvTransforms[0].c_ptr(), 16, &transforms[0]);
        std::copy_n(mvTransforms[1].c_ptr(), 16, &transforms[16]);

        glUniformMatrix4fv(shader->model_view, 2, false, transforms);

        if(btag.mesh_skin)
        {
            renderMesh(btag.mesh_skin);
        }
    }
}

/**
 * Sets up the light calculations for the given entity based on its current
 * room. Returns the used shader, which will have been made current already.
 */
const LitShaderDescription *Render::setupEntityLight(Entity* entity, const matrix4 &modelViewMatrix, bool skin)
{
    // Calculate lighting
    if(!entity->m_self || !entity->m_self->room) {
        const LitShaderDescription *shader = m_shaderManager->getEntityShader(0, skin);
        glUseProgram(shader->program);
        return shader;
    }

    Room* room = entity->m_self->room;

    float ambient_component[4];
    ambient_component[0] = room->ambient_lighting[0];
    ambient_component[1] = room->ambient_lighting[1];
    ambient_component[2] = room->ambient_lighting[2];
    ambient_component[3] = 1.0f;

    if(room->flags & TR_ROOM_FLAG_WATER)
    {
        engine_world.calculateWaterTint(ambient_component, false);
    }

    GLenum current_light_number = 0;
    Light *current_light = NULL;

    GLfloat positions[MAX_NUM_LIGHTS*3];
    GLfloat colors[MAX_NUM_LIGHTS*4];
    GLfloat innerRadiuses[1*MAX_NUM_LIGHTS];
    GLfloat outerRadiuses[1*MAX_NUM_LIGHTS];
    memset(colors, 0, sizeof(colors));
    memset(innerRadiuses, 0, sizeof(innerRadiuses));
    memset(outerRadiuses, 0, sizeof(outerRadiuses));

    for(uint32_t i = 0; i < room->lights.size() && current_light_number < MAX_NUM_LIGHTS; i++)
    {
        current_light = &room->lights[i];

        btVector3 xyz = entity->m_transform.getOrigin() - current_light->pos;
        btScalar distance = xyz.length();

        // Find color
        colors[current_light_number*4 + 0] = std::fmin(std::fmax(current_light->colour[0], 0.0), 1.0);
        colors[current_light_number*4 + 1] = std::fmin(std::fmax(current_light->colour[1], 0.0), 1.0);
        colors[current_light_number*4 + 2] = std::fmin(std::fmax(current_light->colour[2], 0.0), 1.0);
        colors[current_light_number*4 + 3] = std::fmin(std::fmax(current_light->colour[3], 0.0), 1.0);

        if(room->flags & TR_ROOM_FLAG_WATER)
        {
            engine_world.calculateWaterTint(&colors[current_light_number*4], false);
        }

        // Find position
        float4 tmpPos = modelViewMatrix * current_light->pos;
        positions[current_light_number*3 + 0] = tmpPos[0];
        positions[current_light_number*3 + 1] = tmpPos[1];
        positions[current_light_number*3 + 2] = tmpPos[2];

        // Find fall-off
        if(current_light->light_type == LT_SUN)
        {
            innerRadiuses[current_light_number] = 1e20f;
            outerRadiuses[current_light_number] = 1e21f;
            current_light_number++;
        }
        else if(distance <= current_light->outer + 1024.0f && (current_light->light_type == LT_POINT || current_light->light_type == LT_SHADOW))
        {
            innerRadiuses[current_light_number] = std::abs(current_light->inner);
            outerRadiuses[current_light_number] = std::abs(current_light->outer);
            current_light_number++;
        }
    }

    const LitShaderDescription *shader = m_shaderManager->getEntityShader(current_light_number, skin);
    glUseProgram(shader->program);
    glUniform4fv(shader->light_ambient, 1, ambient_component);
    glUniform4fv(shader->light_color, current_light_number, reinterpret_cast<const GLfloat*>(colors));
    glUniform3fv(shader->light_position, current_light_number, reinterpret_cast<const GLfloat*>(positions));
    glUniform1fv(shader->light_inner_radius, current_light_number, innerRadiuses);
    glUniform1fv(shader->light_outer_radius, current_light_number, outerRadiuses);
    return shader;
}

void Render::renderEntity(Entity* entity, const matrix4 &modelViewMatrix, const matrix4 &modelViewProjectionMatrix, const matrix4 &projection)
{
    if(entity->m_wasRendered || !entity->m_visible || (entity->m_bf.animations.model->hide && !m_drawNullMeshes))
    {
        return;
    }

    // Calculate lighting
    const LitShaderDescription *shader = setupEntityLight(entity, modelViewMatrix, false);

    if(entity->m_bf.animations.model && !entity->m_bf.animations.model->animations.empty())
    {
        // base frame offset
        if(entity->m_typeFlags & ENTITY_TYPE_DYNAMIC)
        {
            renderDynamicEntity(shader, entity, modelViewMatrix, modelViewProjectionMatrix);
            ///@TODO: where I need to do bf skinning matrices update? this time ragdoll update function calculates these matrices;
            if (entity->m_bf.bone_tags[0].mesh_skin)
            {
                const LitShaderDescription *skinShader = setupEntityLight(entity, modelViewMatrix, true);
                renderDynamicEntitySkin(skinShader, entity, modelViewMatrix, projection);
            }
        }
        else
        {
            matrix4 scaledTransform( entity->m_transform );
            scaledTransform *= matrix4::diagonal(float4(entity->m_scaling.x(), entity->m_scaling.y(), entity->m_scaling.z()));
            matrix4 subModelView = modelViewMatrix * scaledTransform;
            matrix4 subModelViewProjection = modelViewProjectionMatrix * scaledTransform;
            renderSkeletalModel(shader, &entity->m_bf, subModelView, subModelViewProjection);
            if (entity->m_bf.bone_tags[0].mesh_skin) {
                const LitShaderDescription *skinShader = setupEntityLight(entity, modelViewMatrix, true);
                renderSkeletalModelSkin(skinShader, entity, subModelView, projection);
            }
        }
    }
}

void Render::renderDynamicEntity(const LitShaderDescription *shader, Entity* entity, const matrix4& modelViewMatrix, const matrix4& modelViewProjectionMatrix)
{
    SSBoneTag* btag = entity->m_bf.bone_tags.data();

    for(uint16_t i=0; i<entity->m_bf.bone_tags.size(); i++,btag++)
    {
        matrix4 tr( entity->m_bt.bt_body[i]->getWorldTransform() );
        matrix4 mvTransform = modelViewMatrix * tr;

        glUniformMatrix4fv(shader->model_view, 1, false, mvTransform.c_ptr());

        matrix4 mvpTransform = modelViewProjectionMatrix * tr;
        glUniformMatrix4fv(shader->model_view_projection, 1, false, mvpTransform.c_ptr());

        renderMesh(btag->mesh_base);
        if(btag->mesh_slot)
        {
            renderMesh(btag->mesh_slot);
        }
    }
}

///@TODO: add joint between hair and head; do Lara's skinning by vertex position copy (no inverse matrices and other) by vertex map;
void Render::renderHair(std::shared_ptr<Character> entity, const matrix4 &modelViewMatrix, const matrix4 &projection)
{
    if(!entity || entity->m_hairs.empty())
        return;

    // Calculate lighting
    const LitShaderDescription *shader = setupEntityLight(entity.get(), modelViewMatrix, true);


    for(size_t h=0; h<entity->m_hairs.size(); h++)
    {
        // First: Head attachment
        matrix4 globalHead( entity->m_transform * entity->m_bf.bone_tags[entity->m_hairs[h]->m_ownerBody].full_transform );
        matrix4 globalAttachment = globalHead * entity->m_hairs[h]->m_ownerBodyHairRoot;

        static constexpr int MatrixCount = 10;

        GLfloat hairModelToGlobalMatrices[MatrixCount][16];
        std::copy_n((modelViewMatrix * globalAttachment).c_ptr(), 16, &hairModelToGlobalMatrices[0][0]);

        // Then: Individual hair pieces
        for(size_t i=0; i<entity->m_hairs[h]->m_elements.size(); i++)
        {
            assert(i+1 < MatrixCount);
            /*
             * Definitions: x_o - as in original file. x_h - as in hair model
             * (translated)
             * M_ho - translation matrix. x_g = global position (before modelview)
             * M_go - global position
             *
             * We know:
             * x_h = M_ho * x_o
             * x_g = M_go * x_o
             * We want:
             * M_hg so that x_g = M_gh * x_m
             * We have: M_oh, M_g
             *
             * x_h = M_ho * x_o => x_o = M_oh^-1 * x_h
             * x_g = M_go * M_ho^-1 * x_h
             * (M_ho^-1 = M_oh so x_g = M_go * M_oh * x_h)
             */

            btTransform invOriginToHairModel;
            invOriginToHairModel.setIdentity();
            // Simplification: Always translation matrix, no invert needed
            invOriginToHairModel.getOrigin() -= entity->m_hairs[h]->m_elements[i].position;

            matrix4 globalFromHair( entity->m_hairs[h]->m_elements[i].body->getWorldTransform() * invOriginToHairModel );

            std::copy_n((modelViewMatrix * globalFromHair).c_ptr(), 16, &hairModelToGlobalMatrices[i+1][0]);
        }

        glUniformMatrix4fv(shader->model_view, entity->m_hairs[h]->m_elements.size()+1, GL_FALSE, &hairModelToGlobalMatrices[0][0]);

        glUniformMatrix4fv(shader->projection, 1, GL_FALSE, projection.c_ptr());

        renderMesh(entity->m_hairs[h]->m_mesh);
    }
}

/**
 * drawing world models.
 */
void Render::renderRoom(const Room* room, const matrix4 &modelViewMatrix, const matrix4 &modelViewProjectionMatrix, const matrix4 &projection)
{
#if STENCIL_FRUSTUM
    ////start test stencil test code
    bool need_stencil = false;
    if(!room->frustum.empty()) {
        for(const std::shared_ptr<Room>& r : room->overlapped_room_list)
        {
            if(std::find(m_renderList.begin(), m_renderList.end(), r.get()) != m_renderList.end())
            {
                need_stencil = true;
                break;
            }
        }

        if(need_stencil)
        {
            UnlitShaderDescription *shader = m_shaderManager->getStencilShader();
            glUseProgram(shader->program);
            glUniformMatrix4fv(shader->model_view_projection, 1, false, engine_camera.m_glViewProjMat.c_ptr());
            glEnable(GL_STENCIL_TEST);
            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilFunc(GL_NEVER, 1, 0x00);
            glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

            GLuint stencilVBO;
            glGenBuffers(1, &stencilVBO);

            VertexArrayAttribute attribs[] = {
                VertexArrayAttribute(UnlitShaderDescription::Position, 3, GL_FLOAT, false, stencilVBO, sizeof(GLfloat [3]), 0)
            };

            std::unique_ptr<VertexArray> array( new VertexArray(0, 1, attribs) );
            array->bind();

            for(const auto& f : room->frustum) {
                glBindBuffer(GL_ARRAY_BUFFER, stencilVBO);
                glBufferData(GL_ARRAY_BUFFER, f->vertices.size() * sizeof(GLfloat[3]), nullptr, GL_STREAM_DRAW);

                GLfloat *v = (GLfloat *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

                for(int16_t i=f->vertices.size()-1;i>=0;i--) {
                    *v++ = f->vertices[i].x();
                    *v++ = f->vertices[i].y();
                    *v++ = f->vertices[i].z();
                }

                glUnmapBuffer(GL_ARRAY_BUFFER);

                glDrawArrays(GL_TRIANGLE_FAN, 0, f->vertices.size());
            }
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glDeleteBuffers(1, &stencilVBO);
        }
    }
#endif

    if(!m_skipRoom && room->mesh)
    {
        matrix4 modelViewProjectionTransform = modelViewProjectionMatrix * room->transform;

        UnlitTintedShaderDescription *shader = m_shaderManager->getRoomShader(room->light_mode == 1, room->flags & 1);

        float tint[4];
        engine_world.calculateWaterTint(tint, true);
        glUseProgram(shader->program);

        glUniform4fv(shader->tint_mult, 1, tint);
        glUniform1f(shader->current_tick, (GLfloat) SDL_GetTicks());
        glUniform1i(shader->sampler, 0);
        glUniformMatrix4fv(shader->model_view_projection, 1, false, modelViewProjectionTransform.c_ptr());
        renderMesh(room->mesh);
    }

    if (!room->static_mesh.empty())
    {
        glUseProgram(m_shaderManager->getStaticMeshShader()->program);
        for(auto sm : room->static_mesh)
        {
            if(sm->was_rendered || !Frustum::isOBBVisibleInRoom(sm->obb, *room))
            {
                continue;
            }

            if(sm->hide && !m_drawDummyStatics)
            {
                continue;
            }

            matrix4 transform = modelViewProjectionMatrix * sm->transform;
            glUniformMatrix4fv(m_shaderManager->getStaticMeshShader()->model_view_projection, 1, false, transform.c_ptr());

            auto tint = sm->tint;

            //If this static mesh is in a water room
            if(room->flags & TR_ROOM_FLAG_WATER)
            {
                engine_world.calculateWaterTint(tint.data(), false);
            }
            glUniform4fv(m_shaderManager->getStaticMeshShader()->tint_mult, 1, tint.data());
            renderMesh(sm->mesh);
            sm->was_rendered = 1;
        }
    }

    if (!room->containers.empty())
    {
        for(const std::shared_ptr<EngineContainer>& cont : room->containers)
        {
            switch(cont->object_type)
            {
            case OBJECT_ENTITY:
                Entity* ent = static_cast<Entity*>(cont->object);
                if(!ent->m_wasRendered)
                {
                    if(Frustum::isOBBVisibleInRoom(ent->m_obb.get(), *room))
                    {
                        renderEntity(ent, modelViewMatrix, modelViewProjectionMatrix, projection);
                    }
                    ent->m_wasRendered = true;
                }
                break;
            };
        }
    }
#if STENCIL_FRUSTUM
    if(need_stencil)
    {
        glDisable(GL_STENCIL_TEST);
    }
#endif
}


void Render::renderRoomSprites(const Room* room, const matrix4 &modelViewMatrix, const matrix4 &projectionMatrix)
{
    if (!room->sprites.empty() && room->sprite_buffer)
    {
        SpriteShaderDescription *shader = m_shaderManager->getSpriteShader();
        glUseProgram(shader->program);
        glUniformMatrix4fv(shader->model_view, 1, GL_FALSE, modelViewMatrix.c_ptr());
        glUniformMatrix4fv(shader->projection, 1, GL_FALSE, projectionMatrix.c_ptr());
        glUniform1i(shader->sampler, 0);

        room->sprite_buffer->data->bind();

        unsigned long offset = 0;
        for(uint32_t texture = 0; texture < room->sprite_buffer->num_texture_pages; texture++)
        {
            if(room->sprite_buffer->element_count_per_texture[texture] == 0)
            {
                continue;
            }

            glBindTexture(GL_TEXTURE_2D, m_world->textures[texture]);
            glDrawElements(GL_TRIANGLES, room->sprite_buffer->element_count_per_texture[texture], GL_UNSIGNED_SHORT, (GLvoid *) (offset * sizeof(uint16_t)));
            offset += room->sprite_buffer->element_count_per_texture[texture];
        }
    }
}


/**
 * Add a room to the render list.
 * If the room is already listed - false is returned and the room is not added twice.
 */
bool Render::addRoom(Room* room)
{
    if(std::find(m_renderList.begin(), m_renderList.end(), room) != m_renderList.end() || !room->active)
    {
        return false;
    }

    m_renderList.emplace_back(room);

    if(room->flags & TR_ROOM_FLAG_SKYBOX)
        m_drawSkybox = true;

    for(auto sm : room->static_mesh)
    {
        sm->was_rendered = 0;
        sm->was_rendered_lines = 0;
    }

    for(const std::shared_ptr<EngineContainer>& cont : room->containers)
    {
        switch(cont->object_type)
        {
        case OBJECT_ENTITY:
            static_cast<Entity*>(cont->object)->m_wasRendered = false;
            static_cast<Entity*>(cont->object)->m_wasRenderedLines = false;
            break;
        };
    }

    for(RoomSprite& sp : room->sprites)
    {
        sp.was_rendered = false;
    }

    return true;
}


void Render::cleanList()
{
    if(m_world->character)
    {
        m_world->character->m_wasRendered = false;
        m_world->character->m_wasRenderedLines = false;
    }

    for(Room* room : m_renderList) {
        room->frustum.clear();
    }

    m_drawSkybox = false;
    m_renderList.clear();
}

/**
 * Render all visible rooms
 */
void Render::drawList()
{
    if(!m_world)
    {
        return;
    }

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    renderSkyBox(m_cam->m_glViewProjMat);

    if(m_world->character)
    {
        renderEntity(m_world->character.get(), m_cam->m_glViewMat, m_cam->m_glViewProjMat, m_cam->m_glProjMat);
        renderHair(m_world->character, m_cam->m_glViewMat, m_cam->m_glProjMat);
    }

    /*
     * room rendering
     */
    for(const Room* room : m_renderList)
    {
        renderRoom(room, m_cam->m_glViewMat, m_cam->m_glViewProjMat, m_cam->m_glProjMat);
    }

    glDisable(GL_CULL_FACE);

    ///@FIXME: reduce number of gl state changes
    for(const Room* room : m_renderList)
    {
        renderRoomSprites(room, m_cam->m_glViewMat, m_cam->m_glProjMat);
    }

    /*
     * NOW render transparency polygons
     */
    render_dBSP.reset();
    /*First generate BSP from base room mesh - it has good for start splitter polygons*/
    for(const Room* room : m_renderList)
    {
        if(room->mesh && !room->mesh->m_transparencyPolygons.empty())
        {
            render_dBSP.addNewPolygonList(room->mesh->m_transparentPolygons, room->transform, {m_cam->frustum});
        }
    }

    for(const Room* room : m_renderList)
    {
        // Add transparency polygons from static meshes (if they exists)
        for(auto sm : room->static_mesh)
        {
            if(!sm->mesh->m_transparentPolygons.empty() && Frustum::isOBBVisibleInRoom(sm->obb, *room))
            {
                render_dBSP.addNewPolygonList(sm->mesh->m_transparentPolygons, sm->transform, {m_cam->frustum});
            }
        }

        // Add transparency polygons from all entities (if they exists) // yes, entities may be animated and intersects with each others;
        for(const std::shared_ptr<EngineContainer>& cont : room->containers)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                Entity* ent = static_cast<Entity*>(cont->object);
                if((ent->m_bf.animations.model->transparency_flags == MESH_HAS_TRANSPARENCY) && ent->m_visible && (Frustum::isOBBVisibleInRoom(ent->m_obb.get(), *room)))
                {
                    for(uint16_t j=0;j<ent->m_bf.bone_tags.size();j++)
                    {
                        if(!ent->m_bf.bone_tags[j].mesh_base->m_transparencyPolygons.empty())
                        {
                            auto tr = ent->m_transform * ent->m_bf.bone_tags[j].full_transform;
                            render_dBSP.addNewPolygonList(ent->m_bf.bone_tags[j].mesh_base->m_transparentPolygons, tr, {m_cam->frustum});
                        }
                    }
                }
            }
        }
    }

    if((engine_world.character != NULL) && (engine_world.character->m_bf.animations.model->transparency_flags == MESH_HAS_TRANSPARENCY))
    {
        Entity *ent = engine_world.character.get();
        for(uint16_t j=0;j<ent->m_bf.bone_tags.size();j++)
        {
            if(!ent->m_bf.bone_tags[j].mesh_base->m_transparencyPolygons.empty())
            {
                auto tr = ent->m_transform * ent->m_bf.bone_tags[j].full_transform;
                render_dBSP.addNewPolygonList(ent->m_bf.bone_tags[j].mesh_base->m_transparentPolygons, tr, {m_cam->frustum});
            }
        }
    }

    if(!render_dBSP.root()->polygons_front.empty())
    {
        UnlitTintedShaderDescription *shader = m_shaderManager->getRoomShader(false, false);
        glUseProgram(shader->program);
        glUniform1i(shader->sampler, 0);
        glUniformMatrix4fv(shader->model_view_projection, 1, false, m_cam->m_glViewProjMat.c_ptr());
        glDepthMask(GL_FALSE);
        glDisable(GL_ALPHA_TEST);
        glEnable(GL_BLEND);
        uint16_t transparency = BM_OPAQUE;
        renderBSPBackToFront(transparency, render_dBSP.root(), shader);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}

void Render::drawListDebugLines()
{
    if (!m_world || !(m_drawBoxes || m_drawRoomBoxes || m_drawPortals || m_drawFrustums || m_drawAxis || m_drawNormals || m_drawColl))
    {
        return;
    }

    if(m_world->character)
    {
        debugDrawer.drawEntityDebugLines(m_world->character.get(), this);
    }

    /*
     * Render world debug information
     */
    if(m_drawNormals && m_world && m_world->sky_box)
    {
        btTransform tr;
        tr.setIdentity();
        tr.getOrigin() = m_cam->m_pos + m_world->sky_box->animations.front().frames.front().bone_tags.front().offset;
        tr.setRotation(m_world->sky_box->animations.front().frames.front().bone_tags.front().qrotate);
        debugDrawer.drawMeshDebugLines(m_world->sky_box->mesh_tree.front().mesh_base, tr, {}, {}, this);
    }

    for(const Room* room : m_renderList)
    {
        debugDrawer.drawRoomDebugLines(room, this);
    }

    if(m_drawColl)
    {
        bt_engine_dynamicsWorld->debugDrawWorld();
    }

    if(!debugDrawer.IsEmpty())
    {
        UnlitShaderDescription *shader = m_shaderManager->getDebugLineShader();
        glUseProgram(shader->program);
        glUniform1i(shader->sampler, 0);
        glUniformMatrix4fv(shader->model_view_projection, 1, false, m_cam->m_glViewProjMat.c_ptr());
        glBindTexture(GL_TEXTURE_2D, engine_world.textures.back());
        glPointSize( 6.0f );
        glLineWidth( 3.0f );
        debugDrawer.render();
    }
}

/**
 * The reccursion algorithm: go through the rooms with portal-frustum occlusion test
 * @param portal we entered to the room through that portal
 * @para frus frustum that intersects the portal
 * @return number of added rooms
 */
int Render::processRoom(Portal *portal, const std::shared_ptr<Frustum> &frus)
{
    std::shared_ptr<Room> destination = portal->dest_room;
    std::shared_ptr<Room> current = portal->current_room;

    if(!current || !current->active || !destination || !destination->active)
    {
        return 0;
    }

    int ret = 0;
    for(Portal& p : destination->portals)
    {
        if(p.dest_room && p.dest_room->active && p.dest_room != current)
        {
            auto gen_frus = Frustum::portalFrustumIntersect(&p, frus, this);             // Главная ф-я портального рендерера. Тут и проверка
            if(gen_frus) {
                ret++;
                addRoom(p.dest_room.get());
                processRoom(&p, gen_frus);
            }
        }
    }
    return ret;
}

/**
 * Renderer list generation by current world and camera
 */
void Render::genWorldList()
{
    if(m_world == NULL)
    {
        return;
    }

    cleanList();                                                         // clear old render list
    debugDrawer.reset();
    //cam->frustum->next = NULL;

    Room* curr_room = Room_FindPosCogerrence(m_cam->m_pos, m_cam->m_currentRoom);                // find room that contains camera

    m_cam->m_currentRoom = curr_room;                                     // set camera's cuttent room pointer
    if(curr_room != NULL)                                                       // camera located in some room
    {
        curr_room->frustum.clear();                                              // room with camera inside has no frustums!
        curr_room->max_path = 0;
        addRoom(curr_room);                                              // room with camera inside adds to the render list immediately
        for(Portal& p : curr_room->portals)                   // go through all start room portals
        {
            auto last_frus = Frustum::portalFrustumIntersect(&p, m_cam->frustum, this);
            if(last_frus) {
                addRoom(p.dest_room.get());                                   // portal destination room
                last_frus->parents_count = 1;                                   // created by camera
                processRoom(&p, last_frus);                               // next start reccursion algorithm
            }
        }
    }
    else                                                                        // camera is out of all rooms
    {
        for(auto r : m_world->rooms)
        {
            if(m_cam->frustum->isAABBVisible(r->bb_min, r->bb_max))
            {
                addRoom(r.get());
            }
        }
    }
}

/**
 * Состыковка рендерера и "мира"
 */
void Render::setWorld(World *world)
{
    resetWorld();
    uint32_t list_size = world->rooms.size() + 128;                               // magick 128 was added for debug and testing

    if(m_renderList.size() < list_size)                                    // if old list less than new one requiring
    {
        m_renderList.resize(list_size);
    }

    m_world = world;
    m_drawSkybox = false;
    m_renderList.clear();

    m_cam = &engine_camera;
    //engine_camera.frustum->next = NULL;
    engine_camera.m_currentRoom = NULL;
}

/**
 * DEBUG PRIMITIVES RENDERING
 */
RenderDebugDrawer::RenderDebugDrawer()
    : m_obb(new OBB())
{
}

RenderDebugDrawer::~RenderDebugDrawer()
{
}

void RenderDebugDrawer::reset()
{
    m_buffer.clear();
}

void RenderDebugDrawer::addLine(const std::array<GLfloat,3>& start, const std::array<GLfloat,3>& end)
{
    addLine(start, m_color, end, m_color);
}

void RenderDebugDrawer::addLine(const btVector3& start, const btVector3& end)
{
    std::array<GLfloat,3> startA{{start.x(), start.y(), start.z()}};
    std::array<GLfloat,3> endA{{end.x(), end.y(), end.z()}};
    addLine(startA, m_color, endA, m_color);
}

void RenderDebugDrawer::addLine(const std::array<GLfloat,3>& start, const std::array<GLfloat,3>& startColor, const std::array<GLfloat,3>& end, const std::array<GLfloat,3>& endColor)
{
    m_buffer.emplace_back(start);
    m_buffer.emplace_back(startColor);
    m_buffer.emplace_back(end);
    m_buffer.emplace_back(endColor);
}

void RenderDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3 &color)
{
    std::array<GLfloat,3> fromA{{from.x(), from.y(), from.z()}};
    std::array<GLfloat,3> toA{{to.x(), to.y(), to.z()}};
    std::array<GLfloat,3> colorA{{color.x(), color.y(), color.z()}};
    addLine(fromA, colorA, toA, colorA);
}

void RenderDebugDrawer::setDebugMode(int debugMode)
{
   m_debugMode = debugMode;
}

void RenderDebugDrawer::draw3dText(const btVector3& /*location*/, const char* /*textString*/)
{
   //glRasterPos3f(location.x(),  location.y(),  location.z());
   //BMF_DrawString(BMF_GetFont(BMF_kHelvetica10),textString);
}

void RenderDebugDrawer::reportErrorWarning(const char* warningString)
{
   ConsoleInfo::instance().addLine(warningString, FONTSTYLE_CONSOLE_WARNING);
}

void RenderDebugDrawer::drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int /*lifeTime*/, const btVector3& color)
{
    drawLine(pointOnB, pointOnB + normalOnB * distance, color);
}

void RenderDebugDrawer::render()
{
    if(!m_buffer.empty())
    {
        if (m_glbuffer == 0) {
            glGenBuffers(1, &m_glbuffer);
            VertexArrayAttribute attribs[] = {
                VertexArrayAttribute(UnlitShaderDescription::Position, 3, GL_FLOAT, false, m_glbuffer, sizeof(GLfloat [6]), sizeof(GLfloat [0])),
                VertexArrayAttribute(UnlitShaderDescription::Color, 3, GL_FLOAT, false, m_glbuffer, sizeof(GLfloat [6]), sizeof(GLfloat [3]))
            };
            m_vertexArray.reset( new VertexArray(0, 2, attribs) );
        }

        glBindBuffer(GL_ARRAY_BUFFER, m_glbuffer);
        glBufferData(GL_ARRAY_BUFFER, m_buffer.size() * sizeof(decltype(m_buffer[0])), nullptr, GL_STREAM_DRAW);

        std::array<GLfloat,3>* data = static_cast<std::array<GLfloat,3>*>( glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY) );
        std::copy(m_buffer.begin(), m_buffer.end(), data);
        glUnmapBuffer(GL_ARRAY_BUFFER);

        m_vertexArray->bind();
        glDrawArrays(GL_LINES, 0, m_buffer.size()/2);
    }

    m_color.fill(0);
    m_buffer.clear();
}

void RenderDebugDrawer::drawAxis(btScalar r, const btTransform &transform)
{
    std::array<GLfloat,3> origin{{ transform.getOrigin().x(), transform.getOrigin().y(), transform.getOrigin().z() }};

    btVector3 v = transform.getBasis().getColumn(0) * r;
    v += transform.getOrigin();
    m_buffer.push_back(origin);
    m_buffer.push_back({{1.0, 0.0, 0.0}});
    m_buffer.push_back({{v.x(), v.y(), v.z()}});
    m_buffer.push_back({{1.0, 0.0, 0.0}});

    v = transform.getBasis().getColumn(1) * r;
    v += transform.getOrigin();
    m_buffer.push_back(origin);
    m_buffer.push_back({{0.0, 1.0, 0.0}});
    m_buffer.push_back({{v.x(), v.y(), v.z()}});
    m_buffer.push_back({{0.0, 1.0, 0.0}});

    v = transform.getBasis().getColumn(2) * r;
    v += transform.getOrigin();
    m_buffer.push_back(origin);
    m_buffer.push_back({{0.0, 0.0, 1.0}});
    m_buffer.push_back({{v.x(), v.y(), v.z()}});
    m_buffer.push_back({{0.0, 0.0, 1.0}});
}

void RenderDebugDrawer::drawFrustum(const Frustum& f)
{
    for(size_t i=0; i<f.vertices.size()-1; i++)
    {
        addLine(f.vertices[i], f.vertices[i+1]);
    }

    addLine(f.vertices.back(), f.vertices.front());
}

void RenderDebugDrawer::drawPortal(const Portal& p)
{
    for(size_t i=0; i<p.vertices.size()-1; i++)
    {
        addLine(p.vertices[i], p.vertices[i+1]);
    }

    addLine(p.vertices.back(), p.vertices.front());
}

void RenderDebugDrawer::drawBBox(const btVector3& bb_min, const btVector3& bb_max, const btTransform *transform)
{
    m_obb->rebuild(bb_min, bb_max);
    m_obb->transform = transform;
    m_obb->doTransform();
    drawOBB(m_obb.get());
}

void RenderDebugDrawer::drawOBB(OBB *obb)
{
    struct Polygon *p = obb->polygons;
    addLine(p->vertices[0].position, (p+1)->vertices[0].position);
    addLine(p->vertices[1].position, (p+1)->vertices[3].position);
    addLine(p->vertices[2].position, (p+1)->vertices[2].position);
    addLine(p->vertices[3].position, (p+1)->vertices[1].position);

    for(uint16_t i=0; i<2; i++,p++)
    {
        for(size_t j=0; j<p->vertices.size()-1; j++)
        {
            addLine(p->vertices[j].position, p->vertices[j+1].position);
        }
        addLine(p->vertices.back().position, p->vertices.front().position);
    }
}

void RenderDebugDrawer::drawMeshDebugLines(const std::shared_ptr<BaseMesh> &mesh, const btTransform& transform, const std::vector<btVector3> &overrideVertices, const std::vector<btVector3> &overrideNormals, Render* render)
{
    if(render->m_drawNormals)
    {
        setColor(0.8, 0.0, 0.9);
        if(!overrideVertices.empty())
        {
            const btVector3* ov = &overrideVertices.front();
            const btVector3* on = &overrideNormals.front();
            for(uint32_t i=0; i<mesh->m_vertices.size(); i++,ov++,on++)
            {
                btVector3 v = transform * *ov;
                m_buffer.push_back({{v.x(), v.y(), v.z()}});
                m_buffer.emplace_back( m_color );
                v += transform.getBasis() * *on * 128;
                m_buffer.push_back({{v.x(), v.y(), v.z()}});
                m_buffer.emplace_back( m_color );
            }
        }
        else
        {
            Vertex* mv = mesh->m_vertices.data();
            for (uint32_t i = 0; i < mesh->m_vertices.size(); i++,mv++)
            {
                btVector3 v = transform * mv->position;
                m_buffer.push_back({{v.x(), v.y(), v.z()}});
                m_buffer.emplace_back(m_color);
                v += transform.getBasis() * mv->normal * 128;
                m_buffer.push_back({{v.x(), v.y(), v.z()}});
                m_buffer.emplace_back(m_color);
            }
        }
    }
}

void RenderDebugDrawer::drawSkeletalModelDebugLines(SSBoneFrame *bframe, const btTransform& transform, Render *render)
{
    if(render->m_drawNormals)
    {
        SSBoneTag* btag = bframe->bone_tags.data();
        for(uint16_t i=0; i<bframe->bone_tags.size(); i++,btag++)
        {
            btTransform tr = transform * btag->full_transform;
            drawMeshDebugLines(btag->mesh_base, tr, {}, {}, render);
        }
    }
}

void RenderDebugDrawer::drawEntityDebugLines(Entity* entity, Render* render)
{
    if(entity->m_wasRenderedLines || !(render->m_drawAxis || render->m_drawNormals || render->m_drawBoxes) ||
       !entity->m_visible || (entity->m_bf.animations.model->hide && !(render->m_drawNullMeshes)))
    {
        return;
    }

    if(render->m_drawBoxes)
    {
        debugDrawer.setColor(0.0, 0.0, 1.0);
        debugDrawer.drawOBB(entity->m_obb.get());
    }

    if(render->m_drawAxis)
    {
        // If this happens, the lines after this will get drawn with random colors. I don't care.
        debugDrawer.drawAxis(1000.0, entity->m_transform);
    }

    if(entity->m_bf.animations.model && !entity->m_bf.animations.model->animations.empty())
    {
        debugDrawer.drawSkeletalModelDebugLines(&entity->m_bf, entity->m_transform, render);
    }

    entity->m_wasRenderedLines = true;
}


void RenderDebugDrawer::drawSectorDebugLines(RoomSector *rs)
{
    btVector3 bb_min = {(btScalar)(rs->pos[0] - TR_METERING_SECTORSIZE / 2.0), (btScalar)(rs->pos[1] - TR_METERING_SECTORSIZE / 2.0), (btScalar)rs->floor};
    btVector3 bb_max = {(btScalar)(rs->pos[0] + TR_METERING_SECTORSIZE / 2.0), (btScalar)(rs->pos[1] + TR_METERING_SECTORSIZE / 2.0), (btScalar)rs->ceiling};

    drawBBox(bb_min, bb_max, NULL);
}


void RenderDebugDrawer::drawRoomDebugLines(const Room* room, Render* render)
{
    if(render->m_drawRoomBoxes)
    {
        debugDrawer.setColor(0.0, 0.1, 0.9);
        debugDrawer.drawBBox(room->bb_min, room->bb_max, NULL);
        /*for(uint32_t s=0;s<room->sectors_count;s++)
        {
            drawSectorDebugLines(room->sectors + s);
        }*/
    }

    if(render->m_drawPortals)
    {
        debugDrawer.setColor(0.0, 0.0, 0.0);
        for(const auto& p : room->portals)
        {
            debugDrawer.drawPortal(p);
        }
    }

    if(render->m_drawFrustums)
    {
        debugDrawer.setColor(1.0, 0.0, 0.0);
        for(const auto& frus : room->frustum) {
            debugDrawer.drawFrustum(*frus);
        }
    }

    if(!render->m_skipRoom && (room->mesh != NULL))
    {
        debugDrawer.drawMeshDebugLines(room->mesh, room->transform, {}, {}, render);
    }

    for(auto sm : room->static_mesh)
    {
        if(sm->was_rendered_lines || !Frustum::isOBBVisibleInRoom(sm->obb, *room) ||
          (sm->hide && !render->m_drawDummyStatics))
        {
            continue;
        }

        if(render->m_drawBoxes)
        {
            debugDrawer.setColor(0.0, 1.0, 0.1);
            debugDrawer.drawOBB(sm->obb);
        }

        if(render->m_drawAxis)
        {
            debugDrawer.drawAxis(1000.0, sm->transform);
        }

        debugDrawer.drawMeshDebugLines(sm->mesh, sm->transform, {}, {}, render);

        sm->was_rendered_lines = 1;
    }

    for(const std::shared_ptr<EngineContainer>& cont : room->containers)
    {
        switch(cont->object_type)
        {
            case OBJECT_ENTITY: {
                Entity* ent = static_cast<Entity*>(cont->object);
                if(!ent->m_wasRenderedLines)
                {
                    if(Frustum::isOBBVisibleInRoom(ent->m_obb.get(), *room))
                    {
                        debugDrawer.drawEntityDebugLines(ent, render);
                    }
                    ent->m_wasRenderedLines = true;
                }
                break;
            }
        };
    }
}
