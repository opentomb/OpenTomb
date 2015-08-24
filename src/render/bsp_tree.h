#ifndef BSP_TREE_H
#define BSP_TREE_H

#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include "../core/vmath.h"

struct polygon_s;
struct frustum_s;
struct anim_seq_s;

typedef struct bsp_polygon_s 
{
    uint16_t                vertex_count;                                       // number of vertices
    GLuint                 *indexes;                                            // vertices indexes
    uint16_t                tex_index;                                          // texture index
    uint16_t                transparency;                                       // transparency information
    
    struct bsp_polygon_s   *next;                                               // polygon list (for BSP using)
} bsp_polygon_t, *bsp_polygon_p;


typedef struct bsp_node_s
{
    float                   plane[4];
    
    struct bsp_polygon_s   *polygons_front;
    struct bsp_polygon_s   *polygons_back;
    
    struct bsp_node_s      *front;
    struct bsp_node_s      *back;
} bsp_node_t, *bsp_node_p;


class CDynamicBSP
{
    uint8_t             *m_tree_buffer;
    uint32_t             m_tree_buffer_size;
    uint32_t             m_tree_allocated;
    
    uint8_t             *m_temp_buffer;
    uint32_t             m_temp_buffer_size;
    uint32_t             m_temp_allocated;
    
    struct vertex_s     *m_vertex_buffer;
    uint32_t             m_vertex_buffer_size;
    uint32_t             m_vertex_allocated;
    
    uint32_t             m_realloc_state;
    struct anim_seq_s   *m_anim_seq;
    
    struct bsp_node_s     *CreateBSPNode();
    struct polygon_s      *CreatePolygon(uint16_t vertex_count);
    void AddBSPPolygon(struct bsp_node_s *leaf, struct polygon_s *p);
    void AddPolygon(struct bsp_node_s *root, struct polygon_s *p);
    
public:
    struct bsp_node_s   *m_root;
    GLuint m_vbo;
    
    CDynamicBSP(uint32_t size);
   ~CDynamicBSP();
   
    void AddNewPolygonList(struct polygon_s *p, float *transform, struct frustum_s *f);
    void Reset(struct anim_seq_s *seq);
    
    struct vertex_s *GetVertexArray()
    {
        return m_vertex_buffer;
    }
    
    uint32_t GetActiveVertexCount()
    {
        return m_vertex_allocated;
    }
};


#endif