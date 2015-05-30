
#include <stdlib.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include "gl_util.h"

#include "bullet/LinearMath/btScalar.h"

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

render_t renderer;
class dynamicBSP render_dBSP(16 * 1024 * 1024);
extern render_DebugDrawer debugDrawer;

/*
 * Shaders section
 */
struct shader_description
{
    GLhandleARB program;
    GLint current_tick;
    GLint tint_mult;
    GLint model_view_projection;
    GLint sampler;

    shader_description(const char *vertexFilename, const char *fragmentFilename);
    shader_description(const char *vertexFilename, GLhandleARB fragmentShader);
    void load(GLhandleARB vertexShader, GLhandleARB fragmentShader);

    ~shader_description();
};

enum room_shader_type
{
    ROOM_SHADER_NORMAL,
    ROOM_SHADER_FLICKERING,
    ROOM_SHADER_FLICKERING_WATER,
    ROOM_SHADER_WATER
};

shader_description *room_shaders[4];
shader_description *static_mesh_shader;

/*GLhandleARB main_vsh, main_fsh, main_program;
GLint       main_model_mat_pos, main_proj_mat_pos, main_model_proj_mat_pos, main_tr_mat_pos;
*/
/*bool btCollisionObjectIsVisible(btCollisionObject *colObj)
{
    engine_container_p cont = (engine_container_p)colObj->getUserPointer();
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

shader_description::shader_description(const char *vertexFilename, const char *fragmentFilename)
{
    GLhandleARB vertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    loadShaderFromFile(vertexShader, vertexFilename);

    GLhandleARB fragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    loadShaderFromFile(fragmentShader, fragmentFilename);

    load(vertexShader, fragmentShader);

    glDeleteObjectARB(vertexShader);
    glDeleteObjectARB(fragmentShader);
}

shader_description::shader_description(const char *filename, GLhandleARB fragmentShader)
{
    GLhandleARB vertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    loadShaderFromFile(vertexShader, filename);

    load(vertexShader, fragmentShader);

    glDeleteObjectARB(vertexShader);
}

void shader_description::load(GLhandleARB vertexShader, GLhandleARB fragmentShader)
{
    program = glCreateProgramObjectARB();
    glAttachObjectARB(program, vertexShader);
    glAttachObjectARB(program, fragmentShader);
    glLinkProgramARB(program);
    printInfoLog(program);

    glDeleteObjectARB(vertexShader);

    current_tick = glGetUniformLocationARB(program, "fCurrentTick");
    tint_mult = glGetUniformLocationARB(program, "tintMult");
    model_view_projection = glGetUniformLocationARB(program, "modelViewProjection");
    sampler = glGetUniformLocationARB(program, "color_map");
}

shader_description::~shader_description()
{
    glDeleteObjectARB(program);
}

void Render_DoShaders()
{
    /*main_program = glCreateProgramObjectARB();
    main_vsh = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    loadShaderFromFile(main_vsh, "shaders/main.vsh");
    glAttachObjectARB(main_program, main_vsh);
    glLinkProgramARB(main_program);
    printInfoLog(main_program);

    glUseProgramObjectARB(main_program);
    main_model_proj_mat_pos = glGetUniformLocationARB(main_program, "modelViewProjectionMat");      //uniform   mat4
    main_model_proj_mat_pos = glGetUniformLocationARB(main_program, "modelViewMat");                //uniform   mat4
    main_model_proj_mat_pos = glGetUniformLocationARB(main_program, "projectionMat");               //uniform   mat4
    main_model_proj_mat_pos = glGetUniformLocationARB(main_program, "transformMat");                //uniform   mat4
    glUseProgramObjectARB(0);*/

    //Color mult prog
    static_mesh_shader = new shader_description("shaders/static_mesh.vsh", "shaders/static_mesh.fsh");

    //Room prog
    GLhandleARB fragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    loadShaderFromFile(fragmentShader, "shaders/room.fsh");

    room_shaders[ROOM_SHADER_NORMAL] = new shader_description("shaders/room_normal.vsh", fragmentShader);
    room_shaders[ROOM_SHADER_FLICKERING] = new shader_description("shaders/room_flickering.vsh", fragmentShader);
    room_shaders[ROOM_SHADER_FLICKERING_WATER] = new shader_description("shaders/room_flickering_water.vsh", fragmentShader);
    room_shaders[ROOM_SHADER_WATER] = new shader_description("shaders/room_water.vsh", fragmentShader);
    glDeleteObjectARB(fragmentShader);
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

    if(static_mesh_shader != 0)
    {
        delete static_mesh_shader;
        static_mesh_shader = 0;
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


/**
 * sprite draw
 * @FIXME: use pixel and pixel size for vertex position calculation disabling
 */
void Render_Sprite(struct sprite_s *sprite, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat *v, buf[24], *up, *right;

    up = renderer.cam->up_dir;
    right = renderer.cam->right_dir;
    v = buf;
    *v = sprite->right * right[0] + sprite->top * up[0] + x;    v++;
    *v = sprite->right * right[1] + sprite->top * up[1] + y;    v++;
    *v = sprite->right * right[2] + sprite->top * up[2] + z;    v++;
    vec3_set_one(v);    v += 3;

    *v = sprite->left * right[0] + sprite->top * up[0] + x;     v++;
    *v = sprite->left * right[1] + sprite->top * up[1] + y;     v++;
    *v = sprite->left * right[2] + sprite->top * up[2] + z;     v++;
    vec3_set_one(v);    v += 3;

    *v = sprite->left * right[0] + sprite->bottom * up[0] + x;  v++;
    *v = sprite->left * right[1] + sprite->bottom * up[1] + y;  v++;
    *v = sprite->left * right[2] + sprite->bottom * up[2] + z;  v++;
    vec3_set_one(v);    v += 3;

    *v = sprite->right * right[0] + sprite->bottom * up[0] + x; v++;
    *v = sprite->right * right[1] + sprite->bottom * up[1] + y; v++;
    *v = sprite->right * right[2] + sprite->bottom * up[2] + z; v++;
    vec3_set_one(v);

    glBindTexture(GL_TEXTURE_2D, renderer.world->textures[sprite->texture]);

    /// Perfect and easy Cochrane's optimisation!
    if(glBindBufferARB)glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glVertexPointer(3, GL_FLOAT, 6 * sizeof(GLfloat), buf);
    glColorPointer(3, GL_FLOAT, 6 * sizeof(GLfloat), buf + 3);
    glTexCoordPointer(2, GL_FLOAT, 0, sprite->tex_coord);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


void Render_SkyBox(const btScalar matrix[16])
{
    btScalar tr[16];
    btScalar *p;

    if((renderer.style & R_DRAW_SKYBOX) && (renderer.world != NULL) && (renderer.world->sky_box != NULL))
    {
        glDepthMask(GL_FALSE);
        tr[15] = 1.0;
        p = renderer.world->sky_box->animations->frames->bone_tags->offset;
        vec3_add(tr+12, renderer.cam->pos, p);
        p = renderer.world->sky_box->animations->frames->bone_tags->qrotate;
        Mat4_set_qrotation(tr, p);
        btScalar full[16];
        Mat4_Mat4_mul(full, matrix, tr);
        glLoadMatrixbt(full);
        Render_Mesh(renderer.world->sky_box->mesh_tree->mesh_base, NULL, NULL);
        glDepthMask(GL_TRUE);
    }
}

/**
 * Opaque meshes drawing
 */
void Render_Mesh(struct base_mesh_s *mesh, const btScalar *overrideVertices, const btScalar *overrideNormals)
{
    for(polygon_p p=mesh->animated_polygons;p!=NULL;p=p->next)
    {
        Render_PolygonTransparency(p);
    }

    if(mesh->vertex_count == 0)
    {
        return;
    }

    if(mesh->vbo_vertex_array)
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vbo_vertex_array);
        glVertexPointer(3, GL_BT_SCALAR, sizeof(vertex_t), (void*)offsetof(vertex_t, position));
        glColorPointer(4, GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, color));
        glNormalPointer(GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));
        glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, tex_coord));
    }

    // Bind overriden vertices if they exist
    if (overrideVertices != NULL)
    {
        // Standard normals are always float. Overridden normals (from skinning)
        // are btScalar.
        if(glBindBufferARB)glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glVertexPointer(3, GL_BT_SCALAR, 0, overrideVertices);
        glNormalPointer(GL_BT_SCALAR, 0, overrideNormals);
    }

    const uint32_t *elementsbase = mesh->elements;
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->vbo_index_array);
        elementsbase = NULL;

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


/**
 * draw transparency polygons
 */
void Render_PolygonTransparency(struct polygon_s *p)
{
    // Blending mode switcher.
    // Note that modes above 2 aren't explicitly used in TR textures, only for
    // internal particle processing. Theoretically it's still possible to use
    // them if you will force type via TRTextur utility.
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

    if((p->anim_id > 0) && (p->anim_id <= engine_world.anim_sequences_count))
    {
        anim_seq_p seq = engine_world.anim_sequences + p->anim_id - 1;
        GLfloat *v, uv[2], *uv0 = (GLfloat*)GetTempbtScalar(2 * p->vertex_count);
        uint16_t frame = (seq->current_frame + p->frame_offset) % seq->frames_count;
        tex_frame_p tf = seq->frames + frame;
        for(uint16_t i=0;i<p->vertex_count;i++)
        {
            v = p->vertices[i].tex_coord;
            uv0[2*i+0] = v[0];
            uv0[2*i+1] = v[1];
            uv[0] = tf->mat[0+0*2] * v[0] + tf->mat[0+1*2] * v[1] + tf->move[0];
            uv[1] = tf->mat[1+0*2] * v[0] + tf->mat[1+1*2] * v[1] + tf->move[1];
            v[0] = uv[0];
            v[1] = uv[1];
        }

        glBindTexture(GL_TEXTURE_2D, renderer.world->textures[tf->tex_ind]);
        if(glBindBufferARB)glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glVertexPointer(3, GL_BT_SCALAR, sizeof(vertex_t), p->vertices->position);
        glColorPointer(4, GL_FLOAT, sizeof(vertex_t), p->vertices->color);
        glNormalPointer(GL_BT_SCALAR, sizeof(vertex_t), p->vertices->normal);
        glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), p->vertices->tex_coord);
        glDrawArrays(GL_TRIANGLE_FAN, 0, p->vertex_count);

        for(uint16_t i=0;i<p->vertex_count;i++)
        {
            v = p->vertices[i].tex_coord;
            v[0] = uv0[2*i+0];
            v[1] = uv0[2*i+1];
        }
        ReturnTempbtScalar(2 * p->vertex_count);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, renderer.world->textures[p->tex_index]);
        if(glBindBufferARB)glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glVertexPointer(3, GL_BT_SCALAR, sizeof(vertex_t), p->vertices->position);
        glColorPointer(4, GL_FLOAT, sizeof(vertex_t), p->vertices->color);
        glNormalPointer(GL_BT_SCALAR, sizeof(vertex_t), p->vertices->normal);
        glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), p->vertices->tex_coord);
        glDrawArrays(GL_TRIANGLE_FAN, 0, p->vertex_count);
    }
}


void Render_BSPFrontToBack(struct bsp_node_s *root)
{
    btScalar d = vec3_plane_dist(root->plane, engine_camera.pos);

    if(d >= 0)
    {
        if(root->front != NULL)
        {
            Render_BSPFrontToBack(root->front);
        }

        for(polygon_p p=root->polygons_front;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(p);
        }
        for(polygon_p p=root->polygons_back;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(p);
        }

        if(root->back != NULL)
        {
            Render_BSPFrontToBack(root->back);
        }
    }
    else
    {
        if(root->back != NULL)
        {
            Render_BSPFrontToBack(root->back);
        }

        for(polygon_p p=root->polygons_back;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(p);
        }
        for(polygon_p p=root->polygons_front;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(p);
        }

        if(root->front != NULL)
        {
            Render_BSPFrontToBack(root->front);
        }
    }
}

void Render_BSPBackToFront(struct bsp_node_s *root)
{
    btScalar d = vec3_plane_dist(root->plane, engine_camera.pos);

    if(d >= 0)
    {
        if(root->back != NULL)
        {
            Render_BSPBackToFront(root->back);
        }

        for(polygon_p p=root->polygons_back;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(p);
        }
        for(polygon_p p=root->polygons_front;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(p);
        }

        if(root->front != NULL)
        {
            Render_BSPBackToFront(root->front);
        }
    }
    else
    {
        if(root->front != NULL)
        {
            Render_BSPBackToFront(root->front);
        }

        for(polygon_p p=root->polygons_front;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(p);
        }
        for(polygon_p p=root->polygons_back;p!=NULL;p=p->next)
        {
            Render_PolygonTransparency(p);
        }

        if(root->back != NULL)
        {
            Render_BSPBackToFront(root->back);
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


void Render_SkinMesh(struct base_mesh_s *mesh, btScalar transform[16])
{
    uint32_t i;
    vertex_p v;
    btScalar *p_vertex, *src_v, *dst_v, t;
    GLfloat *p_normale, *src_n, *dst_n;
    int8_t *ch = mesh->skin_map;

    p_vertex  = (GLfloat*)GetTempbtScalar(3 * mesh->vertex_count);
    p_normale = (GLfloat*)GetTempbtScalar(3 * mesh->vertex_count);
    dst_v = p_vertex;
    dst_n = p_normale;
    v = mesh->vertices;
    for(i=0; i<mesh->vertex_count; i++,v++)
    {
        src_v = v->position;
        src_n = v->normal;
        switch(*ch)
        {
        case 0:
            dst_v[0]  = transform[0] * src_v[0] + transform[1] * src_v[1] + transform[2]  * src_v[2];             // (M^-1 * src).x
            dst_v[1]  = transform[4] * src_v[0] + transform[5] * src_v[1] + transform[6]  * src_v[2];             // (M^-1 * src).y
            dst_v[2]  = transform[8] * src_v[0] + transform[9] * src_v[1] + transform[10] * src_v[2];             // (M^-1 * src).z

            dst_n[0]  = transform[0] * src_n[0] + transform[1] * src_n[1] + transform[2]  * src_n[2];             // (M^-1 * src).x
            dst_n[1]  = transform[4] * src_n[0] + transform[5] * src_n[1] + transform[6]  * src_n[2];             // (M^-1 * src).y
            dst_n[2]  = transform[8] * src_n[0] + transform[9] * src_n[1] + transform[10] * src_n[2];             // (M^-1 * src).z

            vec3_add(dst_v, dst_v, src_v);
            dst_v[0] /= 2.0;
            dst_v[1] /= 2.0;
            dst_v[2] /= 2.0;
            vec3_add(dst_n, dst_n, src_n);
            vec3_norm(dst_n, t);
            break;

        case 2:
            dst_v[0]  = transform[0] * src_v[0] + transform[1] * src_v[1] + transform[2]  * src_v[2];             // (M^-1 * src).x
            dst_v[1]  = transform[4] * src_v[0] + transform[5] * src_v[1] + transform[6]  * src_v[2];             // (M^-1 * src).y
            dst_v[2]  = transform[8] * src_v[0] + transform[9] * src_v[1] + transform[10] * src_v[2];             // (M^-1 * src).z

            dst_n[0]  = transform[0] * src_n[0] + transform[1] * src_n[1] + transform[2]  * src_n[2];             // (M^-1 * src).x
            dst_n[1]  = transform[4] * src_n[0] + transform[5] * src_n[1] + transform[6]  * src_n[2];             // (M^-1 * src).y
            dst_n[2]  = transform[8] * src_n[0] + transform[9] * src_n[1] + transform[10] * src_n[2];             // (M^-1 * src).z
            //vec3_copy(dst_n, src_n);
            break;

        case 1:
            vec3_copy(dst_v, src_v);
            vec3_copy(dst_n, src_n);
            break;
        }
        ch++;
        dst_v += 3;
        dst_n += 3;
    }

    Render_Mesh(mesh, p_vertex, p_normale);
    ReturnTempbtScalar(6 * mesh->vertex_count);
}

/**
 * skeletal model drawing
 */
void Render_SkeletalModel(struct ss_bone_frame_s *bframe, const btScalar matrix[16])
{
    ss_bone_tag_p btag = bframe->bone_tags;

    for(uint16_t i=0; i<bframe->bone_tag_count; i++,btag++)
    {
        btScalar transform[16];
        Mat4_Mat4_mul(transform, matrix, btag->full_transform);
        glLoadMatrixbt(transform);
        Render_Mesh(btag->mesh_base, NULL, NULL);
        if(btag->mesh_slot)
        {
            Render_Mesh(btag->mesh_slot, NULL, NULL);
        }
        if(btag->mesh_skin)
        {
            Render_SkinMesh(btag->mesh_skin, btag->transform);
        }
    }
}


void Render_Entity(struct entity_s *entity, const btScalar matrix[16])
{
    if(entity->was_rendered || !(entity->state_flags & ENTITY_STATE_VISIBLE) || (entity->bf.animations.model->hide && !(renderer.style & R_DRAW_NULLMESHES)))
    {
        return;
    }

    // Calculate lighting
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4);
    glDisable(GL_LIGHT5);
    glDisable(GL_LIGHT6);
    glDisable(GL_LIGHT7);

    room_s *room = entity->self->room;
    if(room != NULL)
    {
        GLfloat ambient_component[4];

        ambient_component[0] = room->ambient_lighting[0];
        ambient_component[1] = room->ambient_lighting[1];
        ambient_component[2] = room->ambient_lighting[2];
        ambient_component[3] = 1.0f;

        if(room->flags & TR_ROOM_FLAG_WATER)
        {
            Render_CalculateWaterTint(ambient_component, 0);
        }

        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_component);

        GLenum current_light_number = GL_LIGHT0;
        light_s *current_light = NULL;

        for(uint32_t i = 0; i < room->light_count; i++)
        {
            if(current_light_number < GL_LIGHT7)
            {
                current_light = &room->lights[i];

                float x = entity->transform[12] - current_light->pos[0];
                float y = entity->transform[13] - current_light->pos[1];
                float z = entity->transform[14] - current_light->pos[2];

                float distance = sqrt(x * x + y * y + z * z);

                float directional_component[4];
                directional_component[0] = current_light->colour[0];
                directional_component[1] = current_light->colour[1];
                directional_component[2] = current_light->colour[2];
                directional_component[3] = current_light->colour[3];


                if(room->flags & TR_ROOM_FLAG_WATER)
                {
                    Render_CalculateWaterTint(directional_component, 0);
                }

                if(current_light->light_type == LT_SUN)
                {
                    glEnable(current_light_number);
                    glLightfv(current_light_number, GL_POSITION, current_light->pos);
                    glLightfv(current_light_number, GL_DIFFUSE, directional_component);

                    glLightf(current_light_number, GL_CONSTANT_ATTENUATION, 1.0f);
                    glLightf(current_light_number, GL_LINEAR_ATTENUATION, 0.0f);
                    glLightf(current_light_number, GL_QUADRATIC_ATTENUATION, 0.0);

                    current_light_number++;
                }
                else if(distance <= current_light->outer + 1024.0f)
                {
                    if(current_light->light_type == LT_POINT)
                    {
                        glEnable(current_light_number);
                        glLightfv(current_light_number, GL_POSITION, current_light->pos);
                        glLightfv(current_light_number, GL_DIFFUSE, directional_component);

                        glLightf(current_light_number, GL_CONSTANT_ATTENUATION, 1.0f);
                        glLightf(current_light_number, GL_LINEAR_ATTENUATION, 0.0f);
                        glLightf(current_light_number, GL_QUADRATIC_ATTENUATION, current_light->falloff);

                        current_light_number++;
                    }
                    else if(current_light->light_type == LT_SHADOW)
                    {
                        glEnable(current_light_number);
                        glLightfv(current_light_number, GL_POSITION, current_light->pos);
                        glLightfv(current_light_number, GL_DIFFUSE, directional_component);

                        glLightf(current_light_number, GL_CONSTANT_ATTENUATION, 1.0f);
                        glLightf(current_light_number, GL_LINEAR_ATTENUATION, 0.0f);
                        glLightf(current_light_number, GL_QUADRATIC_ATTENUATION, current_light->falloff);

                        current_light_number++;
                    }
                }
            }
        }
    }

    if(entity->bf.animations.model && entity->bf.animations.model->animations)
    {
        // base frame offset
        btScalar transform[16];
        if(entity->type_flags & ENTITY_TYPE_KINEMATIC)
        {
            btScalar null_transform[16];
            Mat4_E_macro(null_transform);
            Mat4_Mat4_mul(transform, matrix, null_transform);
        }
        else
        {
            Mat4_Mat4_mul(transform, matrix, entity->transform);
        }
        Render_SkeletalModel(&entity->bf, transform);
    }
}

void Render_Hair(struct entity_s *entity, const btScalar matrix[16])
{
    if((!entity) || !(entity->character) || (entity->character->hair_count == 0) || !(entity->character->hairs))
        return;

    btTransform bt_tr;

    for(int h=0; h<entity->character->hair_count; h++)
    {
        for(uint16_t i=0; i<entity->character->hairs[h].element_count; i++)
        {
            btScalar transform[16];

            bt_tr = entity->character->hairs[h].elements[i].body->getWorldTransform();
            //entity->character->hairs[h].elements[i].body->getMotionState()->getWorldTransform(bt_tr);
            bt_tr.getOpenGLMatrix(transform);

            Mat4_Mat4_mul(transform, matrix, transform);
            glLoadMatrixbt(transform);
            Render_Mesh(entity->character->hairs[h].elements[i].mesh, NULL, NULL);
        }
    }
}

/**
 * drawing world models.
 */
void Render_Room(struct room_s *room, struct render_s *render, const btScalar modelViewMatrix[16], const btScalar modelViewProjectionMatrix[16])
{
    engine_container_p cont;
    entity_p ent;

    if(!(renderer.style & R_SKIP_ROOM) && room->mesh)
    {
        btScalar modelViewProjectionTransform[16];
        Mat4_Mat4_mul(modelViewProjectionTransform, modelViewProjectionMatrix, room->transform);

        shader_description *shader = room_shaders[ROOM_SHADER_NORMAL];
        if (room->flags & 1) // Is water
        {
            if (room->light_mode == 1) // Also flickering
                shader = room_shaders[ROOM_SHADER_FLICKERING_WATER];
            else
                shader = room_shaders[ROOM_SHADER_WATER];
        } else if (room->light_mode == 1) // Only flickering
        {
            shader = room_shaders[ROOM_SHADER_FLICKERING];
        }

        GLfloat tint[4];
        Render_CalculateWaterTint(tint, 1);
        glUseProgramObjectARB(shader->program);
        glUniform4fvARB(shader->tint_mult, 1, tint);
        glUniform1fARB(shader->current_tick, (GLfloat) SDL_GetTicks());
        glUniform1iARB(shader->sampler, 0);
        glUniformMatrix4fvARB(shader->model_view_projection, 1, false, modelViewProjectionTransform);
        Render_Mesh(room->mesh, NULL, NULL);
        glUseProgramObjectARB(0);
    }

    glUseProgramObjectARB(static_mesh_shader->program);
    for(uint32_t i=0; i<room->static_mesh_count; i++)
    {
        if(room->static_mesh[i].was_rendered || !Frustum_IsOBBVisibleInRoom(room->static_mesh[i].obb, room))
        {
            continue;
        }

        if((room->static_mesh[i].hide == 1) && !(renderer.style & R_DRAW_DUMMY_STATICS))
        {
            continue;
        }

        btScalar transform[16];
        Mat4_Mat4_mul(transform, modelViewProjectionMatrix, room->static_mesh[i].transform);
        glUniformMatrix4fvARB(static_mesh_shader->model_view_projection, 1, false, transform);
        glLoadMatrixbt(transform);
        base_mesh_s *mesh = room->static_mesh[i].mesh;
        GLfloat tint[4];

        vec4_copy(tint, room->static_mesh[i].tint);

        //If this static mesh is in a water room
        if(room->flags & TR_ROOM_FLAG_WATER)
        {
            Render_CalculateWaterTint(tint, 0);
        }
        glUniform4fvARB(static_mesh_shader->tint_mult, 1, tint);
        Render_Mesh(mesh, NULL, NULL);
        room->static_mesh[i].was_rendered = 1;
    }
    glUseProgramObjectARB(0);

    glEnable(GL_LIGHTING);
    for(cont=room->containers; cont; cont=cont->next)
    {
        switch(cont->object_type)
        {
        case OBJECT_ENTITY:
            ent = (entity_p)cont->object;
            if(ent->was_rendered == 0)
            {
                if(Frustum_IsOBBVisibleInRoom(ent->obb, room))
                {
                    Render_Entity(ent, modelViewMatrix);
                }
                ent->was_rendered = 1;
            }
            break;
        };
    }
    glDisable(GL_LIGHTING);
}


void Render_Room_Sprites(struct room_s *room, struct render_s *render, const btScalar matrix[16])
{
    glLoadMatrixbt(matrix);
    for(unsigned int i=0; i<room->sprites_count; i++)
    {
        if(!room->sprites[i].was_rendered)
        {
            btScalar *v = room->sprites[i].pos;
            if(room->sprites[i].sprite)
            {
                Render_Sprite(room->sprites[i].sprite, v[0], v[1], v[2]);
            }
        }
        room->sprites[i].was_rendered = 1;
    }
}


/**
 * Безопасное добавление комнаты в список рендерера.
 * Если комната уже есть в списке - возвращается ноль и комната повторно не добавляется.
 * Если список полон, то ничего не добавляется
 */
int Render_AddRoom(struct room_s *room)
{
    int ret = 0;
    engine_container_p cont;
    btScalar dist, centre[3];

    if(room->is_in_r_list || !room->active)
    {
        return 0;
    }

    centre[0] = (room->bb_min[0] + room->bb_max[0]) / 2;
    centre[1] = (room->bb_min[1] + room->bb_max[1]) / 2;
    centre[2] = (room->bb_min[2] + room->bb_max[2]) / 2;
    dist = vec3_dist(renderer.cam->pos, centre);

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

    for(uint32_t i=0; i<room->static_mesh_count; i++)
    {
        room->static_mesh[i].was_rendered = 0;
        room->static_mesh[i].was_rendered_lines = 0;
    }

    for(cont=room->containers; cont; cont=cont->next)
    {
        switch(cont->object_type)
        {
        case OBJECT_ENTITY:
            ((entity_p)cont->object)->was_rendered = 0;
            ((entity_p)cont->object)->was_rendered_lines = 0;
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
        renderer.world->Character->was_rendered = 0;
        renderer.world->Character->was_rendered_lines = 0;
    }

    for(uint32_t i=0; i<renderer.r_list_active_count; i++)
    {
        renderer.r_list[i].active = 0;
        renderer.r_list[i].dist = 0.0;
        room_p r = renderer.r_list[i].room;
        renderer.r_list[i].room = NULL;

        r->is_in_r_list = 0;
        r->active_frustums = 0;
        frustum_p f = r->last_frustum = r->frustum;
        for(; f; f=f->next)
        {
            f->active = 0;
            f->parent = NULL;
            f->parents_count = 0;
        }
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

    glDisable(GL_LIGHTING);
    Render_SkyBox(renderer.cam->gl_view_mat);

    glEnable(GL_LIGHTING);
    if(renderer.world->Character)
    {
        Render_Entity(renderer.world->Character, renderer.cam->gl_view_mat);
        Render_Hair(renderer.world->Character, renderer.cam->gl_view_mat);
    }
    glDisable(GL_LIGHTING);

    /*
     * room rendering
     */
    for(uint32_t i=0; i<renderer.r_list_active_count; i++)
    {
        Render_Room(renderer.r_list[i].room, &renderer, renderer.cam->gl_view_mat, renderer.cam->gl_view_proj_mat);
    }

    glDisable(GL_CULL_FACE);
    glDisableClientState(GL_NORMAL_ARRAY);                                      ///@FIXME: reduce number of gl state changes
    for(uint32_t i=0; i<renderer.r_list_active_count; i++)
    {
        Render_Room_Sprites(renderer.r_list[i].room, &renderer, renderer.cam->gl_view_mat);
    }
    glEnableClientState(GL_NORMAL_ARRAY);

    /*
     * NOW render transparency polygons
     */
    glLoadMatrixbt(renderer.cam->gl_view_mat);
    render_dBSP.reset();
    /*First generate BSP from base room mesh - it has good for start splitter polygons*/
    for(uint32_t i=0;i<renderer.r_list_active_count;i++)
    {
        room_p r = renderer.r_list[i].room;
        if((r->mesh != NULL) && (r->mesh->transparency_polygons != NULL))
        {
            render_dBSP.addNewPolygonList(r->mesh->transparency_polygons, r->transform);
        }
    }

    for(uint32_t i=0;i<renderer.r_list_active_count;i++)
    {
        room_p r = renderer.r_list[i].room;
        // Add transparency polygons from static meshes (if they exists)
        for(uint16_t j=0;j<r->static_mesh_count;j++)
        {
            if((r->static_mesh[j].mesh->transparency_polygons != NULL) && Frustum_IsOBBVisibleInRoom(r->static_mesh[j].obb, r))
            {
                render_dBSP.addNewPolygonList(r->static_mesh[j].mesh->transparency_polygons, r->static_mesh[j].transform);
            }
        }

        // Add transparency polygons from all entities (if they exists) // yes, entities may be animated and intersects with each others;
        for(engine_container_p cont=r->containers;cont!=NULL;cont=cont->next)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                entity_p ent = (entity_p)cont->object;
                if((ent->bf.animations.model->transparency_flags == MESH_HAS_TRANSPARENCY) && (ent->state_flags & ENTITY_STATE_VISIBLE) && (Frustum_IsOBBVisibleInRoom(ent->obb, r)))
                {
                    btScalar tr[16];
                    for(uint16_t j=0;j<ent->bf.bone_tag_count;j++)
                    {
                        if(ent->bf.bone_tags[j].mesh_base->transparency_polygons != NULL)
                        {
                            Mat4_Mat4_mul(tr, ent->transform, ent->bf.bone_tags[j].full_transform);
                            render_dBSP.addNewPolygonList(ent->bf.bone_tags[j].mesh_base->transparency_polygons, tr);
                        }
                    }
                }
            }
        }
    }

    if((engine_world.Character != NULL) && (engine_world.Character->bf.animations.model->transparency_flags == MESH_HAS_TRANSPARENCY))
    {
        btScalar tr[16];
        entity_p ent = engine_world.Character;
        for(uint16_t j=0;j<ent->bf.bone_tag_count;j++)
        {
            if(ent->bf.bone_tags[j].mesh_base->transparency_polygons != NULL)
            {
                Mat4_Mat4_mul(tr, ent->transform, ent->bf.bone_tags[j].full_transform);
                render_dBSP.addNewPolygonList(ent->bf.bone_tags[j].mesh_base->transparency_polygons, tr);
            }
        }
    }

    if(render_dBSP.m_root->polygons_front != NULL)
    {
        glDepthMask(GL_FALSE);
        glDisable(GL_ALPHA_TEST);
        glEnable(GL_BLEND);
        Render_BSPBackToFront(render_dBSP.m_root);
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

    debugDrawer.reset();

    if(renderer.world->Character)
    {
        debugDrawer.drawEntityDebugLines(renderer.world->Character);
    }

    /*
     * Render world debug information
     */
    if((renderer.style & R_DRAW_NORMALS) && (renderer.world != NULL) && (renderer.world->sky_box != NULL))
    {
        GLfloat tr[16];
        btScalar *p;
        Mat4_E_macro(tr);
        p = renderer.world->sky_box->animations->frames->bone_tags->offset;
        vec3_add(tr+12, renderer.cam->pos, p);
        p = renderer.world->sky_box->animations->frames->bone_tags->qrotate;
        Mat4_set_qrotation(tr, p);
        debugDrawer.drawMeshDebugLines(renderer.world->sky_box->mesh_tree->mesh_base, tr, NULL, NULL);
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
        if(glBindBufferARB)glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindTexture(GL_TEXTURE_2D, engine_world.textures[engine_world.tex_count - 1]);
        if(glBindBufferARB)glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
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
int Render_ProcessRoom(struct portal_s *portal, struct frustum_s *frus)
{
    int ret = 0;
    room_p room = portal->dest_room;                                            // куда ведет портал
    room_p src_room = portal->current_room;                                     // откуда ведет портал
    portal_p p;                                                                 // указатель на массив порталов входной ф-ии
    frustum_p gen_frus;                                                         // новый генерируемый фрустум

    if((src_room == NULL) || !src_room->active || (room == NULL) || !room->active)
    {
        return 0;
    }

    p = room->portals;

    for(uint16_t i=0; i<room->portal_count; i++,p++)                            // перебираем все порталы входной комнаты
    {
        if((p->dest_room->active) && (p->dest_room != src_room))                // обратно идти даже не пытаемся
        {
            gen_frus = Portal_FrustumIntersect(p, frus, &renderer);             // Главная ф-я портального рендерера. Тут и проверка
            if(NULL != gen_frus)                                                // на пересечение и генерация фрустума по порталу
            {
                ret++;
                Render_AddRoom(p->dest_room);
                Render_ProcessRoom(p, gen_frus);
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

    room_p curr_room = Room_FindPosCogerrence(renderer.cam->pos, renderer.cam->current_room);                // find room that contains camera

    renderer.cam->current_room = curr_room;                                     // set camera's cuttent room pointer
    if(curr_room != NULL)                                                       // camera located in some room
    {
        curr_room->frustum->parent = NULL;                                      // room with camera inside has no frustums!
        curr_room->frustum->parents_count = 0;
        curr_room->frustum->active = 0;
        curr_room->max_path = 0;
        Render_AddRoom(curr_room);                                              // room with camera inside adds to the render list immediately
        portal_p p = curr_room->portals;                                        // pointer to the portals array
        for(uint32_t i=0; i<curr_room->portal_count; i++,p++)                   // go through all start room portals
        {
            frustum_p last_frus = Portal_FrustumIntersect(p, renderer.cam->frustum, &renderer);
            if(last_frus)
            {
                Render_AddRoom(p->dest_room);                                   // portal destination room
                last_frus->parents_count = 1;                                   // created by camera
                Render_ProcessRoom(p, last_frus);                               // next start reccursion algorithm
            }
        }
    }
    else                                                                        // camera is out of all rooms
    {
        curr_room = renderer.world->rooms;                                      // draw full level. Yes - it is slow, but it is not gameplay - it is debug.
        for(uint32_t i=0; i<renderer.world->room_count; i++,curr_room++)
        {
            if(Frustum_IsAABBVisible(curr_room->bb_min, curr_room->bb_max, renderer.cam->frustum))
            {
                Render_AddRoom(curr_room);
            }
        }
    }
}

/**
 * Состыковка рендерера и "мира"
 */
void Render_SetWorld(struct world_s *world)
{
    uint32_t list_size = world->room_count + 128;                               // magick 128 was added for debug and testing

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
    engine_camera.frustum->next = NULL;
    engine_camera.current_room = NULL;

    for(uint32_t i=0; i<world->room_count; i++)
    {
        world->rooms[i].is_in_r_list = 0;
    }
}


void Render_CalculateWaterTint(GLfloat *tint, uint8_t fixed_colour)
{
    if(engine_world.version < TR_IV)  // If water room and level is TR1-3
    {
        if(engine_world.version < TR_III)
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

/**
 * DEBUG PRIMITIVES RENDERING
 */
render_DebugDrawer::render_DebugDrawer()
:m_debugMode(0)
{
    m_buffer = (GLfloat*)malloc(2 * 3 * 2 * DEBUG_DRAWER_DEFAULT_BUFFER_SIZE * sizeof(GLfloat));
    m_max_lines = DEBUG_DRAWER_DEFAULT_BUFFER_SIZE;
    m_lines = 0;
    vec3_set_zero(m_color);
    m_obb = OBB_Create();
}

render_DebugDrawer::~render_DebugDrawer()
{
    free(m_buffer);
    m_buffer = NULL;
    OBB_Clear(m_obb);
    m_obb = NULL;
}

void render_DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    GLfloat *v;

    if(m_lines < m_max_lines - 1)
    {
        v = m_buffer + 3 * 4 * m_lines;
        m_lines++;

        vec3_copy(v, from.m_floats);
        v += 3;
        vec3_copy(v, color.m_floats);
        v += 3;
        vec3_copy(v, to.m_floats);
        v += 3;
        vec3_copy(v, color.m_floats);
    }
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
   Con_AddLine(warningString, FONTSTYLE_CONSOLE_WARNING);
}

void render_DebugDrawer::drawContactPoint(const btVector3& pointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
{
    if(m_lines + 2 < m_max_lines)
    {
        GLfloat *v = m_buffer + 3 * 4 * m_lines;
        m_lines += 2;
        btVector3 to = pointOnB + normalOnB * distance;

        vec3_copy(v, pointOnB.m_floats);
        v += 3;
        vec3_copy(v, color.m_floats);
        v += 3;

        vec3_copy(v, to.m_floats);
        v += 3;
        vec3_copy(v, color.m_floats);

        //glRasterPos3f(from.x(),  from.y(),  from.z());
        //char buf[12];
        //sprintf(buf," %d",lifeTime);
        //BMF_DrawString(BMF_GetFont(BMF_kHelvetica10),buf);
    }
}

void render_DebugDrawer::render()
{
    if(m_lines > 0)
    {
        glVertexPointer(3, GL_FLOAT, sizeof(GLfloat [6]), m_buffer);
        glColorPointer(3, GL_FLOAT, sizeof(GLfloat [6]),  m_buffer + 3);
        glDrawArrays(GL_LINES, 0, 4 * m_lines);
    }

    vec3_set_zero(m_color);
    m_lines = 0;
}

void render_DebugDrawer::drawAxis(btScalar r, btScalar transform[16])
{
    GLfloat *v0, *v;

    if(m_lines + 3 >= m_max_lines)
    {
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

void render_DebugDrawer::drawFrustum(struct frustum_s *f)
{
    if((f != NULL) && (m_lines + f->count < m_max_lines))
    {
        GLfloat *v, *v0;
        btScalar *fv = f->vertex;

        v = v0 = m_buffer + 3 * 4 * m_lines;
        m_lines += f->count;

        for(uint16_t i=0;i<f->count-1;i++,fv += 3)
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

void render_DebugDrawer::drawPortal(struct portal_s *p)
{
    if((p != NULL) && (m_lines + p->vertex_count < m_max_lines))
    {
        GLfloat *v, *v0;
        btScalar *pv = p->vertex;

        v = v0 = m_buffer + 3 * 4 * m_lines;
        m_lines += p->vertex_count;

        for(uint16_t i=0;i<p->vertex_count-1;i++,pv += 3)
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

void render_DebugDrawer::drawBBox(btScalar bb_min[3], btScalar bb_max[3], btScalar *transform)
{
    if(m_lines + 12 < m_max_lines)
    {
        OBB_Rebuild(m_obb, bb_min, bb_max);
        m_obb->transform = transform;
        OBB_Transform(m_obb);
        drawOBB(m_obb);
    }
}

void render_DebugDrawer::drawOBB(struct obb_s *obb)
{
    GLfloat *v, *v0;
    polygon_p p = obb->polygons;

    if(m_lines + 12 >= m_max_lines)
    {
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

    for(uint16_t i=0; i<2; i++,p++)
    {
        vertex_p pv = p->vertices;
        v0 = v;
        for(uint16_t j=0;j<p->vertex_count-1;j++,pv++)
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

void render_DebugDrawer::drawMeshDebugLines(struct base_mesh_s *mesh, btScalar transform[16], const btScalar *overrideVertices, const btScalar *overrideNormals)
{
    if((renderer.style & R_DRAW_NORMALS) && (m_lines + mesh->vertex_count < m_max_lines))
    {
        GLfloat *v = m_buffer + 3 * 4 * m_lines;
        btScalar n[3];

        setColor(0.8, 0.0, 0.9);
        m_lines += mesh->vertex_count;
        if(overrideVertices)
        {
            btScalar *ov = (btScalar*)overrideVertices;
            btScalar *on = (btScalar*)overrideNormals;
            for(uint32_t i=0; i<mesh->vertex_count; i++,ov+=3,on+=3,v+=12)
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

void render_DebugDrawer::drawSkeletalModelDebugLines(struct ss_bone_frame_s *bframe, btScalar transform[16])
{
    if(renderer.style & R_DRAW_NORMALS)
    {
        btScalar tr[16];

        ss_bone_tag_p btag = bframe->bone_tags;
        for(uint16_t i=0; i<bframe->bone_tag_count; i++,btag++)
        {
            Mat4_Mat4_mul(tr, transform, btag->full_transform);
            drawMeshDebugLines(btag->mesh_base, tr, NULL, NULL);
        }
    }
}

void render_DebugDrawer::drawEntityDebugLines(struct entity_s *entity)
{
    if(entity->was_rendered_lines || !(renderer.style & (R_DRAW_AXIS | R_DRAW_NORMALS | R_DRAW_BOXES)) ||
       !(entity->state_flags & ENTITY_STATE_VISIBLE) || (entity->bf.animations.model->hide && !(renderer.style & R_DRAW_NULLMESHES)))
    {
        return;
    }

    if(renderer.style & R_DRAW_BOXES)
    {
        debugDrawer.setColor(0.0, 0.0, 1.0);
        debugDrawer.drawOBB(entity->obb);
    }

    if(renderer.style & R_DRAW_AXIS)
    {
        // If this happens, the lines after this will get drawn with random colors. I don't care.
        debugDrawer.drawAxis(1000.0, entity->transform);
    }

    if(entity->bf.animations.model && entity->bf.animations.model->animations)
    {
        debugDrawer.drawSkeletalModelDebugLines(&entity->bf, entity->transform);
    }

    entity->was_rendered_lines = 1;
}


void render_DebugDrawer::drawSectorDebugLines(struct room_sector_s *rs)
{
    if(m_lines + 12 < m_max_lines)
    {
        btScalar bb_min[3] = {(btScalar)(rs->pos[0] - TR_METERING_SECTORSIZE / 2.0), (btScalar)(rs->pos[1] - TR_METERING_SECTORSIZE / 2.0), (btScalar)rs->floor};
        btScalar bb_max[3] = {(btScalar)(rs->pos[0] + TR_METERING_SECTORSIZE / 2.0), (btScalar)(rs->pos[1] + TR_METERING_SECTORSIZE / 2.0), (btScalar)rs->ceiling};

        drawBBox(bb_min, bb_max, NULL);
    }
}


void render_DebugDrawer::drawRoomDebugLines(struct room_s *room, struct render_s *render)
{
    uint32_t flag;
    frustum_p frus;
    engine_container_p cont;
    entity_p ent;

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
        for(uint16_t i=0; i<room->portal_count; i++)
        {
            debugDrawer.drawPortal(room->portals+i);
        }
    }

    flag = render->style & R_DRAW_FRUSTUMS;
    if(flag)
    {
        debugDrawer.setColor(1.0, 0.0, 0.0);
        for(frus=room->frustum; frus && frus->active; frus=frus->next)
        {
            debugDrawer.drawFrustum(frus);
        }
    }

    if(!(renderer.style & R_SKIP_ROOM) && (room->mesh != NULL))
    {
        debugDrawer.drawMeshDebugLines(room->mesh, room->transform, NULL, NULL);
    }

    flag = render->style & R_DRAW_BOXES;
    for(uint32_t i=0; i<room->static_mesh_count; i++)
    {
        if(room->static_mesh[i].was_rendered_lines || !Frustum_IsOBBVisibleInRoom(room->static_mesh[i].obb, room) ||
          ((room->static_mesh[i].hide == 1) && !(renderer.style & R_DRAW_DUMMY_STATICS)))
        {
            continue;
        }

        if(flag)
        {
            debugDrawer.setColor(0.0, 1.0, 0.1);
            debugDrawer.drawOBB(room->static_mesh[i].obb);
        }

        if(render->style & R_DRAW_AXIS)
        {
            debugDrawer.drawAxis(1000.0, room->static_mesh[i].transform);
        }

        debugDrawer.drawMeshDebugLines(room->static_mesh[i].mesh, room->static_mesh[i].transform, NULL, NULL);

        room->static_mesh[i].was_rendered_lines = 1;
    }

    for(cont=room->containers; cont; cont=cont->next)
    {
        switch(cont->object_type)
        {
        case OBJECT_ENTITY:
            ent = (entity_p)cont->object;
            if(ent->was_rendered_lines == 0)
            {
                if(Frustum_IsOBBVisibleInRoom(ent->obb, room))
                {
                    debugDrawer.drawEntityDebugLines(ent);
                }
                ent->was_rendered_lines = 1;
            }
            break;
        };
    }
}

