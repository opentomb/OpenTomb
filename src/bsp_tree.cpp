#include <cassert>
#include <cstdint>

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <bullet/LinearMath/btScalar.h>
#include "polygon.h"
#include "bsp_tree.h"
#include "vmath.h"
#include "mesh.h"
#include "frustum.h"

void DynamicBSP::addPolygon(const std::unique_ptr<BSPNode>& root, struct BSPFaceRef *const face, struct polygon_s *transformed)
{
    if(root->polygons_front == NULL)
    {
        // we though root->front == NULL and root->back == NULL
        root->plane = transformed->plane;
        root->polygons_front = face;
        return;
    }

    size_t positive = 0;
    size_t negative = 0;
    size_t in_plane = 0;
    for(const vertex_s& v : transformed->vertices)
    {
        const auto dist = planeDist(root->plane, v.position);
        if (dist > SPLIT_EPSILON)
            positive++;
        else if (dist < -SPLIT_EPSILON)
            negative++;
        else
            in_plane++;
    }

    if(positive > 0 && negative == 0)                   // SPLIT_FRONT
    {
        if(!root->front)
        {
            root->front.reset( new BSPNode() );
        }
        this->addPolygon(root->front, face, transformed);
    }
    else if((positive == 0) && (negative > 0))              // SPLIT_BACK
    {
        if(!root->back)
        {
            root->back.reset( new BSPNode() );
        }
        this->addPolygon(root->back, face, transformed);
    }
    else //((positive == 0) && (negative == 0))             // SPLIT_IN_PLANE
    {
        if(transformed->plane.dot(root->plane) > 0.9)
        {
            face->next = root->polygons_front;
            root->polygons_front = face;
        }
        else
        {
            face->next = root->polygons_back;
            root->polygons_back = face;
        }
    }
}


void DynamicBSP::addNewPolygonList(size_t count, const struct transparent_polygon_reference_s *p, const btTransform& transform, struct frustum_s *f)
{
    polygon_s transformed;
    for(size_t i = 0; i < count; i++)
    {
        bool visible = (f == nullptr);

        transformed.vertices.resize( p[i].polygon->vertices.size() );
        Polygon_Transform(&transformed, p[i].polygon, transform);
        transformed.double_side = p[i].polygon->double_side;

        for(frustum_p ff=f;(!visible)&&(ff!=NULL);ff=ff->next)
        {
            if(Frustum_IsPolyVisible(&transformed, ff))
            {
                visible = true;
                break;
            }
        }

        if(visible)
        {
            BSPFaceRef *face = new BSPFaceRef(transform, &p[i]);
            this->addPolygon(m_root, face, &transformed);
        }
    }
}

