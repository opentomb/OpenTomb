#ifndef BSP_TREE_H
#define BSP_TREE_H

#include <cstring>
#include <cstdint>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <bullet/LinearMath/btScalar.h>

#include "vmath.h"

struct polygon_s;
struct frustum_s;

typedef struct bsp_face_ref_s {
    struct bsp_face_ref_s *next;
    btTransform transform;
    const struct transparent_polygon_reference_s *const polygon;
    
    bsp_face_ref_s(const btTransform& matrix, const struct transparent_polygon_reference_s *polygon)
        : next(nullptr)
        , transform(matrix)
        , polygon(polygon)
    {
    }
} bsp_face_ref_t, *bsp_face_ref_p;

typedef struct bsp_node_s
{
    btVector3 plane;
    
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
    struct bsp_face_ref_s *createFace(const btTransform &transform, const struct transparent_polygon_reference_s *polygon);
    struct polygon_s  *createPolygon(uint16_t vertex_count);
    void addPolygon(struct bsp_node_s *root, struct bsp_face_ref_s *const p, struct polygon_s *transformed);
    
public:
    struct bsp_node_s   *m_root;
    
    dynamicBSP(uint32_t size);
   ~dynamicBSP();
    void addNewPolygonList(size_t count, const struct transparent_polygon_reference_s *p, const btTransform &transform, struct frustum_s *f);
    void reset()
    {
        m_allocated = 0;
        m_root = this->createBSPNode();
    }
};


#endif
