
#include <cmath>
#include <stdlib.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "../core/gl_util.h"
#include "../core/gl_text.h"
#include "../core/system.h"
#include "../core/console.h"
#include "../core/vmath.h"
#include "../core/polygon.h"
#include "../core/obb.h"
#include "../vt/tr_versions.h"
#include "camera.h"
#include "render.h"
#include "bsp_tree.h"
#include "frustum.h"
#include "shader_description.h"
#include "shader_manager.h"
#include "../room.h"
#include "../world.h"
#include "../script.h"
#include "../mesh.h"
#include "../skeletal_model.h"
#include "../entity.h"
#include "../character_controller.h"
#include "../engine.h"
#include "../physics.h"

CRender renderer;

void CalculateWaterTint(GLfloat *tint, uint8_t fixed_colour);

#define DEBUG_DRAWER_DEFAULT_BUFFER_SIZE        (128 * 1024)

/*
 * =============================================================================
 */

CRender::CRender():
m_camera(NULL),
m_rooms(NULL),
m_rooms_count(0),
m_anim_sequences(NULL),
m_anim_sequences_count(0),
m_active_transparency(0),
m_active_texture(0),
r_list_size(0),
r_list_active_count(0),
r_list(NULL),
frustumManager(NULL),
shaderManager(NULL),
debugDrawer(NULL),
dynamicBSP(NULL),
r_flags(0x00)
{
    this->InitSettings();
    frustumManager = new CFrustumManager(32768);
    debugDrawer    = new CRenderDebugDrawer();
    dynamicBSP     = new CDynamicBSP(512 * 1024);
}

CRender::~CRender()
{
    m_camera = NULL;

    if(r_list)
    {
        r_list_active_count = 0;
        r_list_size = 0;
        free(r_list);
        r_list = NULL;
    }

    if(frustumManager)
    {
        delete frustumManager;
        frustumManager = NULL;
    }

    if(debugDrawer)
    {
        delete debugDrawer;
        debugDrawer = NULL;
    }

    if(dynamicBSP)
    {
        delete dynamicBSP;
        dynamicBSP = NULL;
    }

    if(shaderManager)
    {
        delete shaderManager;
        shaderManager = NULL;
    }
}

void CRender::InitSettings()
{
    settings.anisotropy = 0;
    settings.lod_bias = 0;
    settings.antialias = 0;
    settings.antialias_samples = 0;
    settings.mipmaps = 3;
    settings.mipmap_mode = 3;
    settings.texture_border = 8;
    settings.z_depth = 16;
    settings.fog_enabled = 1;
    settings.fog_color[0] = 0.0f;
    settings.fog_color[1] = 0.0f;
    settings.fog_color[2] = 0.0f;
    settings.fog_start_depth = 10000.0f;
    settings.fog_end_depth = 16000.0f;
}

void CRender::DoShaders()
{
    if(shaderManager == NULL)
    {
        shaderManager = new shader_manager();
    }
}

void CRender::ResetWorld(struct room_s *rooms, uint32_t rooms_count, struct anim_seq_s *anim_sequences, uint32_t anim_sequences_count)
{
    this->CleanList();
    r_flags = 0x00;

    m_rooms = rooms;
    m_rooms_count = rooms_count;
    m_anim_sequences = anim_sequences;
    m_anim_sequences_count = anim_sequences_count;

    if(m_rooms)
    {
        uint32_t list_size = rooms_count + 128;                                 // magick 128 was added for debug and testing
        if(r_list)
        {
            free(r_list);
        }
        r_list = (struct render_list_s*)malloc(list_size * sizeof(struct render_list_s));
        for(uint32_t i = 0; i < list_size; i++)
        {
            r_list[i].active = 0;
            r_list[i].room = NULL;
            r_list[i].dist = 0.0;
        }

        r_list_size = list_size;
        r_list_active_count = 0;

        for(uint32_t i = 0; i < m_rooms_count; i++)
        {
            m_rooms[i].is_in_r_list = 0;
        }
    }
}

// This function is used for updating global animated texture frame
void CRender::UpdateAnimTextures()
{
    if(m_anim_sequences)
    {
        anim_seq_p seq = m_anim_sequences;
        for(uint16_t i = 0; i < m_anim_sequences_count; i++, seq++)
        {
            if(seq->frame_lock)
            {
                continue;
            }

            seq->frame_time += engine_frame_time;
            if(seq->uvrotate)
            {
                int j = (seq->frame_time / seq->frame_rate);
                seq->frame_time -= (float)j * seq->frame_rate;
                seq->frames[seq->current_frame].current_uvrotate = seq->frame_time * seq->frames[seq->current_frame].uvrotate_max / seq->frame_rate;
            }
            else if(seq->frame_time >= seq->frame_rate)
            {
                int j = (seq->frame_time / seq->frame_rate);
                seq->frame_time -= (float)j * seq->frame_rate;

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
                            seq->current_frame %= seq->frames_count;            ///@PARANOID
                        }
                        break;

                    case TR_ANIMTEXTURE_FORWARD:                                // inversed in polygon anim. texture frames
                    case TR_ANIMTEXTURE_BACKWARD:
                        seq->current_frame++;
                        seq->current_frame %= seq->frames_count;
                        break;
                };
            }
        }
    }
}

/**
 * Renderer list generation by current world and camera
 */
void CRender::GenWorldList(struct camera_s *cam)
{
    this->CleanList();                                                          // clear old render list
    this->dynamicBSP->Reset(m_anim_sequences);
    this->frustumManager->Reset();
    cam->frustum->next = NULL;
    m_camera = cam;

    if(m_rooms == NULL)
    {
        return;
    }

    room_p curr_room = World_FindRoomByPosCogerrence(cam->pos, cam->current_room);     // find room that contains camera

    cam->current_room = curr_room;                                              // set camera's cuttent room pointer
    if(curr_room != NULL)                                                       // camera located in some room
    {
        const float eps = 1.0f;
        portal_p p = curr_room->portals;
        curr_room->frustum = NULL;                                              // room with camera inside has no frustums!
        this->AddRoom(curr_room);                                               // room with camera inside adds to the render list immediately
        for(uint16_t i = 0; i < curr_room->portals_count; i++, p++)             // go through all start room portals
        {
            room_p dest_room = Room_CheckFlip(p->dest_room);
            frustum_p last_frus = this->frustumManager->PortalFrustumIntersect(p, cam->frustum, cam);
            if(last_frus)
            {
                this->AddRoom(dest_room);                                       // portal destination room
                last_frus->parents_count = 1;                                   // created by camera
                this->ProcessRoom(p, last_frus);                                // next start reccursion algorithm
            }
            else if(fabs((vec3_plane_dist(p->norm, cam->pos)) <= eps) &&
                (cam->pos[0] <= dest_room->bb_max[0] + eps) && (cam->pos[0] >= dest_room->bb_min[0] - eps) &&
                (cam->pos[1] <= dest_room->bb_max[1] + eps) && (cam->pos[1] >= dest_room->bb_min[1] - eps) &&
                (cam->pos[2] <= dest_room->bb_max[2] + eps) && (cam->pos[2] >= dest_room->bb_min[2] - eps))
            {
                portal_p np = dest_room->portals;
                dest_room->frustum = NULL;                                      // room with camera inside has no frustums!
                if(this->AddRoom(dest_room))                                    // room with camera inside adds to the render list immediately
                {
                    for(uint16_t ii = 0; ii < dest_room->portals_count; ii++, np++)// go through all start room portals
                    {
                        room_p ndest_room = Room_CheckFlip(np->dest_room);
                        frustum_p last_frus = this->frustumManager->PortalFrustumIntersect(np, cam->frustum, cam);
                        if(last_frus)
                        {
                            this->AddRoom(ndest_room);                          // portal destination room
                            last_frus->parents_count = 1;                       // created by camera
                            this->ProcessRoom(np, last_frus);                   // next start reccursion algorithm
                        }
                    }
                }
            }
        }

        if(curr_room->base_room)
        {
            p = curr_room->base_room->portals;
            for(uint16_t i = 0; i < curr_room->base_room->portals_count; i++, p++)// go through all start room portals
            {
                room_p dest_room = Room_CheckFlip(p->dest_room);
                frustum_p last_frus = this->frustumManager->PortalFrustumIntersect(p, cam->frustum, cam);
                if(last_frus)
                {
                    this->AddRoom(dest_room);                                   // portal destination room
                    last_frus->parents_count = 1;                               // created by camera
                    this->ProcessRoom(p, last_frus);                            // next start reccursion algorithm
                }
                else if(fabs((vec3_plane_dist(p->norm, cam->pos)) <= eps) &&
                    (cam->pos[0] <= dest_room->bb_max[0] + eps) && (cam->pos[0] >= dest_room->bb_min[0] - eps) &&
                    (cam->pos[1] <= dest_room->bb_max[1] + eps) && (cam->pos[1] >= dest_room->bb_min[1] - eps) &&
                    (cam->pos[2] <= dest_room->bb_max[2] + eps) && (cam->pos[2] >= dest_room->bb_min[2] - eps))
                {
                    portal_p np = dest_room->portals;
                    dest_room->frustum = NULL;                                  // room with camera inside has no frustums!
                    if(this->AddRoom(dest_room))                                // room with camera inside adds to the render list immediately
                    {
                        for(uint16_t ii = 0; ii < dest_room->portals_count; ii++, np++)// go through all start room portals
                        {
                            room_p ndest_room = Room_CheckFlip(np->dest_room);
                            frustum_p last_frus = this->frustumManager->PortalFrustumIntersect(np, cam->frustum, cam);
                            if(last_frus)
                            {
                                this->AddRoom(ndest_room);                      // portal destination room
                                last_frus->parents_count = 1;                   // created by camera
                                this->ProcessRoom(np, last_frus);               // next start reccursion algorithm
                            }
                        }
                    }
                }
            }
        }
    }
    else                                                                        // camera is out of all rooms
    {
        curr_room = m_rooms;                                                    // draw full level. Yes - it is slow, but it is not gameplay - it is debug.
        for(uint32_t i = 0; i < m_rooms_count; i++, curr_room++)
        {
            if(Frustum_IsAABBVisible(curr_room->bb_min, curr_room->bb_max, cam->frustum))
            {
                this->AddRoom(Room_CheckFlip(curr_room));
            }
        }
    }
}

/**
 * Render all visible rooms
 */
void CRender::DrawList()
{
    if(m_camera)
    {
        if(r_flags & R_DRAW_WIRE)
        {
            qglPolygonMode(GL_FRONT, GL_LINE);
        }
        else if(r_flags & R_DRAW_POINTS)
        {
            qglEnable(GL_POINT_SMOOTH);
            qglPointSize(4);
            qglPolygonMode(GL_FRONT, GL_POINT);
        }
        else
        {
            qglPolygonMode(GL_FRONT, GL_FILL);
        }

        qglEnable(GL_CULL_FACE);
        qglDisable(GL_BLEND);
        qglEnable(GL_ALPHA_TEST);

        m_active_texture = 0;
        this->DrawSkyBox(m_camera->gl_view_proj_mat);
        entity_p player = World_GetPlayer();

        if(player)
        {
            this->DrawEntity(player, m_camera->gl_view_mat, m_camera->gl_view_proj_mat);
        }

        /*
         * room rendering
         */
        for(uint32_t i = 0; i < r_list_active_count; i++)
        {
            this->DrawRoom(r_list[i].room, m_camera->gl_view_mat, m_camera->gl_view_proj_mat);
        }

        qglDisable(GL_CULL_FACE);
        for(uint32_t i = 0; i < r_list_active_count; i++)
        {
            this->DrawRoomSprites(r_list[i].room);
        }

        /*
         * NOW render transparency polygons
         */
        /*First generate BSP from base room mesh - it has good for start splitter polygons*/
        for(uint32_t i = 0; i < r_list_active_count; i++)
        {
            room_p r = r_list[i].room;
            if((r->content->mesh != NULL) && (r->content->mesh->transparency_polygons != NULL))
            {
                dynamicBSP->AddNewPolygonList(r->content->mesh->transparency_polygons, r->transform, m_camera->frustum);
            }
        }

        for(uint32_t i = 0; i < r_list_active_count; i++)
        {
            room_p r = r_list[i].room;
            // Add transparency polygons from static meshes (if they exists)
            for(uint16_t j = 0; j < r->content->static_mesh_count; j++)
            {
                if((r->content->static_mesh[j].mesh->transparency_polygons != NULL) && Frustum_IsOBBVisibleInFrustumList(r->content->static_mesh[j].obb, (r->frustum) ? (r->frustum) : (m_camera->frustum)))
                {
                    dynamicBSP->AddNewPolygonList(r->content->static_mesh[j].mesh->transparency_polygons, r->content->static_mesh[j].transform, m_camera->frustum);
                }
            }

            // Add transparency polygons from all entities (if they exists) // yes, entities may be animated and intersects with each others;
            for(engine_container_p cont = r->content->containers; cont; cont = cont->next)
            {
                if(cont->object_type == OBJECT_ENTITY)
                {
                    entity_p ent = (entity_p)cont->object;
                    if((ent->bf->animations.model->transparency_flags == MESH_HAS_TRANSPARENCY) && (ent->state_flags & ENTITY_STATE_VISIBLE) && Frustum_IsOBBVisibleInFrustumList(ent->obb, (r->frustum) ? (r->frustum) : (m_camera->frustum)))
                    {
                        float tr[16];
                        for(uint16_t j = 0; j < ent->bf->bone_tag_count; j++)
                        {
                            if(ent->bf->bone_tags[j].mesh_base->transparency_polygons != NULL)
                            {
                                Mat4_Mat4_mul(tr, ent->transform, ent->bf->bone_tags[j].full_transform);
                                dynamicBSP->AddNewPolygonList(ent->bf->bone_tags[j].mesh_base->transparency_polygons, tr, m_camera->frustum);
                            }
                        }
                    }
                }
            }
        }

        if(player && (player->bf->animations.model->transparency_flags == MESH_HAS_TRANSPARENCY))
        {
            float tr[16];
            for(uint16_t j = 0; j < player->bf->bone_tag_count; j++)
            {
                if(player->bf->bone_tags[j].mesh_base->transparency_polygons != NULL)
                {
                    Mat4_Mat4_mul(tr, player->transform, player->bf->bone_tags[j].full_transform);
                    dynamicBSP->AddNewPolygonList(player->bf->bone_tags[j].mesh_base->transparency_polygons, tr, m_camera->frustum);
                }
            }
        }

        if(dynamicBSP->m_root->polygons_front && (dynamicBSP->m_vbo != 0))
        {
            const unlit_tinted_shader_description *shader = shaderManager->getRoomShader(false, false);
            qglUseProgramObjectARB(shader->program);
            qglUniform1iARB(shader->sampler, 0);
            qglUniformMatrix4fvARB(shader->model_view_projection, 1, false, m_camera->gl_view_proj_mat);
            qglDepthMask(GL_FALSE);
            qglDisable(GL_ALPHA_TEST);
            qglEnable(GL_BLEND);
            m_active_transparency = 0;
            qglBindBufferARB(GL_ARRAY_BUFFER_ARB, dynamicBSP->m_vbo);
            qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
            qglBufferDataARB(GL_ARRAY_BUFFER_ARB, dynamicBSP->GetActiveVertexCount() * sizeof(vertex_t), dynamicBSP->GetVertexArray(), GL_DYNAMIC_DRAW);
            qglVertexPointer(3, GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, position));
            qglColorPointer(4, GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, color));
            qglNormalPointer(GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));
            qglTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, tex_coord));
            this->DrawBSPBackToFront(dynamicBSP->m_root);
            qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
            qglDepthMask(GL_TRUE);
            qglDisable(GL_BLEND);
        }
        //Reset polygon draw mode
        qglPolygonMode(GL_FRONT, GL_FILL);
        m_active_texture = 0;
    }
}

void CRender::DrawListDebugLines()
{
    if(r_flags && m_camera)
    {
        debugDrawer->SetDrawFlags(r_flags);

        if(World_GetPlayer())
        {
            debugDrawer->DrawEntityDebugLines(World_GetPlayer());
        }

        /*
         * Render world debug information
         */
        skeletal_model_p skybox = World_GetSkybox();
        if((r_flags & R_DRAW_NORMALS) && skybox)
        {
            GLfloat tr[16];
            float *p;
            Mat4_E_macro(tr);
            p = skybox->animations->frames->bone_tags->offset;
            vec3_add(tr+12, m_camera->pos, p);
            p = skybox->animations->frames->bone_tags->qrotate;
            Mat4_set_qrotation(tr, p);
            debugDrawer->DrawMeshDebugLines(skybox->mesh_tree->mesh_base, tr, NULL, NULL);
        }

        for(uint32_t i = 0; i < r_list_active_count; i++)
        {
            debugDrawer->DrawRoomDebugLines(r_list[i].room, m_camera);
        }

        if(r_flags & R_DRAW_COLL)
        {
            Physics_DebugDrawWorld();
        }

        if(r_flags & R_DRAW_FLYBY)
        {
            const float color_r[3] = {1.0f, 0.0f, 0.0f};
            const float color_g[3] = {0.0f, 1.0f, 0.0f};
            float v0[3], v1[3];

            for(flyby_camera_sequence_p s = World_GetFlyBySequences(); s; s = s->next)
            {
                const float max_s = s->pos_x->base_points_count - 1;
                const float dt = max_s / 256.0f;
                for(float t = 0.0f; t <= max_s - dt; t += dt)
                {
                    v0[0] = Spline_Get(s->pos_x, t);
                    v0[1] = Spline_Get(s->pos_y, t);
                    v0[2] = Spline_Get(s->pos_z, t);
                    v1[0] = Spline_Get(s->pos_x, t + dt);
                    v1[1] = Spline_Get(s->pos_y, t + dt);
                    v1[2] = Spline_Get(s->pos_z, t + dt);
                    debugDrawer->DrawLine(v0, v1, color_r, color_r);

                    v0[0] = Spline_Get(s->target_x, t);
                    v0[1] = Spline_Get(s->target_y, t);
                    v0[2] = Spline_Get(s->target_z, t);
                    v1[0] = Spline_Get(s->target_x, t + dt);
                    v1[1] = Spline_Get(s->target_y, t + dt);
                    v1[2] = Spline_Get(s->target_z, t + dt);
                    debugDrawer->DrawLine(v0, v1, color_g, color_g);
                }
            }
        }
    }

    if(!debugDrawer->IsEmpty() && m_camera)
    {
        const unlit_tinted_shader_description *shader = shaderManager->getRoomShader(false, false);
        qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
        qglUseProgramObjectARB(shader->program);
        qglUniform1iARB(shader->sampler, 0);
        qglUniformMatrix4fvARB(shader->model_view_projection, 1, false, m_camera->gl_view_proj_mat);
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        m_active_texture = 0;
        BindWhiteTexture();
        qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        qglPointSize( 6.0f );
        qglLineWidth( 3.0f );
        debugDrawer->Render();
    }
    debugDrawer->Reset();
}

void CRender::CleanList()
{
    for(uint32_t i = 0; i < r_list_active_count; i++)
    {
        r_list[i].active = 0;
        r_list[i].dist = 0.0;
        r_list[i].room = NULL;
    }

    if(m_rooms)
    {
        for(uint32_t i = 0; i < m_rooms_count; i++)
        {
            m_rooms[i].is_in_r_list = 0;
            m_rooms[i].frustum = NULL;
        }
    }

    r_flags &= ~R_DRAW_SKYBOX;
    r_list_active_count = 0;
}

/*
 * Draw objects functions
 */
void CRender::DrawBSPPolygon(struct bsp_polygon_s *p)
{
    // Blending mode switcher.
    // Note that modes above 2 aren't explicitly used in TR textures, only for
    // internal particle processing. Theoretically it's still possible to use
    // them if you will force type via TRTextur utility.
    if(m_active_transparency != p->transparency)
    {
        m_active_transparency = p->transparency;
        switch(m_active_transparency)
        {
            case BM_MULTIPLY:                                    // Classic PC alpha
                qglBlendFunc(GL_ONE, GL_ONE);
                break;

            case BM_INVERT_SRC:                                  // Inversion by src (PS darkness) - SAME AS IN TR3-TR5
                qglBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
                break;

            case BM_INVERT_DEST:                                 // Inversion by dest
                qglBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
                break;

            case BM_SCREEN:                                      // Screen (smoke, etc.)
                qglBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
                break;

            case BM_ANIMATED_TEX:
                qglBlendFunc(GL_ONE, GL_ZERO);
                break;

            default:                                             // opaque animated textures case
                break;
        };
    }

    if(m_active_texture != p->texture_index)
    {
        m_active_texture = p->texture_index;
        qglBindTexture(GL_TEXTURE_2D, m_active_texture);
    }
    qglDrawElements(GL_TRIANGLE_FAN, p->vertex_count, GL_UNSIGNED_INT, p->indexes);
}

void CRender::DrawBSPFrontToBack(struct bsp_node_s *root)
{
    float d = vec3_plane_dist(root->plane, engine_camera.pos);

    if(d >= 0)
    {
        if(root->front != NULL)
        {
            this->DrawBSPFrontToBack(root->front);
        }

        for(bsp_polygon_p p = root->polygons_front; p; p = p->next)
        {
            this->DrawBSPPolygon(p);
        }
        for(bsp_polygon_p p = root->polygons_back; p; p = p->next)
        {
            this->DrawBSPPolygon(p);
        }

        if(root->back != NULL)
        {
            this->DrawBSPFrontToBack(root->back);
        }
    }
    else
    {
        if(root->back != NULL)
        {
            this->DrawBSPFrontToBack(root->back);
        }

        for(bsp_polygon_p p = root->polygons_back; p; p = p->next)
        {
            this->DrawBSPPolygon(p);
        }
        for(bsp_polygon_p p = root->polygons_front; p; p = p->next)
        {
            this->DrawBSPPolygon(p);
        }

        if(root->front != NULL)
        {
            this->DrawBSPFrontToBack(root->front);
        }
    }
}

void CRender::DrawBSPBackToFront(struct bsp_node_s *root)
{
    float d = vec3_plane_dist(root->plane, engine_camera.pos);

    if(d >= 0)
    {
        if(root->back != NULL)
        {
            this->DrawBSPBackToFront(root->back);
        }

        for(bsp_polygon_p p = root->polygons_back; p; p = p->next)
        {
            this->DrawBSPPolygon(p);
        }
        for(bsp_polygon_p p = root->polygons_front; p; p = p->next)
        {
            this->DrawBSPPolygon(p);
        }

        if(root->front != NULL)
        {
            this->DrawBSPBackToFront(root->front);
        }
    }
    else
    {
        if(root->front != NULL)
        {
            this->DrawBSPBackToFront(root->front);
        }

        for(bsp_polygon_p p = root->polygons_front; p; p = p->next)
        {
            this->DrawBSPPolygon(p);
        }
        for(bsp_polygon_p p = root->polygons_back; p; p = p->next)
        {
            this->DrawBSPPolygon(p);
        }

        if(root->back != NULL)
        {
            this->DrawBSPBackToFront(root->back);
        }
    }
}

void CRender::DrawMesh(struct base_mesh_s *mesh, const float *overrideVertices, const float *overrideNormals)
{
    if(mesh->animated_vertex_count)
    {
        // Respecify the tex coord buffer
        qglBindBufferARB(GL_ARRAY_BUFFER, mesh->vbo_animated_texcoord_array);
        // Tell OpenGL to discard the old values
        qglBufferDataARB(GL_ARRAY_BUFFER, mesh->animated_vertex_count * sizeof(GLfloat [2]), 0, GL_STREAM_DRAW);
        // Get writable data (to avoid copy)
        GLfloat *data = (GLfloat *) qglMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

        for(polygon_p p = mesh->animated_polygons; p; p = p->next)
        {
            anim_seq_p seq = m_anim_sequences + p->anim_id - 1;
            uint16_t frame = (seq->current_frame + p->frame_offset) % seq->frames_count;
            tex_frame_p tf = seq->frames + frame;
            for(uint16_t i = 0; i < p->vertex_count; i++, data += 2)
            {
                ApplyAnimTextureTransformation(data, p->vertices[i].tex_coord, tf);
            }
        }
        qglUnmapBufferARB(GL_ARRAY_BUFFER);

        // Setup altered buffer
        qglTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat [2]), 0);
        // Setup static data
        qglBindBufferARB(GL_ARRAY_BUFFER, mesh->vbo_animated_vertex_array);
        qglVertexPointer(3, GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, position));
        qglColorPointer(4, GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, color));
        qglNormalPointer(GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));

        mesh_face_p face = mesh->animated_faces;
        for(uint32_t face_index = 0; face_index < mesh->animated_faces_count; face_index++, face++)
        {
            if(m_active_texture != face->texture_index)
            {
                m_active_texture = face->texture_index;
                qglBindTexture(GL_TEXTURE_2D, m_active_texture);
            }
            qglDrawElements(GL_TRIANGLES, face->elements_count, GL_UNSIGNED_INT, face->elements);
        }
    }

    if(mesh->vertex_count == 0)
    {
        return;
    }

    if(mesh->vbo_vertex_array)
    {
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vbo_vertex_array);
        qglVertexPointer(3, GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, position));
        qglColorPointer(4, GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, color));
        qglNormalPointer(GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));
        qglTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, tex_coord));
    }

    // Bind overriden vertices if they exist
    if (overrideVertices != NULL)
    {
        // Standard normals are always float. Overridden normals (from skinning)
        // are float.
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        qglVertexPointer(3, GL_FLOAT, 0, overrideVertices);
        qglNormalPointer(GL_FLOAT, 0, overrideNormals);
    }

    mesh_face_p face = mesh->faces;
    for(uint32_t face_index = 0; face_index < mesh->faces_count; face_index++, face++)
    {
        if(m_active_texture != face->texture_index)
        {
            m_active_texture = face->texture_index;
            qglBindTexture(GL_TEXTURE_2D, m_active_texture);
        }
        qglDrawElements(GL_TRIANGLES, face->elements_count, GL_UNSIGNED_INT, face->elements);
    }
}

void CRender::DrawSkinMesh(struct base_mesh_s *mesh, struct base_mesh_s *parent_mesh, float transform[16])
{
    uint32_t i;
    vertex_p v;
    float *p_vertex, *src_v, *dst_v;
    GLfloat *p_normale, *src_n, *dst_n;
    uint32_t *ch = mesh->skin_map;
    size_t buf_size = mesh->vertex_count * 3 * sizeof(GLfloat);

    p_vertex  = (GLfloat*)Sys_GetTempMem(buf_size);
    p_normale = (GLfloat*)Sys_GetTempMem(buf_size);
    dst_v = p_vertex;
    dst_n = p_normale;
    v = mesh->vertices;
    for(i = 0; i < mesh->vertex_count; i++, v++, ch++)
    {
        src_v = v->position;
        src_n = v->normal;
        if(*ch == 0xFFFFFFFF)
        {
            vec3_copy(dst_v, src_v);
            vec3_copy(dst_n, src_n);
        }
        else
        {
            Mat4_vec3_mul_inv(dst_v, transform, parent_mesh->vertices[*ch].position);
            dst_n[0]  = transform[0] * src_n[0] + transform[1] * src_n[1] + transform[2]  * src_n[2];             // (M^-1 * src).x
            dst_n[1]  = transform[4] * src_n[0] + transform[5] * src_n[1] + transform[6]  * src_n[2];             // (M^-1 * src).y
            dst_n[2]  = transform[8] * src_n[0] + transform[9] * src_n[1] + transform[10] * src_n[2];             // (M^-1 * src).z
        }
        dst_v += 3;
        dst_n += 3;
    }

    this->DrawMesh(mesh, p_vertex, p_normale);
    Sys_ReturnTempMem(2 * buf_size);
}

void CRender::DrawSkyBox(const float modelViewProjectionMatrix[16])
{
    skeletal_model_p skybox;
    if((r_flags & R_DRAW_SKYBOX) && (skybox = World_GetSkybox()))
    {
        float tr[16];
        float *p;
        qglDepthMask(GL_FALSE);
        tr[15] = 1.0;
        p = skybox->animations->frames->bone_tags->offset;
        vec3_add(tr+12, m_camera->pos, p);
        p = skybox->animations->frames->bone_tags->qrotate;
        Mat4_set_qrotation(tr, p);
        float fullView[16];
        Mat4_Mat4_mul(fullView, modelViewProjectionMatrix, tr);

        const unlit_tinted_shader_description *shader = shaderManager->getStaticMeshShader();
        qglUseProgramObjectARB(shader->program);
        qglUniformMatrix4fvARB(shader->model_view_projection, 1, false, fullView);
        qglUniform1iARB(shader->sampler, 0);
        GLfloat tint[] = { 1, 1, 1, 1 };
        qglUniform4fvARB(shader->tint_mult, 1, tint);

        this->DrawMesh(skybox->mesh_tree->mesh_base, NULL, NULL);
        qglDepthMask(GL_TRUE);
    }
}

/**
 * skeletal model drawing
 */
void CRender::DrawSkeletalModel(const lit_shader_description *shader, struct ss_bone_frame_s *bframe, const float mvMatrix[16], const float mvpMatrix[16])
{
    ss_bone_tag_p btag = bframe->bone_tags;

    //mvMatrix = modelViewMatrix x entity->transform
    //mvpMatrix = modelViewProjectionMatrix x entity->transform

    for(uint16_t i = 0; i < bframe->bone_tag_count; i++, btag++)
    {
        float mvTransform[16];
        Mat4_Mat4_mul(mvTransform, mvMatrix, btag->full_transform);
        qglUniformMatrix4fvARB(shader->model_view, 1, false, mvTransform);

        float mvpTransform[16];
        Mat4_Mat4_mul(mvpTransform, mvpMatrix, btag->full_transform);
        qglUniformMatrix4fvARB(shader->model_view_projection, 1, false, mvpTransform);

        this->DrawMesh(btag->mesh_base, NULL, NULL);
        if(btag->mesh_slot)
        {
            this->DrawMesh(btag->mesh_slot, NULL, NULL);
        }
        if(btag->mesh_skin && btag->parent)
        {
            this->DrawSkinMesh(btag->mesh_skin, btag->parent->mesh_base, btag->transform);
        }
    }
}

void CRender::DrawEntity(struct entity_s *entity, const float modelViewMatrix[16], const float modelViewProjectionMatrix[16])
{
    if(!(entity->state_flags & ENTITY_STATE_VISIBLE) || (entity->bf->animations.model->hide && !(r_flags & R_DRAW_NULLMESHES)))
    {
        return;
    }

    // Calculate lighting
    const lit_shader_description *shader = this->SetupEntityLight(entity, modelViewMatrix);

    if(entity->bf->animations.model && entity->bf->animations.model->animations)
    {
        float subModelView[16];
        float subModelViewProjection[16];
        if(entity->bf->bone_tag_count == 1)
        {
            float scaledTransform[16];
            memcpy(scaledTransform, entity->transform, sizeof(float) * 16);
            Mat4_Scale(scaledTransform, entity->scaling[0], entity->scaling[1], entity->scaling[2]);
            Mat4_Mat4_mul(subModelView, modelViewMatrix, scaledTransform);
            Mat4_Mat4_mul(subModelViewProjection, modelViewProjectionMatrix, scaledTransform);
        }
        else
        {
            Mat4_Mat4_mul(subModelView, modelViewMatrix, entity->transform);
            Mat4_Mat4_mul(subModelViewProjection, modelViewProjectionMatrix, entity->transform);
        }

        this->DrawSkeletalModel(shader, entity->bf, subModelView, subModelViewProjection);

        if(entity->character && entity->character->hair_count)
        {
            base_mesh_p mesh;
            float transform[16];
            for(int h = 0; h < entity->character->hair_count; h++)
            {
                int num_elements = Hair_GetElementsCount(entity->character->hairs[h]);
                for(uint16_t i = 0; i < num_elements; i++)
                {
                    Hair_GetElementInfo(entity->character->hairs[h], i, &mesh, transform);
                    Mat4_Mat4_mul(subModelView, modelViewMatrix, transform);
                    Mat4_Mat4_mul(subModelViewProjection, modelViewProjectionMatrix, transform);

                    qglUniformMatrix4fvARB(shader->model_view, 1, GL_FALSE, subModelView);
                    qglUniformMatrix4fvARB(shader->model_view_projection, 1, GL_FALSE, subModelViewProjection);
                    this->DrawMesh(mesh, NULL, NULL);
                }
            }
        }
    }
}

void CRender::DrawRoom(struct room_s *room, const float modelViewMatrix[16], const float modelViewProjectionMatrix[16])
{
    float transform[16];
    engine_container_p cont;
    entity_p ent;

    const shader_description *lastShader = 0;

#if STENCIL_FRUSTUM
    ////start test stencil test code
    bool need_stencil = false;
    if(room->frustum != NULL)
    {
        for(uint16_t i = 0; i < room->overlapped_room_list_size; i++)
        {
            if(room->overlapped_room_list[i]->is_in_r_list)
            {
                need_stencil = true;
                break;
            }
        }

        if(need_stencil)
        {
            const int elem_size = (3 + 3 + 4 + 2) * sizeof(GLfloat);
            const unlit_tinted_shader_description *shader = shaderManager->getRoomShader(false, false);
            size_t buf_size;

            qglUseProgramObjectARB(shader->program);
            qglUniform1iARB(shader->sampler, 0);
            qglUniformMatrix4fvARB(shader->model_view_projection, 1, false, engine_camera.gl_view_proj_mat);
            qglEnable(GL_STENCIL_TEST);
            qglClear(GL_STENCIL_BUFFER_BIT);
            qglStencilFunc(GL_NEVER, 1, 0x00);
            qglStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
            for(frustum_p f = room->frustum; f; f = f->next)
            {
                buf_size = f->vertex_count * elem_size;
                GLfloat *v, *buf = (GLfloat*)Sys_GetTempMem(buf_size);
                v=buf;
                for(int16_t i = f->vertex_count - 1; i >= 0; i--)
                {
                    vec3_copy(v, f->vertex+3*i);                    v+=3;
                    vec3_copy_inv(v, engine_camera.view_dir);       v+=3;
                    vec4_set_one(v);                                v+=4;
                    v[0] = v[1] = 0.0;                              v+=2;
                }

                m_active_texture = 0;
                BindWhiteTexture();
                qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
                qglVertexPointer(3, GL_FLOAT, elem_size, buf+0);
                qglNormalPointer(GL_FLOAT, elem_size, buf+3);
                qglColorPointer(4, GL_FLOAT, elem_size, buf+3+3);
                qglTexCoordPointer(2, GL_FLOAT, elem_size, buf+3+3+4);
                qglDrawArrays(GL_TRIANGLE_FAN, 0, f->vertex_count);

                Sys_ReturnTempMem(buf_size);
            }
            qglStencilFunc(GL_EQUAL, 1, 0xFF);
        }
    }
#endif

    if(!(r_flags & R_SKIP_ROOM) && room->content->mesh)
    {
        float modelViewProjectionTransform[16];
        Mat4_Mat4_mul(modelViewProjectionTransform, modelViewProjectionMatrix, room->transform);

        const unlit_tinted_shader_description *shader = shaderManager->getRoomShader(room->content->light_mode == 1, room->flags & 1);

        GLfloat tint[4];
        CalculateWaterTint(tint, 1);
        if (shader != lastShader)
        {
            qglUseProgramObjectARB(shader->program);
        }

        lastShader = shader;
        qglUniform4fvARB(shader->tint_mult, 1, tint);
        qglUniform1fARB(shader->current_tick, (GLfloat) SDL_GetTicks());
        qglUniform1iARB(shader->sampler, 0);
        qglUniformMatrix4fvARB(shader->model_view_projection, 1, false, modelViewProjectionTransform);
        this->DrawMesh(room->content->mesh, NULL, NULL);
    }

    if (room->content->static_mesh_count > 0)
    {
        qglUseProgramObjectARB(shaderManager->getStaticMeshShader()->program);
        for(uint32_t i = 0; i < room->content->static_mesh_count; i++)
        {
            if(Frustum_IsOBBVisibleInFrustumList(room->content->static_mesh[i].obb, (room->frustum) ? (room->frustum) : (m_camera->frustum)) &&
               (!room->content->static_mesh[i].hide || (r_flags & R_DRAW_DUMMY_STATICS)))
            {
                Mat4_Mat4_mul(transform, modelViewProjectionMatrix, room->content->static_mesh[i].transform);
                qglUniformMatrix4fvARB(shaderManager->getStaticMeshShader()->model_view_projection, 1, false, transform);
                base_mesh_s *mesh = room->content->static_mesh[i].mesh;
                GLfloat tint[4];

                vec4_copy(tint, room->content->static_mesh[i].tint);

                //If this static mesh is in a water room
                if(room->flags & TR_ROOM_FLAG_WATER)
                {
                    CalculateWaterTint(tint, 0);
                }
                qglUniform4fvARB(shaderManager->getStaticMeshShader()->tint_mult, 1, tint);
                this->DrawMesh(mesh, NULL, NULL);
            }
        }
    }

    for(cont = room->content->containers; cont; cont = cont->next)
    {
        switch(cont->object_type)
        {
        case OBJECT_ENTITY:
            ent = (entity_p)cont->object;
            if(Frustum_IsOBBVisibleInFrustumList(ent->obb, (room->frustum) ? (room->frustum) : (m_camera->frustum)))
            {
                this->DrawEntity(ent, modelViewMatrix, modelViewProjectionMatrix);
            }
            break;
        };
    }

    for(uint16_t ni = 0; ni < room->near_room_list_size; ni++)
    {
        room_p near_room = room->near_room_list[ni];
        near_room = (!near_room->active && near_room->alternate_room) ? (near_room->alternate_room) : (near_room);
        if(near_room->active && !room->near_room_list[ni]->is_in_r_list)
        {
            if (near_room->content->static_mesh_count > 0)
            {
                for(uint32_t si = 0; si < near_room->content->static_mesh_count; si++)
                {
                    if(OBB_OBB_Test(near_room->content->static_mesh[si].obb, room->obb) &&
                       Frustum_IsOBBVisibleInFrustumList(near_room->content->static_mesh[si].obb, (room->frustum) ? (room->frustum) : (m_camera->frustum)) &&
                       (!near_room->content->static_mesh[si].hide || (r_flags & R_DRAW_DUMMY_STATICS)))
                    {
                        qglUseProgramObjectARB(shaderManager->getStaticMeshShader()->program);
                        Mat4_Mat4_mul(transform, modelViewProjectionMatrix, near_room->content->static_mesh[si].transform);
                        qglUniformMatrix4fvARB(shaderManager->getStaticMeshShader()->model_view_projection, 1, false, transform);
                        base_mesh_s *mesh = near_room->content->static_mesh[si].mesh;
                        GLfloat tint[4];

                        vec4_copy(tint, near_room->content->static_mesh[si].tint);

                        //If this static mesh is in a water near_room
                        if(near_room->flags & TR_ROOM_FLAG_WATER)
                        {
                            CalculateWaterTint(tint, 0);
                        }
                        qglUniform4fvARB(shaderManager->getStaticMeshShader()->tint_mult, 1, tint);
                        this->DrawMesh(mesh, NULL, NULL);
                    }
                }
            }

            for(cont = near_room->content->containers; cont; cont=cont->next)
            {
                switch(cont->object_type)
                {
                case OBJECT_ENTITY:
                    ent = (entity_p)cont->object;
                    if(OBB_OBB_Test(ent->obb, room->obb) &&
                       Frustum_IsOBBVisibleInFrustumList(ent->obb, (room->frustum) ? (room->frustum) : (m_camera->frustum)))
                    {
                        this->DrawEntity(ent, modelViewMatrix, modelViewProjectionMatrix);
                    }
                    break;
                };
            }
        }
    }

#if STENCIL_FRUSTUM
    if(need_stencil)
    {
        qglDisable(GL_STENCIL_TEST);
    }
#endif
}


void CRender::DrawRoomSprites(struct room_s *room)
{
    if (room->content->sprites_count > 0)
    {
        const unlit_tinted_shader_description *shader = shaderManager->getRoomShader(false, false);
        GLfloat *up = m_camera->up_dir;
        GLfloat *right = m_camera->right_dir;

        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        qglUseProgramObjectARB(shader->program);
        qglUniform1iARB(shader->sampler, 0);
        qglUniformMatrix4fvARB(shader->model_view_projection, 1, false, m_camera->gl_view_proj_mat);

        for(uint32_t i = 0; i < room->content->sprites_count; i++)
        {
            room_sprite_p s = room->content->sprites + i;
            vertex_p v = room->content->sprites_vertices + i * 4;
            vec3_copy_inv(v[0].normal, m_camera->view_dir);
            vec3_copy_inv(v[1].normal, m_camera->view_dir);
            vec3_copy_inv(v[2].normal, m_camera->view_dir);
            vec3_copy_inv(v[3].normal, m_camera->view_dir);

            v[0].position[0] = s->pos[0] + s->sprite->right * right[0] + s->sprite->top * up[0];
            v[0].position[1] = s->pos[1] + s->sprite->right * right[1] + s->sprite->top * up[1];
            v[0].position[2] = s->pos[2] + s->sprite->right * right[2] + s->sprite->top * up[2];

            v[1].position[0] = s->pos[0] + s->sprite->left * right[0] + s->sprite->top * up[0];
            v[1].position[1] = s->pos[1] + s->sprite->left * right[1] + s->sprite->top * up[1];
            v[1].position[2] = s->pos[2] + s->sprite->left * right[2] + s->sprite->top * up[2];

            v[2].position[0] = s->pos[0] + s->sprite->left * right[0] + s->sprite->bottom * up[0];
            v[2].position[1] = s->pos[1] + s->sprite->left * right[1] + s->sprite->bottom * up[1];
            v[2].position[2] = s->pos[2] + s->sprite->left * right[2] + s->sprite->bottom * up[2];

            v[3].position[0] = s->pos[0] + s->sprite->right * right[0] + s->sprite->bottom * up[0];
            v[3].position[1] = s->pos[1] + s->sprite->right * right[1] + s->sprite->bottom * up[1];
            v[3].position[2] = s->pos[2] + s->sprite->right * right[2] + s->sprite->bottom * up[2];
        }

        qglBindTexture(GL_TEXTURE_2D, room->content->sprites->sprite->texture_index);
        qglVertexPointer(3, GL_FLOAT, sizeof(vertex_t), room->content->sprites_vertices->position);
        qglColorPointer(4, GL_FLOAT, sizeof(vertex_t), room->content->sprites_vertices->color);
        qglNormalPointer(GL_FLOAT, sizeof(vertex_t), room->content->sprites_vertices->normal);
        qglTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), room->content->sprites_vertices->tex_coord);
        qglDrawArrays(GL_QUADS, 0, 4 * room->content->sprites_count);
    }
}


struct gl_text_line_s *CRender::OutTextXYZ(GLfloat x, GLfloat y, GLfloat z, const char *fmt, ...)
{
    gl_text_line_p ret = NULL;
    if(m_camera)
    {
        float v[4] = {x, y, z, 1.0f};
        float result[4];
        Mat4_vec4_mul_macro(result, m_camera->gl_view_proj_mat, v);
        result[0] = result[0] * 0.5f / result[3] + 0.5f;
        result[0] *= (float)screen_info.w;
        result[1] = result[1] * 0.5f / result[3] + 0.5f;
        result[1] *= (float)screen_info.h;

        if(result[2] >= 0.0)
        {
            va_list argptr;
            va_start(argptr, fmt);
            ret = GLText_VOutTextXY(result[0], result[1], fmt, argptr);
            va_end(argptr);
        }
    }
    return ret;
}


int  CRender::AddRoom(struct room_s *room)
{
    int ret = 0;

    if(!room->is_in_r_list && room->active)
    {
        float dist, centre[3];
        centre[0] = (room->bb_min[0] + room->bb_max[0]) / 2;
        centre[1] = (room->bb_min[1] + room->bb_max[1]) / 2;
        centre[2] = (room->bb_min[2] + room->bb_max[2]) / 2;
        dist = vec3_dist(m_camera->pos, centre);

        if(r_list_active_count < r_list_size)
        {
            r_list[r_list_active_count].room = room;
            r_list[r_list_active_count].active = 1;
            r_list[r_list_active_count].dist = dist;
            r_list_active_count++;
            ret++;

            if(room->flags & TR_ROOM_FLAG_SKYBOX)
            {
                r_flags |= R_DRAW_SKYBOX;
            }
        }

        room->is_in_r_list = 1;
    }

    return ret;
}

/**
 * The reccursion algorithm: go through the rooms with portal - frustum occlusion test
 * @portal - we entered to the room through that portal
 * @frus - frustum that intersects the portal
 * @return number of added rooms
 */
int CRender::ProcessRoom(struct portal_s *portal, struct frustum_s *frus)
{
    int ret = 0;
    room_p room = portal->dest_room;
    room_p src_room = portal->current_room;

    for(uint16_t i = 0; i < room->portals_count; i++)
    {
        portal_p p = room->portals + i;
        room_p dest_room = Room_CheckFlip(p->dest_room);
        if(dest_room && (dest_room != src_room))   // do not go back
        {
            frustum_p gen_frus = frustumManager->PortalFrustumIntersect(p, frus, m_camera);
            if(gen_frus)
            {
                ret++;
                this->AddRoom(dest_room);
                this->ProcessRoom(p, gen_frus);
            }
        }
    }

    if(room->base_room)
    {
        for(uint16_t i = 0; i < room->base_room->portals_count; i++)
        {
            portal_p p = room->base_room->portals + i;
            room_p dest_room = Room_CheckFlip(p->dest_room);
            if(dest_room && (dest_room != src_room))      // do not go back
            {
                frustum_p gen_frus = frustumManager->PortalFrustumIntersect(p, frus, m_camera);
                if(gen_frus)
                {
                    ret++;
                    this->AddRoom(Room_CheckFlip(dest_room));
                    this->ProcessRoom(p, gen_frus);
                }
            }
        }
    }
    return ret;
}

/**
 * Sets up the light calculations for the given entity based on its current
 * room. Returns the used shader, which will have been made current already.
 */
const lit_shader_description *CRender::SetupEntityLight(struct entity_s *entity, const float modelViewMatrix[16])
{
    // Calculate lighting
    const lit_shader_description *shader;

    room_s *room = entity->self->room;
    if(room != NULL)
    {
        GLfloat ambient_component[4];

        ambient_component[0] = room->content->ambient_lighting[0];
        ambient_component[1] = room->content->ambient_lighting[1];
        ambient_component[2] = room->content->ambient_lighting[2];
        ambient_component[3] = 1.0f;

        if(room->flags & TR_ROOM_FLAG_WATER)
        {
            CalculateWaterTint(ambient_component, 0);
        }

        GLenum current_light_number = 0;
        light_s *current_light = NULL;

        GLfloat positions[3*MAX_NUM_LIGHTS];
        GLfloat colors[4*MAX_NUM_LIGHTS];
        GLfloat innerRadiuses[1*MAX_NUM_LIGHTS];
        GLfloat outerRadiuses[1*MAX_NUM_LIGHTS];
        memset(positions, 0, sizeof(positions));
        memset(colors, 0, sizeof(colors));
        memset(innerRadiuses, 0, sizeof(innerRadiuses));
        memset(outerRadiuses, 0, sizeof(outerRadiuses));

        float *entity_pos = entity->transform + 12;

        for(uint32_t i = 0; (i < room->content->lights_count) && (current_light_number < MAX_NUM_LIGHTS); i++)
        {
            current_light = &room->content->lights[i];

            float x = entity_pos[0] - current_light->pos[0];
            float y = entity_pos[1] - current_light->pos[1];
            float z = entity_pos[2] - current_light->pos[2];

            float distance = sqrtf(x * x + y * y + z * z);

            // Find color
            colors[current_light_number*4 + 0] = std::fmin(std::fmax(current_light->colour[0], 0.0), 1.0);
            colors[current_light_number*4 + 1] = std::fmin(std::fmax(current_light->colour[1], 0.0), 1.0);
            colors[current_light_number*4 + 2] = std::fmin(std::fmax(current_light->colour[2], 0.0), 1.0);
            colors[current_light_number*4 + 3] = std::fmin(std::fmax(current_light->colour[3], 0.0), 1.0);

            if(room->flags & TR_ROOM_FLAG_WATER)
            {
                CalculateWaterTint(colors + current_light_number * 4, 0);
            }

            // Find position
            Mat4_vec3_mul(&positions[3*current_light_number], modelViewMatrix, current_light->pos);

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

        for(uint32_t room_index = 0; (current_light_number < MAX_NUM_LIGHTS) && (room_index < room->near_room_list_size); room_index++)
        {
            room_p near_room = room->near_room_list[room_index];
            for(uint32_t i = 0; (i < near_room->content->lights_count) && (current_light_number < MAX_NUM_LIGHTS); i++)
            {
                current_light = &near_room->content->lights[i];

                float x = entity_pos[0] - current_light->pos[0];
                float y = entity_pos[1] - current_light->pos[1];
                float z = entity_pos[2] - current_light->pos[2];

                float distance = sqrtf(x * x + y * y + z * z);

                // Find color
                colors[current_light_number*4 + 0] = std::fmin(std::fmax(current_light->colour[0], 0.0), 1.0);
                colors[current_light_number*4 + 1] = std::fmin(std::fmax(current_light->colour[1], 0.0), 1.0);
                colors[current_light_number*4 + 2] = std::fmin(std::fmax(current_light->colour[2], 0.0), 1.0);
                colors[current_light_number*4 + 3] = std::fmin(std::fmax(current_light->colour[3], 0.0), 1.0);

                // Find position
                Mat4_vec3_mul(&positions[3*current_light_number], modelViewMatrix, current_light->pos);

                // Find fall-off
                if(distance <= current_light->outer + 1024.0f && (current_light->light_type == LT_POINT || current_light->light_type == LT_SHADOW))
                {
                    innerRadiuses[current_light_number] = std::fabs(current_light->inner);
                    outerRadiuses[current_light_number] = std::fabs(current_light->outer);
                    current_light_number++;
                }
            }
        }

        shader = shaderManager->getEntityShader(current_light_number);
        qglUseProgramObjectARB(shader->program);
        qglUniform4fvARB(shader->light_ambient, 1, ambient_component);
        qglUniform4fvARB(shader->light_color, current_light_number, colors);
        qglUniform3fvARB(shader->light_position, current_light_number, positions);
        qglUniform1fvARB(shader->light_inner_radius, current_light_number, innerRadiuses);
        qglUniform1fvARB(shader->light_outer_radius, current_light_number, outerRadiuses);
    }
    else
    {
        shader = shaderManager->getEntityShader(0);
        qglUseProgramObjectARB(shader->program);
    }
    return shader;
}

/**
 * DEBUG PRIMITIVES RENDERING
 */
CRenderDebugDrawer::CRenderDebugDrawer():
m_drawFlags(0x00000000),
m_max_lines(DEBUG_DRAWER_DEFAULT_BUFFER_SIZE),
m_lines(0),
m_need_realloc(false),
m_gl_vbo(0),
m_buffer(NULL),
m_obb(NULL)
{
    m_buffer = (GLfloat*)malloc(2 * 6 * m_max_lines * sizeof(GLfloat));
    vec3_set_zero(m_color);
    m_obb = OBB_Create();
}

CRenderDebugDrawer::~CRenderDebugDrawer()
{
    free(m_buffer);
    m_buffer = NULL;
    if(m_gl_vbo != 0)
    {
        qglDeleteBuffersARB(1, &m_gl_vbo);
        m_gl_vbo = 0;
    }
    OBB_Clear(m_obb);
    m_obb = NULL;
}

void CRenderDebugDrawer::Reset()
{
    if(m_need_realloc)
    {
        uint32_t new_buffer_size = m_max_lines * 12 * 2;
        GLfloat *new_buffer = (GLfloat*)malloc(new_buffer_size * sizeof(GLfloat));
        if(new_buffer != NULL)
        {
            free(m_buffer);
            m_buffer = new_buffer;
            m_max_lines *= 2;
        }
        m_need_realloc = false;
    }
    if(m_gl_vbo == 0)
    {
        qglGenBuffersARB(1, &m_gl_vbo);
    }
    m_lines = 0;
}

void CRenderDebugDrawer::Render()
{
    if((m_lines > 0) && (m_gl_vbo != 0))
    {
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, m_gl_vbo);
        qglBufferDataARB(GL_ARRAY_BUFFER_ARB, m_lines * 12 * sizeof(GLfloat), m_buffer, GL_STREAM_DRAW);
        qglVertexPointer(3, GL_FLOAT, 6 * sizeof(GLfloat), (void*)0);
        qglColorPointer(3, GL_FLOAT, 6 * sizeof(GLfloat),  (void*)(3 * sizeof(GLfloat)));
        qglDrawArrays(GL_LINES, 0, 2 * m_lines);
        qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }

    vec3_set_zero(m_color);
    m_lines = 0;
}

void CRenderDebugDrawer::DrawAxis(float r, float transform[16])
{
    GLfloat *v0, *v;

    if(m_lines + 3 >= m_max_lines)
    {
        m_need_realloc = true;
        return;
    }

    v0 = v = m_buffer + 3 * 4 * m_lines;
    m_lines += 3;
    vec3_copy(v0, transform + 12);

    // OX
    v += 3;
    v[0] = 1.0;
    v[1] = 0.0;
    v[2] = 0.0;
    v += 3;
    vec3_add_mul(v, v0, transform + 0, r);
    v += 3;
    v[0] = 1.0;
    v[1] = 0.0;
    v[2] = 0.0;
    v += 3;

    // OY
    vec3_copy(v, v0);
    v += 3;
    v[0] = 0.0;
    v[1] = 1.0;
    v[2] = 0.0;
    v += 3;
    vec3_add_mul(v, v0, transform + 4, r);
    v += 3;
    v[0] = 0.0;
    v[1] = 1.0;
    v[2] = 0.0;
    v += 3;

    // OZ
    vec3_copy(v, v0);
    v += 3;
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 1.0;
    v += 3;
    vec3_add_mul(v, v0, transform + 8, r);
    v += 3;
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 1.0;
}

void CRenderDebugDrawer::DrawFrustum(struct frustum_s *f)
{
    if(f != NULL)
    {
        GLfloat *v, *v0;
        float *fv = f->vertex;

        if(m_lines + f->vertex_count >= m_max_lines)
        {
            m_need_realloc = true;
            return;
        }

        v = v0 = m_buffer + 3 * 4 * m_lines;
        m_lines += f->vertex_count;

        for(uint16_t i = 0; i < f->vertex_count - 1; i++, fv += 3)
        {
            vec3_copy(v, fv);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;

            vec3_copy(v, fv + 3);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;
        }

        vec3_copy(v, fv);
        v += 3;
        vec3_copy(v, m_color);
        v += 3;
        vec3_copy(v, v0);
        v += 3;
        vec3_copy(v, m_color);
    }
}

void CRenderDebugDrawer::DrawPortal(struct portal_s *p)
{
    if(p != NULL)
    {
        GLfloat *v, *v0;
        float *pv = p->vertex;

        if(m_lines + p->vertex_count >= m_max_lines)
        {
            m_need_realloc = true;
            return;
        }

        v = v0 = m_buffer + 3 * 4 * m_lines;
        m_lines += p->vertex_count;

        for(uint16_t i = 0; i < p->vertex_count - 1; i++, pv += 3)
        {
            vec3_copy(v, pv);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;

            vec3_copy(v, pv + 3);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;
        }

        vec3_copy(v, pv);
        v += 3;
        vec3_copy(v, m_color);
        v += 3;
        vec3_copy(v, v0);
        v += 3;
        vec3_copy(v, m_color);
    }
}

void CRenderDebugDrawer::DrawBBox(float bb_min[3], float bb_max[3], float *transform)
{
    if(m_lines + 12 < m_max_lines)
    {
        OBB_Rebuild(m_obb, bb_min, bb_max);
        m_obb->transform = transform;
        OBB_Transform(m_obb);
        this->DrawOBB(m_obb);
    }
    else
    {
        m_need_realloc = true;
    }
}

void CRenderDebugDrawer::DrawOBB(struct obb_s *obb)
{
    GLfloat *v, *v0;
    polygon_p p = obb->polygons;

    if(m_lines + 12 >= m_max_lines)
    {
        m_need_realloc = true;
        return;
    }

    v = v0 = m_buffer + 3 * 4 * m_lines;
    m_lines += 12;

    vec3_copy(v, p->vertices[0].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;
    vec3_copy(v, (p+1)->vertices[0].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;

    vec3_copy(v, p->vertices[1].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;
    vec3_copy(v, (p+1)->vertices[3].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;

    vec3_copy(v, p->vertices[2].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;
    vec3_copy(v, (p+1)->vertices[2].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;

    vec3_copy(v, p->vertices[3].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;
    vec3_copy(v, (p+1)->vertices[1].position);
    v += 3;
    vec3_copy(v, m_color);
    v += 3;

    for(uint16_t i = 0; i < 2; i++, p++)
    {
        vertex_p pv = p->vertices;
        v0 = v;
        for(uint16_t j = 0; j < p->vertex_count - 1; j++, pv++)
        {
            vec3_copy(v, pv->position);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;

            vec3_copy(v, (pv+1)->position);
            v += 3;
            vec3_copy(v, m_color);
            v += 3;
        }

        vec3_copy(v, pv->position);
        v += 3;
        vec3_copy(v, m_color);
        v += 3;
        vec3_copy(v, v0);
        v += 3;
        vec3_copy(v, m_color);
        v += 3;
    }
}

void CRenderDebugDrawer::DrawMeshDebugLines(struct base_mesh_s *mesh, float transform[16], const float *overrideVertices, const float *overrideNormals)
{
    if((!m_need_realloc) && (m_drawFlags & R_DRAW_NORMALS))
    {
        GLfloat *v = m_buffer + 3 * 4 * m_lines;
        float n[3];

        if(m_lines + mesh->vertex_count >= m_max_lines)
        {
            m_need_realloc = true;
            return;
        }

        this->SetColor(0.8, 0.0, 0.9);
        m_lines += mesh->vertex_count;
        if(overrideVertices)
        {
            float *ov = (float*)overrideVertices;
            float *on = (float*)overrideNormals;
            for(uint32_t i = 0; i < mesh->vertex_count; i++, ov += 3, on += 3, v += 12)
            {
                Mat4_vec3_mul_macro(v, transform, ov);
                Mat4_vec3_rot_macro(n, transform, on);

                v[6 + 0] = v[0] + n[0] * 128.0;
                v[6 + 1] = v[1] + n[1] * 128.0;
                v[6 + 2] = v[2] + n[2] * 128.0;
                vec3_copy(v+3, m_color);
                vec3_copy(v+9, m_color);
            }
        }
        else
        {
            vertex_p mv = mesh->vertices;
            for (uint32_t i = 0; i < mesh->vertex_count; i++,mv++,v+=12)
            {
                Mat4_vec3_mul_macro(v, transform, mv->position);
                Mat4_vec3_rot_macro(n, transform, mv->normal);

                v[6 + 0] = v[0] + n[0] * 128.0;
                v[6 + 1] = v[1] + n[1] * 128.0;
                v[6 + 2] = v[2] + n[2] * 128.0;
                vec3_copy(v+3, m_color);
                vec3_copy(v+9, m_color);
            }
        }
    }
}

void CRenderDebugDrawer::DrawSkeletalModelDebugLines(struct ss_bone_frame_s *bframe, float transform[16])
{
    if((!m_need_realloc) && m_drawFlags & R_DRAW_NORMALS)
    {
        float tr[16];

        ss_bone_tag_p btag = bframe->bone_tags;
        for(uint16_t i = 0; i < bframe->bone_tag_count; i++, btag++)
        {
            Mat4_Mat4_mul(tr, transform, btag->full_transform);
            this->DrawMeshDebugLines(btag->mesh_base, tr, NULL, NULL);
        }
    }
}

void CRenderDebugDrawer::DrawEntityDebugLines(struct entity_s *entity)
{
    if(m_need_realloc || !(m_drawFlags & (R_DRAW_AXIS | R_DRAW_NORMALS | R_DRAW_BOXES)) ||
       !(entity->state_flags & ENTITY_STATE_VISIBLE) || (entity->bf->animations.model->hide && !(m_drawFlags & R_DRAW_NULLMESHES)))
    {
        return;
    }

    if(m_drawFlags & R_DRAW_BOXES)
    {
        this->SetColor(0.0, 0.0, 1.0);
        this->DrawOBB(entity->obb);
    }

    if(m_drawFlags & R_DRAW_AXIS)
    {
        this->DrawAxis(1000.0, entity->transform);
    }

    if(entity->bf->animations.model && entity->bf->animations.model->animations)
    {
        this->DrawSkeletalModelDebugLines(entity->bf, entity->transform);
    }
}

void CRenderDebugDrawer::DrawSectorDebugLines(struct room_sector_s *rs)
{
    if(m_lines + 12 < m_max_lines)
    {
        float bb_min[3] = {(float)(rs->pos[0] - TR_METERING_SECTORSIZE / 2.0), (float)(rs->pos[1] - TR_METERING_SECTORSIZE / 2.0), (float)rs->floor};
        float bb_max[3] = {(float)(rs->pos[0] + TR_METERING_SECTORSIZE / 2.0), (float)(rs->pos[1] + TR_METERING_SECTORSIZE / 2.0), (float)rs->ceiling};

        this->DrawBBox(bb_min, bb_max, NULL);
    }
    else
    {
        m_need_realloc = true;
    }
}

void CRenderDebugDrawer::DrawRoomDebugLines(struct room_s *room, struct camera_s *cam)
{
    frustum_p frus;
    engine_container_p cont;
    entity_p ent;

    if(m_need_realloc)
    {
        return;
    }

    if(m_drawFlags & R_DRAW_ROOMBOXES)
    {
        this->SetColor(0.0, 0.1, 0.9);
        this->DrawBBox(room->bb_min, room->bb_max, NULL);
        /*for(uint32_t s = 0; s < room->sectors_count; s++)
        {
            drawSectorDebugLines(room->sectors + s);
        }*/
    }

    if(m_drawFlags & R_DRAW_PORTALS)
    {
        this->SetColor(0.0, 0.0, 0.0);
        for(uint16_t i = 0; i < room->portals_count; i++)
        {
            this->DrawPortal(room->portals+i);
        }
    }

    if(m_drawFlags & R_DRAW_FRUSTUMS)
    {
        this->SetColor(1.0, 0.0, 0.0);
        for(frus = room->frustum; frus; frus = frus->next)
        {
            this->DrawFrustum(frus);
        }
    }

    if(!(m_drawFlags & R_SKIP_ROOM) && (room->content->mesh != NULL))
    {
        this->DrawMeshDebugLines(room->content->mesh, room->transform, NULL, NULL);
    }

    if(m_drawFlags & R_DRAW_TRIGGERS)
    {
        this->SetColor(1.0, 0.0, 1.0);
        for(uint32_t i = 0; i < room->sectors_count; i++)
        {
            if(room->sectors[i].trigger)
            {
                this->DrawSectorDebugLines(room->sectors + i);
            }
        }
    }

    for(uint32_t i = 0; i < room->content->static_mesh_count; i++)
    {
        if(Frustum_IsOBBVisibleInFrustumList(room->content->static_mesh[i].obb, (room->frustum) ? (room->frustum) : (cam->frustum)) &&
           (!room->content->static_mesh[i].hide || (m_drawFlags & R_DRAW_DUMMY_STATICS)))
        {
            if(m_drawFlags & R_DRAW_BOXES)
            {
                this->SetColor(0.0, 1.0, 0.1);
                this->DrawOBB(room->content->static_mesh[i].obb);
            }

            if(m_drawFlags & R_DRAW_AXIS)
            {
                this->DrawAxis(1000.0, room->content->static_mesh[i].transform);
            }

            this->DrawMeshDebugLines(room->content->static_mesh[i].mesh, room->content->static_mesh[i].transform, NULL, NULL);
        }
    }

    for(cont = room->content->containers; cont; cont = cont->next)
    {
        switch(cont->object_type)
        {
            case OBJECT_ENTITY:
                ent = (entity_p)cont->object;
                if(Frustum_IsOBBVisibleInFrustumList(ent->obb, (room->frustum) ? (room->frustum) : (cam->frustum)))
                {
                    this->DrawEntityDebugLines(ent);
                }
                break;
        };
    }
}

void CRenderDebugDrawer::DrawLine(const float from[3], const float to[3], const float color_from[3], const float color_to[3])
{
    GLfloat *v;

    if(m_lines < m_max_lines - 1)
    {
        v = m_buffer + 3 * 4 * m_lines;
        m_lines++;

        vec3_copy(v, from);
        v += 3;
        vec3_copy(v, color_from);
        v += 3;
        vec3_copy(v, to);
        v += 3;
        vec3_copy(v, color_to);
    }
    else
    {
        m_need_realloc = true;
    }
}

void CRenderDebugDrawer::DrawContactPoint(const float pointOnB[3], const float normalOnB[3], float distance, int lifeTime, const float color[3])
{
    if(m_lines + 2 < m_max_lines)
    {
        float to[3];
        GLfloat *v = m_buffer + 3 * 4 * m_lines;

        m_lines += 2;
        vec3_add_mul(to, pointOnB, normalOnB, distance);

        vec3_copy(v, pointOnB);
        v += 3;
        vec3_copy(v, color);
        v += 3;

        vec3_copy(v, to);
        v += 3;
        vec3_copy(v, color);

        //qglRasterPos3f(from.x(),  from.y(),  from.z());
        //char buf[12];
        //sprintf(buf," %d",lifeTime);
        //BMF_DrawString(BMF_GetFont(BMF_kHelvetica10),buf);
    }
    else
    {
        m_need_realloc = true;
    }
}

/*
 * Other functions
 */
void CalculateWaterTint(GLfloat *tint, uint8_t fixed_colour)
{
    int version = World_GetVersion();
    if(version < TR_IV)  // If water room and level is TR1-3
    {
        if(version < TR_III)
        {
             // Placeholder, color very similar to TR1 PSX ver.
            if(fixed_colour > 0)
            {
                tint[0] = 0.585f;
                tint[1] = 0.9f;
                tint[2] = 0.9f;
                tint[3] = 1.0f;
            }
            else
            {
                tint[0] *= 0.585f;
                tint[1] *= 0.9f;
                tint[2] *= 0.9f;
            }
        }
        else
        {
            // TOMB3 - closely matches TOMB3
            if(fixed_colour > 0)
            {
                tint[0] = 0.275f;
                tint[1] = 0.45f;
                tint[2] = 0.5f;
                tint[3] = 1.0f;
            }
            else
            {
                tint[0] *= 0.275f;
                tint[1] *= 0.45f;
                tint[2] *= 0.5f;
            }
        }
    }
    else
    {
        if(fixed_colour > 0)
        {
            tint[0] = 1.0f;
            tint[1] = 1.0f;
            tint[2] = 1.0f;
            tint[3] = 1.0f;
        }
    }
}
