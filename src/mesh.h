
#ifndef MESH_H
#define MESH_H

#ifdef	__cplusplus
extern "C" {
#endif

#define MESH_FULL_OPAQUE      0x00  // Fully opaque object (all polygons are opaque: all t.flags < 0x02)
#define MESH_HAS_TRANSPARENCY 0x01  // Fully transparency or has transparency and opaque polygon / object

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <stdint.h>

struct polygon_s;
struct vertex_s;

/*
 * base mesh, uses everywhere
 */
typedef struct base_mesh_s
{
    uint32_t                id;                                                 // mesh's ID
    uint32_t                polygons_count;                                     // number of all mesh's polygons
    struct polygon_s       *polygons;                                           // polygons data

    struct polygon_s       *transparency_polygons;                              // transparency mesh's polygons list
    struct polygon_s       *animated_polygons;                                  // opaque animated mesh's polygons list

    uint32_t                num_texture_pages;                                  // face without structure wrapping
    uint32_t               *element_count_per_texture;                          //
    uint32_t               *elements;                                           //

    uint32_t                vertex_count;                                       // number of mesh's vertices
    struct vertex_s        *vertices;

    float                   centre[3];                                          // geometry centre of mesh
    float                   bb_min[3];                                          // AABB bounding volume
    float                   bb_max[3];                                          // AABB bounding volume
    float                   R;                                                  // radius of the bounding sphere
    uint32_t               *skin_map;                                           // vertices map for skin mesh

    GLuint                  vbo_vertex_array;
    GLuint                  vbo_index_array;
    
    // Buffers for animated polygons
    // The first contains position, normal and color.
    // The second contains the texture coordinates. It gets updated every frame.
    size_t                  num_animated_elements;
    GLuint                  animated_vertex_array;
    GLuint                  animated_texcoord_array;
    GLuint                  animated_index_array;
    size_t                  animated_index_array_length;
}base_mesh_t, *base_mesh_p;


/*
 * base sprite structure
 */
typedef struct sprite_s
{
    uint32_t            id;                                                     // object's ID
    uint32_t            texture;                                                // texture number
    GLfloat             tex_coord[8];                                           // texture coordinates
    uint32_t            flag;
    float               left;                                                   // world sprite's gabarites
    float               right;
    float               top;
    float               bottom;
}sprite_t, *sprite_p;

/*
 * Structure for all the sprites in a room
 */
typedef struct sprite_buffer_s
{
    // Vertex data for the sprites
    GLuint                array_buffer;
    // Element data for the sprites
    GLuint                element_array_buffer;
    
    // How many sub-ranges the element_array_buffer contains. It has one for each texture listed.
    uint32_t              num_texture_pages;
    // The element count for each sub-range.
    uint32_t             *element_count_per_texture;
}sprite_buffer_t, *sprite_buffer_p;

/*
 * lights
 */
enum LightType
{
    LT_NULL,
    LT_POINT,
    LT_SPOTLIGHT,
    LT_SUN,
    LT_SHADOW
};


typedef struct light_s
{
    float                       pos[4];                                         // world position
    float                       colour[4];                                      // RGBA value

    float                       inner;
    float                       outer;
    float                       length;
    float                       cutoff;

    float                       falloff;

    enum LightType              light_type;
}light_t, *light_p;

/*
 * Animated skeletal model. Taken from openraider.
 * model -> animation -> frame -> bone
 * thanks to Terry 'Mongoose' Hendrix II
 */
void BaseMesh_Clear(base_mesh_p mesh);
void BaseMesh_FindBB(base_mesh_p mesh);
void BaseMesh_GenVBO(struct base_mesh_s *mesh);

uint32_t BaseMesh_AddVertex(base_mesh_p mesh, struct vertex_s *vertex);
uint32_t BaseMesh_FindVertexIndex(base_mesh_p mesh, float v[3]);
void     BaseMesh_GenFaces(base_mesh_p mesh);


#ifdef	__cplusplus
}
#endif

#endif
