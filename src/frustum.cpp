
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "core/system.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "frustum.h"
#include "camera.h"
#include "portal.h"
#include "render.h"
#include "world.h"
#include "obb.h"


frustumManager engine_frustumManager(32768);

frustumManager::frustumManager(uint32_t buffer_size)
{
    m_buffer_size = buffer_size;
    m_allocated = 0;
    m_buffer = (uint8_t*)malloc(buffer_size * sizeof(uint8_t));
    m_need_realloc = false;
}

frustumManager::~frustumManager()
{
    if(m_buffer != NULL)
    {
        free(m_buffer);
        m_buffer = NULL;
    }
}

void frustumManager::reset()
{
    m_allocated = 0;
    if(m_need_realloc)
    {
        uint32_t new_buffer_size = m_buffer_size * 1.5;
        uint8_t *new_buffer = (uint8_t*)realloc(m_buffer, new_buffer_size * sizeof(uint8_t));
        if(new_buffer != NULL)
        {
            m_buffer = new_buffer;
            m_buffer_size = new_buffer_size;
        }
        m_need_realloc = false;
    }
}

frustum_p frustumManager::createFrustum()
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

btScalar *frustumManager::alloc(uint32_t size)
{
    size *= sizeof(btScalar);
    if((!m_need_realloc) && (m_allocated + size < m_buffer_size))
    {
        btScalar *ret = (btScalar*)(m_buffer + m_allocated);
        m_allocated += size;
        return ret;
    }

    m_need_realloc = true;
    return NULL;
}

void frustumManager::splitPrepare(frustum_p frustum, struct portal_s *p, frustum_p emitter)
{
    frustum->vertex_count = p->vertex_count;
    frustum->vertex = this->alloc(3 * (p->vertex_count + emitter->vertex_count + 1));
    if(frustum->vertex != NULL)
    {
        memcpy(frustum->vertex, p->vertex, 3 * p->vertex_count * sizeof(btScalar));
        vec4_copy_inv(frustum->norm, p->norm);
    }
    else
    {
        frustum->vertex_count = 0;
        m_need_realloc = true;
    }
    frustum->parent = NULL;
}

int frustumManager::split_by_plane(frustum_p p, btScalar n[4], btScalar *buf)
{
    if(!m_need_realloc)
    {
        btScalar *curr_v, *prev_v, *v, t, dir[3];
        btScalar dist[2];
        uint16_t added = 0;

        curr_v = p->vertex;
        prev_v = p->vertex + 3*(p->vertex_count-1);
        dist[0] = vec3_plane_dist(n, prev_v);
        v = buf;
        for(uint16_t i=0;i<p->vertex_count;i++)
        {
            dist[1] = vec3_plane_dist(n, curr_v);

            if(dist[1] > SPLIT_EPSILON)
            {
                if(dist[0] < -SPLIT_EPSILON)
                {
                    vec3_sub(dir, curr_v, prev_v)
                    vec3_ray_plane_intersect(prev_v, dir, n, v, t)                  // ищем точку пересечения
                    v += 3;                                                         // сдвигаем
                    added++;
                }
                vec3_copy(v, curr_v);                                               // добавляем
                v += 3;                                                             // инкрементируем указатель на буффер вершин
                added++;                                                            // инкрементируем результат
            }
            else if(dist[1] < -SPLIT_EPSILON)
            {
                if(dist[0] > SPLIT_EPSILON)
                {
                    vec3_sub(dir, curr_v, prev_v)
                    vec3_ray_plane_intersect(prev_v, dir, n, v, t)                  // ищем точку пересечения
                    v += 3;                                                         // сдвигаем
                    added++;
                }
            }
            else
            {
                vec3_copy(v, curr_v);                                               // добавляем
                v += 3;                                                             // инкрементируем указатель на буффер вершин
                added++;                                                            // инкрементируем результат
            }

            prev_v = curr_v;
            curr_v += 3;
            dist[0] = dist[1];
        }

        if(added <= 2)                                                              // ничего не добавлено или вырождено
        {
            p->vertex_count = 0;
            return SPLIT_EMPTY;
        }

    #if 0
        p->vertex_count = added;
        memcpy(p->vertex, buf, added*3*sizeof(btScalar));
    #else       // filter repeating (too closest) points
        curr_v = buf;
        prev_v = buf + 3 * (added - 1);
        v = p->vertex;
        p->vertex_count = 0;
        for(uint16_t i=0;i<added;i++)
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

void frustumManager::genClipPlanes(frustum_p p, struct camera_s *cam)
{
    if(m_allocated + p->vertex_count * 4 * sizeof(btScalar) >= m_buffer_size)
    {
        m_need_realloc = true;
    }

    if((!m_need_realloc) && (p->vertex_count > 0))
    {
        btScalar V1[3], V2[3], *prev_v, *curr_v, *next_v, *r;
        p->planes = this->alloc(4 * p->vertex_count);

        next_v = p->vertex;
        curr_v = p->vertex + 3 * (p->vertex_count - 1);
        prev_v = curr_v - 3;
        r = p->planes;

        //==========================================================================

        for(uint16_t i=0;i<p->vertex_count;i++,r+=4)
        {
            btScalar t;
            vec3_sub(V1, prev_v, cam->pos)                                      // вектор от наблюдателя до вершины полигона
            vec3_sub(V2, curr_v, prev_v)                                        // вектор соединяющий соседние вершины полигона
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

        p->cam_pos = cam->pos;
    }
}

/*
 * receiver - указатель на базовый фрустум рума, куда ведет портал - берется из портала!!!
 * возвращает указатель на свежесгенеренный фрустум
 */
frustum_p frustumManager::portalFrustumIntersect(struct portal_s *portal, frustum_p emitter, struct render_s *render)
{
    if(!m_need_realloc)
    {
        if(vec3_plane_dist(portal->norm, render->cam->pos) < -SPLIT_EPSILON)    // non face or degenerate to the line portal
        {
            return NULL;
        }

        if((portal->dest_room->frustum != NULL) && Frustum_HaveParent(portal->dest_room->frustum, emitter))
        {
            return NULL;                                                        // abort infinite cycling!
        }

        int in_dist = 0, in_face = 0;
        btScalar *n = render->cam->frustum->norm;
        btScalar *v = portal->vertex;
        for(uint16_t i=0;i<portal->vertex_count;i++,v+=3)
        {
            if((in_dist == 0) && (vec3_plane_dist(n, v) < render->cam->dist_far))
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
        if(portal->dest_room->frustum == NULL)
        {
            current_gen = portal->dest_room->frustum = this->createFrustum();
        }
        else
        {
            prev = portal->dest_room->frustum;
            while(prev->next)
            {
                prev = prev->next;
            }
            current_gen = prev->next = this->createFrustum();                   // generate new frustum.
        }

        if(m_need_realloc)
        {
            return NULL;
        }

        this->splitPrepare(current_gen, portal, emitter);                       // prepare to the clipping
        if(m_need_realloc)
        {
            if(prev)
            {
                prev->next = NULL;
            }
            else
            {
                portal->dest_room->frustum = NULL;
            }
            return NULL;
        }

        int buf_size = (current_gen->vertex_count + emitter->vertex_count + 4) * 3 * sizeof(btScalar);
        btScalar *tmp = (btScalar*)Sys_GetTempMem(buf_size);
        if(this->split_by_plane(current_gen, emitter->norm, tmp))               // splitting by main frustum clip plane
        {
            n = emitter->planes;
            for(uint16_t i=0;i<emitter->vertex_count;i++,n+=4)
            {
                if(!this->split_by_plane(current_gen, n, tmp))
                {
                    if(prev)
                    {
                        prev->next = NULL;
                    }
                    else
                    {
                        portal->dest_room->frustum = NULL;
                    }
                    Sys_ReturnTempMem(buf_size);
                    m_allocated = original_allocated;
                    return NULL;
                }
            }

            this->genClipPlanes(current_gen, render->cam);                      // all is OK, let us generate clipplanes
            if(m_need_realloc)
            {
                if(prev)
                {
                    prev->next = NULL;
                }
                else
                {
                    portal->dest_room->frustum = NULL;
                }
                Sys_ReturnTempMem(buf_size);
                m_allocated = original_allocated;
                return NULL;
            }

            current_gen->parent = emitter;                                      // add parent pointer
            current_gen->parents_count = emitter->parents_count + 1;
            portal->dest_room->active_frustums++;
            if(portal->dest_room->max_path < current_gen->parents_count)
            {
                portal->dest_room->max_path = current_gen->parents_count;       // maximum path to the room
            }
            Sys_ReturnTempMem(buf_size);
            return current_gen;
        }

        if(prev)
        {
            prev->next = NULL;
        }
        else
        {
            portal->dest_room->frustum = NULL;
        }
        m_allocated = original_allocated;
        Sys_ReturnTempMem(buf_size);
    }

    return NULL;
}

/*
 ************************* END FRUSTUM MANAGER IMPLEMENTATION*******************
 */

int Frustum_GetFrustumsCount(struct frustum_s *f)
{
    int i;

    for(i=0;f;f=f->next,i++);

    return i - 1;
}

/**
 * ф-я разрыватель замкнутых реккурсий
 * если в комнате есть фрустум, породивший текущий, то возвращаем 1
 * и тогда порочный цикл рвется
 */
int Frustum_HaveParent(frustum_p parent, frustum_p frustum)
{
    while(frustum)
    {
        if(parent == frustum)
        {
            return 1;
        }
        frustum = frustum->parent;
    }
    return 0;
}


/**
 * Проверка полигона на видимость через портал.
 * данный метод НЕ для реалтайма, т.к. проверка в общем случае выходит дороже отрисовки...
 */
int Frustum_IsPolyVisible(struct polygon_s *p, struct frustum_s *frustum)
{
    btScalar t, dir[3], T[3], dist[2];
    btScalar *prev_n, *curr_n, *next_n;
    vertex_p curr_v, prev_v;
    char ins, outs;

    if(vec3_plane_dist(p->plane, frustum->cam_pos) < 0.0)
    {
        return 0;
    }

    vec3_sub(dir, frustum->vertex, frustum->cam_pos)                            // направление от позици камеры до произвольной вершины фрустума
    if(Polygon_RayIntersect(p, dir, frustum->cam_pos, &t))                      // полигон вмещает фрустум портала (бреед, но проверить надо)
    {
        return 1;
    }

    next_n = frustum->planes;                                                   // генерим очередь проверки
    curr_n = frustum->planes + 4*(frustum->vertex_count-1);                     // 3 соседних плоскости отсечения
    prev_n = curr_n - 4;                                                        //
    ins = 1;                                                                    // на случай если нет пересечений
    for(uint16_t i=0;i<frustum->vertex_count;i++)                               // перебираем все плоскости текущего фрустума
    {
        curr_v = p->vertices;                                                   // генерим очередь вершин под проверку
        prev_v = p->vertices + p->vertex_count - 1;                             //
        dist[0] = vec3_plane_dist(curr_n, prev_v->position);                    // расстояние со знаком от текущей точки до предыдущей плоскости
        outs = 1;
        for(uint16_t j=0;j<p->vertex_count;j++)                                 // перебираем все вершины полигона
        {
            dist[1] = vec3_plane_dist(curr_n, curr_v->position);
            if(ABS(dist[0]) < SPLIT_EPSILON)                                    // точка на плоскости отсечения
            {
                if((vec3_plane_dist(prev_n, prev_v->position) > -SPLIT_EPSILON) &&
                   (vec3_plane_dist(next_n, prev_v->position) > -SPLIT_EPSILON) &&
                   (vec3_plane_dist(frustum->norm, prev_v->position) > -SPLIT_EPSILON))
                {
                    return 1;                                                   // прошли проверку на пересечение вершины многоугльника и фрустума
                }
            }

            if((dist[0] * dist[1] < 0) && ABS(dist[1]) >= SPLIT_EPSILON)        // вершины с разных сторон плоскости (или на ней)
            {
                vec3_sub(dir, curr_v->position, prev_v->position)               // вектор, соединяющий вершины
                vec3_ray_plane_intersect(prev_v->position, dir, curr_n, T, t)   // ищем точку пересечения
                if((vec3_plane_dist(prev_n, T) > -SPLIT_EPSILON) && (vec3_plane_dist(next_n, T) > -SPLIT_EPSILON))
                {
                    return 1;                                                   // прошли проверку на пересечение отрезка многоугльника и фрустума
                }
            }

            if(dist[1] < -SPLIT_EPSILON)                                        // точка снаружи
            {
                ins = 0;
            }
            else
            {
                outs = 0;
            }

            prev_v = curr_v;                                                    // сдвинули очередь вершин полигона
            curr_v ++;                                                          //
            dist[0] = dist[1];                                                  // сдвинули очередь дистанций
        }                                                                       // закончили переборку вершин полигона

        if(outs)
        {
            return 0;                                                           // все точки снаружи относительно текущей плоскости - однозначно выход
        }
        prev_n = curr_n;                                                        // сдвинули очередь плоскостей отсечения
        curr_n = next_n;
        next_n += 4;
    }                                                                           // закончили перебирать все плоскости текущего фрустума
    if(ins)
    {
        return 1;                                                               // все вершины внутренние - тест пройден
    }

    return 0;
}

/**
 *
 * @param bbmin - aabb corner (x_min, y_min, z_min)
 * @param bbmax - aabb corner (x_max, y_max, z_max)
 * @param frustum - test frustum
 * @return 1 if aabb is in frustum.
 */
int Frustum_IsAABBVisible(btScalar bbmin[3], btScalar bbmax[3], struct frustum_s *frustum)
{
    char ins;
    polygon_t poly;
    vertex_t vert[4];

    poly.vertices = vert;
    poly.vertex_count = 4;
    ins = 1;

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

        if(Frustum_IsPolyVisible(&poly, frustum))
        {
            return 1;
        }
        ins = 0;
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

        if(Frustum_IsPolyVisible(&poly, frustum))
        {
            return 1;
        }
        ins = 0;
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

        if(Frustum_IsPolyVisible(&poly, frustum))
        {
            return 1;
        }
        ins = 0;
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

        if(Frustum_IsPolyVisible(&poly, frustum))
        {
            return 1;
        }
        ins = 0;
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

        if(Frustum_IsPolyVisible(&poly, frustum))
        {
            return 1;
        }
        ins = 0;
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

        if(Frustum_IsPolyVisible(&poly, frustum))
        {
            return 1;
        }
        ins = 0;
    }

    return ins;
}


int Frustum_IsOBBVisible(struct obb_s *obb, struct frustum_s *frustum)
{
    int ins = 1;
    btScalar t;
    polygon_p p;

    p = obb->polygons;
    for(int i=0;i<6;i++,p++)
    {
        t = vec3_plane_dist(p->plane, frustum->cam_pos);
        if((t > 0.0) && Frustum_IsPolyVisible(p, frustum))
        {
            return 1;
        }
        if(ins && (t > 0))
        {
            ins = 0;
        }
    }

    return ins;
}

int Frustum_IsOBBVisibleInRoom(struct obb_s *obb, struct room_s *room)
{
    int                         ins;
    polygon_p                   p;
    frustum_p                   frustum;
    btScalar                    t;
    extern struct camera_s      engine_camera;

    frustum = room->frustum;
    if(frustum == NULL)                                                         // В комнате нет активного фрустума, значит применяем фрустум камеры
    {
        ins = 1;                                                                // считаем, что камера внутри OBB
        p = obb->polygons;
        for(int i=0;i<6;i++,p++)
        {
            t = vec3_plane_dist(p->plane, engine_camera.pos);
            if((t > 0.0) && Frustum_IsPolyVisible(p, engine_camera.frustum))
            {
                return 1;
            }
            if(ins && (t > 0.0))                                                // проверка на принадлежность точки наблюдателя OBB
            {
                ins = 0;                                                        // хоть один провал проверки - и камера не может быть внутри
            }
        }
        return ins;                                                             // если камера внутри OBB объекта, то объект виден
    }

    for(;frustum;frustum=frustum->next)                                         // Если хоть в одном активном фрустуме виден объект, то возвращаем 1
    {
        p = obb->polygons;
        for(int i=0;i<6;i++,p++)
        {
            t = vec3_plane_dist(p->plane, frustum->cam_pos);
            if((t > 0.0) && Frustum_IsPolyVisible(p, frustum))
            {
                return 1;
            }
        }
    }

    return 0;
}

