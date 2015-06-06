
#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include "bullet/LinearMath/btScalar.h"
#include "polygon.h"
#include "bsp_tree.h"
#include "vmath.h"
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


struct polygon_s *dynamicBSP::createPolygon(uint16_t vertex_count)
{
    polygon_p ret = (polygon_p)((uint8_t*)m_data + m_allocated);
    m_allocated += sizeof(polygon_t);
    ret->next = NULL;
    ret->vertex_count = vertex_count;
    ret->vertices = (vertex_p)((uint8_t*)m_data + m_allocated);
    m_allocated += vertex_count * sizeof(vertex_t);
    return ret;
}


void dynamicBSP::addPolygon(struct bsp_node_s *root, struct polygon_s *p)
{
    if(root->polygons_front == NULL)
    {
        // we though root->front == NULL and root->back == NULL
        vec4_copy(root->plane, p->plane);
        p->next = NULL;
        root->polygons_front = p;
        return;
    }

    uint16_t positive = 0;
    uint16_t negative = 0;
    uint16_t in_plane = 0;
    btScalar dist;
    vertex_p v = p->vertices;
    for(uint16_t i=0;i<p->vertex_count;i++,v++)
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
        this->addPolygon(root->front, p);
    }
    else if((positive == 0) && (negative > 0))              // SPLIT_BACK
    {
        if(root->back == NULL)
        {
            root->back = this->createBSPNode();
        }
        this->addPolygon(root->back, p);
    }
    else if((positive == 0) && (negative == 0))             // SPLIT_IN_PLANE
    {
        if(vec3_dot(p->plane, root->plane) > 0.9)
        {
            p->next = root->polygons_front;
            root->polygons_front = p;
        }
        else
        {
            p->next = root->polygons_back;
            root->polygons_back = p;
        }
    }
    else                                                    // SPLIT_IN_BOTH
    {
        polygon_p front, back;
        front = this->createPolygon(positive + in_plane + 2);
        front->vertex_count = 0;
        back = this->createPolygon(negative + in_plane + 2);
        back->vertex_count = 0;
        Polygon_Split(p, root->plane, front, back);

        if(root->front == NULL)
        {
            root->front = this->createBSPNode();
        }
        this->addPolygon(root->front, front);
        if(root->back == NULL)
        {
            root->back = this->createBSPNode();
        }
        this->addPolygon(root->back, back);
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


void dynamicBSP::addNewPolygon(struct polygon_s *p, btScalar *transform)
{
    polygon_p np = this->createPolygon(p->vertex_count);
    Polygon_Copy(np, p);
    Polygon_Transform(np, p, transform);
    if(m_root == NULL)
    {
        m_root = this->createBSPNode();
    }
    this->addPolygon(m_root, np);
}


void dynamicBSP::addNewPolygonList(struct polygon_s *p, btScalar *transform, struct frustum_s *f)
{
    if(m_data_size - m_allocated < 1024)                                        ///@FIXME: magick 1024
    {
        return;
    }

    for(;p!=NULL;p=p->next)
    {
        uint32_t orig_allocated = m_allocated;
        polygon_p np = this->createPolygon(p->vertex_count);
        bool visible = (f == NULL) || (f->active == 0);
        Polygon_Copy(np, p);
        Polygon_Transform(np, p, transform);

        for(frustum_p ff=f;(ff!=NULL)&&(ff->active);ff=ff->next)
        {
            if(Frustum_IsPolyVisible(np, ff))
            {
                visible = true;
                break;
            }
        }

        if(visible)
        {
            this->addPolygon(m_root, np);
        }
        else
        {
            m_allocated = orig_allocated;
        }
    }
}

