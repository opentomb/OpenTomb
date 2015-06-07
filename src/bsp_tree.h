#ifndef BSP_TREE_H
#define BSP_TREE_H

#include <string.h>
#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include "bullet/LinearMath/btScalar.h"

struct polygon_s;

typedef struct bsp_face_ref_s {
    struct bsp_face_ref_s *next;
    btScalar transform[16];
    const struct transparent_polygon_reference_s *const polygon;
    
    bsp_face_ref_s(const btScalar matrix[16], const struct transparent_polygon_reference_s *polygon)
    : next(0), polygon(polygon)
    {
        memcpy(transform, matrix, sizeof(transform));
    }
} bsp_face_ref_t, *bsp_face_ref_p;

typedef struct bsp_node_s
{
    btScalar            plane[4];
    
    struct bsp_face_ref_s   *polygons_front;
    struct bsp_face_ref_s   *polygons_back;
    
    struct bsp_node_s  *front;
    struct bsp_node_s  *back;
} bsp_node_t, *bsp_node_p;

/**
 * Warning! that class has too primitive and rough (but fast) memory allocation space check! Maybe I will fix it in future; 
 */
class dynamicBSP
{
    void                *m_data;
    uint32_t             m_data_size;
    uint32_t             m_allocated;
    
    struct bsp_node_s *createBSPNode();
    struct bsp_face_ref_s *createFace(const btScalar transform[16], const struct transparent_polygon_reference_s *polygon);
    struct polygon_s  *createPolygon(uint16_t vertex_count);
    void addPolygon(struct bsp_node_s *root, struct bsp_face_ref_s *const p, struct polygon_s *transformed);
    
public:
    struct bsp_node_s   *m_root;
    
    dynamicBSP(uint32_t size);
   ~dynamicBSP();
    void addNewPolygonList(size_t count, const struct transparent_polygon_reference_s *p, const btScalar *transform);
    void reset()
    {
        m_allocated = 0;
        m_root = this->createBSPNode();
    }
};


#endif