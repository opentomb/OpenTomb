#include <new>
#include <assert.h>
#include <stdint.h>

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "bullet/LinearMath/btScalar.h"

#include "polygon.h"
#include "bsp_tree.h"
#include "vmath.h"
#include "mesh.h"
#include "frustum.h"


struct bsp_node_s *dynamicBSP::createBSPNode()
{
    bsp_node_p ret = (bsp_node_p)((uint8_t*)m_data + m_allocated);
    m_allocated += sizeof(bsp_node_t);
    ret->front = NULL;
    ret->back = NULL;
    ret->polygons_front = NULL;
    ret->polygons_back = NULL;
    return ret;
}


struct bsp_face_ref_s *dynamicBSP::createFace(const btScalar transform[16], const struct transparent_polygon_reference_s *polygon)
{
    assert(polygon && polygon->polygon);
    uint8_t *start = ((uint8_t*)m_data + m_allocated);
    bsp_face_ref_s *ret = new (start) bsp_face_ref_s(transform, polygon);
    m_allocated += sizeof(bsp_face_ref_s);
    return ret;
}

void dynamicBSP::addPolygon(struct bsp_node_s *root, struct bsp_face_ref_s *const face, struct polygon_s *transformed)
{
    if(root->polygons_front == NULL)
    {
        // we though root->front == NULL and root->back == NULL
        vec4_copy(root->plane, transformed->plane);
        root->polygons_front = face;
        return;
    }

    uint16_t positive = 0;
    uint16_t negative = 0;
    uint16_t in_plane = 0;
    btScalar dist;
    vertex_p v = transformed->vertices;
    for(uint16_t i=0;i<transformed->vertex_count;i++,v++)
    {
        dist = vec3_plane_dist(root->plane, v->position);
        if (dist > SPLIT_EPSILON)
        {
            positive++;
        }
        else if (dist < -SPLIT_EPSILON)
        {
            negative++;
        }
        else
        {
            in_plane++;
        }
    }

    if((positive > 0) && (negative == 0))                   // SPLIT_FRONT
    {
        if(root->front == NULL)
        {
            root->front = this->createBSPNode();
        }
        this->addPolygon(root->front, face, transformed);
    }
    else if((positive == 0) && (negative > 0))              // SPLIT_BACK
    {
        if(root->back == NULL)
        {
            root->back = this->createBSPNode();
        }
        this->addPolygon(root->back, face, transformed);
    }
    else //((positive == 0) && (negative == 0))             // SPLIT_IN_PLANE
    {
        if(vec3_dot(transformed->plane, root->plane) > 0.9)
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


dynamicBSP::dynamicBSP(uint32_t size)
{
    m_data = malloc(size);
    m_data_size = size;
    m_allocated = 0;
    m_root = this->createBSPNode();
}


dynamicBSP::~dynamicBSP()
{
    if(m_data != NULL)
    {
        free(m_data);
        m_data = NULL;
    }
    m_root = NULL;
    m_allocated = 0;
    m_data_size = 0;
}


void dynamicBSP::addNewPolygonList(size_t count, const struct transparent_polygon_reference_s *p, const btScalar *transform, struct frustum_s *f)
{
    if(m_data_size - m_allocated < 1024)                                        ///@FIXME: magick 1024
    {
        return;
    }

    polygon_s *transformed = Polygon_CreateArray(1);
    for(size_t i = 0; i < count; i++)
    {
        uint32_t orig_allocated = m_allocated;
        bool visible = (f == NULL) || (f->active == 0);

        Polygon_Resize(transformed, p[i].polygon->vertex_count);
        Polygon_Transform(transformed, p[i].polygon, transform);
        transformed->double_side = p[i].polygon->double_side;
        
        for(frustum_p ff=f;(!visible)&&(ff!=NULL)&&(ff->active);ff=ff->next)
        {
            if(Frustum_IsPolyVisible(transformed, ff))
            {
                visible = true;
                break;
            }
        }

        if(visible)
        {
            bsp_face_ref_s *face = this->createFace(transform, &p[i]);
            this->addPolygon(m_root, face, transformed);
        }
        else
        {
            m_allocated = orig_allocated;
        }
    }
    Polygon_Clear(transformed);
}

