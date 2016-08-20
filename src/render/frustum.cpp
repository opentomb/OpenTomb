
#include <stdio.h>
#include <stdlib.h>

#include "../core/system.h"
#include "../core/vmath.h"
#include "../core/polygon.h"
#include "../core/obb.h"
#include "render.h"
#include "frustum.h"
#include "camera.h"
#include "../room.h"


#define SPLIT_EMPTY         (0x00)
#define SPLIT_SUCCES        (0x01)

CFrustumManager::CFrustumManager(uint32_t buffer_size)
{
    m_buffer_size = buffer_size;
    m_allocated = 0;
    m_buffer = (uint8_t*)malloc(buffer_size * sizeof(uint8_t));
    memset(m_buffer, 0, (buffer_size * sizeof(uint8_t)));
    m_need_realloc = false;
}

CFrustumManager::~CFrustumManager()
{
    if(m_buffer != NULL)
    {
        free(m_buffer);
        m_buffer = NULL;
    }
}

void CFrustumManager::Reset()
{
    m_allocated = 0;
    if(m_need_realloc)
    {
        uint32_t new_buffer_size = m_buffer_size * 1.5;
        uint8_t *new_buffer = (uint8_t*)malloc(new_buffer_size * sizeof(uint8_t));
        if(new_buffer != NULL)
        {
            free(m_buffer);
            m_buffer = new_buffer;
            m_buffer_size = new_buffer_size;
        }
        m_need_realloc = false;
    }
}

frustum_p CFrustumManager::CreateFrustum()
{
    if((!m_need_realloc) && (m_allocated + sizeof(frustum_t) < m_buffer_size))
    {
        frustum_p ret = (frustum_p)(m_buffer + m_allocated);
        m_allocated += sizeof(frustum_t);
        ret->vertex_count = 0;
        ret->parents_count = 0;
        ret->next = NULL;
        ret->parent = NULL;
        ret->planes = NULL;
        ret->vertex = NULL;
        ret->cam_pos = NULL;
        vec4_set_zero(ret->norm);
        return ret;
    }

    m_need_realloc = true;
    return NULL;
}

float *CFrustumManager::Alloc(uint32_t size)
{
    size *= sizeof(float);
    if((!m_need_realloc) && (m_allocated + size < m_buffer_size))
    {
        float *ret = (float*)(m_buffer + m_allocated);
        m_allocated += size;
        return ret;
    }

    m_need_realloc = true;
    return NULL;
}

void CFrustumManager::SplitPrepare(frustum_p frustum, struct portal_s *p, frustum_p emitter)
{
    frustum->vertex_count = p->vertex_count;
    frustum->vertex = this->Alloc(3 * (p->vertex_count + emitter->vertex_count + 1));
    if(frustum->vertex != NULL)
    {
        memcpy(frustum->vertex, p->vertex, 3 * p->vertex_count * sizeof(float));
        vec4_copy_inv(frustum->norm, p->norm);
    }
    else
    {
        frustum->vertex_count = 0;
        m_need_realloc = true;
    }
    frustum->parent = NULL;
}

int CFrustumManager::SplitByPlane(frustum_p p, float n[4], float *buf)
{
    if(!m_need_realloc)
    {
        float *curr_v, *prev_v, *v, t, dir[3];
        float dist[2];
        uint16_t added = 0;

        curr_v = p->vertex;
        prev_v = p->vertex + 3*(p->vertex_count-1);
        dist[0] = vec3_plane_dist(n, prev_v);
        v = buf;
        for(uint16_t i = 0; i < p->vertex_count; i++)
        {
            dist[1] = vec3_plane_dist(n, curr_v);

            if(dist[1] > SPLIT_EPSILON)
            {
                if(dist[0] < -SPLIT_EPSILON)
                {
                    vec3_sub(dir, curr_v, prev_v);
                    vec3_ray_plane_intersect(prev_v, dir, n, v, t);
                    v += 3;
                    added++;
                }
                vec3_copy(v, curr_v);
                v += 3;
                added++;
            }
            else if(dist[1] < -SPLIT_EPSILON)
            {
                if(dist[0] > SPLIT_EPSILON)
                {
                    vec3_sub(dir, curr_v, prev_v);
                    vec3_ray_plane_intersect(prev_v, dir, n, v, t);
                    v += 3;
                    added++;
                }
            }
            else
            {
                vec3_copy(v, curr_v);
                v += 3;
                added++;
            }

            prev_v = curr_v;
            curr_v += 3;
            dist[0] = dist[1];
        }

        if(added <= 2)
        {
            p->vertex_count = 0;
            return SPLIT_EMPTY;
        }

    #if 0
        p->vertex_count = added;
        memcpy(p->vertex, buf, added*3*sizeof(float));
    #else       // filter repeating (too closest) points
        curr_v = buf;
        prev_v = buf + 3 * (added - 1);
        v = p->vertex;
        p->vertex_count = 0;
        for(uint16_t i = 0; i < added; i++)
        {
            if(vec3_dist_sq(prev_v, curr_v) > SPLIT_EPSILON * SPLIT_EPSILON)
            {
                vec3_copy(v, curr_v);
                v += 3;
                p->vertex_count++;
            }
            prev_v = curr_v;
            curr_v += 3;
        }

        if(p->vertex_count <= 2)
        {
            p->vertex_count = 0;
            return SPLIT_EMPTY;
        }
    #endif
        return SPLIT_SUCCES;
    }

    return SPLIT_EMPTY;
}

void CFrustumManager::GenClipPlanes(frustum_p p, struct camera_s *cam)
{
    if(m_allocated + p->vertex_count * 4 * sizeof(float) >= m_buffer_size)
    {
        m_need_realloc = true;
    }

    if((!m_need_realloc) && (p->vertex_count > 0))
    {
        float V1[3], V2[3], *prev_v, *curr_v, *next_v, *r;
        p->planes = this->Alloc(4 * p->vertex_count);

        next_v = p->vertex;
        curr_v = p->vertex + 3 * (p->vertex_count - 1);
        prev_v = curr_v - 3;
        r = p->planes;

        //==========================================================================

        for(uint16_t i = 0; i < p->vertex_count; i++, r += 4)
        {
            float t;
            vec3_sub(V1, prev_v, cam->gl_transform + 12);
            vec3_sub(V2, curr_v, prev_v);
            vec3_norm(V1, t);
            vec3_norm(V2, t);
            vec3_cross(r, V1, V2)
            vec3_norm(r, t);
            r[3] = -vec3_dot(r, curr_v);
            vec4_inv(r);

            prev_v = curr_v;
            curr_v = next_v;
            next_v += 3;
        }

        p->cam_pos = cam->gl_transform + 12;
    }
}

frustum_p CFrustumManager::PortalFrustumIntersect(struct portal_s *portal, frustum_p emitter, struct camera_s *cam)
{
    if(!m_need_realloc)
    {
        room_p dest_room = portal->dest_room;
        int in_dist = 0, in_face = 0;
        float *n = cam->frustum->norm;
        float *v = portal->vertex;

        if(vec3_plane_dist(portal->norm, cam->gl_transform + 12) < -SPLIT_EPSILON)            // non face or degenerate to the line portal
        {
            return NULL;
        }

        for(uint16_t i = 0; i < portal->vertex_count; i++, v += 3)
        {
            if((in_dist == 0) && (vec3_plane_dist(n, v) < cam->dist_far))
            {
                in_dist = 1;
            }
            if((in_face == 0) && (vec3_plane_dist(emitter->norm, v) > 0.0))
            {
                in_face = 1;
            }
        }

        if((in_dist == 0) || (in_face == 0))
        {
            return NULL;
        }

        /*
         * Search for the first free room's frustum
         */
        uint32_t original_allocated = m_allocated;
        frustum_p prev = NULL, current_gen = NULL;
        if(dest_room->frustum == NULL)
        {
            current_gen = dest_room->frustum = this->CreateFrustum();
        }
        else
        {
            prev = dest_room->frustum;
            while(prev->next)
            {
                prev = prev->next;
            }
            current_gen = prev->next = this->CreateFrustum();                   // generate new frustum.
        }

        if(m_need_realloc)
        {
            return NULL;
        }

        this->SplitPrepare(current_gen, portal, emitter);                       // prepare to the clipping
        if(m_need_realloc)
        {
            if(prev)
            {
                prev->next = NULL;
            }
            else
            {
                dest_room->frustum = NULL;
            }
            return NULL;
        }

        int buf_size = (current_gen->vertex_count + emitter->vertex_count + 4) * 3 * sizeof(float);
        float *tmp = (float*)Sys_GetTempMem(buf_size);
        if(this->SplitByPlane(current_gen, emitter->norm, tmp))                 // splitting by main frustum clip plane
        {
            n = emitter->planes;
            for(uint16_t i = 0; i < emitter->vertex_count; i++, n += 4)
            {
                if(!this->SplitByPlane(current_gen, n, tmp))
                {
                    if(prev)
                    {
                        prev->next = NULL;
                    }
                    else
                    {
                        dest_room->frustum = NULL;
                    }
                    Sys_ReturnTempMem(buf_size);
                    m_allocated = original_allocated;
                    return NULL;
                }
            }

            this->GenClipPlanes(current_gen, cam);                              // all is OK, let us generate clipplanes
            if(m_need_realloc)
            {
                if(prev)
                {
                    prev->next = NULL;
                }
                else
                {
                    dest_room->frustum = NULL;
                }
                Sys_ReturnTempMem(buf_size);
                m_allocated = original_allocated;
                return NULL;
            }

            current_gen->parent = emitter;                                      // add parent pointer
            current_gen->parents_count = emitter->parents_count + 1;
            Sys_ReturnTempMem(buf_size);
            return current_gen;
        }

        if(prev)
        {
            prev->next = NULL;
        }
        else
        {
            dest_room->frustum = NULL;
        }
        m_allocated = original_allocated;
        Sys_ReturnTempMem(buf_size);
    }

    return NULL;
}

/*
 ************************* END FRUSTUM MANAGER IMPLEMENTATION*******************
 */

/**
 * we need that checking to avoid infinite recursions
 */
bool Frustum_HaveParent(frustum_p parent, frustum_p frustum)
{
    while(frustum)
    {
        if(parent == frustum)
        {
            return true;
        }
        frustum = frustum->parent;
    }
    return false;
}

bool Frustum_IsPolyVisible(struct polygon_s *p, struct frustum_s *frustum, bool check_backface)
{
    float t, dir[3], T[3], dist[2];
    float *prev_n, *curr_n, *next_n;
    vertex_p curr_v, prev_v;
    char ins, outs;

    if(check_backface && (vec3_plane_dist(p->plane, frustum->cam_pos) < 0.0))
    {
        return false;
    }

    vec3_sub(dir, frustum->vertex, frustum->cam_pos);
    if(Polygon_RayIntersect(p, dir, frustum->cam_pos, &t))
    {
        return true;
    }

    next_n = frustum->planes;
    curr_n = frustum->planes + 4*(frustum->vertex_count-1);
    prev_n = curr_n - 4;
    ins = 1;
    for(uint16_t i = 0; i < frustum->vertex_count; i++)
    {
        curr_v = p->vertices;
        prev_v = p->vertices + p->vertex_count - 1;
        dist[0] = vec3_plane_dist(curr_n, prev_v->position);
        outs = 1;
        for(uint16_t j = 0; j < p->vertex_count; j++)
        {
            dist[1] = vec3_plane_dist(curr_n, curr_v->position);
            if(ABS(dist[0]) < SPLIT_EPSILON)
            {
                if((vec3_plane_dist(prev_n, prev_v->position) > -SPLIT_EPSILON) &&
                   (vec3_plane_dist(next_n, prev_v->position) > -SPLIT_EPSILON) &&
                   (vec3_plane_dist(frustum->norm, prev_v->position) > -SPLIT_EPSILON))
                {
                    return true;
                }
            }

            if((dist[0] * dist[1] < 0) && ABS(dist[1]) >= SPLIT_EPSILON)
            {
                vec3_sub(dir, curr_v->position, prev_v->position)
                vec3_ray_plane_intersect(prev_v->position, dir, curr_n, T, t)
                if((vec3_plane_dist(prev_n, T) > -SPLIT_EPSILON) && (vec3_plane_dist(next_n, T) > -SPLIT_EPSILON))
                {
                    return 1;
                }
            }

            if(dist[1] < -SPLIT_EPSILON)
            {
                ins = 0;
            }
            else
            {
                outs = 0;
            }

            prev_v = curr_v;
            curr_v ++;
            dist[0] = dist[1];
        }

        if(outs)
        {
            return false;
        }
        prev_n = curr_n;
        curr_n = next_n;
        next_n += 4;
    }
    if(ins)
    {
        return true;
    }

    return false;
}

/**
 *
 * @param bbmin - aabb corner (x_min, y_min, z_min)
 * @param bbmax - aabb corner (x_max, y_max, z_max)
 * @param frustum - test frustum
 * @return 1 if aabb is in frustum.
 */
bool Frustum_IsAABBVisible(float bbmin[3], float bbmax[3], struct frustum_s *frustum)
{
    bool inside = true;
    polygon_t poly;
    vertex_t vert[4];

    poly.vertices = vert;
    poly.vertex_count = 4;

    /* X_AXIS */

    poly.plane[1] = 0.0;
    poly.plane[2] = 0.0;
    if(frustum->cam_pos[0] < bbmin[0])
    {
        poly.plane[0] = -1.0;
        poly.plane[3] = bbmin[0];
        vert[0].position[0] = bbmin[0];
        vert[0].position[1] = bbmax[1];
        vert[0].position[2] = bbmax[2];

        vert[1].position[0] = bbmin[0];
        vert[1].position[1] = bbmin[1];
        vert[1].position[2] = bbmax[2];

        vert[2].position[0] = bbmin[0];
        vert[2].position[1] = bbmin[1];
        vert[2].position[2] = bbmin[2];

        vert[3].position[0] = bbmin[0];
        vert[3].position[1] = bbmax[1];
        vert[3].position[2] = bbmin[2];

        if(Frustum_IsPolyVisible(&poly, frustum, true))
        {
            return true;
        }
        inside = false;
    }
    else if(frustum->cam_pos[0] > bbmax[0])
    {
        poly.plane[0] = 1.0;
        poly.plane[3] =-bbmax[0];
        vert[0].position[0] = bbmax[0];
        vert[0].position[1] = bbmax[1];
        vert[0].position[2] = bbmax[2];

        vert[1].position[0] = bbmax[0];
        vert[1].position[1] = bbmin[1];
        vert[1].position[2] = bbmax[2];

        vert[2].position[0] = bbmax[0];
        vert[2].position[1] = bbmin[1];
        vert[2].position[2] = bbmin[2];

        vert[3].position[0] = bbmax[0];
        vert[3].position[1] = bbmax[1];
        vert[3].position[2] = bbmin[2];

        if(Frustum_IsPolyVisible(&poly, frustum, true))
        {
            return true;
        }
        inside = false;
    }

    /* Y AXIS */

    poly.plane[0] = 0.0;
    poly.plane[2] = 0.0;
    if(frustum->cam_pos[1] < bbmin[1])
    {
        poly.plane[1] = -1.0;
        poly.plane[3] = bbmin[1];
        vert[0].position[0] = bbmax[0];
        vert[0].position[1] = bbmin[1];
        vert[0].position[2] = bbmax[2];

        vert[1].position[0] = bbmin[0];
        vert[1].position[1] = bbmin[1];
        vert[1].position[2] = bbmax[2];

        vert[2].position[0] = bbmin[0];
        vert[2].position[1] = bbmin[1];
        vert[2].position[2] = bbmin[2];

        vert[3].position[0] = bbmax[0];
        vert[3].position[1] = bbmin[1];
        vert[3].position[2] = bbmin[2];

        if(Frustum_IsPolyVisible(&poly, frustum, true))
        {
            return true;
        }
        inside = false;
    }
    else if(frustum->cam_pos[1] > bbmax[1])
    {
        poly.plane[1] = 1.0;
        poly.plane[3] = -bbmax[1];
        vert[0].position[0] = bbmax[0];
        vert[0].position[1] = bbmax[1];
        vert[0].position[2] = bbmax[2];

        vert[1].position[0] = bbmin[0];
        vert[1].position[1] = bbmax[1];
        vert[1].position[2] = bbmax[2];

        vert[2].position[0] = bbmin[0];
        vert[2].position[1] = bbmax[1];
        vert[2].position[2] = bbmin[2];

        vert[3].position[0] = bbmax[0];
        vert[3].position[1] = bbmax[1];
        vert[3].position[2] = bbmin[2];

        if(Frustum_IsPolyVisible(&poly, frustum, true))
        {
            return true;
        }
        inside = false;
    }

    /* Z AXIS */

    poly.plane[0] = 0.0;
    poly.plane[1] = 0.0;
    if(frustum->cam_pos[2] < bbmin[2])
    {
        poly.plane[2] = -1.0;
        poly.plane[3] = bbmin[2];
        vert[0].position[0] = bbmax[0];
        vert[0].position[1] = bbmax[1];
        vert[0].position[2] = bbmin[2];

        vert[1].position[0] = bbmin[0];
        vert[1].position[1] = bbmax[1];
        vert[1].position[2] = bbmin[2];

        vert[2].position[0] = bbmin[0];
        vert[2].position[1] = bbmin[1];
        vert[2].position[2] = bbmin[2];

        vert[3].position[0] = bbmax[0];
        vert[3].position[1] = bbmin[1];
        vert[3].position[2] = bbmin[2];

        if(Frustum_IsPolyVisible(&poly, frustum, true))
        {
            return true;
        }
        inside = false;
    }
    else if(frustum->cam_pos[2] > bbmax[2])
    {
        poly.plane[2] = 1.0;
        poly.plane[3] = -bbmax[2];
        vert[0].position[0] = bbmax[0];
        vert[0].position[1] = bbmax[1];
        vert[0].position[2] = bbmax[2];

        vert[1].position[0] = bbmin[0];
        vert[1].position[1] = bbmax[1];
        vert[1].position[2] = bbmax[2];

        vert[2].position[0] = bbmin[0];
        vert[2].position[1] = bbmin[1];
        vert[2].position[2] = bbmax[2];

        vert[3].position[0] = bbmax[0];
        vert[3].position[1] = bbmin[1];
        vert[3].position[2] = bbmax[2];

        if(Frustum_IsPolyVisible(&poly, frustum, true))
        {
            return true;
        }
        inside = false;
    }

    return inside;
}


bool Frustum_IsOBBVisible(struct obb_s *obb, struct frustum_s *frustum)
{
    bool inside = true;
    float t;
    polygon_p p = obb->polygons;

    for(int i = 0; i < 6; i++, p++)
    {
        t = vec3_plane_dist(p->plane, frustum->cam_pos);
        if((t > 0.0) && Frustum_IsPolyVisible(p, frustum, true))
        {
            return true;
        }
        if(inside && (t > 0))
        {
            inside = false;
        }
    }

    return inside;
}

bool Frustum_IsOBBVisibleInFrustumList(struct obb_s *obb, struct frustum_s *frustum)
{
    for(; frustum; frustum = frustum->next)
    {
        if(Frustum_IsOBBVisible(obb, frustum))
        {
            return true;
        }
    }

    return false;
}

/*
 * PORTALS
 */
portal_p Portal_Create(unsigned int vcount)
{
    portal_p p = (portal_p)malloc(sizeof(portal_t));
    p->vertex_count = vcount;
    p->vertex = (float*)malloc(3*vcount*sizeof(float));
    p->dest_room = NULL;
    return p;
}

void Portal_Clear(portal_p p)
{
    if(p)
    {
        if(p->vertex)
        {
            free(p->vertex);
            p->vertex = NULL;
        }
        p->vertex_count = 0;
        p->dest_room = NULL;
    }
}

void Portal_Move(portal_p p, float mv[3])
{
    float *v = p->vertex;

    vec3_add(p->centre, p->centre, mv);
    for(uint16_t i = 0; i < p->vertex_count; i++, v+=3)
    {
        vec3_add(v, v, mv);
    }

    p->norm[3] = -vec3_dot(p->norm, p->vertex);
}

/**
 * Барицентрический метод определения пересечения луча и треугольника
 * p - tested portal
 * dir - ray directionа
 * dot - point of ray - plane intersection
 */
bool Portal_RayIntersect(portal_p p, float dir[3], float dot[3])
{
    float t, u, v, E1[3], E2[3], P[3], Q[3], T[3], *vd;

    u = vec3_dot(p->norm, dir);
    if(ABS(u) < SPLIT_EPSILON)
    {
        return false;
    }
    t = - p->norm[3] - vec3_dot(p->norm, dot);
    t /= u;
    if(0.0 > t)
    {
        return false;
    }

    vd = p->vertex;
    vec3_sub(T, dot, vd)

    vec3_sub(E2, vd+3, vd)
    for(uint16_t i = 0; i < p->vertex_count - 2; i++, vd += 3)
    {
        vec3_copy(E1, E2)                                                       // PREV
        vec3_sub(E2, vd+6, p->vertex)                                           // NEXT

        vec3_cross(P, dir, E2)
        vec3_cross(Q, T, E1)

        t = vec3_dot(P, E1);
        u = vec3_dot(P, T);
        u /= t;
        v = vec3_dot(Q, dir);
        v /= t;
        t = 1.0 - u - v;
        if((u <= 1.0) && (u >= 0.0) && (v <= 1.0) && (v >= 0.0) && (t <= 1.0) && (t >= 0.0))
        {
            return true;
        }
    }

    return false;
}

void Portal_GenNormale(portal_p p)
{
    float v1[3], v2[3];

    vec3_sub(v1, p->vertex+3, p->vertex)
    vec3_sub(v2, p->vertex+6, p->vertex+3)
    vec3_cross(p->norm, v1, v2)
    p->norm[3] = vec3_abs(p->norm);
    vec3_norm_plane(p->norm, p->vertex, p->norm[3])
}
