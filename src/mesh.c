
#include <stdlib.h>

//#include "core/system.h"
#include "core/gl_util.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "mesh.h"


vertex_p FindVertexInMesh(base_mesh_p mesh, float v[3]);

void BaseMesh_Clear(base_mesh_p mesh)
{
    if(mesh->vbo_vertex_array)
    {
        qglDeleteBuffersARB(1, &mesh->vbo_vertex_array);
        mesh->vbo_vertex_array = 0;
    }

    if(mesh->vbo_index_array)
    {
        qglDeleteBuffersARB(1, &mesh->vbo_index_array);
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
    if(qglGenBuffersARB == NULL)                                                // if not supported, pointer is NULL
    {
        abort();
    }

    /// now, begin VBO filling!
    qglGenBuffersARB(1, &mesh->vbo_vertex_array);
    if(mesh->vbo_vertex_array == 0)
    {
        abort();
    }

    qglBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vbo_vertex_array);
    qglBufferDataARB(GL_ARRAY_BUFFER_ARB, mesh->vertex_count * sizeof(vertex_t), mesh->vertices, GL_STATIC_DRAW_ARB);

    // Fill indexes vbo
    qglGenBuffersARB(1, &mesh->vbo_index_array);
    qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->vbo_index_array);

    GLsizeiptr elementsSize = 0;
    for (uint32_t i = 0; i < mesh->num_texture_pages; i++)
    {
        elementsSize += sizeof(uint32_t) * mesh->element_count_per_texture[i];
    }
    qglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, elementsSize, mesh->elements, GL_STATIC_DRAW_ARB);

    // Now for animated polygons, if any
    mesh->num_animated_elements = 0;
    mesh->animated_index_array_length = 0;
    if (mesh->animated_polygons != 0)
    {
        for (polygon_p p = mesh->animated_polygons; p != 0; p = p->next)
        {
            mesh->num_animated_elements += p->vertex_count;
            mesh->animated_index_array_length += 3*(p->vertex_count - 2);
            if (p->double_side)
            {
                mesh->animated_index_array_length += 3*(p->vertex_count - 2);
            }
        }

        // Prepare buffer data
        size_t stride = sizeof(GLfloat[3]) + sizeof(GLfloat [4]) + sizeof(GLfloat[3]);
        uint8_t *vertexData = (uint8_t*)malloc(mesh->num_animated_elements * stride * sizeof(uint8_t));
        uint32_t *elementData = (uint32_t*)malloc(mesh->animated_index_array_length * sizeof(uint32_t));

        // Fill it.
        size_t offset = 0;
        size_t elementOffset = 0;
        for (polygon_p p = mesh->animated_polygons; p != 0; p = p->next)
        {
            size_t begin = offset;
            for (int i = 0; i < p->vertex_count; i++)
            {
                memcpy(&vertexData[offset*stride + 0], p->vertices[i].position, sizeof(GLfloat [3]));
                memcpy(&vertexData[offset*stride + 12], p->vertices[i].color, sizeof(GLfloat [4]));
                memcpy(&vertexData[offset*stride + 28], p->vertices[i].normal, sizeof(GLfloat [3]));

                if (i >= 2)
                {
                    elementData[elementOffset+0] = begin;
                    elementData[elementOffset+1] = offset-1;
                    elementData[elementOffset+2] = offset;
                    elementOffset += 3;

                    if (p->double_side)
                    {
                        elementData[elementOffset+0] = offset;
                        elementData[elementOffset+1] = offset-1;
                        elementData[elementOffset+2] = begin;
                        elementOffset += 3;
                    }
                }
                offset++;
            }
        }

        // And upload.
        qglGenBuffersARB(1, &mesh->animated_vertex_array);
        qglBindBufferARB(GL_ARRAY_BUFFER, mesh->animated_vertex_array);
        qglBufferDataARB(GL_ARRAY_BUFFER, stride * mesh->num_animated_elements, vertexData, GL_STATIC_DRAW);
        free(vertexData);

        qglGenBuffersARB(1, &mesh->animated_index_array);
        qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, mesh->animated_index_array);
        qglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * mesh->animated_index_array_length, elementData, GL_STATIC_DRAW);
        free(elementData);

        // Prepare empty buffer for tex coords
        qglGenBuffersARB(1, &mesh->animated_texcoord_array);
        qglBindBufferARB(GL_ARRAY_BUFFER, mesh->animated_texcoord_array);
        qglBufferDataARB(GL_ARRAY_BUFFER, sizeof(GLfloat [2]) * mesh->num_animated_elements, 0, GL_STREAM_DRAW);
    }
    else
    {
        // No animated data
        mesh->animated_vertex_array = 0;
        mesh->animated_texcoord_array = 0;
    }

    qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
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
            uint32_t elementCount = (p->vertex_count - 2) * 3;
            uint32_t backwardsStart = oldStart + elementCount;
            if (p->double_side)
            {
                elementCount *= 2;
            }

            mesh->element_count_per_texture[texture] += elementCount;
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

                if (p->double_side)
                {
                    elements_for_texture[texture][backwardsStart + (j - 2)*3 + 0] = startElement;
                    elements_for_texture[texture][backwardsStart + (j - 2)*3 + 1] = thisElement;
                    elements_for_texture[texture][backwardsStart + (j - 2)*3 + 2] = previousElement;
                }

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
