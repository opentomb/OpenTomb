
#ifndef POLYGON_H
#define POLYGON_H

#include <cstdint>
#include <vector>
#include <array>

#include <GL/glew.h>

#include "bullet/LinearMath/btScalar.h"
#include "vmath.h"

#define SPLIT_FRONT    0x00
#define SPLIT_BACK     0x01
#define SPLIT_IN_PLANE 0x02
#define SPLIT_IN_BOTH  0x03

#define SPLIT_EPSILON (0.02)

/*
 * The structure taken from Cochrane. Next I realise one in my style.
 * make it aligned... is it enough good?
 */
struct Vertex
{
    btVector3 position;
    btVector3 normal;
    std::array<GLfloat,4> color;
    std::array<GLfloat,2> tex_coord;
};


struct Polygon
{
    std::vector<Vertex> vertices{4};                                               // vertices data
    uint16_t            tex_index;                                              // texture index
    uint16_t            anim_id;                                                // anim texture ID
    uint16_t            frame_offset;                                           // anim texture frame offset
    uint16_t            transparency;                                           // transparency information
    bool                double_side;                                            // double side flag
    btVector3 plane;                                               // polygon plane equation
    
    Polygon() = default;

    Polygon(const Polygon& rhs)
        : vertices(rhs.vertices)
        , tex_index(rhs.tex_index)
        , anim_id(rhs.anim_id)
        , frame_offset(rhs.frame_offset)
        , transparency(rhs.transparency)
        , double_side(rhs.double_side)
        , plane(rhs.plane)
    {
    }

    Polygon& operator=(const Polygon& rhs)
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

    bool isBroken() const;

    void moveSelf(const btVector3 &move);
    void move(Polygon* src, const btVector3 &move);
    void vTransform(Polygon* src, const btTransform &tr);
    void transform(const Polygon &src, const btTransform &tr);
    void transformSelf(const btTransform &tr);

    void findNormal();
    int  rayIntersect(const btVector3 &dir, const btVector3 &dot, btScalar *t) const;
    bool intersectPolygon(Polygon* p2);

    int  splitClassify(const btVector3 &n);
    void split(const btVector3 &n, Polygon* front, Polygon* back);

    bool isInsideBBox(const btVector3 &bb_min, const btVector3 &bb_max);
    bool isInsideBQuad(const btVector3 &bb_min, const btVector3 &bb_max);
};

/*
 * polygons functions
 */

#endif
