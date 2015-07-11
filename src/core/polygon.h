
#ifndef POLYGON_H
#define POLYGON_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "vmath.h"

#define SPLIT_FRONT    0x00
#define SPLIT_BACK     0x01
#define SPLIT_IN_PLANE 0x02
#define SPLIT_IN_BOTH  0x03

#define SPLIT_EPSILON (0.02)

#define Polygon_AddVertexMacro(p, v)\
{\
    *((p)->vertices + (p)->vertex_count) = *(v);\
    (p)->vertex_count++;\
}

/*
 * The structure taken from Cochrane. Next I realise one in my style.
 * make it aligned... is it enough good?
 */
typedef struct vertex_s
{
    btScalar        position[4];
    GLfloat         normal[4];
    GLfloat         color[4];
    GLfloat         tex_coord[2];
} vertex_t, *vertex_p;


typedef struct polygon_s
{
    uint16_t            vertex_count;                                           // number of vertices
    struct vertex_s    *vertices;                                               // vertices data
    uint16_t            tex_index;                                              // texture index
    uint16_t            anim_id;                                                // anim texture ID
    uint16_t            frame_offset;                                           // anim texture frame offset
    uint16_t            transparency;                                           // transparency information
    uint8_t             double_side;                                            // double side flag
    btScalar            plane[4];                                               // polygon plane equation
    
    struct polygon_s   *next;                                                   // polygon list (for BSP using)
}polygon_t, *polygon_p;

/*
 * polygons functions
 */
polygon_p Polygon_CreateArray(unsigned int pcount);

void Polygon_Resize(polygon_p p, unsigned int count);
void Polygon_Clear(polygon_p p);
int  Polygon_IsBroken(polygon_p p);
void Polygon_Copy(polygon_p dst, polygon_p src);

void Polygon_MoveSelf(polygon_p p, btScalar move[3]);
void Polygon_Move(polygon_p ret, polygon_p src, btScalar move[3]);
void Polygon_Transform(polygon_p ret, polygon_p src, btScalar tr[16]);
void Polygon_TransformSelf(polygon_p p, btScalar tr[16]);

void Polygon_FindNormale(polygon_p p);
int  Polygon_RayIntersect(polygon_p p, btScalar dir[3], btScalar dot[3], btScalar *t);
int  Polygon_IntersectPolygon(polygon_p p1, polygon_p p2);

int  Polygon_SplitClassify(polygon_p p, btScalar n[4]);
void Polygon_Split(polygon_p src, btScalar n[4], polygon_p front, polygon_p back);
void Polygon_AddVertex(polygon_p p, struct vertex_s *v);

int Polygon_IsInsideBBox(polygon_p p, btScalar bb_min[3], btScalar bb_max[3]);
int Polygon_IsInsideBQuad(polygon_p p, btScalar bb_min[3], btScalar bb_max[3]);

#ifdef	__cplusplus
}
#endif
#endif
