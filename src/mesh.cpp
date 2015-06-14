
#include "stdlib.h"

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
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"


vertex_p FindVertexInMesh(base_mesh_p mesh, btScalar v[3]);

void BaseMesh_Clear(base_mesh_p mesh)
{
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
        for(uint32_t i=0;i<mesh->polygons_count;i++)
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

    if(mesh->vertex_count)
    {
        free(mesh->vertices);
        mesh->vertices = NULL;
        mesh->vertex_count = 0;
    }

    if(mesh->matrix_indices)
    {
        free(mesh->matrix_indices);
        mesh->matrix_indices = NULL;
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


void Mesh_GenVBO(const struct render_s *renderer, struct base_mesh_s *mesh)
{
    mesh->vbo_vertex_array = 0;
    mesh->vbo_index_array = 0;

    /// now, begin VBO filling!
    glGenBuffersARB(1, &mesh->vbo_vertex_array);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vbo_vertex_array);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, mesh->vertex_count * sizeof(vertex_t), mesh->vertices, GL_STATIC_DRAW_ARB);

    // Store additional skinning information
    if (mesh->matrix_indices)
    {
        glGenBuffersARB(1, &mesh->vbo_skin_array);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vbo_skin_array);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, mesh->vertex_count * 2, mesh->matrix_indices, GL_STATIC_DRAW_ARB);
    }

    // Fill indexes vbo
    glGenBuffersARB(1, &mesh->vbo_index_array);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->vbo_index_array);

    GLsizeiptr elementsSize = sizeof(uint32_t) * mesh->alpha_elements;
    for (uint32_t i = 0; i < mesh->num_texture_pages; i++)
    {
        elementsSize += sizeof(uint32_t) * mesh->element_count_per_texture[i];
    }
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, elementsSize, mesh->elements, GL_STATIC_DRAW_ARB);

    // Prepare vertex array
    vertex_array_attribute attribs[] = {
        vertex_array_attribute(lit_shader_description::vertex_attribs::position, 3, GL_FLOAT, false, mesh->vbo_vertex_array, sizeof(vertex_t), offsetof(vertex_t, position)),
        vertex_array_attribute(lit_shader_description::vertex_attribs::normal, 3, GL_FLOAT, false, mesh->vbo_vertex_array, sizeof(vertex_t), offsetof(vertex_t, normal)),
        vertex_array_attribute(lit_shader_description::vertex_attribs::color, 4, GL_FLOAT, false, mesh->vbo_vertex_array, sizeof(vertex_t), offsetof(vertex_t, color)),
        vertex_array_attribute(lit_shader_description::vertex_attribs::tex_coord, 2, GL_FLOAT, false, mesh->vbo_vertex_array, sizeof(vertex_t), offsetof(vertex_t, tex_coord)),
        // Only used for skinned meshes
        vertex_array_attribute(lit_shader_description::vertex_attribs::matrix_index, 2, GL_UNSIGNED_BYTE, false, mesh->vbo_skin_array, 2, 0),
    };
    int numAttribs = mesh->matrix_indices ? 5 : 4;
    mesh->main_vertex_array = renderer->vertex_array_manager->createArray(mesh->vbo_index_array, numAttribs, attribs);

    // Now for animated polygons, if any
    if (mesh->num_animated_elements != 0 || mesh->num_alpha_animated_elements != 0)
    {
        // And upload.
        glGenBuffersARB(1, &mesh->animated_vbo_vertex_array);
        glBindBufferARB(GL_ARRAY_BUFFER, mesh->animated_vbo_vertex_array);
        glBufferDataARB(GL_ARRAY_BUFFER, sizeof(animated_vertex_t) * mesh->animated_vertex_count, mesh->animated_vertices, GL_STATIC_DRAW);

        glGenBuffersARB(1, &mesh->animated_vbo_index_array);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, mesh->animated_vbo_index_array);
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * (mesh->num_animated_elements + mesh->num_alpha_animated_elements), mesh->animated_elements, GL_STATIC_DRAW);

        // Prepare empty buffer for tex coords
        glGenBuffersARB(1, &mesh->animated_vbo_texcoord_array);
        glBindBufferARB(GL_ARRAY_BUFFER, mesh->animated_vbo_texcoord_array);
        glBufferDataARB(GL_ARRAY_BUFFER, sizeof(GLfloat [2]) * mesh->animated_vertex_count, 0, GL_STREAM_DRAW);

        // Create vertex array object.
        vertex_array_attribute attribs[] = {
            vertex_array_attribute(lit_shader_description::vertex_attribs::position, 3, GL_FLOAT, false, mesh->animated_vbo_vertex_array, sizeof(animated_vertex_t), offsetof(animated_vertex_t, position)),
            vertex_array_attribute(lit_shader_description::vertex_attribs::color, 4, GL_FLOAT, false, mesh->animated_vbo_vertex_array, sizeof(animated_vertex_t), offsetof(animated_vertex_t, color)),
            vertex_array_attribute(lit_shader_description::vertex_attribs::normal, 3, GL_FLOAT, false, mesh->animated_vbo_vertex_array, sizeof(animated_vertex_t), offsetof(animated_vertex_t, normal)),

            vertex_array_attribute(lit_shader_description::vertex_attribs::tex_coord, 2, GL_FLOAT, false, mesh->animated_vbo_texcoord_array, sizeof(GLfloat [2]), 0),
        };
        mesh->animated_vertex_array = renderer->vertex_array_manager->createArray(mesh->animated_vbo_index_array, 4, attribs);
    }
    else
    {
        // No animated data
        mesh->animated_vbo_vertex_array = 0;
        mesh->animated_vbo_texcoord_array = 0;
        mesh->animated_vertex_array = 0;
    }

    // Update references for transparent polygons
    for (uint32_t i = 0; i < mesh->transparent_polygon_count; i++)
    {
        mesh->transparent_polygons[i].used_vertex_array = mesh->transparent_polygons[i].isAnimated ? mesh->animated_vertex_array : mesh->main_vertex_array;
    }
}


void SkeletalModel_Clear(skeletal_model_p model)
{
    if(model != NULL)
    {
        if(model->mesh_tree)
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
            animation_frame_p anim = model->animations;
            for(uint16_t i=0;i<model->animation_count;i++,anim++)
            {
                if(anim->state_change_count)
                {
                    for(uint16_t j=0;j<anim->state_change_count;j++)
                    {
                        anim->state_change[j].anim_dispatch_count = 0;
                        free(anim->state_change[j].anim_dispatch);
                        anim->state_change[j].anim_dispatch = NULL;
                        anim->state_change[j].id = 0;
                    }
                    anim->state_change_count = 0;
                    free(anim->state_change);
                    anim->state_change = NULL;
                }

                if(anim->frames_count)
                {
                    for(uint16_t j=0;j<anim->frames_count;j++)
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


void SSBoneFrame_CreateFromModel(ss_bone_frame_p bf, skeletal_model_p model)
{
    bf->hasSkin = false;
    vec3_set_zero(bf->bb_min);
    vec3_set_zero(bf->bb_max);
    vec3_set_zero(bf->centre);
    vec3_set_zero(bf->pos);
    bf->animations.anim_flags = 0x0000;
    bf->animations.frame_time = 0.0;
    bf->animations.period = 1.0 / 30.0;
    bf->animations.next_state = 0;
    bf->animations.lerp = 0.0;
    bf->animations.current_animation = 0;
    bf->animations.current_frame = 0;
    bf->animations.next_animation = 0;
    bf->animations.next_frame = 0;

    bf->animations.next = NULL;
    bf->animations.onFrame = NULL;
    bf->animations.model = model;
    bf->bone_tag_count = model->mesh_count;
    bf->bone_tags = (ss_bone_tag_p)malloc(bf->bone_tag_count * sizeof(ss_bone_tag_t));

    int stack = 0;
    ss_bone_tag_p parents[bf->bone_tag_count];
    parents[0] = NULL;
    bf->bone_tags[0].parent = NULL;                                             // root
    for(uint16_t i=0;i<bf->bone_tag_count;i++)
    {
        bf->bone_tags[i].index = i;
        bf->bone_tags[i].mesh_base = model->mesh_tree[i].mesh_base;
        bf->bone_tags[i].mesh_skin = model->mesh_tree[i].mesh_skin;
        if (bf->bone_tags[i].mesh_skin)
            bf->hasSkin = true;
        bf->bone_tags[i].mesh_slot = NULL;
        bf->bone_tags[i].body_part = model->mesh_tree[i].body_part;

        vec3_copy(bf->bone_tags[i].offset, model->mesh_tree[i].offset);
        vec4_set_zero(bf->bone_tags[i].qrotate);
        Mat4_E_macro(bf->bone_tags[i].transform);
        Mat4_E_macro(bf->bone_tags[i].full_transform);

        if(i > 0)
        {
            bf->bone_tags[i].parent = &bf->bone_tags[i-1];
            if(model->mesh_tree[i].flag & 0x01)                                 // POP
            {
                if(stack > 0)
                {
                    bf->bone_tags[i].parent = parents[stack];
                    stack--;
                }
            }
            if(model->mesh_tree[i].flag & 0x02)                                 // PUSH
            {
                if(stack + 1 < (int16_t)model->mesh_count)
                {
                    stack++;
                    parents[stack] = bf->bone_tags[i].parent;
                }
            }
        }
    }
}


void BoneFrame_Copy(bone_frame_p dst, bone_frame_p src)
{
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

    for(uint16_t i=0;i<dst->bone_tag_count;i++)
    {
        vec4_copy(dst->bone_tags[i].qrotate, src->bone_tags[i].qrotate);
        vec3_copy(dst->bone_tags[i].offset, src->bone_tags[i].offset);
    }
}

void SkeletalModel_InterpolateFrames(skeletal_model_p model)
{
    uint16_t new_frames_count;
    animation_frame_p anim = model->animations;
    bone_frame_p bf, new_bone_frames;
    btScalar lerp, t;

    for(uint16_t i=0;i<model->animation_count;i++,anim++)
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
            for(uint16_t k=0;k<model->mesh_count;k++)
            {
                vec3_copy(bf->bone_tags[k].offset, anim->frames[0].bone_tags[k].offset);
                vec4_copy(bf->bone_tags[k].qrotate, anim->frames[0].bone_tags[k].qrotate);
            }
            bf++;

            for(uint16_t j=1;j<anim->frames_count;j++)
            {
                for(uint16_t l=1;l<=anim->original_frame_rate;l++)
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

                    for(uint16_t k=0;k<model->mesh_count;k++)
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
            for(uint16_t j=0;j<anim->frames_count;j++)
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


void SkeletonModel_FillTransparency(skeletal_model_p model)
{
    model->transparency_flags = MESH_FULL_OPAQUE;
    for(uint16_t i=0;i<model->mesh_count;i++)
    {
        if(model->mesh_tree[i].mesh_base->transparency_polygons != NULL)
        {
            model->transparency_flags = MESH_HAS_TRANSPARENCY;
            return;
        }
    }
}


mesh_tree_tag_p SkeletonClone(mesh_tree_tag_p src, int tags_count)
{
    mesh_tree_tag_p ret = (mesh_tree_tag_p)malloc(tags_count * sizeof(mesh_tree_tag_t));

    for(int i=0;i<tags_count;i++)
    {
        ret[i].mesh_base = src[i].mesh_base;
        ret[i].mesh_skin = src[i].mesh_skin;
        ret[i].flag = src[i].flag;
        vec3_copy(ret[i].offset, src[i].offset);
        ret[i].replace_anim = src[i].replace_anim;
        ret[i].replace_mesh = src[i].replace_mesh;
    }
    return ret;
}

void SkeletonCopyMeshes(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count)
{
    for(int i=0;i<tags_count;i++)
    {
        dst[i].mesh_base = src[i].mesh_base;
    }
}

void SkeletonCopyMeshes2(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count)
{
    for(int i=0;i<tags_count;i++)
    {
        dst[i].mesh_skin = src[i].mesh_base;
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
    base_mesh_p mesh_base, mesh_skin;
    mesh_tree_tag_p tree_tag, prev_tree_tag;

    tree_tag = model->mesh_tree;
    for(uint16_t i=0;i<model->mesh_count;i++,tree_tag++)
    {
        mesh_base = tree_tag->mesh_base;
        mesh_skin = tree_tag->mesh_skin;

        if(!mesh_skin)
        {
            return;
        }

        ch = mesh_skin->matrix_indices = (int8_t*)malloc(mesh_skin->vertex_count * sizeof(int8_t [2]));
        v = mesh_skin->vertices;
        for(uint32_t k=0;k<mesh_skin->vertex_count;k++,v++,ch += 2)
        {
            rv = FindVertexInMesh(mesh_base, v->position);
            if(rv != NULL)
            {
                ch[0] = 0;
                ch[1] = 0;
                vec3_copy(v->position, rv->position);
                vec3_copy(v->normal, rv->normal);
            }
            else
            {
                ch[0] = 0;
                ch[1] = 1;
                vec3_add(tv, v->position, tree_tag->offset);
                prev_tree_tag = model->mesh_tree;
                for(uint16_t l=0;l<model->mesh_count;l++,prev_tree_tag++)
                {
                    rv = FindVertexInMesh(prev_tree_tag->mesh_base, tv);
                    if(rv != NULL)
                    {
                        ch[0] = 1;
                        ch[1] = 1;
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
    vertex_p v = mesh->vertices;
    uint32_t ind = 0;

    for(ind=0;ind<mesh->vertex_count;ind++,v++)
    {
        if(v->position[0] == vertex->position[0] && v->position[1] == vertex->position[1] && v->position[2] == vertex->position[2] &&
           v->tex_coord[0] == vertex->tex_coord[0] && v->tex_coord[1] == vertex->tex_coord[1])
            ///@QUESTION: color check?
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

uint32_t Mesh_AddAnimatedVertex(base_mesh_p mesh, struct vertex_s *vertex)
{
    animated_vertex_p v = mesh->animated_vertices;
    uint32_t ind = 0;

    // Skip search for equal vertex; tex coords may differ but aren't stored in
    // animated_vertex_s

    ind = mesh->animated_vertex_count;                                                   // paranoid
    mesh->animated_vertex_count++;
    mesh->animated_vertices = (animated_vertex_p)realloc(mesh->animated_vertices, mesh->animated_vertex_count * sizeof(animated_vertex_t));

    v = mesh->animated_vertices + ind;
    vec3_copy(v->position, vertex->position);
    vec3_copy(v->normal, vertex->normal);
    vec4_copy(v->color, vertex->color);

    return ind;
}

void Mesh_GenFaces(base_mesh_p mesh)
{
    mesh->element_count_per_texture = (uint32_t *)calloc(sizeof(uint32_t), mesh->num_texture_pages);

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
    mesh->alpha_elements = 0;
    size_t numNormalElements = 0;
    mesh->animated_vertex_count = 0;
    mesh->num_animated_elements = 0;
    mesh->num_alpha_animated_elements = 0;
    for (uint32_t i = 0; i < mesh->polygons_count; i++)
    {
        if (Polygon_IsBroken(&mesh->polygons[i]))
            continue;

        uint32_t elementCount = (mesh->polygons[i].vertex_count - 2) * 3;
        if (mesh->polygons[i].double_side) elementCount *= 2;

        if (mesh->polygons[i].anim_id == 0)
        {
            if (mesh->polygons[i].transparency < 2)
            {
                mesh->element_count_per_texture[mesh->polygons[i].tex_index] += elementCount;
                numNormalElements += elementCount;
            }
            else
            {
                mesh->alpha_elements += elementCount;
                mesh->transparent_polygon_count += 1;
            }
        }
        else
        {
            if (mesh->polygons[i].transparency < 2)
                mesh->num_animated_elements += elementCount;
            else
            {
                mesh->num_alpha_animated_elements += elementCount;
                mesh->transparent_polygon_count += 1;
            }
        }
    }

    mesh->elements = (uint32_t *) calloc(sizeof(uint32_t), numNormalElements + mesh->alpha_elements);
    uint32_t elementOffset = 0;
    uint32_t *startPerTexture = (uint32_t *) calloc(sizeof(uint32_t), mesh->num_texture_pages);
    for (uint32_t i = 0; i < mesh->num_texture_pages; i++)
    {
        startPerTexture[i] = elementOffset;
        elementOffset += mesh->element_count_per_texture[i];
    }
    uint32_t startTransparent = elementOffset;

    mesh->animated_elements = (uint32_t *) calloc(sizeof(uint32_t), mesh->num_animated_elements + mesh->num_alpha_animated_elements);
    uint32_t animatedStart = 0;
    uint32_t animatedStartTransparent = mesh->num_animated_elements;

    mesh->transparent_polygons = (transparent_polygon_reference_s *) calloc(sizeof(transparent_polygon_reference_t), mesh->transparent_polygon_count);
    uint32_t transparentPolygonStart = 0;

    polygon_p p = mesh->polygons;
    for(uint32_t i=0;i<mesh->polygons_count;i++,p++)
    {
        if (Polygon_IsBroken(p)) continue;

        uint32_t elementCount = (p->vertex_count - 2) * 3;
        uint32_t backwardsStartOffset = elementCount;
        if (p->double_side)
        {
            elementCount *= 2;
        }

        if(p->anim_id == 0)
        {
            // Not animated
            uint32_t texture = p->tex_index;

            uint32_t oldStart;
            if (p->transparency < 2)
            {
                oldStart = startPerTexture[texture];
                startPerTexture[texture] += elementCount;
            }
            else
            {
                oldStart = startTransparent;
                startTransparent += elementCount;
                mesh->transparent_polygons[transparentPolygonStart].firstIndex = oldStart;
                mesh->transparent_polygons[transparentPolygonStart].count = elementCount;
                mesh->transparent_polygons[transparentPolygonStart].polygon = p;
                mesh->transparent_polygons[transparentPolygonStart].isAnimated = false;
                transparentPolygonStart += 1;
            }
            uint32_t backwardsStart = oldStart + backwardsStartOffset;

            // Render the polygon as a triangle fan. That is obviously correct for
            // a triangle and also correct for any quad.
            uint32_t startElement = Mesh_AddVertex(mesh, p->vertices);
            uint32_t previousElement = Mesh_AddVertex(mesh, p->vertices + 1);

            for(uint16_t j = 2; j < p->vertex_count; j++)
            {
                uint32_t thisElement = Mesh_AddVertex(mesh, p->vertices + j);

                mesh->elements[oldStart + (j - 2)*3 + 0] = startElement;
                mesh->elements[oldStart + (j - 2)*3 + 1] = previousElement;
                mesh->elements[oldStart + (j - 2)*3 + 2] = thisElement;

                if (p->double_side)
                {
                    mesh->elements[backwardsStart + (j - 2)*3 + 0] = startElement;
                    mesh->elements[backwardsStart + (j - 2)*3 + 1] = thisElement;
                    mesh->elements[backwardsStart + (j - 2)*3 + 2] = previousElement;
                }

                previousElement = thisElement;
            }
        }
        else
        {
            // Animated
            uint32_t oldStart;
            if (p->transparency < 2)
            {
                oldStart = animatedStart;
                animatedStart += elementCount;
            }
            else
            {
                oldStart = animatedStartTransparent;
                animatedStartTransparent += elementCount;
                mesh->transparent_polygons[transparentPolygonStart].firstIndex = oldStart;
                mesh->transparent_polygons[transparentPolygonStart].count = elementCount;
                mesh->transparent_polygons[transparentPolygonStart].polygon = p;
                mesh->transparent_polygons[transparentPolygonStart].isAnimated = true;
                transparentPolygonStart += 1;
            }
            uint32_t backwardsStart = oldStart + backwardsStartOffset;

            // Render the polygon as a triangle fan. That is obviously correct for
            // a triangle and also correct for any quad.
            uint32_t startElement = Mesh_AddAnimatedVertex(mesh, p->vertices);
            uint32_t previousElement = Mesh_AddAnimatedVertex(mesh, p->vertices + 1);

            for(uint16_t j = 2; j < p->vertex_count; j++)
            {
                uint32_t thisElement = Mesh_AddAnimatedVertex(mesh, p->vertices + j);

                mesh->animated_elements[oldStart + (j - 2)*3 + 0] = startElement;
                mesh->animated_elements[oldStart + (j - 2)*3 + 1] = previousElement;
                mesh->animated_elements[oldStart + (j - 2)*3 + 2] = thisElement;

                if (p->double_side)
                {
                    mesh->animated_elements[backwardsStart + (j - 2)*3 + 0] = startElement;
                    mesh->animated_elements[backwardsStart + (j - 2)*3 + 1] = thisElement;
                    mesh->animated_elements[backwardsStart + (j - 2)*3 + 2] = previousElement;
                }

                previousElement = thisElement;
            }
        }
    }
    free(startPerTexture);

    // Now same for animated triangles
}


btCollisionShape *BT_CSfromBBox(btScalar *bb_min, btScalar *bb_max, bool useCompression, bool buildBvh, bool is_static)
{
    obb_p obb = OBB_Create();
    polygon_p p = obb->base_polygons;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btVector3 v0, v1, v2;
    btCollisionShape* ret;
    int cnt = 0;

    OBB_Rebuild(obb, bb_min, bb_max);
    for(uint16_t i=0;i<6;i++,p++)
    {
        if(Polygon_IsBroken(p))
        {
            continue;
        }
        for(uint16_t j=1;j+1<p->vertex_count;j++)
        {
            vec3_copy(v0.m_floats, p->vertices[j + 1].position);
            vec3_copy(v1.m_floats, p->vertices[j].position);
            vec3_copy(v2.m_floats, p->vertices[0].position);
            trimesh->addTriangle(v0, v1, v2, true);
        }
        cnt ++;
    }

    if(cnt == 0)                                                        // fixed: without that condition engine may easily crash
    {
        delete trimesh;
        return NULL;
    }

    OBB_Clear(obb);
    free(obb);

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


btCollisionShape *BT_CSfromMesh(struct base_mesh_s *mesh, bool useCompression, bool buildBvh, bool is_static)
{
    uint32_t cnt = 0;
    polygon_p p;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;
    btVector3 v0, v1, v2;

    p = mesh->polygons;
    for(uint32_t i=0;i<mesh->polygons_count;i++,p++)
    {
        if(Polygon_IsBroken(p))
        {
            continue;
        }

        for(uint16_t j=1;j+1<p->vertex_count;j++)
        {
            vec3_copy(v0.m_floats, p->vertices[j + 1].position);
            vec3_copy(v1.m_floats, p->vertices[j].position);
            vec3_copy(v2.m_floats, p->vertices[0].position);
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
btCollisionShape *BT_CSfromHeightmap(struct room_sector_s *heightmap, struct sector_tween_s *tweens, int tweens_size, bool useCompression, bool buildBvh)
{
    uint32_t cnt = 0;
    room_p r = heightmap->owner_room;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;

    for(uint32_t i = 0; i < r->sectors_count; i++)
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

