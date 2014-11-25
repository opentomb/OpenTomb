#ifndef BSP_TREE_H
#define BSP_TREE_H

#include <stdint.h>
#include <SDL2/SDL_opengl.h>
#include "bullet/LinearMath/btScalar.h"

struct polygon_s;

typedef struct bsp_node_s
{
    btScalar            plane[4];
    
    struct polygon_s   *polygons_front;
    struct polygon_s   *polygons_back;
    
    struct bsp_node_s  *front;
    struct bsp_node_s  *back;
} bsp_node_t, *bsp_node_p;

bsp_node_p BSP_CreateNode();
void BSP_AddPolygon(struct bsp_node_s *root, struct polygon_s *p);
void SBP_FreeTree(struct bsp_node_s *root);

#endif