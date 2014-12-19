
#include "stdlib.h"

#include "mesh.h"
#include "polygon.h"
#include "world.h"
#include "vmath.h"
#include "engine.h"
#include "system.h"
#include "gl_util.h"

vertex_p FindVertexInMesh(base_mesh_p mesh, btScalar v[3]);

void BaseMesh_Clear(base_mesh_p mesh)
{
    unsigned int i;

    if(mesh->vbo_vertex_array)
    {
        glDeleteBuffersARB(1, &mesh->vbo_vertex_array);
        mesh->vbo_vertex_array = 0;
    }

    if(mesh->vbo_index_array)
    {
        glDeleteBuffersARB(1, &mesh->vbo_index_array);
        mesh->vbo_index_array = 0;
    }

    if(mesh->polygons != NULL)
    {
        for(i=0;i<mesh->polygons_count;i++)
        {
            Polygon_Clear(mesh->polygons+i);
        }
        free(mesh->polygons);
        mesh->polygons = NULL;
        mesh->polygons_count = 0;
    }

    if(mesh->transparency_polygons != NULL)
    {
        polygon_p p = mesh->transparency_polygons;
        for(polygon_p next=p->next;p!=NULL;)
        {
            Polygon_Clear(p);
            free(p);
            p = next;
            if(p != NULL)
            {
                next = p->next;
            }
        }
        mesh->transparency_polygons = NULL;
    }

    if(mesh->animated_polygons != NULL)
    {
        polygon_p p = mesh->animated_polygons;
        for(polygon_p next=p->next;p!=NULL;)
        {
            Polygon_Clear(p);
            free(p);
            p = next;
            if(p != NULL)
            {
                next = p->next;
            }
        }
        mesh->animated_polygons = NULL;
    }

    if(mesh->vertex_count)
    {
        free(mesh->vertices);
        mesh->vertices = NULL;
        mesh->vertex_count = 0;
    }

    if(mesh->skin_map)
    {
        free(mesh->skin_map);
        mesh->skin_map = NULL;
    }

    if(mesh->element_count_per_texture)
    {
        free(mesh->element_count_per_texture);
        mesh->element_count_per_texture = NULL;
    }

    if (mesh->elements)
    {
        free(mesh->elements);
        mesh->elements = NULL;
    }

    mesh->vertex_count = 0;
}


/**
 * Bounding box calculation
 */
void BaseMesh_FindBB(base_mesh_p mesh)
{
    if(mesh->vertex_count > 0)
    {
        vertex_p v = mesh->vertices;
        vec3_copy(mesh->bb_min, v->position);
        vec3_copy(mesh->bb_max, v->position);
        v ++;
        for(uint32_t i=1;i<mesh->vertex_count;i++,v++)
        {
            // X
            if(mesh->bb_min[0] > v->position[0])
            {
                mesh->bb_min[0] = v->position[0];
            }
            if(mesh->bb_max[0] < v->position[0])
            {
                mesh->bb_max[0] = v->position[0];
            }
            // Y
            if(mesh->bb_min[1] > v->position[1])
            {
                mesh->bb_min[1] = v->position[1];
            }
            if(mesh->bb_max[1] < v->position[1])
            {
                mesh->bb_max[1] = v->position[1];
            }
            // Z
            if(mesh->bb_min[2] > v->position[2])
            {
                mesh->bb_min[2] = v->position[2];
            }
            if(mesh->bb_max[2] < v->position[2])
            {
                mesh->bb_max[2] = v->position[2];
            }
        }

        mesh->centre[0] = (mesh->bb_min[0] + mesh->bb_max[0]) / 2.0;
        mesh->centre[1] = (mesh->bb_min[1] + mesh->bb_max[1]) / 2.0;
        mesh->centre[2] = (mesh->bb_min[2] + mesh->bb_max[2]) / 2.0;
    }
}


void Mesh_GenVBO(struct base_mesh_s *mesh)
{
    mesh->vbo_vertex_array = 0;
    mesh->vbo_index_array = 0;
    if(glGenBuffersARB == NULL)                                                 // if not supported, pointer is NULL
    {
        return;
    }

    /// now, begin VBO filling!
    glGenBuffersARB(1, &mesh->vbo_vertex_array);
    if(mesh->vbo_vertex_array == 0)
    {
        return;
    }

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vbo_vertex_array);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, mesh->vertex_count * sizeof(vertex_t), mesh->vertices, GL_STATIC_DRAW_ARB);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    // Fill indexes vbo
    glGenBuffersARB(1, &mesh->vbo_index_array);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->vbo_index_array);

    GLsizeiptr elementsSize = 0;
    for (uint32_t i = 0; i < mesh->num_texture_pages; i++)
    {
        elementsSize += sizeof(uint32_t) * mesh->element_count_per_texture[i];
    }
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, elementsSize, mesh->elements, GL_STATIC_DRAW_ARB);

    glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}


void SkeletalModel_Clear(skeletal_model_p model)
{
    int i, j;
    animation_frame_p anim;

    if(model)
    {
        if(model->mesh_count)
        {
            model->mesh_count = 0;
            free(model->mesh_tree);
            model->mesh_tree = NULL;
        }

        if(model->collision_map)
        {
            free(model->collision_map);
            model->collision_map = NULL;
            model->collision_map_size = 0;
        }

        if(model->animation_count)
        {
            anim = model->animations;
            for(i=0;i<model->animation_count;i++,anim++)
            {
                if(anim->state_change_count)
                {
                    for(j=0;j<anim->state_change_count;j++)
                    {
                        anim->state_change[j].anim_dispath_count = 0;
                        free(anim->state_change[j].anim_dispath);
                        anim->state_change[j].anim_dispath = NULL;
                        anim->state_change[j].id = 0;
                    }
                    anim->state_change_count = 0;
                    free(anim->state_change);
                    anim->state_change = NULL;
                }

                if(anim->frames_count)
                {
                    for(j=0;j<anim->frames_count;j++)
                    {
                        if(anim->frames[j].bone_tag_count)
                        {
                            anim->frames[j].bone_tag_count = 0;
                            free(anim->frames[j].bone_tags);
                            anim->frames[j].bone_tags = NULL;
                        }
                    }
                    anim->frames_count = 0;
                    free(anim->frames);
                    anim->frames = NULL;
                }
            }
            model->animation_count= 0;
            free(model->animations);
            model->animations = NULL;
        }
    }
}


void BoneFrame_Copy(bone_frame_p dst, bone_frame_p src)
{
    unsigned int i;
    if(dst->bone_tag_count < src->bone_tag_count)
    {
        dst->bone_tags = (bone_tag_p)realloc(dst->bone_tags, src->bone_tag_count * sizeof(bone_tag_t));
    }
    dst->bone_tag_count = src->bone_tag_count;
    vec3_copy(dst->pos, src->pos);
    vec3_copy(dst->centre, src->centre);
    vec3_copy(dst->bb_max, src->bb_max);
    vec3_copy(dst->bb_min, src->bb_min);

    dst->command = src->command;
    vec3_copy(dst->move, src->move);

    for(i=0;i<dst->bone_tag_count;i++)
    {
        vec4_copy(dst->bone_tags[i].qrotate, src->bone_tags[i].qrotate);
        vec3_copy(dst->bone_tags[i].offset, src->bone_tags[i].offset);
    }
}

void SkeletalModel_InterpolateFrames(skeletal_model_p model)
{
    uint16_t i, j, k, l, new_frames_count;
    animation_frame_p anim = model->animations;
    bone_frame_p bf, new_bone_frames;
    btScalar lerp, t;

    for(i=0;i<model->animation_count;i++,anim++)
    {
        if(anim->frames_count > 1 && anim->original_frame_rate > 1)                      // we can't interpolate one frame or rate < 2!
        {
            new_frames_count = (uint16_t)anim->original_frame_rate * (anim->frames_count - 1) + 1;
            bf = new_bone_frames = (bone_frame_p)malloc(new_frames_count * sizeof(bone_frame_t));

            /*
             * the first frame does not changes
             */
            bf->bone_tags = (bone_tag_p)malloc(model->mesh_count * sizeof(bone_tag_t));
            bf->bone_tag_count = model->mesh_count;
            vec3_set_zero(bf->pos);
            vec3_set_zero(bf->move);
            bf->command = 0x00;
            vec3_copy(bf->centre, anim->frames[0].centre);
            vec3_copy(bf->pos, anim->frames[0].pos);
            vec3_copy(bf->bb_max, anim->frames[0].bb_max);
            vec3_copy(bf->bb_min, anim->frames[0].bb_min);
            for(k=0;k<model->mesh_count;k++)
            {
                vec3_copy(bf->bone_tags[k].offset, anim->frames[0].bone_tags[k].offset);
                vec4_copy(bf->bone_tags[k].qrotate, anim->frames[0].bone_tags[k].qrotate);
            }
            bf++;

            for(j=1;j<anim->frames_count;j++)
            {
                for(l=1;l<=anim->original_frame_rate;l++)
                {
                    vec3_set_zero(bf->pos);
                    vec3_set_zero(bf->move);
                    bf->command = 0x00;
                    lerp = ((btScalar)l) / (btScalar)anim->original_frame_rate;
                    t = 1.0 - lerp;

                    bf->bone_tags = (bone_tag_p)malloc(model->mesh_count * sizeof(bone_tag_t));
                    bf->bone_tag_count = model->mesh_count;

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

                    for(k=0;k<model->mesh_count;k++)
                    {
                        bf->bone_tags[k].offset[0] = t * anim->frames[j-1].bone_tags[k].offset[0] + lerp * anim->frames[j].bone_tags[k].offset[0];
                        bf->bone_tags[k].offset[1] = t * anim->frames[j-1].bone_tags[k].offset[1] + lerp * anim->frames[j].bone_tags[k].offset[1];
                        bf->bone_tags[k].offset[2] = t * anim->frames[j-1].bone_tags[k].offset[2] + lerp * anim->frames[j].bone_tags[k].offset[2];

                        vec4_slerp(bf->bone_tags[k].qrotate, anim->frames[j-1].bone_tags[k].qrotate, anim->frames[j].bone_tags[k].qrotate, lerp);
                    }
                    bf++;
                }
            }

            /*
             * swap old and new animation bone brames
             * free old bone frames;
             */
            for(j=0;j<anim->frames_count;j++)
            {
                if(anim->frames[j].bone_tag_count)
                {
                    anim->frames[j].bone_tag_count = 0;
                    free(anim->frames[j].bone_tags);
                    anim->frames[j].bone_tags = NULL;
                }
            }
            free(anim->frames);
            anim->frames = new_bone_frames;
            anim->frames_count = new_frames_count;
        }
    }
}


void SkeletonModel_FillTransparancy(skeletal_model_p model)
{
    model->transparancy_flags = MESH_FULL_OPAQUE;
    for(uint16_t i=0;i<model->mesh_count;i++)
    {
        if(model->mesh_tree[i].mesh->transparency_polygons != NULL)
        {
            model->transparancy_flags = MESH_HAS_TRANSPERENCY;
            return;
        }
    }
}


mesh_tree_tag_p SkeletonClone(mesh_tree_tag_p src, int tags_count)
{
    int i;
    mesh_tree_tag_p ret;

    ret = (mesh_tree_tag_p)malloc(tags_count * sizeof(mesh_tree_tag_t));
    for(i=0;i<tags_count;i++)
    {
        ret[i].mesh = src[i].mesh;
        ret[i].flag = src[i].flag;
        vec3_copy(ret[i].offset, src[i].offset);
        ret[i].overrided = src[i].overrided;
    }
    return ret;
}

void SkeletonCopyMeshes(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count)
{
    int i;
    //Sys_DebugLog(LOG_FILENAME, "tree_1:");
    for(i=0;i<tags_count;i++)
    {
        //Sys_DebugLog(LOG_FILENAME, "id = %d\n", src[i].mesh->id);
        dst[i].mesh = src[i].mesh;
    }
}

void SkeletonCopyMeshes2(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count)
{
    int i;
    //Sys_DebugLog(LOG_FILENAME, "tree_2:");
    for(i=0;i<tags_count;i++)
    {
        //Sys_DebugLog(LOG_FILENAME, "id = %d\n", src[i].mesh->id);
        dst[i].mesh2 = src[i].mesh;
    }
}

vertex_p FindVertexInMesh(base_mesh_p mesh, btScalar v[3])
{
    vertex_p mv = mesh->vertices;
    for(uint32_t i=0;i<mesh->vertex_count;i++,mv++)
    {
        if(vec3_dist_sq(v, mv->position) < 4.0)
        {
            return mv;
        }
    }

    return NULL;
}

void FillSkinnedMeshMap(skeletal_model_p model)
{
    int8_t *ch;
    btScalar tv[3];
    vertex_p v, rv;
    base_mesh_p mesh, mesh2;
    mesh_tree_tag_p tree_tag, prev_tree_tag;

    tree_tag = model->mesh_tree;
    for(uint16_t i=0;i<model->mesh_count;i++,tree_tag++)
    {
        mesh = tree_tag->mesh;
        mesh2 = tree_tag->mesh2;

        if(!mesh2)
        {
            return;
        }

        ch = mesh2->skin_map = (int8_t*)malloc(mesh2->vertex_count * sizeof(int8_t));
        v = mesh2->vertices;
        for(uint32_t k=0;k<mesh2->vertex_count;k++,v++,ch++)
        {
            rv = FindVertexInMesh(mesh, v->position);
            if(rv != NULL)
            {
                *ch = 1;
                vec3_copy(v->position, rv->position);
                vec3_copy(v->normal, rv->normal);
            }
            else
            {
                *ch = 0;
                vec3_add(tv, v->position, tree_tag->offset);
                prev_tree_tag = model->mesh_tree;
                for(uint16_t l=0;l<model->mesh_count;l++,prev_tree_tag++)
                {
                    rv = FindVertexInMesh(prev_tree_tag->mesh, tv);
                    if(rv != NULL)
                    {
                        *ch = 2;
                        vec3_sub(v->position, rv->position, tree_tag->offset);
                        vec3_copy(v->normal, rv->normal);
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
uint32_t Mesh_AddVertex(base_mesh_p mesh, struct vertex_s *vertex)
{
    uint32_t ind = 0;
    vertex_p v;

    v = mesh->vertices;
    for(ind=0;ind<mesh->vertex_count;ind++,v++)
    {
        if(v->position[0] == vertex->position[0] && v->position[1] == vertex->position[1] && v->position[2] == vertex->position[2] &&
           v->tex_coord[0] == vertex->tex_coord[0] && v->tex_coord[1] == vertex->tex_coord[1])
        {
            return ind;
        }
    }

    ind = mesh->vertex_count;                                                   // paranoid
    mesh->vertex_count++;
    mesh->vertices = (vertex_p)realloc(mesh->vertices, mesh->vertex_count * sizeof(vertex_t));

    v = mesh->vertices + ind;
    vec3_copy(v->position, vertex->position);
    vec3_copy(v->normal, vertex->normal);
    vec4_copy(v->color, vertex->color);
    v->tex_coord[0] = vertex->tex_coord[0];
    v->tex_coord[1] = vertex->tex_coord[1];

    return ind;
}


void Mesh_GenFaces(base_mesh_p mesh)
{
    // Note: This code relies on NULL being an all-zero value, which is true on
    // any reasonable system these days.
    if(mesh->element_count_per_texture == NULL)
    {
        mesh->element_count_per_texture = (uint32_t *)calloc(sizeof(uint32_t), mesh->num_texture_pages);
    }
    // First collect indices on a per-texture basis
    uint32_t **elements_for_texture = (uint32_t **)calloc(sizeof(uint32_t*), mesh->num_texture_pages);

    polygon_p p = mesh->polygons;
    for(uint32_t i=0;i<mesh->polygons_count;i++,p++)
    {
        if((p->transparency < 2) && (p->anim_id == 0) && !Polygon_IsBroken(p))
        {
            uint32_t texture = p->tex_index;
            uint32_t oldStart = mesh->element_count_per_texture[texture];

            mesh->element_count_per_texture[texture] += (p->vertex_count - 2) * 3;
            elements_for_texture[texture] = (uint32_t *)realloc(elements_for_texture[texture], mesh->element_count_per_texture[texture] * sizeof(uint32_t));

            // Render the polygon as a triangle fan. That is obviously correct for
            // a triangle and also correct for any quad.
            uint32_t startElement = Mesh_AddVertex(mesh, p->vertices);
            uint32_t previousElement = Mesh_AddVertex(mesh, p->vertices + 1);

            for(uint16_t j = 2; j < p->vertex_count; j++)
            {
                uint32_t thisElement = Mesh_AddVertex(mesh, p->vertices + j);

                elements_for_texture[texture][oldStart + (j - 2)*3 + 0] = startElement;
                elements_for_texture[texture][oldStart + (j - 2)*3 + 1] = previousElement;
                elements_for_texture[texture][oldStart + (j - 2)*3 + 2] = thisElement;

                previousElement = thisElement;
            }
        }
    }

    // Now flatten all these indices to a single array
    mesh->elements = NULL;
    uint32_t elementsSoFar = 0;
    for(uint32_t i = 0; i < mesh->num_texture_pages; i++)
    {
        if(elements_for_texture[i] == NULL)
        {
            continue;
        }
        mesh->elements = (uint32_t*)realloc(mesh->elements, (elementsSoFar + mesh->element_count_per_texture[i])*sizeof(elements_for_texture[i][0]));
        memcpy(mesh->elements + elementsSoFar, elements_for_texture[i], mesh->element_count_per_texture[i]*sizeof(elements_for_texture[i][0]));

        elementsSoFar += mesh->element_count_per_texture[i];
        free(elements_for_texture[i]);
    }
    free(elements_for_texture);
}

