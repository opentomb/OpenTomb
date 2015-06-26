
#ifndef POLYGON_H
#define POLYGON_H

#include <cstdint>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include "bullet/LinearMath/btScalar.h"
#include "vmath.h"
#include <vector>
#include <array>

#define SPLIT_FRONT    0x00
#define SPLIT_BACK     0x01
#define SPLIT_IN_PLANE 0x02
#define SPLIT_IN_BOTH  0x03

#define SPLIT_EPSILON (0.02)

/*
 * The structure taken from Cochrane. Next I realise one in my style.
 * make it aligned... is it enough good?
 */
typedef struct vertex_s
{
    btVector3 position;
    btVector3 normal;
    std::array<GLfloat,4> color;
    std::array<GLfloat,2> tex_coord;
} vertex_t, *vertex_p;


typedef struct polygon_s
{
    std::vector<vertex_s> vertices{4};                                               // vertices data
    uint16_t            tex_index;                                              // texture index
    uint16_t            anim_id;                                                // anim texture ID
    uint16_t            frame_offset;                                           // anim texture frame offset
    uint16_t            transparency;                                           // transparency information
    bool                double_side;                                            // double side flag
    btVector3 plane;                                               // polygon plane equation
    
    polygon_s() = default;

    polygon_s(const polygon_s& rhs)
        : vertices(rhs.vertices)
        , tex_index(rhs.tex_index)
        , anim_id(rhs.anim_id)
        , frame_offset(rhs.frame_offset)
        , transparency(rhs.transparency)
        , double_side(rhs.double_side)
        , plane(rhs.plane)
        , next(nullptr)
    {
    }

    polygon_s& operator=(const polygon_s& rhs)
    {
        vertices = rhs.vertices;
        tex_index = rhs.tex_index;
        anim_id = rhs.anim_id;
        frame_offset = rhs.frame_offset;
        transparency = rhs.transparency;
        double_side = rhs.double_side;
        plane = rhs.plane;
        // keep next
        return *this;
    }

    polygon_s *next = nullptr;                                                   // polygon list (for BSP using)
}polygon_t, *polygon_p;

/*
 * polygons functions
 */

bool Polygon_IsBroken(const polygon_s *p);

void Polygon_MoveSelf(polygon_p p, const btVector3 &move);
void Polygon_Move(polygon_p ret, polygon_p src, const btVector3 &move);
void Polygon_vTransform(polygon_p ret, polygon_p src, const btTransform &tr);
void Polygon_Transform(polygon_p ret, const polygon_t *src, const btTransform &tr);
void Polygon_TransformSelf(polygon_p p, const btTransform &tr);

void Polygon_FindNormale(polygon_p p);
int  Polygon_RayIntersect(const polygon_t *p, const btVector3 &dir, const btVector3 &dot, btScalar *t);
bool Polygon_IntersectPolygon(polygon_p p1, polygon_p p2);

int  Polygon_SplitClassify(polygon_p p, const btVector3 &n);
void Polygon_Split(polygon_p src, const btVector3 &n, polygon_p front, polygon_p back);

bool Polygon_IsInsideBBox(polygon_p p, const btVector3 &bb_min, const btVector3 &bb_max);
bool Polygon_IsInsideBQuad(polygon_p p, const btVector3 &bb_min, const btVector3 &bb_max);

#endif
