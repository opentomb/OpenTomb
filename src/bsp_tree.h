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
struct Frustum;
struct TransparentPolygonReference;

struct BSPFaceRef {
    BSPFaceRef *next = nullptr;
    btTransform transform;
    const TransparentPolygonReference *const polygon;
    
    BSPFaceRef(const btTransform& matrix, const struct TransparentPolygonReference *polygon)
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
    void addNewPolygonList(const std::vector<TransparentPolygonReference> &p, const btTransform &transform, const std::vector<std::shared_ptr<Frustum> > &f);

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
