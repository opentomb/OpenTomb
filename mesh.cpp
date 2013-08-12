
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
    
    if(mesh->polygons)
    {
        for(i=0;i<mesh->poly_count;i++)
        {
            Polygon_Clear(mesh->polygons+i);
        }
        free(mesh->polygons);
        mesh->polygons = NULL;
        mesh->poly_count = 0;
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

    /*if(mesh->face_count)
    {
        for(i=0;i<mesh->face_count;i++)
        {
            Face_Clear(mesh->faces + i);
        }
        free(mesh->faces);
        mesh->faces = NULL;
        mesh->face_count = 0;
    }*/
      
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
 * Поиск ограничивающего объема для меша
 */
void BaseMesh_FindBB(base_mesh_p mesh)
{
    vertex_p v;
    int i;

    v = mesh->vertices;
    vec3_copy(mesh->bb_min, v->position);
    vec3_copy(mesh->bb_max, v->position);
    v ++;
    for(i=1;i<mesh->vertex_count;i++,v++)
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


void Mesh_GenVBO(struct base_mesh_s *mesh)
{      
    mesh->vbo_vertex_array = 0;
    mesh->vbo_index_array = 0;
    if(!glGenBuffersARB)                                                        // if not supported, pointer is NULL
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
    for (GLsizeiptr i = 0; i < mesh->num_texture_pages; i++)
    {
        elementsSize += sizeof(uint32_t) * mesh->element_count_per_texture[i];
    }
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, elementsSize, mesh->elements, GL_STATIC_DRAW_ARB);

    glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}


void Mesh_DefaultColor(struct base_mesh_s *mesh)
{
    long int i, j;
    polygon_p p;
    vertex_p v;
    
    p = mesh->polygons;
    for(i=0;i<mesh->poly_count;i++,p++)
    {
        v = p->vertices;
        for(j=0;j<p->vertex_count;j++,v++)
        {
            vec4_copy(v->color, v->base_color);
        }
    }
}

/**
 * Mesh color mult
 */
void Mesh_MullColors(struct base_mesh_s *mesh, float *cl_mult)
{
    long int i, j;
    polygon_p p;
    vertex_p v;

    p = mesh->polygons;
    for(i=0;i<mesh->poly_count;i++,p++)
    {
        v = p->vertices;
        for(j=0;j<p->vertex_count;j++,v++)
        {
            v->color[0] = v->base_color[0] * cl_mult[0];
            v->color[1] = v->base_color[1] * cl_mult[1];
            v->color[2] = v->base_color[2] * cl_mult[2];
            v->color[3] = v->base_color[3] * cl_mult[3];
        }
    }
}


void SkeletalModel_Clear(skeletal_model_p model)
{
    int i, j;
    animation_frame_p anim;

    if(model)
    {
        if(model->mesh_count)
        {
            free(model->mesh_tree);
            model->mesh_tree = NULL;
            model->mesh_count = 0;
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
                        free(anim->state_change[j].anim_dispath);
                        anim->state_change[j].anim_dispath = NULL;
                        anim->state_change[j].ID = 0;
                        anim->state_change[j].anim_dispath_count = 0;
                    }
                    anim->state_change_count = 0;
                    free(anim->state_change);
                    anim->state_change = NULL;
                }
            }
            free(model->animations);
            model->animations = NULL;
            model->animation_count= 0;
        }

        if(model->all_frames_count)
        {
            free(model->all_bone_frames);
            model->all_bone_frames = NULL;

            free(model->all_bone_tags);
            model->all_bone_tags = NULL;

            model->all_frames_count = 0;
        }
    }
}


void SkeletalModel_FillRotations(skeletal_model_p model)
{
    int i, j;
    bone_frame_p bf;
    bone_tag_p btag;
    btScalar angle, sin_t2, cos_t2, qt[4], qX[4], qY[4], qZ[4];
    
    bf = model->all_bone_frames;
    for(i=0;i<model->all_frames_count;i++,bf++)
    {
        btag = bf->bone_tags;
        for(j=0;j<bf->bone_tag_count;j++,btag++)
        {
            // OZ    Mat4_RotateZ(btag->transform, btag->rotate[2]);
            angle = M_PI * btag->rotate[2] / 360.0;
            sin_t2 = sin(angle);
            cos_t2 = cos(angle);
            
            qZ[3] = cos_t2;
            qZ[0] = 0.0 * sin_t2;
            qZ[1] = 0.0 * sin_t2;
            qZ[2] = 1.0 * sin_t2;
            
            // OX    Mat4_RotateX(btag->transform, btag->rotate[0]);
            angle = M_PI * btag->rotate[0] / 360.0;
            sin_t2 = sin(angle);
            cos_t2 = cos(angle);
            
            qX[3] = cos_t2;
            qX[0] = 1.0 * sin_t2;
            qX[1] = 0.0 * sin_t2;
            qX[2] = 0.0 * sin_t2;
            
            // OY    Mat4_RotateY(btag->transform, btag->rotate[1]);
            angle = M_PI * btag->rotate[1] / 360.0;
            sin_t2 = sin(angle);
            cos_t2 = cos(angle);
            
            qY[3] = cos_t2;
            qY[0] = 0.0 * sin_t2;
            qY[1] = 1.0 * sin_t2;
            qY[2] = 0.0 * sin_t2;
            
            vec4_mul(qt, qZ, qX); 
            vec4_mul(btag->qrotate, qt, qY); 
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
    vec3_copy(dst->pos,src->pos);

    for(i=0;i<dst->bone_tag_count;i++)
    {
        vec3_copy(dst->bone_tags[i].rotate, src->bone_tags[i].rotate);
    }
}

void SkeletonModelFillTransparancy(skeletal_model_p model)
{
    unsigned int i;

    model->transparancy_flags = model->mesh_tree[0].mesh->transparancy_flags;
    for(i=1;i<model->mesh_count;i++)
    {
        if((model->transparancy_flags != model->mesh_tree[i].mesh->transparancy_flags) ||
           (model->mesh_tree[i].mesh->transparancy_flags == MESH_PART_TRANSPERENCY))
        {
            model->transparancy_flags = MESH_PART_TRANSPERENCY;
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
    //Sys_DebugLog("d_log.txt", "tree_1:\n");
    for(i=0;i<tags_count;i++)
    {
        //Sys_DebugLog("d_log.txt", "id = %d\n", src[i].mesh->ID);
        dst[i].mesh = src[i].mesh;
    }
}

void SkeletonCopyMeshes2(mesh_tree_tag_p dst, mesh_tree_tag_p src, int tags_count)
{
    int i;
    //Sys_DebugLog("d_log.txt", "tree_2:\n");
    for(i=0;i<tags_count;i++)
    {
        //Sys_DebugLog("d_log.txt", "id = %d\n", src[i].mesh->ID);
        dst[i].mesh2 = src[i].mesh;
    }
}

vertex_p FindVertexInMesh(base_mesh_p mesh, btScalar v[3])
{
    int i;
    vertex_p mv = mesh->vertices;

    for(i=0;i<mesh->vertex_count;i++,mv++)
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
    int32_t i, k, l;
    int8_t *ch;
    btScalar tv[3];
    vertex_p v, rv;
    base_mesh_p mesh, mesh2;
    mesh_tree_tag_p tree_tag, prev_tree_tag;

    tree_tag = model->mesh_tree;
    for(i=0;i<model->mesh_count;i++,tree_tag++)
    {
        mesh = tree_tag->mesh;
        mesh2 = tree_tag->mesh2;

        ch = mesh2->skin_map = (int8_t*)malloc(mesh2->vertex_count * sizeof(int8_t));
        v = mesh2->vertices;
        for(k=0;k<mesh2->vertex_count;k++,v++,ch++)
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
                for(l=0;l<model->mesh_count;l++,prev_tree_tag++)
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
    vec4_copy(v->base_color, vertex->base_color);
    vec4_copy(v->color, vertex->base_color);
    v->tex_coord[0] = vertex->tex_coord[0];
    v->tex_coord[1] = vertex->tex_coord[1];
    
    return ind;
}


void Mesh_GenFaces(base_mesh_p mesh)
{
    int i, j;
    polygon_p p;

    // Note: This code relies on NULL being an all-zero value, which is true on
    // any reasonable system these days.
    if(mesh->element_count_per_texture == NULL)
    {
        mesh->element_count_per_texture = (uint32_t *)calloc(sizeof(uint32_t), mesh->num_texture_pages);
    }
    // First collect indices on a per-texture basis
    uint32_t **elements_for_texture = (uint32_t **)calloc(sizeof(uint32_t*), mesh->num_texture_pages);

    p = mesh->polygons;
    for(i=0;i<mesh->poly_count;i++,p++)
    {
        if(p->transparency < 2 && !Polygon_IsBroken(p))
        {
            uint32_t texture = p->tex_index;
            uint32_t oldStart = mesh->element_count_per_texture[texture];

            mesh->element_count_per_texture[texture] += (p->vertex_count - 2) * 3;
            elements_for_texture[texture] = (uint32_t *)realloc(elements_for_texture[texture], mesh->element_count_per_texture[texture] * sizeof(uint32_t));

            // Render the polygon as a triangle fan. That is obviously correct for
            // a triangle and also correct for any quad.
            uint32_t startElement = Mesh_AddVertex(mesh, p->vertices);
            uint32_t previousElement = Mesh_AddVertex(mesh, p->vertices + 1);

            for(j = 2; j < p->vertex_count; j++)
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
    for(i = 0; i < mesh->num_texture_pages; i++)
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

