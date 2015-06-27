
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

render_t renderer;
DynamicBSP render_dBSP;
extern render_DebugDrawer debugDrawer;

/*GLhandleARB main_vsh, main_fsh, main_program;
GLint       main_model_mat_pos, main_proj_mat_pos, main_model_proj_mat_pos, main_tr_mat_pos;
*/
/*bool btCollisionObjectIsVisible(btCollisionObject *colObj)
{
    EngineContainer* cont = (EngineContainer*)colObj->getUserPointer();
    return (cont == NULL) || (cont->room == NULL) || (cont->room->is_in_r_list && cont->room->active);
}*/

void Render_InitGlobals()
{
    renderer.settings.anisotropy = 0;
    renderer.settings.lod_bias = 0;
    renderer.settings.antialias = 0;
    renderer.settings.antialias_samples = 0;
    renderer.settings.mipmaps = 3;
    renderer.settings.mipmap_mode = 3;
    renderer.settings.texture_border = 8;
    renderer.settings.z_depth = 16;
    renderer.settings.fog_enabled = 1;
    renderer.settings.fog_color[0] = 0.0f;
    renderer.settings.fog_color[1] = 0.0f;
    renderer.settings.fog_color[2] = 0.0f;
    renderer.settings.fog_start_depth = 10000.0f;
    renderer.settings.fog_end_depth = 16000.0f;
}

void Render_DoShaders()
{
    renderer.shader_manager = new shader_manager();
    renderer.vertex_array_manager = vertex_array_manager::createManager();
}


void Render_Init()
{
    renderer.blocked = 1;
    renderer.cam = NULL;

    renderer.r_list = NULL;
    renderer.r_list_size = 0;
    renderer.r_list_active_count= 0;

    renderer.world = NULL;
    renderer.style = 0x00;
}


void Render_Empty(render_p render)
{
    render->world = NULL;

    if(render->r_list)
    {
        render->r_list_active_count = 0;
        render->r_list_size = 0;
        free(render->r_list);
        render->r_list = NULL;
    }

    if (render->shader_manager)
    {
        delete render->shader_manager;
        render->shader_manager = 0;
    }
}


render_list_p Render_CreateRoomListArray(unsigned int count)
{
    render_list_p ret = (render_list_p)malloc(count * sizeof(render_list_t));

    for(unsigned int i=0; i<count; i++)
    {
        ret[i].active = 0;
        ret[i].room = NULL;
        ret[i].dist = 0.0;
    }
    return ret;
}

void Render_SkyBox(const btTransform& modelViewProjectionMatrix)
{
    if((renderer.style & R_DRAW_SKYBOX) && (renderer.world != NULL) && (renderer.world->sky_box != NULL))
    {
        glDepthMask(GL_FALSE);
        btTransform tr;
        tr.setIdentity();
        tr.getOrigin() = renderer.cam->m_pos + renderer.world->sky_box->animations->frames->bone_tags->offset;
        tr.getOrigin().setW(1);
        tr.setRotation( renderer.world->sky_box->animations->frames->bone_tags->qrotate );
        btTransform fullView = modelViewProjectionMatrix * tr;

        const unlit_tinted_shader_description *shader = renderer.shader_manager->getStaticMeshShader();
        glUseProgramObjectARB(shader->program);
        btScalar glFullView[16];
        fullView.getOpenGLMatrix(glFullView);
        glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glFullView);
        glUniform1iARB(shader->sampler, 0);
        GLfloat tint[] = { 1, 1, 1, 1 };
        glUniform4fvARB(shader->tint_mult, 1, tint);

        Render_Mesh(renderer.world->sky_box->mesh_tree->mesh_base);
        glDepthMask(GL_TRUE);
    }
}

/**
 * Opaque meshes drawing
 */
void Render_Mesh(struct base_mesh_s *mesh)
{
    if(mesh->num_animated_elements > 0 || mesh->num_alpha_animated_elements > 0)
    {
        // Respecify the tex coord buffer
        glBindBufferARB(GL_ARRAY_BUFFER, mesh->animated_vbo_texcoord_array);
        // Tell OpenGL to discard the old values
        glBufferDataARB(GL_ARRAY_BUFFER, mesh->animated_vertex_count * sizeof(GLfloat [2]), 0, GL_STREAM_DRAW);
        // Get writable data (to avoid copy)
        GLfloat *data = (GLfloat *) glMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

        size_t offset = 0;
        for(const polygon_s& p : mesh->polygons)
        {
            if (p.anim_id == 0 || Polygon_IsBroken(&p))
            {
                continue;
            }
             anim_seq_p seq = engine_world.anim_sequences + p.anim_id - 1;

            if (seq->uvrotate) {
                printf("?");
            }

            uint16_t frame = (seq->current_frame + p.frame_offset) % seq->frames_count;
            tex_frame_p tf = seq->frames + frame;
            for(const vertex_s& vert : p.vertices)
            {
                const auto& v = vert.tex_coord;
                data[offset + 0] = tf->mat[0+0*2] * v[0] + tf->mat[0+1*2] * v[1] + tf->move[0];
                data[offset + 1] = tf->mat[1+0*2] * v[0] + tf->mat[1+1*2] * v[1] + tf->move[1];

                offset += 2;
            }
        }
        glUnmapBufferARB(GL_ARRAY_BUFFER);

        if (mesh->num_animated_elements > 0)
        {
            mesh->animated_vertex_array->use();

            glBindTexture(GL_TEXTURE_2D, renderer.world->textures[0]);
            glDrawElements(GL_TRIANGLES, mesh->num_animated_elements, GL_UNSIGNED_INT, 0);
        }
    }

    if(!mesh->vertices.empty())
    {
        mesh->main_vertex_array->use();

        const uint32_t *elementsbase = NULL;

        unsigned long offset = 0;
        for(uint32_t texture = 0; texture < mesh->num_texture_pages; texture++)
        {
            if(mesh->element_count_per_texture[texture] == 0)
            {
                continue;
            }

            glBindTexture(GL_TEXTURE_2D, renderer.world->textures[texture]);
            glDrawElements(GL_TRIANGLES, mesh->element_count_per_texture[texture], GL_UNSIGNED_INT, elementsbase + offset);
            offset += mesh->element_count_per_texture[texture];
        }
    }
}


/**
 * draw transparency polygons
 */
void Render_PolygonTransparency(uint16_t &currentTransparency, const struct BSPFaceRef *bsp_ref, const unlit_tinted_shader_description *shader)
{
    // Blending mode switcher.
    // Note that modes above 2 aren't explicitly used in TR textures, only for
    // internal particle processing. Theoretically it's still possible to use
    // them if you will force type via TRTextur utility.
    const struct transparent_polygon_reference_s *ref = bsp_ref->polygon;
    const struct polygon_s *p = ref->polygon;
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

    btTransform mvp = renderer.cam->m_glViewProjMat * bsp_ref->transform;
    btScalar glMvp[16];
    mvp.getOpenGLMatrix(glMvp);

    glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glMvp);

    ref->used_vertex_array->use();
    glBindTexture(GL_TEXTURE_2D, renderer.world->textures[p->tex_index]);

    glDrawElements(GL_TRIANGLES, ref->count, GL_UNSIGNED_INT, (GLvoid *) (sizeof(GLuint) * ref->firstIndex));
}


void Render_BSPFrontToBack(uint16_t &currentTransparency, const std::unique_ptr<BSPNode>& root, const unlit_tinted_shader_description *shader)
{
    btScalar d = planeDist(root->plane, engine_camera.m_pos);

    if(d >= 0)
    {
        if(root->front != NULL)
        {
            Render_BSPFrontToBack(currentTransparency, root->front, shader);
        }

        for(const BSPFaceRef *p=root->polygons_front;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(currentTransparency, p, shader);
        }
        for(const BSPFaceRef *p=root->polygons_back;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(currentTransparency, p, shader);
        }

        if(root->back != NULL)
        {
            Render_BSPFrontToBack(currentTransparency, root->back, shader);
        }
    }
    else
    {
        if(root->back != NULL)
        {
            Render_BSPFrontToBack(currentTransparency, root->back, shader);
        }

        for(const BSPFaceRef *p=root->polygons_back;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(currentTransparency, p, shader);
        }
        for(const BSPFaceRef *p=root->polygons_front;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(currentTransparency, p, shader);
        }

        if(root->front != NULL)
        {
            Render_BSPFrontToBack(currentTransparency, root->front, shader);
        }
    }
}

void Render_BSPBackToFront(uint16_t &currentTransparency, const std::unique_ptr<BSPNode>& root, const unlit_tinted_shader_description *shader)
{
    btScalar d = planeDist(root->plane, engine_camera.m_pos);

    if(d >= 0)
    {
        if(root->back != NULL)
        {
            Render_BSPBackToFront(currentTransparency, root->back, shader);
        }

        for(const BSPFaceRef *p=root->polygons_back;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(currentTransparency, p, shader);
        }
        for(const BSPFaceRef *p=root->polygons_front;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(currentTransparency, p, shader);
        }

        if(root->front != NULL)
        {
            Render_BSPBackToFront(currentTransparency, root->front, shader);
        }
    }
    else
    {
        if(root->front != NULL)
        {
            Render_BSPBackToFront(currentTransparency, root->front, shader);
        }

        for(const BSPFaceRef *p=root->polygons_front;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(currentTransparency, p, shader);
        }
        for(const BSPFaceRef *p=root->polygons_back;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(currentTransparency, p, shader);
        }

        if(root->back != NULL)
        {
            Render_BSPBackToFront(currentTransparency, root->back, shader);
        }
    }
}

void Render_UpdateAnimTextures()                                                // This function is used for updating global animated texture frame
{
    anim_seq_p seq = engine_world.anim_sequences;
    for(uint16_t i=0;i<engine_world.anim_sequences_count;i++,seq++)
    {
        if(seq->frame_lock)
        {
            continue;
        }

        seq->frame_time += engine_frame_time;
        if(seq->frame_time >= seq->frame_rate)
        {
            int j = (seq->frame_time / seq->frame_rate);
            seq->frame_time -= (btScalar)j * seq->frame_rate;

            switch(seq->anim_type)
            {
                case TR_ANIMTEXTURE_REVERSE:
                    if(seq->reverse_direction)
                    {
                        if(seq->current_frame == 0)
                        {
                            seq->current_frame++;
                            seq->reverse_direction = false;
                        }
                        else if(seq->current_frame > 0)
                        {
                            seq->current_frame--;
                        }
                    }
                    else
                    {
                        if(seq->current_frame == seq->frames_count - 1)
                        {
                            seq->current_frame--;
                            seq->reverse_direction = true;
                        }
                        else if(seq->current_frame < seq->frames_count - 1)
                        {
                            seq->current_frame++;
                        }
                        seq->current_frame %= seq->frames_count;                ///@PARANOID
                    }
                    break;

                case TR_ANIMTEXTURE_FORWARD:                                    // inversed in polygon anim. texture frames
                case TR_ANIMTEXTURE_BACKWARD:
                    seq->current_frame++;
                    seq->current_frame %= seq->frames_count;
                    break;
            };
        }
    }
}

/**
 * skeletal model drawing
 */
void Render_SkeletalModel(const lit_shader_description *shader, struct ss_bone_frame_s *bframe, const btTransform& mvMatrix, const btTransform& mvpMatrix)
{
    ss_bone_tag_p btag = bframe->bone_tags;

    for(uint16_t i=0; i<bframe->bone_tag_count; i++,btag++)
    {
        btTransform mvTransform = mvMatrix * btag->full_transform;
        btScalar glMatrix[16];
        mvTransform.getOpenGLMatrix(glMatrix);
        glUniformMatrix4fvARB(shader->model_view, 1, false, glMatrix);

        btTransform mvpTransform = mvpMatrix * btag->full_transform;
        mvpTransform.getOpenGLMatrix(glMatrix);
        glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glMatrix);

        Render_Mesh(btag->mesh_base);
        if(btag->mesh_slot)
        {
            Render_Mesh(btag->mesh_slot);
        }
    }
}

void Render_SkeletalModelSkin(const struct lit_shader_description *shader, std::shared_ptr<Entity> ent, const btTransform& mvMatrix, const btTransform& pMatrix)
{
    ss_bone_tag_p btag = ent->m_bf.bone_tags;

    btScalar glMatrix[16+16];
    pMatrix.getOpenGLMatrix(glMatrix);

    glUniformMatrix4fvARB(shader->projection, 1, false, glMatrix);

    for(uint16_t i=0; i<ent->m_bf.bone_tag_count; i++,btag++)
    {
        btTransform mvTransforms = mvMatrix * btag->full_transform;
        mvTransforms.getOpenGLMatrix(glMatrix+0);

        // Calculate parent transform
        const btTransform* parentTransform = btag->parent ? &btag->parent->full_transform : &ent->m_transform;

        btTransform translate;
        translate.setIdentity();
        Mat4_Translate(translate, btag->offset);

        btTransform secondTransform = *parentTransform * translate;

        mvTransforms = mvMatrix * secondTransform;
        mvTransforms.getOpenGLMatrix(glMatrix+16);
        glUniformMatrix4fvARB(shader->model_view, 2, false, glMatrix);

        if(btag->mesh_skin)
        {
            Render_Mesh(btag->mesh_skin);
        }
    }
}

void Render_DynamicEntitySkin(const struct lit_shader_description *shader, std::shared_ptr<Entity> ent, const btTransform& mvMatrix, const btTransform& pMatrix)
{
    btScalar glMatrix[16+16];
    pMatrix.getOpenGLMatrix(glMatrix);
    glUniformMatrix4fvARB(shader->projection, 1, false, glMatrix);

    for(uint16_t i=0; i<ent->m_bf.bone_tag_count; i++)
    {
        btTransform mvTransforms[2];

        btTransform tr0 = ent->m_bt.bt_body[i]->getWorldTransform();
        btTransform tr1;

        mvTransforms[0] = mvMatrix * tr0;

        // Calculate parent transform
        struct ss_bone_tag_s &btag = ent->m_bf.bone_tags[i];
        bool foundParentTransform = false;
        for (int j = 0; j < ent->m_bf.bone_tag_count; j++) {
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
        Mat4_Translate(translate, btag.offset);

        btTransform secondTransform = tr1 * translate;
        mvTransforms[1] = mvMatrix * secondTransform;

        mvTransforms[0].getOpenGLMatrix(glMatrix+0);
        mvTransforms[1].getOpenGLMatrix(glMatrix+16);

        glUniformMatrix4fvARB(shader->model_view, 2, false, glMatrix);

        if(btag.mesh_skin)
        {
            Render_Mesh(btag.mesh_skin);
        }
    }
}

/**
 * Sets up the light calculations for the given entity based on its current
 * room. Returns the used shader, which will have been made current already.
 */
const lit_shader_description *render_setupEntityLight(std::shared_ptr<Entity> entity, const btTransform& modelViewMatrix, bool skin)
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
            Render_CalculateWaterTint(&ambient_component, 0);
        }

        GLenum current_light_number = 0;
        light_s *current_light = NULL;

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
                Render_CalculateWaterTint(&colors[current_light_number], 0);
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

        shader = renderer.shader_manager->getEntityShader(current_light_number, skin);
        glUseProgramObjectARB(shader->program);
        glUniform4fvARB(shader->light_ambient, 1, ambient_component.data());
        glUniform4fvARB(shader->light_color, current_light_number, reinterpret_cast<const GLfloat*>(colors));
        glUniform3fvARB(shader->light_position, current_light_number, reinterpret_cast<const GLfloat*>(positions));
        glUniform1fvARB(shader->light_inner_radius, current_light_number, innerRadiuses);
        glUniform1fvARB(shader->light_outer_radius, current_light_number, outerRadiuses);
    } else {
        shader = renderer.shader_manager->getEntityShader(0, skin);
        glUseProgramObjectARB(shader->program);
    }
    return shader;
}

void Render_Entity(std::shared_ptr<Entity> entity, const btTransform& modelViewMatrix, const btTransform& modelViewProjectionMatrix, const btTransform& projection)
{
    if(entity->m_wasRendered || !(entity->m_stateFlags & ENTITY_STATE_VISIBLE) || (entity->m_bf.animations.model->hide && !(renderer.style & R_DRAW_NULLMESHES)))
    {
        return;
    }

    // Calculate lighting
    const lit_shader_description *shader = render_setupEntityLight(entity, modelViewMatrix, false);

    if(entity->m_bf.animations.model && entity->m_bf.animations.model->animations)
    {
        // base frame offset
        if(entity->m_typeFlags & ENTITY_TYPE_DYNAMIC)
        {
            Render_DynamicEntity(shader, entity, modelViewMatrix, modelViewProjectionMatrix);
            ///@TODO: where I need to do bf skinning matrices update? this time ragdoll update function calculates these matrices;
            if (entity->m_bf.bone_tags[0].mesh_skin)
            {
                const lit_shader_description *skinShader = render_setupEntityLight(entity, modelViewMatrix, true);
                Render_DynamicEntitySkin(skinShader, entity, modelViewMatrix, projection);
            }
        }
        else
        {
            btTransform subModelView = modelViewMatrix * entity->m_transform;
            btTransform subModelViewProjection = modelViewProjectionMatrix * entity->m_transform;
            Render_SkeletalModel(shader, &entity->m_bf, subModelView, subModelViewProjection);
            if (entity->m_bf.bone_tags[0].mesh_skin)
            {
                const lit_shader_description *skinShader = render_setupEntityLight(entity, modelViewMatrix, true);
                Render_SkeletalModelSkin(skinShader, entity, subModelView, projection);
            }
        }
    }
}

void Render_DynamicEntity(const struct lit_shader_description *shader, std::shared_ptr<Entity> entity, const btTransform& modelViewMatrix, const btTransform& modelViewProjectionMatrix)
{
    ss_bone_tag_p btag = entity->m_bf.bone_tags;

    for(uint16_t i=0; i<entity->m_bf.bone_tag_count; i++,btag++)
    {
        btTransform tr = entity->m_bt.bt_body[i]->getWorldTransform();
        btTransform mvTransform = modelViewMatrix * tr;

        btScalar glMatrix[16];
        mvTransform.getOpenGLMatrix(glMatrix);

        glUniformMatrix4fvARB(shader->model_view, 1, false, glMatrix);

        btTransform mvpTransform = modelViewProjectionMatrix * tr;
        mvpTransform.getOpenGLMatrix(glMatrix);
        glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glMatrix);

        Render_Mesh(btag->mesh_base);
        if(btag->mesh_slot)
        {
            Render_Mesh(btag->mesh_slot);
        }
    }
}

///@TODO: add joint between hair and head; do Lara's skinning by vertex position copy (no inverse matrices and other) by vertex map;
void Render_Hair(std::shared_ptr<Entity> entity, const btTransform &modelViewMatrix, const btTransform &projection)
{
    if(!entity || !entity->m_character || entity->m_character->m_hairs.empty())
        return;

    // Calculate lighting
    const lit_shader_description *shader = render_setupEntityLight(entity, modelViewMatrix, true);


    for(size_t h=0; h<entity->m_character->m_hairs.size(); h++)
    {
        // First: Head attachment
        btTransform globalHead = entity->m_transform * entity->m_bf.bone_tags[entity->m_character->m_hairs[h]->owner_body].full_transform;
        btTransform globalAttachment = globalHead * entity->m_character->m_hairs[h]->owner_body_hair_root;

        static constexpr int MatrixCount = 10;

        btScalar hairModelToGlobalMatrices[MatrixCount][16];
        (modelViewMatrix * globalAttachment).getOpenGLMatrix(hairModelToGlobalMatrices[0]);

        // Then: Individual hair pieces
        for(uint16_t i=0; i<entity->m_character->m_hairs[h]->element_count; i++)
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
            Mat4_Translate(invOriginToHairModel, -entity->m_character->m_hairs[h]->elements[i].position);

            const btTransform &bt_tr = entity->m_character->m_hairs[h]->elements[i].body->getWorldTransform();

            btTransform globalFromHair = bt_tr * invOriginToHairModel;

            (modelViewMatrix * globalFromHair).getOpenGLMatrix(hairModelToGlobalMatrices[i+1]);
        }

        glUniformMatrix4fvARB(shader->model_view, entity->m_character->m_hairs[h]->element_count+1, GL_FALSE, reinterpret_cast<btScalar*>(hairModelToGlobalMatrices));

        projection.getOpenGLMatrix(hairModelToGlobalMatrices[0]);
        glUniformMatrix4fvARB(shader->projection, 1, GL_FALSE, hairModelToGlobalMatrices[0]);

        Render_Mesh(entity->m_character->m_hairs[h]->mesh);
    }
}

/**
 * drawing world models.
 */
void Render_Room(std::shared_ptr<Room> room, struct render_s *render, const btTransform &modelViewMatrix, const btTransform &modelViewProjectionMatrix, const btTransform &projection)
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
            const unlit_shader_description *shader = render->shader_manager->getStencilShader();
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

            vertex_array *array = render->vertex_array_manager->createArray(0, 1, attribs);
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

    if(!(renderer.style & R_SKIP_ROOM) && room->mesh)
    {
        btTransform modelViewProjectionTransform = modelViewProjectionMatrix * room->transform;

        const unlit_tinted_shader_description *shader = render->shader_manager->getRoomShader(room->light_mode == 1, room->flags & 1);

        std::array<GLfloat,4> tint;
        Render_CalculateWaterTint(&tint, 1);
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
        Render_Mesh(room->mesh);
    }

    if (!room->static_mesh.empty())
    {
        glUseProgramObjectARB(render->shader_manager->getStaticMeshShader()->program);
        for(auto sm : room->static_mesh)
        {
            if(sm->was_rendered || !Frustum::isOBBVisibleInRoom(sm->obb, room))
            {
                continue;
            }

            if((sm->hide == 1) && !(renderer.style & R_DRAW_DUMMY_STATICS))
            {
                continue;
            }

            btTransform transform = modelViewProjectionMatrix * sm->transform;
            transform.getOpenGLMatrix(glMat);
            glUniformMatrix4fvARB(render->shader_manager->getStaticMeshShader()->model_view_projection, 1, false, glMat);
            base_mesh_s *mesh = sm->mesh;

            auto tint = sm->tint;

            //If this static mesh is in a water room
            if(room->flags & TR_ROOM_FLAG_WATER)
            {
                Render_CalculateWaterTint(&tint, 0);
            }
            glUniform4fvARB(render->shader_manager->getStaticMeshShader()->tint_mult, 1, tint.data());
            Render_Mesh(mesh);
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
                if(ent->m_wasRendered == 0)
                {
                    if(Frustum::isOBBVisibleInRoom(ent->m_obb.get(), room))
                    {
                        Render_Entity(ent, modelViewMatrix, modelViewProjectionMatrix, projection);
                    }
                    ent->m_wasRendered = 1;
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


void Render_Room_Sprites(std::shared_ptr<Room> room, struct render_s *render, const btTransform &modelViewMatrix, const btTransform &projectionMatrix)
{
    if (room->sprites_count > 0 && room->sprite_buffer)
    {
        const sprite_shader_description *shader = render->shader_manager->getSpriteShader();
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

            glBindTexture(GL_TEXTURE_2D, renderer.world->textures[texture]);
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
int Render_AddRoom(std::shared_ptr<Room> room)
{
    int ret = 0;

    if(room->is_in_r_list || !room->active)
    {
        return 0;
    }

    btVector3 centre = (room->bb_min + room->bb_max) / 2;
    auto dist = (renderer.cam->m_pos - centre).length();

    if(renderer.r_list_active_count < renderer.r_list_size)
    {
        renderer.r_list[renderer.r_list_active_count].room = room;
        renderer.r_list[renderer.r_list_active_count].active = 1;
        renderer.r_list[renderer.r_list_active_count].dist = dist;
        renderer.r_list_active_count++;
        ret++;

        if(room->flags & TR_ROOM_FLAG_SKYBOX)
            renderer.style |= R_DRAW_SKYBOX;
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
            std::static_pointer_cast<Entity>(cont->object)->m_wasRendered = 0;
            std::static_pointer_cast<Entity>(cont->object)->m_wasRenderedLines = 0;
            break;
        };
    }

    for(uint32_t i=0; i<room->sprites_count; i++)
    {
        room->sprites[i].was_rendered = 0;
    }

    room->is_in_r_list = 1;

    return ret;
}


void Render_CleanList()
{
    if(renderer.world->Character)
    {
        renderer.world->Character->m_wasRendered = 0;
        renderer.world->Character->m_wasRenderedLines = 0;
    }

    for(uint32_t i=0; i<renderer.r_list_active_count; i++)
    {
        renderer.r_list[i].active = 0;
        renderer.r_list[i].dist = 0.0;
        std::shared_ptr<Room> r = renderer.r_list[i].room;
        renderer.r_list[i].room = NULL;

        r->is_in_r_list = 0;
        r->active_frustums = 0;
        r->frustum.clear();
    }

    renderer.style &= ~R_DRAW_SKYBOX;
    renderer.r_list_active_count = 0;
}

/**
 * Render all visible rooms
 */
void Render_DrawList()
{
    if(!renderer.world)
    {
        return;
    }

    if(renderer.style & R_DRAW_WIRE)
    {
        glPolygonMode(GL_FRONT, GL_LINE);
    }
    else if(renderer.style & R_DRAW_POINTS)
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

    Render_SkyBox(renderer.cam->m_glViewProjMat);

    if(renderer.world->Character)
    {
        Render_Entity(renderer.world->Character, renderer.cam->m_glViewMat, renderer.cam->m_glViewProjMat, renderer.cam->m_glProjMat);
        Render_Hair(renderer.world->Character, renderer.cam->m_glViewMat, renderer.cam->m_glProjMat);
    }

    /*
     * room rendering
     */
    for(uint32_t i=0; i<renderer.r_list_active_count; i++)
    {
        Render_Room(renderer.r_list[i].room, &renderer, renderer.cam->m_glViewMat, renderer.cam->m_glViewProjMat, renderer.cam->m_glProjMat);
    }

    glDisable(GL_CULL_FACE);

    ///@FIXME: reduce number of gl state changes
    for(uint32_t i=0; i<renderer.r_list_active_count; i++)
    {
        Render_Room_Sprites(renderer.r_list[i].room, &renderer, renderer.cam->m_glViewMat, renderer.cam->m_glProjMat);
    }

    /*
     * NOW render transparency polygons
     */
    render_dBSP.reset();
    /*First generate BSP from base room mesh - it has good for start splitter polygons*/
    for(uint32_t i=0;i<renderer.r_list_active_count;i++)
    {
        std::shared_ptr<Room> r = renderer.r_list[i].room;
        if((r->mesh != NULL) && (r->mesh->transparency_polygons != NULL))
        {
            render_dBSP.addNewPolygonList(r->mesh->transparent_polygon_count, r->mesh->transparent_polygons, r->transform, r->frustum);
        }
    }

    for(uint32_t i=0;i<renderer.r_list_active_count;i++)
    {
        std::shared_ptr<Room> r = renderer.r_list[i].room;
        // Add transparency polygons from static meshes (if they exists)
        for(auto sm : r->static_mesh)
        {
            if((sm->mesh->transparency_polygons != NULL) && Frustum::isOBBVisibleInRoom(sm->obb, r))
            {
                render_dBSP.addNewPolygonList(sm->mesh->transparent_polygon_count, sm->mesh->transparent_polygons, sm->transform, r->frustum);
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
                    for(uint16_t j=0;j<ent->m_bf.bone_tag_count;j++)
                    {
                        if(ent->m_bf.bone_tags[j].mesh_base->transparency_polygons != NULL)
                        {
                            btTransform tr = ent->m_transform * ent->m_bf.bone_tags[j].full_transform;
                            render_dBSP.addNewPolygonList(ent->m_bf.bone_tags[j].mesh_base->transparent_polygon_count, ent->m_bf.bone_tags[j].mesh_base->transparent_polygons, tr, r->frustum);
                        }
                    }
                }
            }
        }
    }

    if((engine_world.Character != NULL) && (engine_world.Character->m_bf.animations.model->transparency_flags == MESH_HAS_TRANSPARENCY))
    {
        std::shared_ptr<Entity> ent = engine_world.Character;
        for(uint16_t j=0;j<ent->m_bf.bone_tag_count;j++)
        {
            if(ent->m_bf.bone_tags[j].mesh_base->transparency_polygons != NULL)
            {
                btTransform tr = ent->m_transform * ent->m_bf.bone_tags[j].full_transform;
                render_dBSP.addNewPolygonList(ent->m_bf.bone_tags[j].mesh_base->transparent_polygon_count, ent->m_bf.bone_tags[j].mesh_base->transparent_polygons, tr, {});
            }
        }
    }

    if(render_dBSP.root()->polygons_front != NULL)
    {
        const unlit_tinted_shader_description *shader = renderer.shader_manager->getRoomShader(false, false);
        glUseProgramObjectARB(shader->program);
        glUniform1iARB(shader->sampler, 0);
        btScalar glMat[16];
        renderer.cam->m_glViewProjMat.getOpenGLMatrix(glMat);
        glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glMat);
        glDepthMask(GL_FALSE);
        glDisable(GL_ALPHA_TEST);
        glEnable(GL_BLEND);
        uint16_t transparency = BM_OPAQUE;
        Render_BSPBackToFront(transparency, render_dBSP.root(), shader);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    //Reset polygon draw mode
    glPolygonMode(GL_FRONT, GL_FILL);
}

void Render_DrawList_DebugLines()
{
    if (!renderer.world || !(renderer.style & (R_DRAW_BOXES | R_DRAW_ROOMBOXES | R_DRAW_PORTALS | R_DRAW_FRUSTUMS | R_DRAW_AXIS | R_DRAW_NORMALS | R_DRAW_COLL)))
    {
        return;
    }

    if(renderer.world->Character)
    {
        debugDrawer.drawEntityDebugLines(renderer.world->Character);
    }

    /*
     * Render world debug information
     */
    if((renderer.style & R_DRAW_NORMALS) && (renderer.world != NULL) && (renderer.world->sky_box != NULL))
    {
        btTransform tr;
        tr.setIdentity();
        tr.getOrigin() = renderer.cam->m_pos + renderer.world->sky_box->animations->frames->bone_tags->offset;
        tr.setRotation(renderer.world->sky_box->animations->frames->bone_tags->qrotate);
        debugDrawer.drawMeshDebugLines(renderer.world->sky_box->mesh_tree->mesh_base, tr, {}, {});
    }

    for(uint32_t i=0; i<renderer.r_list_active_count; i++)
    {
        debugDrawer.drawRoomDebugLines(renderer.r_list[i].room, &renderer);
    }

    if(renderer.style & R_DRAW_COLL)
    {
        bt_engine_dynamicsWorld->debugDrawWorld();
    }

    if(!debugDrawer.IsEmpty())
    {
        const unlit_shader_description *shader = renderer.shader_manager->getDebugLineShader();
        glUseProgramObjectARB(shader->program);
        glUniform1iARB(shader->sampler, 0);
        btScalar glMat[16];
        renderer.cam->m_glViewProjMat.getOpenGLMatrix(glMat);
        glUniformMatrix4fvARB(shader->model_view_projection, 1, false, glMat);
        glBindTexture(GL_TEXTURE_2D, engine_world.textures[engine_world.tex_count - 1]);
        glPointSize( 6.0f );
        glLineWidth( 3.0f );
        debugDrawer.render();
    }
}

/**
 * The reccursion algorithm: go through the rooms with portal - frustum occlusion test
 * @portal - we entered to the room through that portal
 * @frus - frustum that intersects the portal
 * @return number of added rooms
 */
int Render_ProcessRoom(struct portal_s *portal, const std::shared_ptr<Frustum>& frus)
{
    int ret = 0;
    std::shared_ptr<Room> room = portal->dest_room;                                            // куда ведет портал
    std::shared_ptr<Room> src_room = portal->current_room;                                     // откуда ведет портал

    if((src_room == NULL) || !src_room->active || (room == NULL) || !room->active)
    {
        return 0;
    }

    for(portal_s& p : room->portals)                            // перебираем все порталы входной комнаты
    {
        if((p.dest_room->active) && (p.dest_room != src_room))                // обратно идти даже не пытаемся
        {
            auto gen_frus = Frustum::portalFrustumIntersect(&p, frus, &renderer);             // Главная ф-я портального рендерера. Тут и проверка
            if(gen_frus) {
                ret++;
                Render_AddRoom(p.dest_room);
                Render_ProcessRoom(&p, gen_frus);
            }
        }
    }
    return ret;
}

/**
 * Renderer list generation by current world and camera
 */
void Render_GenWorldList()
{
    if(renderer.world == NULL)
    {
        return;
    }

    Render_CleanList();                                                         // clear old render list
    debugDrawer.reset();
    //renderer.cam->frustum->next = NULL;

    std::shared_ptr<Room> curr_room = Room_FindPosCogerrence(renderer.cam->m_pos, renderer.cam->m_currentRoom);                // find room that contains camera

    renderer.cam->m_currentRoom = curr_room;                                     // set camera's cuttent room pointer
    if(curr_room != NULL)                                                       // camera located in some room
    {
        curr_room->frustum.clear();                                              // room with camera inside has no frustums!
        curr_room->max_path = 0;
        Render_AddRoom(curr_room);                                              // room with camera inside adds to the render list immediately
        for(portal_s& p : curr_room->portals)                   // go through all start room portals
        {
            auto last_frus = Frustum::portalFrustumIntersect(&p, renderer.cam->frustum, &renderer);
            if(last_frus) {
                Render_AddRoom(p.dest_room);                                   // portal destination room
                last_frus->parents_count = 1;                                   // created by camera
                Render_ProcessRoom(&p, last_frus);                               // next start reccursion algorithm
            }
        }
    }
    else                                                                        // camera is out of all rooms
    {
        for(auto r : renderer.world->rooms)
        {
            if(renderer.cam->frustum->isAABBVisible(r->bb_min, r->bb_max))
            {
                Render_AddRoom(r);
            }
        }
    }
}

/**
 * Состыковка рендерера и "мира"
 */
void Render_SetWorld(struct world_s *world)
{
    uint32_t list_size = world->rooms.size() + 128;                               // magick 128 was added for debug and testing

    if(renderer.world)
    {
        if(renderer.r_list_size < list_size)                                    // if old list less than new one requiring
        {
            renderer.r_list = (render_list_p)realloc(renderer.r_list, list_size * sizeof(render_list_t));
            for(uint32_t i=0; i<list_size; i++)
            {
                renderer.r_list[i].active = 0;
                renderer.r_list[i].room = NULL;
                renderer.r_list[i].dist = 0.0;
            }
        }
    }
    else
    {
        renderer.r_list = Render_CreateRoomListArray(list_size);
    }

    renderer.world = world;
    renderer.style &= ~R_DRAW_SKYBOX;
    renderer.r_list_size = list_size;
    renderer.r_list_active_count = 0;

    renderer.cam = &engine_camera;
    //engine_camera.frustum->next = NULL;
    engine_camera.m_currentRoom = NULL;

    for(auto r : world->rooms)
    {
        r->is_in_r_list = 0;
    }
}


void Render_CalculateWaterTint(std::array<float,4>* tint, uint8_t fixed_colour)
{
    if(engine_world.version < TR_IV)  // If water room and level is TR1-3
    {
        if(engine_world.version < TR_III)
        {
             // Placeholder, color very similar to TR1 PSX ver.
            if(fixed_colour > 0)
            {
                (*tint)[0] = 0.585f;
                (*tint)[1] = 0.9f;
                (*tint)[2] = 0.9f;
                (*tint)[3] = 1.0f;
            }
            else
            {
                (*tint)[0] *= 0.585f;
                (*tint)[1] *= 0.9f;
                (*tint)[2] *= 0.9f;
            }
        }
        else
        {
            // TOMB3 - closely matches TOMB3
            if(fixed_colour > 0)
            {
                (*tint)[0] = 0.275f;
                (*tint)[1] = 0.45f;
                (*tint)[2] = 0.5f;
                (*tint)[3] = 1.0f;
            }
            else
            {
                (*tint)[0] *= 0.275f;
                (*tint)[1] *= 0.45f;
                (*tint)[2] *= 0.5f;
            }
        }
    }
    else
    {
        if(fixed_colour > 0)
        {
            (*tint)[0] = 1.0f;
            (*tint)[1] = 1.0f;
            (*tint)[2] = 1.0f;
            (*tint)[3] = 1.0f;
        }
    }
}

/**
 * DEBUG PRIMITIVES RENDERING
 */
render_DebugDrawer::render_DebugDrawer()
    : m_obb(new obb_s())
{
}

render_DebugDrawer::~render_DebugDrawer()
{
}

void render_DebugDrawer::reset()
{
    m_buffer.clear();
}

void render_DebugDrawer::addLine(const std::array<GLfloat,3>& start, const std::array<GLfloat,3>& end)
{
    addLine(start, m_color, end, m_color);
}

void render_DebugDrawer::addLine(const btVector3& start, const btVector3& end)
{
    std::array<GLfloat,3> startA{start.x(), start.y(), start.z()};
    std::array<GLfloat,3> endA{end.x(), end.y(), end.z()};
    addLine(startA, m_color, endA, m_color);
}

void render_DebugDrawer::addLine(const std::array<GLfloat,3>& start, const std::array<GLfloat,3>& startColor, const std::array<GLfloat,3>& end, const std::array<GLfloat,3>& endColor)
{
    m_buffer.emplace_back(start);
    m_buffer.emplace_back(startColor);
    m_buffer.emplace_back(end);
    m_buffer.emplace_back(endColor);
}

void render_DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3 &color)
{
    std::array<GLfloat,3> fromA{from.x(), from.y(), from.z()};
    std::array<GLfloat,3> toA{to.x(), to.y(), to.z()};
    std::array<GLfloat,3> colorA{color.x(), color.y(), color.z()};
    addLine(fromA, colorA, toA, colorA);
}

void render_DebugDrawer::setDebugMode(int debugMode)
{
   m_debugMode = debugMode;
}

void render_DebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
   //glRasterPos3f(location.x(),  location.y(),  location.z());
   //BMF_DrawString(BMF_GetFont(BMF_kHelvetica10),textString);
}

void render_DebugDrawer::reportErrorWarning(const char* warningString)
{
   ConsoleInfo::instance().addLine(warningString, FONTSTYLE_CONSOLE_WARNING);
}

void render_DebugDrawer::drawContactPoint(const btVector3& pointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
{
    drawLine(pointOnB, pointOnB + normalOnB * distance, color);
}

void render_DebugDrawer::render()
{
    if(!m_buffer.empty())
    {
        if (m_glbuffer == 0) {
            glGenBuffersARB(1, &m_glbuffer);
            vertex_array_attribute attribs[] = {
                vertex_array_attribute(unlit_shader_description::position, 3, GL_FLOAT, false, m_glbuffer, sizeof(GLfloat [6]), sizeof(GLfloat [0])),
                vertex_array_attribute(unlit_shader_description::color, 3, GL_FLOAT, false, m_glbuffer, sizeof(GLfloat [6]), sizeof(GLfloat [3]))
            };
            m_vertexArray = renderer.vertex_array_manager->createArray(0, 2, attribs);
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

void render_DebugDrawer::drawAxis(btScalar r, const btTransform &transform)
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

void render_DebugDrawer::drawFrustum(const Frustum& f)
{
    for(size_t i=0; i<f.vertices.size()-1; i++)
    {
        addLine(f.vertices[i], f.vertices[i+1]);
    }

    addLine(f.vertices.back(), f.vertices.front());
}

void render_DebugDrawer::drawPortal(const portal_s& p)
{
    for(size_t i=0; i<p.vertices.size()-1; i++)
    {
        addLine(p.vertices[i], p.vertices[i+1]);
    }

    addLine(p.vertices.back(), p.vertices.front());
}

void render_DebugDrawer::drawBBox(const btVector3& bb_min, const btVector3& bb_max, const btTransform* transform)
{
    OBB_Rebuild(m_obb.get(), bb_min, bb_max);
    m_obb->transform = transform;
    OBB_Transform(m_obb.get());
    drawOBB(m_obb.get());
}

void render_DebugDrawer::drawOBB(struct obb_s *obb)
{
    polygon_s* p = obb->polygons;
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

void render_DebugDrawer::drawMeshDebugLines(struct base_mesh_s *mesh, const btTransform &transform, const std::vector<btVector3>& overrideVertices, const std::vector<btVector3>& overrideNormals)
{
    if(renderer.style & R_DRAW_NORMALS)
    {
        setColor(0.8, 0.0, 0.9);
        if(!overrideVertices.empty())
        {
            const btVector3* ov = &overrideVertices.front();
            const btVector3* on = &overrideNormals.front();
            for(uint32_t i=0; i<mesh->vertices.size(); i++,ov++,on++)
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
            vertex_s* mv = mesh->vertices.data();
            for (uint32_t i = 0; i < mesh->vertices.size(); i++,mv++)
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

void render_DebugDrawer::drawSkeletalModelDebugLines(struct ss_bone_frame_s *bframe, const btTransform &transform)
{
    if(renderer.style & R_DRAW_NORMALS)
    {
        ss_bone_tag_p btag = bframe->bone_tags;
        for(uint16_t i=0; i<bframe->bone_tag_count; i++,btag++)
        {
            btTransform tr = transform * btag->full_transform;
            drawMeshDebugLines(btag->mesh_base, tr, {}, {});
        }
    }
}

void render_DebugDrawer::drawEntityDebugLines(std::shared_ptr<Entity> entity)
{
    if(entity->m_wasRenderedLines || !(renderer.style & (R_DRAW_AXIS | R_DRAW_NORMALS | R_DRAW_BOXES)) ||
       !(entity->m_stateFlags & ENTITY_STATE_VISIBLE) || (entity->m_bf.animations.model->hide && !(renderer.style & R_DRAW_NULLMESHES)))
    {
        return;
    }

    if(renderer.style & R_DRAW_BOXES)
    {
        debugDrawer.setColor(0.0, 0.0, 1.0);
        debugDrawer.drawOBB(entity->m_obb.get());
    }

    if(renderer.style & R_DRAW_AXIS)
    {
        // If this happens, the lines after this will get drawn with random colors. I don't care.
        debugDrawer.drawAxis(1000.0, entity->m_transform);
    }

    if(entity->m_bf.animations.model && entity->m_bf.animations.model->animations)
    {
        debugDrawer.drawSkeletalModelDebugLines(&entity->m_bf, entity->m_transform);
    }

    entity->m_wasRenderedLines = 1;
}


void render_DebugDrawer::drawSectorDebugLines(struct room_sector_s *rs)
{
    btVector3 bb_min = {(btScalar)(rs->pos[0] - TR_METERING_SECTORSIZE / 2.0), (btScalar)(rs->pos[1] - TR_METERING_SECTORSIZE / 2.0), (btScalar)rs->floor};
    btVector3 bb_max = {(btScalar)(rs->pos[0] + TR_METERING_SECTORSIZE / 2.0), (btScalar)(rs->pos[1] + TR_METERING_SECTORSIZE / 2.0), (btScalar)rs->ceiling};

    drawBBox(bb_min, bb_max, NULL);
}


void render_DebugDrawer::drawRoomDebugLines(std::shared_ptr<Room> room, struct render_s *render)
{
    uint32_t flag;

    flag = render->style & R_DRAW_ROOMBOXES;
    if(flag)
    {
        debugDrawer.setColor(0.0, 0.1, 0.9);
        debugDrawer.drawBBox(room->bb_min, room->bb_max, NULL);
        /*for(uint32_t s=0;s<room->sectors_count;s++)
        {
            drawSectorDebugLines(room->sectors + s);
        }*/
    }

    flag = render->style & R_DRAW_PORTALS;
    if(flag)
    {
        debugDrawer.setColor(0.0, 0.0, 0.0);
        for(const auto& p : room->portals)
        {
            debugDrawer.drawPortal(p);
        }
    }

    flag = render->style & R_DRAW_FRUSTUMS;
    if(flag)
    {
        debugDrawer.setColor(1.0, 0.0, 0.0);
        for(const auto& frus : room->frustum) {
            debugDrawer.drawFrustum(*frus);
        }
    }

    if(!(renderer.style & R_SKIP_ROOM) && (room->mesh != NULL))
    {
        debugDrawer.drawMeshDebugLines(room->mesh, room->transform, {}, {});
    }

    flag = render->style & R_DRAW_BOXES;
    for(auto sm : room->static_mesh)
    {
        if(sm->was_rendered_lines || !Frustum::isOBBVisibleInRoom(sm->obb, room) ||
          ((sm->hide == 1) && !(renderer.style & R_DRAW_DUMMY_STATICS)))
        {
            continue;
        }

        if(flag)
        {
            debugDrawer.setColor(0.0, 1.0, 0.1);
            debugDrawer.drawOBB(sm->obb);
        }

        if(render->style & R_DRAW_AXIS)
        {
            debugDrawer.drawAxis(1000.0, sm->transform);
        }

        debugDrawer.drawMeshDebugLines(sm->mesh, sm->transform, {}, {});

        sm->was_rendered_lines = 1;
    }

    for(const std::shared_ptr<EngineContainer>& cont : room->containers)
    {
        switch(cont->object_type)
        {
            case OBJECT_ENTITY: {
                std::shared_ptr<Entity> ent = std::static_pointer_cast<Entity>(cont->object);
                if(ent->m_wasRenderedLines == 0)
                {
                    if(Frustum::isOBBVisibleInRoom(ent->m_obb.get(), room))
                    {
                        debugDrawer.drawEntityDebugLines(ent);
                    }
                    ent->m_wasRenderedLines = 1;
                }
                break;
            }
        };
    }
}
