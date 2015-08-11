
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "core/gl_util.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "bsp_tree.h"
#include "frustum.h"

#define NEED_REALLOC_TREE_BUFF        (1)
#define NEED_REALLOC_VERTEX_BUFF      (2)
#define NEED_REALLOC_TEMP_BUFF        (3)

struct bsp_node_s *CDynamicBSP::createBSPNode()
{
    bsp_node_p ret = (bsp_node_p)(m_tree_buffer + m_tree_allocated);
    m_tree_allocated += sizeof(bsp_node_t);
    ret->front = NULL;
    ret->back = NULL;
    ret->polygons_front = NULL;
    ret->polygons_back = NULL;
    return ret;
}


struct polygon_s *CDynamicBSP::createPolygon(uint16_t vertex_count)
{
    polygon_p ret = (polygon_p)(m_temp_buffer + m_temp_allocated);
    m_temp_allocated += sizeof(polygon_t);
    ret->next = NULL;
    ret->vertex_count = vertex_count;
    ret->vertices = (vertex_p)(m_temp_buffer + m_temp_allocated);
    m_temp_allocated += vertex_count * sizeof(vertex_t);
    return ret;
}


void CDynamicBSP::addBSPPolygon(struct bsp_node_s *leaf, struct polygon_s *p)
{
    if(m_realloc_state || (m_vertex_allocated + p->vertex_count >= m_vertex_buffer_size))
    {
        m_realloc_state = NEED_REALLOC_VERTEX_BUFF;
        return;
    }

    bsp_polygon_p bp = (bsp_polygon_p)(m_tree_buffer + m_tree_allocated);
    m_tree_allocated  += sizeof(bsp_polygon_t);
    bp->tex_index      = p->tex_index;
    bp->transparency   = p->transparency;
    bp->vertex_count   = p->vertex_count;

    bp->indexes        = (GLuint*)(m_tree_buffer + m_tree_allocated);
    m_tree_allocated  += p->vertex_count * sizeof(GLint);
    GLuint *vi = bp->indexes;
    //vertex_p v = m_vertex_buffer + m_vertex_allocated;
    //vertex_p pv = p->vertices;
    memcpy(m_vertex_buffer + m_vertex_allocated, p->vertices, p->vertex_count * sizeof(vertex_t));
    for(uint16_t i=0;i<p->vertex_count;i++,vi++/*,v++,pv++*/)
    {
        *vi = m_vertex_allocated++;
        //*v  = *pv;
    }

    if(vec3_dot(p->plane, leaf->plane) > 0.9)
    {
        bp->next = leaf->polygons_front;
        leaf->polygons_front = bp;
    }
    else
    {
        bp->next = leaf->polygons_back;
        leaf->polygons_back = bp;
    }
}


void CDynamicBSP::addPolygon(struct bsp_node_s *root, struct polygon_s *p)
{
    if(m_realloc_state)
    {
        return;
    }

    if(m_tree_allocated + 1024 > m_tree_buffer_size)
    {
        m_realloc_state = NEED_REALLOC_TREE_BUFF;
        return;
    }

    if(m_temp_allocated + 1024 > m_temp_buffer_size)
    {
        m_realloc_state = NEED_REALLOC_TEMP_BUFF;
        return;
    }

    if(root->polygons_front == NULL)
    {
        // we though root->front == NULL and root->back == NULL
        vec4_copy(root->plane, p->plane);
        p->next = NULL;
        this->addBSPPolygon(root, p);
        return;
    }

    uint16_t positive = 0;
    uint16_t negative = 0;
    uint16_t in_plane = 0;
    float dist;
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

    if ((positive > 0) && (negative == 0))                  // SPLIT_FRONT
    {
        if (root->front == NULL)
        {
            root->front = this->createBSPNode();
        }
        this->addPolygon(root->front, p);
    }
    else if ((positive == 0) && (negative > 0))             // SPLIT_BACK
    {
        if (root->back == NULL)
        {
            root->back = this->createBSPNode();
        }
        this->addPolygon(root->back, p);
    }
    else if ((positive == 0) && (negative == 0))            // SPLIT_IN_PLANE
    {
        this->addBSPPolygon(root, p);
    }
    else                                                    // SPLIT_IN_BOTH
    {
        polygon_p front, back;
        front = this->createPolygon(p->vertex_count + 2);
        front->vertex_count = 0;
        back = this->createPolygon(p->vertex_count + 2);
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


CDynamicBSP::CDynamicBSP(uint32_t size)
{
    size = (size < 8192)?(8192):(size);

    m_temp_buffer = (uint8_t*)malloc(size);
    m_temp_buffer_size = size;
    m_temp_allocated = 0;

    m_tree_buffer = (uint8_t*)malloc(size);
    m_tree_buffer_size = size;
    m_tree_allocated = 0;

    size /= 64;
    m_vertex_buffer = (vertex_p)malloc(size * sizeof(vertex_t));
    m_vertex_buffer_size = size;
    m_vertex_allocated = 0;

    m_vbo = 0;
    m_anim_seq = NULL;
    m_realloc_state = 0;
    m_root = this->createBSPNode();
}


CDynamicBSP::~CDynamicBSP()
{
    if(m_vbo != 0)
    {
        glDeleteBuffersARB(1, &m_vbo);
        m_vbo = 0;
    }

    if(m_tree_buffer)
    {
        free(m_tree_buffer);
        m_tree_buffer = NULL;
    }
    m_tree_buffer_size = 0;

    if(m_temp_buffer)
    {
        free(m_temp_buffer);
        m_temp_buffer = NULL;
    }
    m_temp_buffer_size = 0;

    if(m_vertex_buffer)
    {
        free(m_vertex_buffer);
        m_vertex_buffer = NULL;
    }
    m_vertex_buffer_size = 0;

    m_realloc_state = 0;
    m_anim_seq = NULL;
    m_root = NULL;
}


void CDynamicBSP::addNewPolygonList(struct polygon_s *p, float *transform, struct frustum_s *f)
{
    for(;(p!=NULL)&&(!m_realloc_state);p=p->next)
    {
        m_temp_allocated = 0;
        polygon_p np = this->createPolygon(p->vertex_count);
        bool visible = (f == NULL);
        vertex_p src_v, dst_v;

        np->anim_id = p->anim_id;
        np->frame_offset = p->frame_offset;
        np->double_side  = p->double_side;

        np->transparency = p->transparency;

        Mat4_vec3_rot_macro(np->plane, transform, p->plane);
        for(uint16_t i=0;i<p->vertex_count;i++)
        {
            src_v = p->vertices + i;
            dst_v = np->vertices + i;
            Mat4_vec3_mul_macro(dst_v->position, transform, src_v->position);
        }
        np->plane[3] = -vec3_dot(np->plane, np->vertices[0].position);

        for(frustum_p ff=f;(!visible)&&(ff!=NULL);ff=ff->next)
        {
            if(Frustum_IsPolyVisible(np, ff))
            {
                visible = true;
                break;
            }
        }

        if(visible)
        {
            if(p->anim_id > 0)
            {
                anim_seq_p seq = m_anim_seq + p->anim_id - 1;
                uint16_t frame = (seq->current_frame + p->frame_offset) % seq->frames_count;
                tex_frame_p tf = seq->frames + frame;
                np->tex_index = tf->tex_ind;

                for(uint16_t i=0;i<p->vertex_count;i++)
                {
                    src_v = p->vertices + i;
                    dst_v = np->vertices + i;
                    Mat4_vec3_rot_macro(dst_v->normal, transform, src_v->normal);
                    vec4_copy(dst_v->color, src_v->color);
                    ApplyAnimTextureTransformation(dst_v->tex_coord, src_v->tex_coord, tf);
                }
            }
            else
            {
                np->tex_index = p->tex_index;
                for(uint16_t i=0;i<p->vertex_count;i++)
                {
                    src_v = p->vertices + i;
                    dst_v = np->vertices + i;
                    Mat4_vec3_rot_macro(dst_v->normal, transform, src_v->normal);
                    vec4_copy(dst_v->color, src_v->color);
                    dst_v->tex_coord[0] = src_v->tex_coord[0];
                    dst_v->tex_coord[1] = src_v->tex_coord[1];
                }
            }
            this->addPolygon(m_root, np);
        }
    }
}


void CDynamicBSP::reset(struct anim_seq_s *seq)
{
    if(m_vbo == 0)
    {
        glGenBuffersARB(1, &m_vbo);
    }

    switch(m_realloc_state)
    {
        case NEED_REALLOC_TREE_BUFF:
            {
                uint32_t new_buffer_size = m_tree_buffer_size * 1.5;
                uint8_t *new_buffer = (uint8_t*)malloc(new_buffer_size * sizeof(uint8_t));
                if(new_buffer != NULL)
                {
                    free(m_tree_buffer);
                    m_tree_buffer = new_buffer;
                    m_tree_buffer_size = new_buffer_size;
                }
            }
            break;

        case NEED_REALLOC_TEMP_BUFF:
            {
                uint32_t new_buffer_size = m_temp_buffer_size * 1.5;
                uint8_t *new_buffer = (uint8_t*)malloc(new_buffer_size * sizeof(uint8_t));
                if(new_buffer != NULL)
                {
                    free(m_temp_buffer);
                    m_temp_buffer = new_buffer;
                    m_temp_buffer_size = new_buffer_size;
                }
            }
            break;

        case NEED_REALLOC_VERTEX_BUFF:
            {
                uint32_t new_buffer_size = m_vertex_buffer_size * 1.5;
                vertex_p new_buffer = (vertex_p)malloc(new_buffer_size * sizeof(vertex_t));
                if(new_buffer != NULL)
                {
                    free(m_vertex_buffer);
                    m_vertex_buffer = new_buffer;
                    m_vertex_buffer_size = new_buffer_size;
                }
            }
            break;
    };

    m_anim_seq = seq;
    m_temp_allocated = 0;
    m_tree_allocated = 0;
    m_vertex_allocated = 0;
    m_realloc_state = 0;
    m_root = this->createBSPNode();
}
