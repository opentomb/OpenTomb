
#include <stdlib.h>
#include <SDL2/SDL_opengl.h>
#include "gl_util.h"

#include "bullet/LinearMath/btScalar.h"

#include "render.h"
#include "world.h"
#include "portal.h"
#include "frustum.h"
#include "polygon.h"
#include "camera.h"
#include "script.h"
#include "vmath.h"
#include "mesh.h"
#include "entity.h"
#include "engine.h"
#include "bounding_volume.h"

render_t renderer;
render_settings_t render_settings;

int R_List_Less(const void *v1, const void *v2);
int R_List_Great(const void *v1, const void *v2);

void Render_InitGlobals()
{
    render_settings.anisotropy = 0;
    render_settings.lod_bias = 0;
    render_settings.antialias = 0;
    render_settings.antialias_samples = 0;
    render_settings.mipmaps = 3;
    render_settings.mipmap_mode = 3;
    render_settings.texture_border = 8;
    render_settings.z_depth = 16;
}


void Render_Init()
{
    renderer.blocked = 1;
    renderer.cam = NULL;

    renderer.r_list = NULL;
    renderer.r_list_size = 0;
    renderer.r_list_active_count= 0;

    renderer.r_transparancy_list = NULL;
    renderer.r_transparancy_list_size = 0;
    renderer.r_transparancy_list_active_count= 0;

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
}


render_list_p Render_CreateRoomListArray(unsigned int count)
{
    unsigned int i;
    render_list_p ret = (render_list_p)malloc(count * sizeof(render_list_t));

    for(i=0;i<count;i++)
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
void Render_Sprite(struct sprite_s *sprite)
{
    btScalar v[12], *up, *right;

    up = renderer.cam->up_dir;
    right = renderer.cam->right_dir;

    v[0] = sprite->right * right[0] + sprite->top * up[0];
    v[1] = sprite->right * right[1] + sprite->top * up[1];
    v[2] = sprite->right * right[2] + sprite->top * up[2];

    v[3] = sprite->left * right[0] + sprite->top * up[0];
    v[4] = sprite->left * right[1] + sprite->top * up[1];
    v[5] = sprite->left * right[2] + sprite->top * up[2];

    v[6] = sprite->left * right[0] + sprite->bottom * up[0];
    v[7] = sprite->left * right[1] + sprite->bottom * up[1];
    v[8] = sprite->left * right[2] + sprite->bottom * up[2];

    v[9] = sprite->right * right[0] + sprite->bottom * up[0];
    v[10]= sprite->right * right[1] + sprite->bottom * up[1];
    v[11]= sprite->right * right[2] + sprite->bottom * up[2];

    glBindTexture(GL_TEXTURE_2D, renderer.world->textures[sprite->texture]);

    /// Perfect and easy Cochrane's optimisation!
    glColor3f(1.0, 1.0, 1.0);
    if(glBindBufferARB)glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glVertexPointer(3, GL_BT_SCALAR, 0, v);
    glTexCoordPointer(2, GL_FLOAT, 0, sprite->tex_coord);
    glDrawArrays(GL_QUADS, 0, 4);
}


void Render_SkyBox()
{
    GLfloat tr[16];

    if(renderer.world != NULL && renderer.world->sky_box != NULL)
    {
        //glDisable(GL_LIGHTING);
        glDepthMask(GL_FALSE);
        glPushMatrix();
        tr[15] = 1.0;
        vec3_add(tr+12, renderer.cam->pos, renderer.world->sky_box->all_bone_tags->offset);
        Mat4_set_qrotation(tr, renderer.world->sky_box->all_bone_tags->qrotate);
        glMultMatrixf(tr);
        Render_Mesh(renderer.world->sky_box->mesh_offset, NULL, NULL);
        glPopMatrix();
        glDepthMask(GL_TRUE);
        //glEnable(GL_LIGHTING);
    }
}

/**
 * Opaque meshes drawing
 */
void Render_Mesh(struct base_mesh_s *mesh, const btScalar *overrideVertices, const btScalar *overrideNormals)
{
    if(mesh->vbo_vertex_array)
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vbo_vertex_array);
        glVertexPointer(3, GL_BT_SCALAR, sizeof(vertex_t), (void*)offsetof(vertex_t, position));
        glColorPointer(4, GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, color));
        glNormalPointer(GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));
        glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), (void*)offsetof(vertex_t, tex_coord));
    }
    else
    {
        if(glBindBufferARB)glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glVertexPointer(3, GL_BT_SCALAR, sizeof(vertex_t), mesh->vertices->position);
        glColorPointer(4, GL_FLOAT, sizeof(vertex_t), mesh->vertices->color);
        glNormalPointer(GL_FLOAT, sizeof(vertex_t), mesh->vertices->normal);
        glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), mesh->vertices->tex_coord);
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
    if(mesh->vbo_index_array)
    {
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->vbo_index_array);
        elementsbase = NULL;
    }

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
 * draw transparancy meshs polygons
 */
/**
 * draw transparancy meshs polygons
 */
void Render_MeshTransparency(struct base_mesh_s *mesh)
{
    uint32_t i;
    polygon_p p;

    if(mesh->transparancy_count <= 0)
    {
        return;
    }

    p = mesh->polygons;
    for(i=0;i<mesh->transparancy_count;i++,p++)
    {
        // Blending mode switcher.
        // Note that modes above 2 aren't explicitly used in TR textures, only for
        // internal particle processing. Theoretically it's still possible to use
        // them if you will force type via TRTextur utility.
        switch(p->transparency)
        {
            default:
            case BM_MULTIPLY:                                 // Classic PC alpha
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
        };

        glBindTexture(GL_TEXTURE_2D, renderer.world->textures[p->tex_index]);
        if(glBindBufferARB)glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glVertexPointer(3, GL_BT_SCALAR, sizeof(vertex_t), p->vertices->position);
        glColorPointer(4, GL_FLOAT, sizeof(vertex_t), p->vertices->color);
        glNormalPointer(GL_BT_SCALAR, sizeof(vertex_t), p->vertices->normal);
        glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), p->vertices->tex_coord);
        glDrawArrays(GL_POLYGON, 0, p->vertex_count);
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
    for(i=0;i<mesh->vertex_count;i++,v++)
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
void Render_SkeletalModel(struct ss_bone_frame_s *bframe)
{
    uint16_t i;
    ss_bone_tag_p btag = bframe->bone_tags;

    for(i=0;i<bframe->bone_tag_count;i++,btag++)
    {
        glPushMatrix();
        glMultMatrixbt(btag->full_transform);
        Render_Mesh(btag->mesh, NULL, NULL);
        if(btag->mesh2)
        {
            Render_SkinMesh(btag->mesh2, btag->transform);
        }
        glPopMatrix();
    }
}


void Render_Entity(struct entity_s *entity)
{
    if(entity->was_rendered || (entity->model->hide == 1) && !(renderer.style & R_DRAW_NULLMESHES))
    {
        return;
    }

    if(entity->model && entity->model->animations)
    {
        glPushMatrix();
        // base frame offset
        glMultMatrixbt(entity->transform);
        Render_SkeletalModel(&entity->bf);
        glPopMatrix();
    }
}


/**
 * drawing world models.
 */
void Render_Room(struct room_s *room, struct render_s *render)
{
    uint32_t i;
    engine_container_p cont;
    entity_p ent;

    if(room->use_alternate && room->alternate_room)
    {
        room = room->alternate_room;
    }

    if(!(renderer.style & R_SKIP_ROOM) && room->mesh)
    {
        glPushMatrix();
        glMultMatrixbt(room->transform);
        Render_Mesh(room->mesh, NULL, NULL);
        glPopMatrix();
    }

    for(i=0;i<room->static_mesh_count;i++)
    {
        if(room->static_mesh[i].was_rendered || !Frustum_IsBVVisibleInRoom(room->static_mesh[i].bv, room))
        {
            continue;
        }

        if((room->static_mesh[i].hide == 1) && !(renderer.style & R_DRAW_DUMMY_STATICS))
        {
            continue;
        }

        glPushMatrix();
        glMultMatrixbt(room->static_mesh[i].transform);
        Render_Mesh(room->static_mesh[i].mesh, NULL, NULL);
        glPopMatrix();
        room->static_mesh[i].was_rendered = 1;
    }

    for(cont=room->containers;cont;cont=cont->next)
    {
        switch(cont->object_type)
        {
            case OBJECT_ENTITY:
                ent = (entity_p)cont->object;
                if(ent->was_rendered == 0)
                {
                    if(Frustum_IsBVVisibleInRoom(ent->bv, room))
                    {
                        Render_Entity(ent);
                    }
                    ent->was_rendered = 1;
                }
                break;
        };
    }
}


void Render_Room_Sprites(struct room_s *room, struct render_s *render)
{
    unsigned int i;
    btScalar *v;

    for(i=0;i<room->sprites_count;i++)
    {
        if(!room->sprites[i].was_rendered)
        {
            v = room->sprites[i].pos;
            glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
            glTranslated(v[0], v[1], v[2]);
#else
            glTranslatef(v[0], v[1], v[2]);
#endif

            Render_Sprite(room->sprites[i].sprite);
            glPopMatrix();
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
    uint32_t i;
    engine_container_p cont;
    btScalar dist, centre[3];

    if(room->is_in_r_list)// || (renderer.r_list_active_count && Room_IsOverlapped(room, renderer.r_list->room)))
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
    }

    if(room->mesh && (room->mesh->transparancy_count > 0) &&            // Has tranparancy polygons
       (renderer.r_transparancy_list_active_count < renderer.r_transparancy_list_size-1))     // If we have enough free space
    {
        renderer.r_transparancy_list[renderer.r_transparancy_list_active_count].room = room;
        renderer.r_transparancy_list[renderer.r_transparancy_list_active_count].active = 1;
        renderer.r_transparancy_list[renderer.r_transparancy_list_active_count].dist = dist;
        renderer.r_transparancy_list_active_count++;
        ret++;
    }

    for(i=0;i<room->static_mesh_count;i++)
    {
        room->static_mesh[i].was_rendered = 0;
        room->static_mesh[i].was_rendered_lines = 0;
    }

    for(cont=room->containers;cont;cont=cont->next)
    {
        switch(cont->object_type)
        {
            case OBJECT_ENTITY:
                ((entity_p)cont->object)->was_rendered = 0;
                ((entity_p)cont->object)->was_rendered_lines = 0;
                break;
        };
    }

    for(i=0;i<room->sprites_count;i++)
    {
        room->sprites[i].was_rendered = 0;
    }

    room->is_in_r_list = 1;

    return ret;
}


void Render_CleanList()
{
    unsigned int i;
    room_p r;
    frustum_p f;

    if(renderer.world->Lara)
    {
        renderer.world->Lara->was_rendered = 0;
        renderer.world->Lara->was_rendered_lines = 0;
    }

    for(i=0;i<renderer.r_list_active_count;i++)
    {
        renderer.r_list[i].active = 0;
        renderer.r_list[i].dist = 0.0;
        r = renderer.r_list[i].room;
        renderer.r_list[i].room = NULL;

        r->is_in_r_list = 0;
        r->active_frustums = 0;
        f = r->last_frustum = r->frustum;
        for(;f;f=f->next)
        {
            f->active = 0;
            f->parent = NULL;
            f->parents_count = 0;
        }
    }

    for(i=0;i<renderer.r_transparancy_list_active_count;i++)
    {
        renderer.r_transparancy_list[i].active = 0;
        renderer.r_transparancy_list[i].dist = 0.0;
        renderer.r_transparancy_list[i].room = NULL;
    }

    renderer.r_list_active_count = 0;
    renderer.r_transparancy_list_active_count = 0;
}

/**
 * Render all visible rooms
 */
void Render_DrawList()
{
    int32_t i;
    room_p room;

    if(!renderer.world)
    {
        return;
    }

    if(renderer.style & R_DRAW_WIRE)
    {
        glPolygonMode(GL_FRONT, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT, GL_FILL);
    }

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    if(renderer.world->Lara)
    {
        Render_Entity(renderer.world->Lara);
    }

    /*
     * room rendering
     */
    for(i=0;i<renderer.r_list_active_count;i++)
    {
        Render_Room(renderer.r_list[i].room, &renderer);
    }

    glDisable(GL_CULL_FACE);
    glDisableClientState(GL_COLOR_ARRAY);                                       ///@FIXME: reduce number of gl state changes
    glDisableClientState(GL_NORMAL_ARRAY);
    for(i=0;i<renderer.r_list_active_count;i++)
    {
        Render_Room_Sprites(renderer.r_list[i].room, &renderer);
    }

    /*
     * NOW render transparency
     */
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    for(i=renderer.r_transparancy_list_active_count-1;i>=0;i--)
    {
        room = renderer.r_transparancy_list[i].room;
        if(room->mesh)
        {
            glPushMatrix();
            glMultMatrixbt(room->transform);
            Render_MeshTransparency(room->mesh);
            glPopMatrix();
        }
    }
    glDisable(GL_BLEND);
}

void Render_DrawList_DebugLines()
{
    uint32_t i;

    if (!renderer.world || !(renderer.style & (R_DRAW_BOXES | R_DRAW_ROOMBOXES | R_DRAW_PORTALS | R_DRAW_FRUSTUMS | R_DRAW_AXIS | R_DRAW_NORMALS)))
    {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, engine_world.textures[engine_world.tex_count - 1]);
    if(glBindBufferARB)glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    if(renderer.world->Lara)
    {
        Render_Entity_DebugLines(renderer.world->Lara);
    }

    /*
     * Render world debug information
     */
    for(i=0;i<renderer.r_list_active_count;i++)
    {
        Render_Room_DebugLines(renderer.r_list[i].room, &renderer);
    }
}

/**
 * The reccursion algorithm прохода по комнатам с отсечкой порталов по фрустумам порталов
 * @portal - we entered to the room through that portal
 * @frus - frustum that intersects the portal
 * @return number of added rooms
 */
int Render_ProcessRoom(struct portal_s *portal, struct frustum_s *frus)
{
    int ret = 0, i;
    room_p room = portal->dest_room;                                            // куда ведет портал
    room_p src_room = portal->current_room;                                     // откуда ведет портал
    portal_p p;                                                                 // указатель на массив порталов входной ф-ии
    frustum_p gen_frus;                                                         // новый генерируемый фрустум

    if(src_room && src_room->use_alternate && src_room->alternate_room)
    {
        src_room = src_room->alternate_room;
    }
    if(room && room->use_alternate && room->alternate_room)
    {
        room = room->alternate_room;
    }
    p = room->portals;

    for(i=0;i<room->portal_count;i++,p++)                                       // перебираем все порталы входной комнаты
    {
        if(p->dest_room != src_room)                                            // обратно идти даже не пытаемся
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
 * Генерация списка рендеринга по текущему миру и камере
 * добавить сортировку комнат по удаленности после генерации билда
 */
void Render_GenWorldList()
{
    uint32_t i;
    room_p next_room, curr_room;
    portal_p p;
    frustum_p last_frus;

    if(!renderer.world)
    {
        return;
    }

    Render_CleanList();                                                         // clear old render list

    curr_room = Room_FindPosCogerrence(renderer.world, renderer.cam->pos, renderer.cam->current_room);                // ищем комнату с камерой
    if(curr_room && curr_room->use_alternate && curr_room->alternate_room)
    {
        curr_room = curr_room->alternate_room;
    }

    renderer.cam->current_room = curr_room;                                     // устанавливаем камере текущую комнату
    if(curr_room)                                                               // камера в какой то комнате
    {
        curr_room->frustum->parent = NULL;                                      // room with camera inside has no frustums!
        curr_room->frustum->parents_count = 0;
        curr_room->frustum->active = 0;
        curr_room->max_path = 0;
        Render_AddRoom(curr_room);                                              // room with camera inside adds to the render list immediately
        p = curr_room->portals;                                                 // указатель на массив порталов стартовой комнаты
        for(i=0;i<curr_room->portal_count;i++,p++)                              // ТУПО!!! перебираем все порталы стартовой комнаты
        {
            last_frus = Portal_FrustumIntersect(p, renderer.cam->frustum, &renderer);
            if(last_frus)
            {
                next_room = Room_CheckAlternate(p->dest_room);                  // portal destination room
                Render_AddRoom(next_room);                                      //
                last_frus->parents_count = 1;                                   // created by camera
                Render_ProcessRoom(p, last_frus);                               // next start reccursion algorithm
            }
        }
    }
    else                                                                        // camera is out of all rooms
    {
        curr_room = renderer.world->rooms;                                      // draw full level. Yes - it is slow, but it is not gameplay - it is debug.
        for(i=0;i<renderer.world->room_count;i++,curr_room++)
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
    uint32_t i, list_size;

    list_size = world->room_count + 128;                                        // дополнительная область списка, для дебага и т.п.

    if(renderer.world)
    {
        if(renderer.r_list_size < list_size)                                    // в случае необходимости расширяем список
        {
            renderer.r_list = (render_list_p)realloc(renderer.r_list, list_size * sizeof(render_list_t));
            renderer.r_transparancy_list = (render_list_p)realloc(renderer.r_transparancy_list, list_size * sizeof(render_list_t));
            for(i=0;i<list_size;i++)
            {
                renderer.r_list[i].active = 0;
                renderer.r_list[i].room = NULL;
                renderer.r_list[i].dist = 0.0;

                renderer.r_transparancy_list[i].active = 0;
                renderer.r_transparancy_list[i].room = NULL;
                renderer.r_transparancy_list[i].dist = 0.0;
            }
        }
    }
    else
    {
        renderer.r_list = Render_CreateRoomListArray(list_size);
        renderer.r_transparancy_list = Render_CreateRoomListArray(list_size);
    }

    renderer.world = world;
    renderer.r_list_size = list_size;
    renderer.r_list_active_count = 0;

    renderer.r_transparancy_list_size = list_size;
    renderer.r_transparancy_list_active_count = 0;

    renderer.cam = &engine_camera;
    engine_camera.frustum->next = NULL;
    engine_camera.current_room = NULL;

    for(i=0;i<world->room_count;i++)
    {
        world->rooms[i].is_in_r_list = 0;
    }
}

/**
 * DEBUG PRIMITIVES RENDERING
 */
void Render_Room_DebugLines(struct room_s *room, struct render_s *render)
{
    if (!(render->style & (R_DRAW_BOXES | R_DRAW_ROOMBOXES | R_DRAW_PORTALS | R_DRAW_FRUSTUMS | R_DRAW_AXIS | R_DRAW_NORMALS/* | R_DRAW_NULLMESHES | R_DRAW_DUMMY_STATICS*/)))
    {
        return;
    }

    unsigned int i, flag;
    frustum_p frus;
    engine_container_p cont;
    room_sector_p s;
    entity_p ent;

    if(room->use_alternate && room->alternate_room)
    {
        room = room->alternate_room;
    }
    glPushAttrib(GL_LINE_BIT);
    glLineWidth(4.0);
    flag = render->style & R_DRAW_ROOMBOXES;
    if(flag)
    {
        glColor3f(0.0, 0.1, 0.9);
        Render_BBox(room->bb_min, room->bb_max);
    }

    flag = render->style & R_DRAW_PORTALS;
    if(flag)
    {
        glColor3f(0.0, 0.0, 0.0);
        glLineWidth(3.0f);
        for(i=0;i<room->portal_count;i++)
        {
            Portal_DrawVire(room->portals+i);
        }
    }

    flag = render->style & R_DRAW_FRUSTUMS;
    if(flag)
    {
        glColor3f(1.0, 0.0, 0.0);
        glLineWidth(3.0f);
        for(frus=room->frustum;frus && frus->active;frus=frus->next)
        {
            Frustum_DrawVire(frus);
        }
    }
    glPopAttrib();

    if(!(renderer.style & R_SKIP_ROOM))
    {
        glPushMatrix();
        glMultMatrixbt(room->transform);
        if(room->mesh)
        {
            Render_Mesh_DebugLines(room->mesh, NULL, NULL);
        }
        s = room->sectors;
        for(i=0;i<room->sectors_count;i++,s++)
        {
            //if(s->fd_portal_room != -1)
            if(s->fd_end_level != 0)
            {
                Render_SectorBorders(s);
            }
        }
        glPopMatrix();
    }

    flag = render->style & R_DRAW_BOXES;
    for(i=0;i<room->static_mesh_count;i++)
    {
        if(flag)
        {
            glColor3f(0.0, 1.0, 0.1);
            Render_BV(room->static_mesh[i].bv);
        }
        if(room->static_mesh[i].was_rendered_lines || !Frustum_IsBVVisibleInRoom(room->static_mesh[i].bv, room))
        {
            continue;
        }

        if((room->static_mesh[i].hide == 1) && !(renderer.style & R_DRAW_DUMMY_STATICS))
        {
            continue;
        }

        glPushMatrix();
        glMultMatrixbt(room->static_mesh[i].transform);
        if(render->style & R_DRAW_AXIS)
        {
            Render_DrawAxis(1000.0);
        }
        Render_Mesh_DebugLines(room->static_mesh[i].mesh, NULL, NULL);
        glPopMatrix();
        room->static_mesh[i].was_rendered_lines = 1;
    }

    for(cont=room->containers;cont;cont=cont->next)
    {
        switch(cont->object_type)
        {
            case OBJECT_ENTITY:
                ent = (entity_p)cont->object;
                if(ent->was_rendered_lines == 0)
                {
                    if(Frustum_IsBVVisibleInRoom(ent->bv, room))
                    {
                        Render_Entity_DebugLines(ent);
                    }
                    ent->was_rendered_lines = 1;
                }
                break;
        };
    }
}


void Render_SkyBox_DebugLines()
{
    GLfloat tr[16], *q;
    if(!(renderer.style & R_DRAW_NORMALS))
    {
        return;
    }

    if(renderer.world != NULL && renderer.world->sky_box != NULL)
    {
        glDepthMask(GL_FALSE);
        glPushMatrix();
        tr[15] = 1.0;
        q = renderer.world->sky_box->all_bone_tags->qrotate;
        vec3_add(tr+12, renderer.cam->pos, renderer.world->sky_box->all_bone_tags->offset);
        Mat4_set_qrotation(tr, q);
        glMultMatrixf(tr);

        Render_Mesh_DebugLines(renderer.world->sky_box->mesh_offset, NULL, NULL);
        glPopMatrix();

        glDepthMask(GL_TRUE);
    }
}


void Render_Entity_DebugLines(struct entity_s *entity)
{
    if(entity->was_rendered_lines || !(renderer.style & (R_DRAW_AXIS | R_DRAW_NORMALS | R_DRAW_BOXES)))
    {
        return;
    }

    if((entity->model->hide == 1) && !(renderer.style & R_DRAW_NULLMESHES))
    {
        return;
    }

    if(renderer.style & R_DRAW_BOXES)
    {
        glColor3f(0.0, 0.0, 1.0);
        Render_BV(entity->bv);
    }

    if(entity->model && entity->model->animations)
    {
        glPushMatrix();
        // base frame offset
        glMultMatrixbt(entity->transform);
        Render_SkeletalModel_DebugLines(&entity->bf);
        glPopMatrix();
    }

    entity->was_rendered_lines = 1;
}


void Render_SkeletalModel_DebugLines(struct ss_bone_frame_s *bframe)
{
    if(!(renderer.style & (R_DRAW_NORMALS | R_DRAW_AXIS)))
    {
        return;
    }

    int i;
    ss_bone_tag_p btag = bframe->bone_tags;

    if(renderer.style & R_DRAW_AXIS)
    {
        // If this happens, the lines after this will get drawn with random colors. I don't care.
        Render_DrawAxis(1000.0);
    }
    for(i=0;i<bframe->bone_tag_count;i++,btag++)
    {
        glPushMatrix();
        glMultMatrixbt(btag->full_transform);
        Render_Mesh_DebugLines(btag->mesh, NULL, NULL);

        /*if(btag->mesh2)
        {
            Render_SkinMesh_DebugLines(btag->mesh2, btag->transform);
        }*/
        glPopMatrix();
    }
}


void Render_Mesh_DebugLines(struct base_mesh_s *mesh, const btScalar *overrideVertices, const btScalar *overrideNormals)
{
    uint32_t i;

    if(renderer.style & R_DRAW_NORMALS)
    {
        btScalar *normalLines = GetTempbtScalar(3 * 2 * mesh->vertex_count);

        if(overrideVertices)
        {
            for(i=0;i<mesh->vertex_count;i++)
            {
                memcpy(&normalLines[i*6], overrideVertices+3*i, sizeof(btScalar [3]));
                normalLines[i*6 + 3] = (overrideVertices+3*i)[0] + (overrideNormals+3*i)[0] * 128.0;
                normalLines[i*6 + 4] = (overrideVertices+3*i)[1] + (overrideNormals+3*i)[1] * 128.0;
                normalLines[i*6 + 5] = (overrideVertices+3*i)[2] + (overrideNormals+3*i)[2] * 128.0;
            }
        }
        else
        {
            for (i = 0; i < mesh->vertex_count; i++)
            {
                memcpy(&normalLines[i*6], mesh->vertices[i].position, sizeof(btScalar [3]));
                normalLines[i*6 + 3] = mesh->vertices[i].position[0] + mesh->vertices[i].normal[0] * 128.0;
                normalLines[i*6 + 4] = mesh->vertices[i].position[1] + mesh->vertices[i].normal[1] * 128.0;
                normalLines[i*6 + 5] = mesh->vertices[i].position[2] + mesh->vertices[i].normal[2] * 128.0;
            }
        }

        glVertexPointer(3, GL_BT_SCALAR, 0, normalLines);
        glDrawArrays(GL_LINES, 0, mesh->vertex_count * 2);

        ReturnTempbtScalar(3 * 2 * mesh->vertex_count);
    }
}


void Render_DrawAxis(btScalar r)
{
    // debug local coordinate system axis drawing
    const GLfloat vertexArray[] = {
            1.0, 0.0, 0.0,	0.0, 0.0, 0.0,
            1.0, 0.0, 0.0,	r, 0.0, 0.0,
            0.0, 1.0, 0.0,	0.0, 0.0, 0.0,
            0.0, 1.0, 0.0,	0.0, r, 0.0,
            0.0, 0.0, 1.0,	0.0, 0.0, 0.0,
            0.0, 0.0, 1.0,	0.0, 0.0, r
    };

    glEnableClientState(GL_COLOR_ARRAY);                                        ///@FIXME: reduce number of gl state changes
    if(glBindBufferARB)glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    glVertexPointer(3, GL_FLOAT, sizeof(GLfloat [6]), &vertexArray[3]);
    glColorPointer(3, GL_FLOAT, sizeof(GLfloat [6]), &vertexArray[0]);
    glDrawArrays(GL_LINES, 0, 6);
    glDisableClientState(GL_COLOR_ARRAY);
}

/**
 * bounding box rendering
 */
void Render_BBox(btScalar bb_min[3], btScalar bb_max[3])
{
    btScalar vertices[24] = {
        bb_min[0], bb_min[1], bb_min[2],
        bb_min[0], bb_max[1], bb_min[2],
        bb_max[0], bb_max[1], bb_min[2],
        bb_max[0], bb_min[1], bb_min[2],

        bb_min[0], bb_min[1], bb_max[2],
        bb_min[0], bb_max[1], bb_max[2],
        bb_max[0], bb_max[1], bb_max[2],
        bb_max[0], bb_min[1], bb_max[2]
        };

    const GLushort indices[24] = {
        0, 1,
        1, 2,
        2, 3,
        3, 0,

        4, 5,
        5, 6,
        6, 7,
        7, 4,

        0, 4,
        1, 5,
        2, 6,
        3, 7
        };
    static int createdVBO = 0;
    static GLuint indicesVBO;

    if(createdVBO == 0)
    {
        if (glGenBuffersARB)
        {
            glGenBuffersARB(1, &indicesVBO);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indicesVBO);
            glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(indices), indices, GL_STATIC_DRAW_ARB);
        }
        createdVBO = 0;
    }

    glVertexPointer(3, GL_BT_SCALAR, sizeof(vertices[0]) * 3, vertices);
    if(indicesVBO)
    {
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indicesVBO);
    }
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, NULL);
}

/**
 * sector's borders rendering
 */
void Render_SectorBorders(struct room_sector_s *sector)
{
    btScalar bb_min[3], bb_max[3];

    if(sector->floor == 32512 || sector->ceiling == 32512)
    {
        return;
    }

    bb_min[2] = (btScalar)sector->floor;
    bb_max[2] = (btScalar)sector->ceiling;

    //bb_min[2] = (btScalar)sector->owner_room->bb_min[2];
    //bb_max[2] = (btScalar)sector->owner_room->bb_max[2];

    bb_min[0] = sector->owner_room->transform[12];
    bb_min[0] += 1024.0 * (btScalar) sector->index_x;
    bb_min[1] = sector->owner_room->transform[13];
    bb_min[1] += 1024.0 * (btScalar) sector->index_y;

    bb_max[0] = bb_min[0] + 1024.0;
    bb_max[1] = bb_min[1] + 1024.0;

    Render_BBox(bb_min, bb_max);
}

void Render_BV(struct bounding_volume_s *bv)
{
    uint16_t i;
    polygon_p p;

    p = bv->polygons;
    for(i=0;i<bv->polygons_count;i++,p++)
    {
        glVertexPointer(3, GL_BT_SCALAR, sizeof(vertex_t), p->vertices);
        glDrawArrays(GL_LINE_LOOP, 0, p->vertex_count);
    }
}

