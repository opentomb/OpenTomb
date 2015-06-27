
#include <cmath>
#include <cstdlib>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include "gl_util.h"

#include <bullet/LinearMath/btScalar.h>

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

/*GLhandleARB main_vsh, main_fsh, main_program;
GLint       main_model_mat_pos, main_proj_mat_pos, main_model_proj_mat_pos, main_tr_mat_pos;
*/
/*bool btCollisionObjectIsVisible(btCollisionObject *colObj)
{
    EngineContainer* cont = (EngineContainer*)colObj->getUserPointer();
    return (cont == NULL) || (cont->room == NULL) || (cont->room->is_in_r_list && cont->room->active);
}*/

void Render::initGlobals()
{
    m_settings = RenderSettings();
}

void Render::doShaders()
{
    m_shaderManager.reset( new ShaderManager() );
    m_vertexArrayManager = VertexArrayManager::createManager();
}


void Render::init()
{
    m_blocked = true;
    m_cam = nullptr;

    m_rList.clear();
    m_rListActiveCount= 0;

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

    m_rListActiveCount = 0;
    m_rList.clear();

    m_shaderManager.reset();
}


void Render::renderSkyBox(const btTransform& modelViewProjectionMatrix)
{
    if(m_drawSkybox && (m_world != NULL) && (m_world->sky_box != NULL))
    {
        glDepthMask(GL_FALSE);
        btTransform tr;
        tr.setIdentity();
        tr.getOrigin() = m_cam->m_pos + m_world->sky_box->animations.front().frames.front().bone_tags.front().offset;
        tr.getOrigin().setW(1);
        tr.setRotation( m_world->sky_box->animations.front().frames.front().bone_tags.front().qrotate );
        btTransform fullView = modelViewProjectionMatrix * tr;

        const unlit_tinted_shader_description *shader = m_shaderManager->getStaticMeshShader();
        glUseProgramObjectARB(shader->program);
        btScalar glFullView[16];
        fullView.getOpenGLMatrix(glFullView);
        glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glFullView);
        glUniform1iARB(shader->sampler, 0);
        GLfloat tint[] = { 1, 1, 1, 1 };
        glUniform4fvARB(shader->tint_mult, 1, tint);

        renderer.renderMesh(m_world->sky_box->mesh_tree.front().mesh_base);
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
        glBindBufferARB(GL_ARRAY_BUFFER, mesh->m_animatedVboTexCoordArray);
        // Tell OpenGL to discard the old values
        glBufferDataARB(GL_ARRAY_BUFFER, mesh->m_animatedVertices.size() * sizeof(GLfloat [2]), 0, GL_STREAM_DRAW);
        // Get writable data (to avoid copy)
        GLfloat *data = (GLfloat *) glMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

        size_t offset = 0;
        for(const Polygon& p : mesh->m_polygons)
        {
            if (p.anim_id == 0 || p.isBroken())
            {
                continue;
            }
             AnimSeq* seq = engine_world.anim_sequences + p.anim_id - 1;

            if (seq->uvrotate) {
                printf("?");
            }

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
        glUnmapBufferARB(GL_ARRAY_BUFFER);

        if (mesh->m_animatedElementCount > 0)
        {
            mesh->m_animatedVertexArray->use();

            glBindTexture(GL_TEXTURE_2D, m_world->textures[0]);
            glDrawElements(GL_TRIANGLES, mesh->m_animatedElementCount, GL_UNSIGNED_INT, 0);
        }
    }

    if(!mesh->m_vertices.empty())
    {
        mesh->m_mainVertexArray->use();

        const uint32_t *elementsbase = NULL;

        unsigned long offset = 0;
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
void Render::renderPolygonTransparency(uint16_t &currentTransparency, const struct BSPFaceRef *bsp_ref, const unlit_tinted_shader_description *shader)
{
    // Blending mode switcher.
    // Note that modes above 2 aren't explicitly used in TR textures, only for
    // internal particle processing. Theoretically it's still possible to use
    // them if you will force type via TRTextur utility.
    const struct TransparentPolygonReference *ref = bsp_ref->polygon;
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

    btTransform mvp = m_cam->m_glViewProjMat * bsp_ref->transform;
    btScalar glMvp[16];
    mvp.getOpenGLMatrix(glMvp);

    glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glMvp);

    ref->used_vertex_array->use();
    glBindTexture(GL_TEXTURE_2D, m_world->textures[p->tex_index]);

    glDrawElements(GL_TRIANGLES, ref->count, GL_UNSIGNED_INT, (GLvoid *) (sizeof(GLuint) * ref->firstIndex));
}


void Render::renderBSPFrontToBack(uint16_t &currentTransparency, const std::unique_ptr<BSPNode>& root, const unlit_tinted_shader_description *shader)
{
    btScalar d = planeDist(root->plane, engine_camera.m_pos);

    if(d >= 0)
    {
        if(root->front != NULL)
        {
            renderBSPFrontToBack(currentTransparency, root->front, shader);
        }

        for(const BSPFaceRef *p=root->polygons_front;p!=NULL;p=p->next)
        {
            renderPolygonTransparency(currentTransparency, p, shader);
        }
        for(const BSPFaceRef *p=root->polygons_back;p!=NULL;p=p->next)
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

        for(const BSPFaceRef *p=root->polygons_back;p!=NULL;p=p->next)
        {
            renderPolygonTransparency(currentTransparency, p, shader);
        }
        for(const BSPFaceRef *p=root->polygons_front;p!=NULL;p=p->next)
        {
            renderPolygonTransparency(currentTransparency, p, shader);
        }

        if(root->front != NULL)
        {
            renderBSPFrontToBack(currentTransparency, root->front, shader);
        }
    }
}

void Render::renderBSPBackToFront(uint16_t &currentTransparency, const std::unique_ptr<BSPNode>& root, const unlit_tinted_shader_description *shader)
{
    btScalar d = planeDist(root->plane, engine_camera.m_pos);

    if(d >= 0)
    {
        if(root->back != NULL)
        {
            renderBSPBackToFront(currentTransparency, root->back, shader);
        }

        for(const BSPFaceRef *p=root->polygons_back;p!=NULL;p=p->next)
        {
            renderPolygonTransparency(currentTransparency, p, shader);
        }
        for(const BSPFaceRef *p=root->polygons_front;p!=NULL;p=p->next)
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

        for(const BSPFaceRef *p=root->polygons_front;p!=NULL;p=p->next)
        {
            renderPolygonTransparency(currentTransparency, p, shader);
        }
        for(const BSPFaceRef *p=root->polygons_back;p!=NULL;p=p->next)
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
void Render::renderSkeletalModel(const lit_shader_description *shader, struct SSBoneFrame *bframe, const btTransform& mvMatrix, const btTransform& mvpMatrix)
{
    SSBoneTag* btag = bframe->bone_tags.data();

    for(uint16_t i=0; i<bframe->bone_tags.size(); i++,btag++)
    {
        btTransform mvTransform = mvMatrix * btag->full_transform;
        btScalar glMatrix[16];
        mvTransform.getOpenGLMatrix(glMatrix);
        glUniformMatrix4fvARB(shader->model_view, 1, false, glMatrix);

        btTransform mvpTransform = mvpMatrix * btag->full_transform;
        mvpTransform.getOpenGLMatrix(glMatrix);
        glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glMatrix);

        renderMesh(btag->mesh_base);
        if(btag->mesh_slot)
        {
            renderMesh(btag->mesh_slot);
        }
    }
}

void Render::renderSkeletalModelSkin(const struct lit_shader_description *shader, std::shared_ptr<Entity> ent, const btTransform& mvMatrix, const btTransform& pMatrix)
{
    SSBoneTag* btag = ent->m_bf.bone_tags.data();

    btScalar glMatrix[16+16];
    pMatrix.getOpenGLMatrix(glMatrix);

    glUniformMatrix4fvARB(shader->projection, 1, false, glMatrix);

    for(uint16_t i=0; i<ent->m_bf.bone_tags.size(); i++,btag++)
    {
        btTransform mvTransforms = mvMatrix * btag->full_transform;
        mvTransforms.getOpenGLMatrix(glMatrix+0);

        // Calculate parent transform
        const btTransform* parentTransform = btag->parent ? &btag->parent->full_transform : &ent->m_transform;

        btTransform translate;
        translate.setIdentity();
        translate.getOrigin() += btag->offset;

        btTransform secondTransform = *parentTransform * translate;

        mvTransforms = mvMatrix * secondTransform;
        mvTransforms.getOpenGLMatrix(glMatrix+16);
        glUniformMatrix4fvARB(shader->model_view, 2, false, glMatrix);

        if(btag->mesh_skin)
        {
            renderMesh(btag->mesh_skin);
        }
    }
}

void Render::renderDynamicEntitySkin(const lit_shader_description *shader, std::shared_ptr<Entity> ent, const btTransform& mvMatrix, const btTransform& pMatrix)
{
    btScalar glMatrix[16+16];
    pMatrix.getOpenGLMatrix(glMatrix);
    glUniformMatrix4fvARB(shader->projection, 1, false, glMatrix);

    for(uint16_t i=0; i<ent->m_bf.bone_tags.size(); i++)
    {
        btTransform mvTransforms[2];

        btTransform tr0 = ent->m_bt.bt_body[i]->getWorldTransform();
        btTransform tr1;

        mvTransforms[0] = mvMatrix * tr0;

        // Calculate parent transform
        struct SSBoneTag &btag = ent->m_bf.bone_tags[i];
        bool foundParentTransform = false;
        for (int j = 0; j < ent->m_bf.bone_tags.size(); j++) {
            if (&(ent->m_bf.bone_tags[j]) == btag.parent) {
                tr1 = ent->m_bt.bt_body[j]->getWorldTransform();
                foundParentTransform = true;
                break;
            }
        }
        if (!foundParentTransform)
            tr1 = ent->m_transform;

        btTransform translate;
        translate.setIdentity();
        translate.getOrigin() += btag.offset;

        btTransform secondTransform = tr1 * translate;
        mvTransforms[1] = mvMatrix * secondTransform;

        mvTransforms[0].getOpenGLMatrix(glMatrix+0);
        mvTransforms[1].getOpenGLMatrix(glMatrix+16);

        glUniformMatrix4fvARB(shader->model_view, 2, false, glMatrix);

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
const lit_shader_description* Render::setupEntityLight(std::shared_ptr<Entity> entity, const btTransform& modelViewMatrix, bool skin)
{
    // Calculate lighting
    const lit_shader_description *shader;

    std::shared_ptr<Room> room = entity->m_self->room;
    if(room != NULL)
    {
        std::array<GLfloat,4> ambient_component;

        ambient_component[0] = room->ambient_lighting[0];
        ambient_component[1] = room->ambient_lighting[1];
        ambient_component[2] = room->ambient_lighting[2];
        ambient_component[3] = 1.0f;

        if(room->flags & TR_ROOM_FLAG_WATER)
        {
            engine_world.calculateWaterTint(&ambient_component, false);
        }

        GLenum current_light_number = 0;
        Light *current_light = NULL;

        std::array<GLfloat,3> positions[MAX_NUM_LIGHTS];
        std::array<GLfloat,4> colors[MAX_NUM_LIGHTS];
        GLfloat innerRadiuses[1*MAX_NUM_LIGHTS];
        GLfloat outerRadiuses[1*MAX_NUM_LIGHTS];
        memset(colors, 0, sizeof(colors));
        memset(innerRadiuses, 0, sizeof(innerRadiuses));
        memset(outerRadiuses, 0, sizeof(outerRadiuses));

        for(uint32_t i = 0; i < room->light_count && current_light_number < MAX_NUM_LIGHTS; i++)
        {
            current_light = &room->lights[i];

            btVector3 xyz = entity->m_transform.getOrigin() - current_light->pos;
            btScalar distance = xyz.length();

            // Find color
            colors[current_light_number][0] = std::fmin(std::fmax(current_light->colour[0], 0.0), 1.0);
            colors[current_light_number][1] = std::fmin(std::fmax(current_light->colour[1], 0.0), 1.0);
            colors[current_light_number][2] = std::fmin(std::fmax(current_light->colour[2], 0.0), 1.0);
            colors[current_light_number][3] = std::fmin(std::fmax(current_light->colour[3], 0.0), 1.0);

            if(room->flags & TR_ROOM_FLAG_WATER)
            {
                engine_world.calculateWaterTint(&colors[current_light_number], false);
            }

            // Find position
            btVector3 tmpPos = modelViewMatrix * current_light->pos;
            std::copy(tmpPos.m_floats+0, tmpPos.m_floats+3, positions[current_light_number].begin());

            // Find fall-off
            if(current_light->light_type == LT_SUN)
            {
                innerRadiuses[current_light_number] = 1e20f;
                outerRadiuses[current_light_number] = 1e21f;
                current_light_number++;
            }
            else if(distance <= current_light->outer + 1024.0f && (current_light->light_type == LT_POINT || current_light->light_type == LT_SHADOW))
            {
                innerRadiuses[current_light_number] = std::fabs(current_light->inner);
                outerRadiuses[current_light_number] = std::fabs(current_light->outer);
                current_light_number++;
            }
        }

        shader = m_shaderManager->getEntityShader(current_light_number, skin);
        glUseProgramObjectARB(shader->program);
        glUniform4fvARB(shader->light_ambient, 1, ambient_component.data());
        glUniform4fvARB(shader->light_color, current_light_number, reinterpret_cast<const GLfloat*>(colors));
        glUniform3fvARB(shader->Lightosition, current_light_number, reinterpret_cast<const GLfloat*>(positions));
        glUniform1fvARB(shader->light_inner_radius, current_light_number, innerRadiuses);
        glUniform1fvARB(shader->light_outer_radius, current_light_number, outerRadiuses);
    } else {
        shader = m_shaderManager->getEntityShader(0, skin);
        glUseProgramObjectARB(shader->program);
    }
    return shader;
}

void Render::renderEntity(std::shared_ptr<Entity> entity, const btTransform& modelViewMatrix, const btTransform& modelViewProjectionMatrix, const btTransform& projection)
{
    if(entity->m_wasRendered || !(entity->m_stateFlags & ENTITY_STATE_VISIBLE) || (entity->m_bf.animations.model->hide && !m_drawNullMeshes))
    {
        return;
    }

    // Calculate lighting
    const lit_shader_description *shader = setupEntityLight(entity, modelViewMatrix, false);

    if(entity->m_bf.animations.model && !entity->m_bf.animations.model->animations.empty())
    {
        // base frame offset
        if(entity->m_typeFlags & ENTITY_TYPE_DYNAMIC)
        {
            renderDynamicEntity(shader, entity, modelViewMatrix, modelViewProjectionMatrix);
            ///@TODO: where I need to do bf skinning matrices update? this time ragdoll update function calculates these matrices;
            if (entity->m_bf.bone_tags[0].mesh_skin)
            {
                const lit_shader_description *skinShader = setupEntityLight(entity, modelViewMatrix, true);
                renderDynamicEntitySkin(skinShader, entity, modelViewMatrix, projection);
            }
        }
        else
        {
            btTransform subModelView = modelViewMatrix * entity->m_transform;
            btTransform subModelViewProjection = modelViewProjectionMatrix * entity->m_transform;
            renderSkeletalModel(shader, &entity->m_bf, subModelView, subModelViewProjection);
            if (entity->m_bf.bone_tags[0].mesh_skin)
            {
                const lit_shader_description *skinShader = setupEntityLight(entity, modelViewMatrix, true);
                renderSkeletalModelSkin(skinShader, entity, subModelView, projection);
            }
        }
    }
}

void Render::renderDynamicEntity(const struct lit_shader_description *shader, std::shared_ptr<Entity> entity, const btTransform& modelViewMatrix, const btTransform& modelViewProjectionMatrix)
{
    SSBoneTag* btag = entity->m_bf.bone_tags.data();

    for(uint16_t i=0; i<entity->m_bf.bone_tags.size(); i++,btag++)
    {
        btTransform tr = entity->m_bt.bt_body[i]->getWorldTransform();
        btTransform mvTransform = modelViewMatrix * tr;

        btScalar glMatrix[16];
        mvTransform.getOpenGLMatrix(glMatrix);

        glUniformMatrix4fvARB(shader->model_view, 1, false, glMatrix);

        btTransform mvpTransform = modelViewProjectionMatrix * tr;
        mvpTransform.getOpenGLMatrix(glMatrix);
        glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glMatrix);

        renderMesh(btag->mesh_base);
        if(btag->mesh_slot)
        {
            renderMesh(btag->mesh_slot);
        }
    }
}

///@TODO: add joint between hair and head; do Lara's skinning by vertex position copy (no inverse matrices and other) by vertex map;
void Render::renderHair(std::shared_ptr<Entity> entity, const btTransform &modelViewMatrix, const btTransform &projection)
{
    if(!entity || !entity->m_character || entity->m_character->m_hairs.empty())
        return;

    // Calculate lighting
    const lit_shader_description *shader = setupEntityLight(entity, modelViewMatrix, true);


    for(size_t h=0; h<entity->m_character->m_hairs.size(); h++)
    {
        // First: Head attachment
        btTransform globalHead = entity->m_transform * entity->m_bf.bone_tags[entity->m_character->m_hairs[h]->m_ownerBody].full_transform;
        btTransform globalAttachment = globalHead * entity->m_character->m_hairs[h]->m_ownerBodyHairRoot;

        static constexpr int MatrixCount = 10;

        btScalar hairModelToGlobalMatrices[MatrixCount][16];
        (modelViewMatrix * globalAttachment).getOpenGLMatrix(hairModelToGlobalMatrices[0]);

        // Then: Individual hair pieces
        for(size_t i=0; i<entity->m_character->m_hairs[h]->m_elements.size(); i++)
        {
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
            invOriginToHairModel.getOrigin() -= entity->m_character->m_hairs[h]->m_elements[i].position;

            const btTransform &bt_tr = entity->m_character->m_hairs[h]->m_elements[i].body->getWorldTransform();

            btTransform globalFromHair = bt_tr * invOriginToHairModel;

            (modelViewMatrix * globalFromHair).getOpenGLMatrix(hairModelToGlobalMatrices[i+1]);
        }

        glUniformMatrix4fvARB(shader->model_view, entity->m_character->m_hairs[h]->m_elements.size()+1, GL_FALSE, reinterpret_cast<btScalar*>(hairModelToGlobalMatrices));

        projection.getOpenGLMatrix(hairModelToGlobalMatrices[0]);
        glUniformMatrix4fvARB(shader->projection, 1, GL_FALSE, hairModelToGlobalMatrices[0]);

        renderMesh(entity->m_character->m_hairs[h]->m_mesh);
    }
}

/**
 * drawing world models.
 */
void Render::renderRoom(std::shared_ptr<Room> room, const btTransform &modelViewMatrix, const btTransform &modelViewProjectionMatrix, const btTransform &projection)
{
    btScalar glMat[16];

    const shader_description *lastShader = 0;

    ////start test stencil test code
    bool need_stencil = false;
#if STENCIL_FRUSTUM
    if(!room->frustum.empty()) {
        for(uint16_t i=0;i<room->overlapped_room_list_size;i++)
        {
            if(room->overlapped_room_list[i]->is_in_r_list)
            {
                need_stencil = true;
                break;
            }
        }

        if(need_stencil)
        {
            const unlit_shader_description *shader = m_shaderManager->getStencilShader();
            glUseProgramObjectARB(shader->program);
            engine_camera.m_glViewProjMat.getOpenGLMatrix(glMat);
            glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glMat);
            glEnable(GL_STENCIL_TEST);
            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilFunc(GL_NEVER, 1, 0x00);
            glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

            GLuint stencilVBO;
            glGenBuffersARB(1, &stencilVBO);

            vertex_array_attribute attribs[] = {
                vertex_array_attribute(unlit_shader_description::position, 3, GL_FLOAT, false, stencilVBO, sizeof(GLfloat [3]), 0)
            };

            vertex_array *array = m_vertexArrayManager->createArray(0, 1, attribs);
            array->use();

            for(const auto& f : room->frustum) {
                glBindBufferARB(GL_ARRAY_BUFFER_ARB, stencilVBO);
                glBufferDataARB(GL_ARRAY_BUFFER_ARB, f->vertices.size() * sizeof(GLfloat [3]), NULL, GL_STREAM_DRAW_ARB);

                GLfloat *v = (GLfloat *) glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);

                for(int16_t i=f->vertices.size()-1;i>=0;i--) {
                    *v++ = f->vertices[i].x();
                    *v++ = f->vertices[i].y();
                    *v++ = f->vertices[i].z();
                }

                glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

                glDrawArrays(GL_TRIANGLE_FAN, 0, f->vertices.size());
            }
            glStencilFunc(GL_EQUAL, 1, 0xFF);

            delete array;
            glDeleteBuffersARB(1, &stencilVBO);
        }
    }
#endif

    if(!m_skipRoom && room->mesh)
    {
        btTransform modelViewProjectionTransform = modelViewProjectionMatrix * room->transform;

        const unlit_tinted_shader_description *shader = m_shaderManager->getRoomShader(room->light_mode == 1, room->flags & 1);

        std::array<GLfloat,4> tint;
        engine_world.calculateWaterTint(&tint, true);
        if (shader != lastShader)
        {
            glUseProgramObjectARB(shader->program);
        }

        lastShader = shader;
        glUniform4fvARB(shader->tint_mult, 1, tint.data());
        glUniform1fARB(shader->current_tick, (GLfloat) SDL_GetTicks());
        glUniform1iARB(shader->sampler, 0);
        modelViewProjectionTransform.getOpenGLMatrix(glMat);
        glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glMat);
        renderMesh(room->mesh);
    }

    if (!room->static_mesh.empty())
    {
        glUseProgramObjectARB(m_shaderManager->getStaticMeshShader()->program);
        for(auto sm : room->static_mesh)
        {
            if(sm->was_rendered || !Frustum::isOBBVisibleInRoom(sm->obb, room))
            {
                continue;
            }

            if(sm->hide && !m_drawDummyStatics)
            {
                continue;
            }

            btTransform transform = modelViewProjectionMatrix * sm->transform;
            transform.getOpenGLMatrix(glMat);
            glUniformMatrix4fvARB(m_shaderManager->getStaticMeshShader()->model_view_projection, 1, false, glMat);

            auto tint = sm->tint;

            //If this static mesh is in a water room
            if(room->flags & TR_ROOM_FLAG_WATER)
            {
                engine_world.calculateWaterTint(&tint, false);
            }
            glUniform4fvARB(m_shaderManager->getStaticMeshShader()->tint_mult, 1, tint.data());
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
                std::shared_ptr<Entity> ent = std::static_pointer_cast<Entity>(cont->object);
                if(!ent->m_wasRendered)
                {
                    if(Frustum::isOBBVisibleInRoom(ent->m_obb.get(), room))
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


void Render::renderRoomSprites(std::shared_ptr<Room> room, const btTransform &modelViewMatrix, const btTransform &projectionMatrix)
{
    if (room->sprites_count > 0 && room->sprite_buffer)
    {
        const sprite_shader_description *shader = m_shaderManager->getSpriteShader();
        glUseProgramObjectARB(shader->program);
        btScalar glMat[16];
        modelViewMatrix.getOpenGLMatrix(glMat);
        glUniformMatrix4fvARB(shader->model_view, 1, GL_FALSE, glMat);
        projectionMatrix.getOpenGLMatrix(glMat);
        glUniformMatrix4fvARB(shader->projection, 1, GL_FALSE, glMat);
        glUniform1iARB(shader->sampler, 0);

        room->sprite_buffer->data->use();

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
 * Безопасное добавление комнаты в список рендерера.
 * Если комната уже есть в списке - возвращается ноль и комната повторно не добавляется.
 * Если список полон, то ничего не добавляется
 */
int Render::addRoom(std::shared_ptr<Room> room)
{
    int ret = 0;

    if(room->is_in_r_list || !room->active)
    {
        return 0;
    }

    btVector3 centre = (room->bb_min + room->bb_max) / 2;
    auto dist = m_cam->m_pos.distance(centre);

    if(m_rListActiveCount < m_rList.size())
    {
        m_rList[m_rListActiveCount].room = room;
        m_rList[m_rListActiveCount].active = true;
        m_rList[m_rListActiveCount].dist = dist;
        m_rListActiveCount++;
        ret++;

        if(room->flags & TR_ROOM_FLAG_SKYBOX)
            m_drawSkybox = true;
    }

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
            std::static_pointer_cast<Entity>(cont->object)->m_wasRendered = false;
            std::static_pointer_cast<Entity>(cont->object)->m_wasRenderedLines = false;
            break;
        };
    }

    for(uint32_t i=0; i<room->sprites_count; i++)
    {
        room->sprites[i].was_rendered = false;
    }

    room->is_in_r_list = true;

    return ret;
}


void Render::cleanList()
{
    if(m_world->Character)
    {
        m_world->Character->m_wasRendered = false;
        m_world->Character->m_wasRenderedLines = false;
    }

    for(size_t i=0; i<m_rListActiveCount; i++)
    {
        m_rList[i].active = false;
        m_rList[i].dist = 0.0;
        std::shared_ptr<Room> r = m_rList[i].room;
        m_rList[i].room = NULL;

        r->is_in_r_list = false;
        r->active_frustums = 0;
        r->frustum.clear();
    }

    m_drawSkybox = false;
    m_rListActiveCount = 0;
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

    if(m_drawWire)
    {
        glPolygonMode(GL_FRONT, GL_LINE);
    }
    else if(m_drawPoints)
    {
        glEnable(GL_POINT_SMOOTH);
        glPointSize(4);
        glPolygonMode(GL_FRONT, GL_POINT);
    }
    else
    {
        glPolygonMode(GL_FRONT, GL_FILL);
    }

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);

    renderSkyBox(m_cam->m_glViewProjMat);

    if(m_world->Character)
    {
        renderEntity(m_world->Character, m_cam->m_glViewMat, m_cam->m_glViewProjMat, m_cam->m_glProjMat);
        renderHair(m_world->Character, m_cam->m_glViewMat, m_cam->m_glProjMat);
    }

    /*
     * room rendering
     */
    for(uint32_t i=0; i<m_rListActiveCount; i++)
    {
        renderRoom(m_rList[i].room, m_cam->m_glViewMat, m_cam->m_glViewProjMat, m_cam->m_glProjMat);
    }

    glDisable(GL_CULL_FACE);

    ///@FIXME: reduce number of gl state changes
    for(uint32_t i=0; i<m_rListActiveCount; i++)
    {
        renderRoomSprites(m_rList[i].room, m_cam->m_glViewMat, m_cam->m_glProjMat);
    }

    /*
     * NOW render transparency polygons
     */
    render_dBSP.reset();
    /*First generate BSP from base room mesh - it has good for start splitter polygons*/
    for(uint32_t i=0;i<m_rListActiveCount;i++)
    {
        std::shared_ptr<Room> r = m_rList[i].room;
        if(r->mesh && !r->mesh->m_transparencyPolygons.empty())
        {
            render_dBSP.addNewPolygonList(r->mesh->m_transparentPolygons, r->transform, r->frustum);
        }
    }

    for(uint32_t i=0;i<m_rListActiveCount;i++)
    {
        std::shared_ptr<Room> r = m_rList[i].room;
        // Add transparency polygons from static meshes (if they exists)
        for(auto sm : r->static_mesh)
        {
            if(!sm->mesh->m_transparentPolygons.empty() && Frustum::isOBBVisibleInRoom(sm->obb, r))
            {
                render_dBSP.addNewPolygonList(sm->mesh->m_transparentPolygons, sm->transform, r->frustum);
            }
        }

        // Add transparency polygons from all entities (if they exists) // yes, entities may be animated and intersects with each others;
        for(const std::shared_ptr<EngineContainer>& cont : r->containers)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                std::shared_ptr<Entity> ent = std::static_pointer_cast<Entity>(cont->object);
                if((ent->m_bf.animations.model->transparency_flags == MESH_HAS_TRANSPARENCY) && (ent->m_stateFlags & ENTITY_STATE_VISIBLE) && (Frustum::isOBBVisibleInRoom(ent->m_obb.get(), r)))
                {
                    for(uint16_t j=0;j<ent->m_bf.bone_tags.size();j++)
                    {
                        if(!ent->m_bf.bone_tags[j].mesh_base->m_transparencyPolygons.empty())
                        {
                            btTransform tr = ent->m_transform * ent->m_bf.bone_tags[j].full_transform;
                            render_dBSP.addNewPolygonList(ent->m_bf.bone_tags[j].mesh_base->m_transparentPolygons, tr, r->frustum);
                        }
                    }
                }
            }
        }
    }

    if((engine_world.Character != NULL) && (engine_world.Character->m_bf.animations.model->transparency_flags == MESH_HAS_TRANSPARENCY))
    {
        std::shared_ptr<Entity> ent = engine_world.Character;
        for(uint16_t j=0;j<ent->m_bf.bone_tags.size();j++)
        {
            if(!ent->m_bf.bone_tags[j].mesh_base->m_transparencyPolygons.empty())
            {
                btTransform tr = ent->m_transform * ent->m_bf.bone_tags[j].full_transform;
                render_dBSP.addNewPolygonList(ent->m_bf.bone_tags[j].mesh_base->m_transparentPolygons, tr, {});
            }
        }
    }

    if(render_dBSP.root()->polygons_front != NULL)
    {
        const unlit_tinted_shader_description *shader = m_shaderManager->getRoomShader(false, false);
        glUseProgramObjectARB(shader->program);
        glUniform1iARB(shader->sampler, 0);
        btScalar glMat[16];
        m_cam->m_glViewProjMat.getOpenGLMatrix(glMat);
        glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glMat);
        glDepthMask(GL_FALSE);
        glDisable(GL_ALPHA_TEST);
        glEnable(GL_BLEND);
        uint16_t transparency = BM_OPAQUE;
        renderBSPBackToFront(transparency, render_dBSP.root(), shader);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    //Reset polygon draw mode
    glPolygonMode(GL_FRONT, GL_FILL);
}

void Render::drawListDebugLines()
{
    if (!m_world || !(m_drawBoxes || m_drawRoomBoxes || m_drawPortals || m_drawFrustums || m_drawAxis || m_drawNormals || m_drawColl))
    {
        return;
    }

    if(m_world->Character)
    {
        debugDrawer.drawEntityDebugLines(m_world->Character, this);
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

    for(uint32_t i=0; i<m_rListActiveCount; i++)
    {
        debugDrawer.drawRoomDebugLines(m_rList[i].room, this);
    }

    if(m_drawColl)
    {
        bt_engine_dynamicsWorld->debugDrawWorld();
    }

    if(!debugDrawer.IsEmpty())
    {
        const unlit_shader_description *shader = m_shaderManager->getDebugLineShader();
        glUseProgramObjectARB(shader->program);
        glUniform1iARB(shader->sampler, 0);
        btScalar glMat[16];
        m_cam->m_glViewProjMat.getOpenGLMatrix(glMat);
        glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glMat);
        glBindTexture(GL_TEXTURE_2D, engine_world.textures[engine_world.tex_count - 1]);
        glPointSize( 6.0f );
        glLineWidth( 3.0f );
        debugDrawer.render(this);
    }
}

/**
 * The reccursion algorithm: go through the rooms with portal - frustum occlusion test
 * @portal - we entered to the room through that portal
 * @frus - frustum that intersects the portal
 * @return number of added rooms
 */
int Render::processRoom(struct Portal *portal, const std::shared_ptr<Frustum>& frus)
{
    int ret = 0;
    std::shared_ptr<Room> room = portal->dest_room;                                            // куда ведет портал
    std::shared_ptr<Room> src_room = portal->current_room;                                     // откуда ведет портал

    if((src_room == NULL) || !src_room->active || (room == NULL) || !room->active)
    {
        return 0;
    }

    for(Portal& p : room->portals)                            // перебираем все порталы входной комнаты
    {
        if((p.dest_room->active) && (p.dest_room != src_room))                // обратно идти даже не пытаемся
        {
            auto gen_frus = Frustum::portalFrustumIntersect(&p, frus, this);             // Главная ф-я портального рендерера. Тут и проверка
            if(gen_frus) {
                ret++;
                addRoom(p.dest_room);
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

    std::shared_ptr<Room> curr_room = Room_FindPosCogerrence(m_cam->m_pos, m_cam->m_currentRoom);                // find room that contains camera

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
                addRoom(p.dest_room);                                   // portal destination room
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
                addRoom(r);
            }
        }
    }
}

/**
 * Состыковка рендерера и "мира"
 */
void Render::setWorld(world_s *world)
{
    uint32_t list_size = world->rooms.size() + 128;                               // magick 128 was added for debug and testing

    if(world)
    {
        if(m_rList.size() < list_size)                                    // if old list less than new one requiring
        {
            m_rList.resize(list_size);
        }
    }
    else
    {
        m_rList.resize(list_size);
    }

    world = world;
    m_drawSkybox = false;
    m_rListActiveCount = 0;

    m_cam = &engine_camera;
    //engine_camera.frustum->next = NULL;
    engine_camera.m_currentRoom = NULL;

    for(auto r : world->rooms) {
        r->is_in_r_list = false;
    }
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
    std::array<GLfloat,3> startA{start.x(), start.y(), start.z()};
    std::array<GLfloat,3> endA{end.x(), end.y(), end.z()};
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
    std::array<GLfloat,3> fromA{from.x(), from.y(), from.z()};
    std::array<GLfloat,3> toA{to.x(), to.y(), to.z()};
    std::array<GLfloat,3> colorA{color.x(), color.y(), color.z()};
    addLine(fromA, colorA, toA, colorA);
}

void RenderDebugDrawer::setDebugMode(int debugMode)
{
   m_debugMode = debugMode;
}

void RenderDebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
   //glRasterPos3f(location.x(),  location.y(),  location.z());
   //BMF_DrawString(BMF_GetFont(BMF_kHelvetica10),textString);
}

void RenderDebugDrawer::reportErrorWarning(const char* warningString)
{
   ConsoleInfo::instance().addLine(warningString, FONTSTYLE_CONSOLE_WARNING);
}

void RenderDebugDrawer::drawContactPoint(const btVector3& pointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
{
    drawLine(pointOnB, pointOnB + normalOnB * distance, color);
}

void RenderDebugDrawer::render(Render *render)
{
    if(!m_buffer.empty())
    {
        if (m_glbuffer == 0) {
            glGenBuffersARB(1, &m_glbuffer);
            vertex_array_attribute attribs[] = {
                vertex_array_attribute(unlit_shader_description::position, 3, GL_FLOAT, false, m_glbuffer, sizeof(GLfloat [6]), sizeof(GLfloat [0])),
                vertex_array_attribute(unlit_shader_description::color, 3, GL_FLOAT, false, m_glbuffer, sizeof(GLfloat [6]), sizeof(GLfloat [3]))
            };
            m_vertexArray = render->vertexArrayManager()->createArray(0, 2, attribs);
        }

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_glbuffer);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_buffer.size(), 0, GL_STREAM_DRAW);

        std::array<GLfloat,3>* data = static_cast<std::array<GLfloat,3>*>( glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY) );
        std::copy(m_buffer.begin(), m_buffer.end(), data);
        glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

        m_vertexArray->use();
        glDrawArrays(GL_LINES, 0, m_buffer.size()/2);
    }

    m_color.fill(0);
    m_buffer.clear();
}

void RenderDebugDrawer::drawAxis(btScalar r, const btTransform &transform)
{
    std::array<GLfloat,3> origin{ transform.getOrigin().x(), transform.getOrigin().y(), transform.getOrigin().z() };

    btVector3 v = transform.getBasis()[0] * r;
    m_buffer.push_back(origin);
    m_buffer.push_back({1.0, 0.0, 0.0});
    m_buffer.push_back({v.x(), v.y(), v.z()});
    m_buffer.push_back({1.0, 0.0, 0.0});

    v = transform.getBasis()[1] * r;
    m_buffer.push_back(origin);
    m_buffer.push_back({0.0, 0.0, 1.0});
    m_buffer.push_back({v.x(), v.y(), v.z()});
    m_buffer.push_back({0.0, 0.0, 1.0});

    v = transform.getBasis()[2] * r;
    m_buffer.push_back(origin);
    m_buffer.push_back({0.0, 0.0, 1.0});
    m_buffer.push_back({v.x(), v.y(), v.z()});
    m_buffer.push_back({0.0, 0.0, 1.0});
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

void RenderDebugDrawer::drawBBox(const btVector3& bb_min, const btVector3& bb_max, const btTransform* transform)
{
    m_obb->rebuild(bb_min, bb_max);
    m_obb->transform = transform;
    m_obb->doTransform();
    drawOBB(m_obb.get());
}

void RenderDebugDrawer::drawOBB(struct OBB *obb)
{
    Polygon* p = obb->polygons;
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

void RenderDebugDrawer::drawMeshDebugLines(const std::shared_ptr<BaseMesh>& mesh, const btTransform &transform, const std::vector<btVector3>& overrideVertices, const std::vector<btVector3>& overrideNormals, Render *render)
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
                m_buffer.push_back({v.x(), v.y(), v.z()});
                m_buffer.emplace_back( m_color );
                v += transform.getBasis() * *on * 128;
                m_buffer.push_back({v.x(), v.y(), v.z()});
                m_buffer.emplace_back( m_color );
            }
        }
        else
        {
            Vertex* mv = mesh->m_vertices.data();
            for (uint32_t i = 0; i < mesh->m_vertices.size(); i++,mv++)
            {
                btVector3 v = transform * mv->position;
                m_buffer.push_back({v.x(), v.y(), v.z()});
                m_buffer.emplace_back(m_color);
                v += transform.getBasis() * mv->normal * 128;
                m_buffer.push_back({v.x(), v.y(), v.z()});
                m_buffer.emplace_back(m_color);
            }
        }
    }
}

void RenderDebugDrawer::drawSkeletalModelDebugLines(SSBoneFrame *bframe, const btTransform &transform, Render* render)
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

void RenderDebugDrawer::drawEntityDebugLines(std::shared_ptr<Entity> entity, Render* render)
{
    if(entity->m_wasRenderedLines || !(render->m_drawAxis || render->m_drawNormals || render->m_drawBoxes) ||
       !(entity->m_stateFlags & ENTITY_STATE_VISIBLE) || (entity->m_bf.animations.model->hide && !(render->m_drawNullMeshes)))
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


void RenderDebugDrawer::drawSectorDebugLines(struct room_sector_s *rs)
{
    btVector3 bb_min = {(btScalar)(rs->pos[0] - TR_METERING_SECTORSIZE / 2.0), (btScalar)(rs->pos[1] - TR_METERING_SECTORSIZE / 2.0), (btScalar)rs->floor};
    btVector3 bb_max = {(btScalar)(rs->pos[0] + TR_METERING_SECTORSIZE / 2.0), (btScalar)(rs->pos[1] + TR_METERING_SECTORSIZE / 2.0), (btScalar)rs->ceiling};

    drawBBox(bb_min, bb_max, NULL);
}


void RenderDebugDrawer::drawRoomDebugLines(std::shared_ptr<Room> room, Render* render)
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
        if(sm->was_rendered_lines || !Frustum::isOBBVisibleInRoom(sm->obb, room) ||
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
                std::shared_ptr<Entity> ent = std::static_pointer_cast<Entity>(cont->object);
                if(!ent->m_wasRenderedLines)
                {
                    if(Frustum::isOBBVisibleInRoom(ent->m_obb.get(), room))
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
