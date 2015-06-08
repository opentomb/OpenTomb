#ifndef BSP_TREE_H
#define BSP_TREE_H

#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include "bullet/LinearMath/btScalar.h"

struct polygon_s;
struct frustum_s;

typedef struct bsp_node_s
{
    btScalar            plane[4];
    
    struct polygon_s   *polygons_front;
    struct polygon_s   *polygons_back;
    
    struct bsp_node_s  *front;
    struct bsp_node_s  *back;
} bsp_node_t, *bsp_node_p;

/**
 * Warning! that class has too primitive and rough (but fast) memory allocation space check! Maybe I will fix it in future; 
 */
class dynamicBSP
{
    uint8_t             *m_buffer;
    uint32_t             m_buffer_size;
    uint32_t             m_allocated;
    bool                 m_need_realloc;
    
    struct bsp_node_s *createBSPNode();
    struct polygon_s  *createPolygon(uint16_t vertex_count);
    void addPolygon(struct bsp_node_s *root, struct polygon_s *p);
    
public:
    struct bsp_node_s   *m_root;
    
    dynamicBSP(uint32_t size);
   ~dynamicBSP();
    void addNewPolygonList(struct polygon_s *p, btScalar *transform, struct frustum_s *f);
    void reset()
    {
        if(m_need_realloc)
        {
            uint32_t new_buffer_size = m_buffer_size * 1.5;
            uint8_t *new_buffer = (uint8_t*)realloc(m_buffer, new_buffer_size * sizeof(uint8_t));
            if(new_buffer != NULL)
            {
                m_buffer = new_buffer;
                m_buffer_size = new_buffer_size;
            }
            m_need_realloc = false;
        }
        m_allocated = 0;
        m_root = this->createBSPNode();
    }
};


#endif