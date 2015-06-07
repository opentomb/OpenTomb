
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "bullet/LinearMath/btScalar.h"

#include "frustum.h"
#include "vmath.h"
#include "camera.h"
#include "polygon.h"
#include "portal.h"
#include "render.h"
#include "world.h"
#include "engine.h"
#include "obb.h"

frustum_p Frustum_Create()
{
    frustum_p frus;
    frus = (frustum_p)malloc(sizeof(frustum_t));
    frus->active = 0;
    frus->count = 0;
    frus->parents_count = 0;
    frus->next = NULL;
    frus->planes = NULL;
    frus->vertex = NULL;
    frus->cam_pos = NULL;
    return frus;
}

void Frustum_Delete(frustum_p p)
{
    if(p && (!renderer.cam || p != renderer.cam->frustum))
    {
        frustum_p next = p->next;

        p->next = NULL;
        p->active = 0;
        p->count = 0;
        if(p->planes)
        {
            free(p->planes);
            p->planes = NULL;
        }

        if(p->vertex)
        {
            free(p->vertex);
            p->vertex = NULL;
        }

        free(p);

        if(next)
        {
            Frustum_Delete(next);
            next = NULL;
        }
    }
}

void Frustum_Copy(frustum_p p, frustum_p src)
{
    p->cam_pos = src->cam_pos;
    p->active = src->active;
    p->count = src->count;
    vec4_copy(p->norm, src->norm);
    if(p->count)
    {
        p->planes = (btScalar*)realloc(p->planes, 4*p->count*sizeof(btScalar));
        memcpy(p->planes, src->planes, 4*p->count*sizeof(btScalar));
        p->vertex = (btScalar*)realloc(p->vertex, 3*p->count*sizeof(btScalar));
        memcpy(p->vertex, src->vertex, 3*p->count*sizeof(btScalar));
    }
}


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
    while(frustum && frustum->active)
    {
        if(parent == frustum)
        {
            return 1;
        }
        frustum = frustum->parent;
    }
    return 0;
}

void Frustum_SplitPrepare(frustum_p frustum, struct portal_s *p)
{
    frustum->count = p->vertex_count;
    frustum->vertex = (btScalar*)realloc(frustum->vertex, 3*p->vertex_count*sizeof(btScalar));
    memcpy(frustum->vertex, p->vertex, 3*p->vertex_count*sizeof(btScalar));
    vec4_copy_inv(frustum->norm, p->norm);
    frustum->active = 0;
    frustum->parent = NULL;
}

int Frustum_Split(frustum_p p, btScalar n[4], btScalar *buf)                    // отсечение части фрустума плоскостью
{
    btScalar *curr_v, *prev_v, *v, t, dir[3];
    btScalar dist[2];
    uint16_t added = 0;

    curr_v = p->vertex;
    prev_v = p->vertex + 3*(p->count-1);
    dist[0] = vec3_plane_dist(n, prev_v);
    v = buf;
    for(uint16_t i=0;i<p->count;i++)
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
        p->count = 0;
        p->active = 0;
        return SPLIT_EMPTY;
    }

    p->vertex = (btScalar*)realloc(p->vertex, added*3*sizeof(btScalar));
#if 1
    p->count = added;
    memcpy(p->vertex, buf, added*3*sizeof(btScalar));
#else       // filter repeating (too closest) points
    curr_v = buf;
    prev_v = buf + 3*(added-1);
    v = p->vertex;
    p->count = 0;
    for(uint16_t i=0;i<added;i++)
    {
        if(vec3_dist_sq(prev_v, curr_v) > SPLIT_EPSILON * SPLIT_EPSILON)
        {
            vec3_copy(v, curr_v);
            v += 3;
            p->count++;
        }
        prev_v = curr_v;
        curr_v += 3;
    }

    if(p->count <= 2)
    {
        p->count = 0;
        p->active = 0;
        return SPLIT_EMPTY;
    }
#endif
    p->active = 1;

    return SPLIT_SUCCES;
}

/**
 * Clip planes generation
 */
void Frustum_GenClipPlanes(frustum_p p, struct camera_s *cam)
{
    btScalar V1[3], V2[3], *prev_v, *curr_v, *next_v, *r;

    if(p->count)
    {
        p->planes = (btScalar*)realloc(p->planes, 4*p->count*sizeof(btScalar));

        next_v = p->vertex;
        curr_v = p->vertex + 3 * (p->count - 1);
        prev_v = curr_v - 3;
        r = p->planes;

        //==========================================================================

        for(uint16_t i=0;i<p->count;i++,r+=4)
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
        p->active = 1;
    }
}

/**
 * Проверка полигона на видимость через портал.
 * данный метод НЕ для реалтайма, т.к. проверка в общем случае выходит дороже отрисовки...
 */
int Frustum_IsPolyVisible(const struct polygon_s *p, const struct frustum_s *frustum)
{
    btScalar t, dir[3], T[3], dist[2];
    btScalar *prev_n, *curr_n, *next_n;
    vertex_p curr_v, prev_v;
    char ins, outs;

    if(!p->double_side && vec3_plane_dist(p->plane, frustum->cam_pos) < 0.0)
    {
        return 0;
    }

    vec3_sub(dir, frustum->vertex, frustum->cam_pos)                            // направление от позици камеры до произвольной вершины фрустума
    if(Polygon_RayIntersect(p, dir, frustum->cam_pos, &t))                      // полигон вмещает фрустум портала (бреед, но проверить надо)
    {
        return 1;
    }

    next_n = frustum->planes;                                                   // генерим очередь проверки
    curr_n = frustum->planes + 4*(frustum->count-1);                            // 3 соседних плоскости отсечения
    prev_n = curr_n - 4;                                                        //
    ins = 1;                                                                    // на случай если нет пересечений
    for(uint16_t i=0;i<frustum->count;i++)                                      // перебираем все плоскости текущего фрустума
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
    int ins;
    polygon_p p;
    frustum_p frustum;
    btScalar t;

    frustum = room->frustum;
    if(frustum->active == 0)                                                    // В комнате нет активного фрустума, значит применяем фрустум камеры
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

    for(;frustum && frustum->active;frustum=frustum->next)                      // Если хоть в одном активном фрустуме виден объект, то возвращаем 1
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

