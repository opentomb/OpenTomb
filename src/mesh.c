
#include <stdlib.h>

#include "core/gl_util.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "mesh.h"


void BaseMesh_AddPolygonToFaces(base_mesh_p mesh, struct polygon_s *p);
void BaseMesh_AddAnimatedPolygonToFaces(base_mesh_p mesh, struct vertex_s *vertex_data, uint32_t *vertex_index, struct polygon_s *p);

void BaseMesh_Clear(base_mesh_p mesh)
{
    if(qglIsBufferARB(mesh->vbo_vertex_array))
    {
        qglDeleteBuffersARB(1, &mesh->vbo_vertex_array);
        mesh->vbo_vertex_array = 0;
    }

    if(qglIsBufferARB(mesh->vbo_animated_vertex_array))
    {
        qglDeleteBuffersARB(1, &mesh->vbo_animated_vertex_array);
        mesh->vbo_animated_vertex_array = 0;
    }
    
    if(qglIsBufferARB(mesh->vbo_animated_texcoord_array))
    {
        qglDeleteBuffersARB(1, &mesh->vbo_animated_texcoord_array);
        mesh->vbo_animated_texcoord_array = 0;
    }

    if(mesh->polygons != NULL)
    {
        for(uint32_t i = 0; i < mesh->polygons_count; i++)
        {
            Polygon_Clear(mesh->polygons + i);
        }
        free(mesh->polygons);
        mesh->polygons = NULL;
        mesh->polygons_count = 0;
    }

    if(mesh->transparency_polygons != NULL)
    {
        polygon_p p = mesh->transparency_polygons;
        for(polygon_p next = p->next; p;)
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
        for(polygon_p next = p->next; p;)
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

    if(mesh->faces)
    {
        for(uint32_t i = 0; i < mesh->faces_count; i++)
        {
            if(mesh->faces[i].elements)
            {
                free(mesh->faces[i].elements);
                mesh->faces[i].elements = NULL;
            }
            mesh->faces[i].elements_count = 0;
        }
        free(mesh->faces);
        mesh->faces = NULL;
        mesh->faces_count = 0;
    }

    if(mesh->animated_faces)
    {
        for(uint32_t i = 0; i < mesh->animated_faces_count; i++)
        {
            if(mesh->animated_faces[i].elements)
            {
                free(mesh->animated_faces[i].elements);
                mesh->animated_faces[i].elements = NULL;
            }
            mesh->animated_faces[i].elements_count = 0;
        }
        free(mesh->animated_faces);
        mesh->animated_faces = NULL;
        mesh->animated_faces_count = 0;
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
        for(uint32_t i = 1; i < mesh->vertex_count; i++, v++)
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


void BaseMesh_GenVBO(struct base_mesh_s *mesh)
{
    mesh->vbo_vertex_array = 0;
    mesh->vbo_animated_vertex_array = 0;
    mesh->vbo_animated_texcoord_array = 0;
    
    /// now, begin VBO filling!
    qglGenBuffersARB(1, &mesh->vbo_vertex_array);
    if(mesh->vbo_vertex_array == 0)
    {
        abort();
    }

    qglBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vbo_vertex_array);
    qglBufferDataARB(GL_ARRAY_BUFFER_ARB, mesh->vertex_count * sizeof(vertex_t), mesh->vertices, GL_STATIC_DRAW_ARB);

    // Now for animated polygons, if any
    mesh->animated_faces = NULL;
    mesh->animated_faces_count = 0;
    mesh->animated_vertex_count = 0;
    if(mesh->animated_polygons)
    {
        for (polygon_p p = mesh->animated_polygons; p != 0; p = p->next)
        {
            mesh->animated_vertex_count += p->vertex_count;
        }

        vertex_p vertex_data = (vertex_p)malloc(mesh->animated_vertex_count * sizeof(vertex_t));
        uint32_t vertex_index = 0;
        for (polygon_p p = mesh->animated_polygons; p != 0; p = p->next)
        {
            BaseMesh_AddAnimatedPolygonToFaces(mesh, vertex_data, &vertex_index, p);
        }
        
        // And upload.
        qglGenBuffersARB(1, &mesh->vbo_animated_vertex_array);
        qglBindBufferARB(GL_ARRAY_BUFFER, mesh->vbo_animated_vertex_array);
        qglBufferDataARB(GL_ARRAY_BUFFER, mesh->animated_vertex_count * sizeof(vertex_t), vertex_data, GL_STATIC_DRAW);
        free(vertex_data);

        // Prepare empty buffer for tex coords
        qglGenBuffersARB(1, &mesh->vbo_animated_texcoord_array);
        qglBindBufferARB(GL_ARRAY_BUFFER, mesh->vbo_animated_texcoord_array);
        qglBufferDataARB(GL_ARRAY_BUFFER, mesh->animated_vertex_count * sizeof(GLfloat [2]), 0, GL_STREAM_DRAW);
    }
    qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}


/*
 * FACES FUNCTIONS
 */
uint32_t BaseMesh_AddVertex(base_mesh_p mesh, struct vertex_s *vertex)
{
    vertex_p v = mesh->vertices;
    uint32_t vertex_index = 0;

    for(vertex_index = 0; vertex_index < mesh->vertex_count; vertex_index++, v++)
    {
        if(v->position[0] == vertex->position[0] && v->position[1] == vertex->position[1] && v->position[2] == vertex->position[2] &&
           v->tex_coord[0] == vertex->tex_coord[0] && v->tex_coord[1] == vertex->tex_coord[1])
            ///@QUESTION: color check?
        {
            return vertex_index;
        }
    }

    vertex_index = mesh->vertex_count;
    mesh->vertex_count++;
    mesh->vertices = (vertex_p)realloc(mesh->vertices, mesh->vertex_count * sizeof(vertex_t));

    v = mesh->vertices + vertex_index;
    vec3_copy(v->position, vertex->position);
    vec3_copy(v->normal, vertex->normal);
    vec4_copy(v->color, vertex->color);
    v->tex_coord[0] = vertex->tex_coord[0];
    v->tex_coord[1] = vertex->tex_coord[1];

    return vertex_index;
}


uint32_t BaseMesh_FindVertexIndex(base_mesh_p mesh, float v[3])
{
    vertex_p mv = mesh->vertices;
    for(uint32_t i = 0; i < mesh->vertex_count; i++, mv++)
    {
        if(vec3_dist_sq(v, mv->position) < 4.0)
        {
            return i;
        }
    }

    return 0xFFFFFFFF;
}


void BaseMesh_AddPolygonToFaces(base_mesh_p mesh, struct polygon_s *p)
{
    mesh_face_p current_face = NULL;
    uint32_t add_elements_count = (p->vertex_count - 2) * 3;
    GLuint *current_index;
    
    if (p->double_side)
    {
        add_elements_count *= 2;
    }
    
    for(uint32_t i = 0; i < mesh->faces_count; i++)
    {
        if(mesh->faces[i].texture_index == p->texture_index)
        {
            current_face = mesh->faces + i;
            break;
        }
    }
    
    if(current_face == NULL)
    {
        mesh->faces = (mesh_face_p)realloc(mesh->faces, (mesh->faces_count + 1) * sizeof(mesh_face_t));
        current_face = mesh->faces + mesh->faces_count;
        mesh->faces_count++;
        current_face->elements = NULL;
        current_face->elements_count = 0;
        current_face->texture_index = p->texture_index;
    }
    
    current_face->elements = (GLuint *)realloc(current_face->elements, (current_face->elements_count + add_elements_count) * sizeof(GLuint));
    current_index = current_face->elements + current_face->elements_count;
    current_face->elements_count += add_elements_count;

    // Render the face as a triangle array
    uint32_t startElement = BaseMesh_AddVertex(mesh, p->vertices);
    uint32_t previousElement = BaseMesh_AddVertex(mesh, p->vertices + 1);

    for(uint16_t j = 2; j < p->vertex_count; j++)
    {
        uint32_t thisElement = BaseMesh_AddVertex(mesh, p->vertices + j);

        *current_index++ = startElement;
        *current_index++ = previousElement;
        *current_index++ = thisElement;

        if (p->double_side)
        {
            *current_index++ = startElement;
            *current_index++ = thisElement;
            *current_index++ = previousElement;
        }

        previousElement = thisElement;
    }
}


void BaseMesh_AddAnimatedPolygonToFaces(base_mesh_p mesh, struct vertex_s *vertex_data, uint32_t *vertex_index, struct polygon_s *p)
{
    mesh_face_p current_face = NULL;
    uint32_t add_elements_count = (p->vertex_count - 2) * 3;
    GLuint *current_index;
    
    if (p->double_side)
    {
        add_elements_count *= 2;
    }
    
    for(uint32_t i = 0; i < mesh->animated_faces_count; i++)
    {
        if(mesh->animated_faces[i].texture_index == p->texture_index)
        {
            current_face = mesh->animated_faces + i;
            break;
        }
    }
    
    if(current_face == NULL)
    {
        mesh->animated_faces = (mesh_face_p)realloc(mesh->animated_faces, (mesh->animated_faces_count + 1) * sizeof(mesh_face_t));
        current_face = mesh->animated_faces + mesh->animated_faces_count;
        mesh->animated_faces_count++;
        current_face->elements = NULL;
        current_face->elements_count = 0;
        current_face->texture_index = p->texture_index;
    }
    
    current_face->elements = (GLuint *)realloc(current_face->elements, (current_face->elements_count + add_elements_count) * sizeof(GLuint));
    current_index = current_face->elements + current_face->elements_count;
    current_face->elements_count += add_elements_count;

    // Render the face as a triangle array
    uint32_t startElement = *vertex_index;
    vertex_data[*vertex_index] = p->vertices[0];
    (*vertex_index)++;
    
    uint32_t previousElement = *vertex_index;
    vertex_data[*vertex_index] = p->vertices[1];
    (*vertex_index)++;

    for(uint16_t j = 2; j < p->vertex_count; j++)
    {
        uint32_t thisElement = *vertex_index;
        vertex_data[*vertex_index] = p->vertices[j];
        (*vertex_index)++;
        
        *current_index++ = startElement;
        *current_index++ = previousElement;
        *current_index++ = thisElement;

        if (p->double_side)
        {
            *current_index++ = startElement;
            *current_index++ = thisElement;
            *current_index++ = previousElement;
        }

        previousElement = thisElement;
    }
}


void BaseMesh_GenFaces(base_mesh_p mesh)
{
    polygon_p p = mesh->polygons;
    mesh->faces_count = 0;
    mesh->faces = NULL;
    
    for(uint32_t i = 0; i < mesh->polygons_count; i++, p++)
    {
        if((p->transparency < 2) && (p->anim_id == 0) && !Polygon_IsBroken(p))
        {
            BaseMesh_AddPolygonToFaces(mesh, p);
        }
    }
}
