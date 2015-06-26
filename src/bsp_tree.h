#ifndef BSP_TREE_H
#define BSP_TREE_H

#include <cstring>
#include <cstdint>
#include <memory>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <bullet/LinearMath/btScalar.h>

#include "vmath.h"

struct polygon_s;
struct frustum_s;
struct transparent_polygon_reference_s;

struct BSPFaceRef {
    BSPFaceRef *next = nullptr;
    btTransform transform;
    const transparent_polygon_reference_s *const polygon;
    
    BSPFaceRef(const btTransform& matrix, const struct transparent_polygon_reference_s *polygon)
        : transform(matrix)
        , polygon(polygon)
    {
    }
};

struct BSPNode
{
    btVector3 plane{0,0,0};
    
    BSPFaceRef *polygons_front = nullptr;
    BSPFaceRef *polygons_back = nullptr;
    
    std::unique_ptr<BSPNode> front = nullptr;
    std::unique_ptr<BSPNode> back = nullptr;
};

/**
 * Warning! that class has too primitive and rough (but fast) memory allocation space check! Maybe I will fix it in future; 
 */
class DynamicBSP
{
private:
    std::unique_ptr<BSPNode> m_root{ new BSPNode() };

    void addPolygon(const std::unique_ptr<BSPNode> &root, BSPFaceRef *const p, polygon_s *transformed);
    
public:
    void addNewPolygonList(size_t count, const transparent_polygon_reference_s *p, const btTransform &transform, struct frustum_s *f);

    const std::unique_ptr<BSPNode>& root() const
    {
        return m_root;
    }

    void reset()
    {
        m_root.reset(new BSPNode());
    }
};


#endif
