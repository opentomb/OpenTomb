#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include "core/vmath.h"
#include "core/polygon.h"
#include "portal.h"
#include "camera.h"
#include "world.h"
#include "render.h"
#include "frustum.h"
#include "engine.h"

/*
 * CLIP PLANES
 */

struct room_sector_s;


void Portal_InitGlobals()
{
}


/*
 * PORTALS
 */

portal_p Portal_Create(unsigned int vcount)
{
    portal_p p = (portal_p)malloc(sizeof(portal_t));
    p->vertex_count = vcount;
    p->vertex = (btScalar*)malloc(3*vcount*sizeof(btScalar));
    p->flag = 0;
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
        p->flag = 0;
        p->dest_room = NULL;
    }
}


/*
 * Есть ли портал над сектором
 */
int Portal_IsOnSectorTop(portal_p p, struct room_sector_s *sector)
{
    btScalar bb_min[2], bb_max[2], prtl_range_x[2], prtl_range_y[2];

    /*
     * Если портал вертикальный, то
     */
    if(p->norm[2] < -0.999)
    {
        bb_min[0] = bb_max[0] = sector->owner_room->transform[12] + 1024.0 * sector->index_x;
        bb_min[1] = bb_max[1] = sector->owner_room->transform[13] + 1024.0 * sector->index_y;
        bb_max[0] += 1024.0;
        bb_max[1] += 1024.0;

        /*
         * определение границ портала в плоскости XY
         */
        prtl_range_x[0] = prtl_range_x[1] = p->vertex[0];
        prtl_range_y[0] = prtl_range_y[1] = p->vertex[1];
        for(uint16_t i=1;i<p->vertex_count;i++)
        {
            // portal range x
            if(p->vertex[3*i+0] < prtl_range_x[0])
            {
                prtl_range_x[0] = p->vertex[3*i+0];
            }
            if(p->vertex[3*i+0] > prtl_range_x[1])
            {
                prtl_range_x[1] = p->vertex[3*i+0];
            }

            // portal range y
            if(p->vertex[3*i+1] < prtl_range_y[0])
            {
                prtl_range_y[0] = p->vertex[3*i+1];
            }
            if(p->vertex[3*i+1] > prtl_range_y[1])
            {
                prtl_range_y[1] = p->vertex[3*i+1];
            }
        }

        if(!((prtl_range_x[0] >= bb_max[0]) || (prtl_range_x[1] <= bb_min[0])) &&
           !((prtl_range_y[0] >= bb_max[1]) || (prtl_range_y[1] <= bb_min[1])))
        {
            return 1;
        }
    }

    return 0;
}


/**
 * Ведет ли портал непосредственно в данный сектор.
 * Сектор должен "прилегать" к порталу.
 */
int Portal_IsWayToSector(portal_p p, struct room_sector_s *sector)
{
    btScalar bb_min[2], bb_max[2], prtl_range_x[2], prtl_range_y[2];
    char in_x, in_y;

    bb_min[0] = bb_max[0] = sector->owner_room->transform[12] + 1024.0 * sector->index_x;
    bb_min[1] = bb_max[1] = sector->owner_room->transform[13] + 1024.0 * sector->index_y;
    bb_max[0] += 1024.0;
    bb_max[1] += 1024.0;

    /*
     * определение границ портала в плоскости XY
     */
    prtl_range_x[0] = prtl_range_x[1] = p->vertex[0];
    prtl_range_y[0] = prtl_range_y[1] = p->vertex[1];
    for(uint16_t i=1;i<p->vertex_count;i++)
    {
        // portal range x
        if(p->vertex[3*i+0] < prtl_range_x[0])
        {
            prtl_range_x[0] = p->vertex[3*i+0];
        }
        if(p->vertex[3*i+0] > prtl_range_x[1])
        {
            prtl_range_x[1] = p->vertex[3*i+0];
        }

        // portal range y
        if(p->vertex[3*i+1] < prtl_range_y[0])
        {
            prtl_range_y[0] = p->vertex[3*i+1];
        }
        if(p->vertex[3*i+1] > prtl_range_y[1])
        {
            prtl_range_y[1] = p->vertex[3*i+1];
        }
    }

    // X_MAX
    if(p->norm[0] < -0.999)
    {
        if(ABS(bb_min[0] - p->centre[0]) <= 1.0)                                // прилегание плоскости выполнено
        {
            if(!((prtl_range_y[0] >= bb_max[1]) || (prtl_range_y[1] <= bb_min[1]))) // проверка на пересечение диапазонов
            {
                return 1;
            }
        }
    }
    // X_MIN
    else if(p->norm[0] > 0.999)
    {
        if(ABS(bb_max[0] - p->centre[0]) <= 1.0)                                // прилегание плоскости выполнено
        {
            if(!((prtl_range_y[0] >= bb_max[1]) || (prtl_range_y[1] <= bb_min[1]))) // проверка на пересечение диапазонов
            {
                return 1;
            }
        }
    }
    // Y_MAX
    else if(p->norm[1] < -0.999)
    {
        if(ABS(bb_min[1] - p->centre[1]) <= 1.0)                                // прилегание плоскости выполнено
        {
            if(!((prtl_range_x[0] >= bb_max[0]) || (prtl_range_x[1] <= bb_min[0]))) // проверка на пересечение диапазонов
            {
                return 1;
            }
        }
    }
    // Y_MIN
    else if(p->norm[1] > 0.999)
    {
        if(ABS(bb_max[1] - p->centre[1]) <= 1.0)                                // прилегание плоскости выполнено
        {
            if(!((prtl_range_x[0] >= bb_max[0]) || (prtl_range_x[1] <= bb_min[0]))) // проверка на пересечение диапазонов
            {
                return 1;
            }
        }
    }

    /*
     * Если портал вертикальный, то
     */
    if(p->norm[2] < -0.999)
    {
        in_x = !((prtl_range_x[0] >= bb_max[0]) || (prtl_range_x[1] <= bb_min[0]));
        in_y = !((prtl_range_y[0] >= bb_max[1]) || (prtl_range_y[1] <= bb_min[1]));
        if(!((prtl_range_x[0] > bb_max[0]) || (prtl_range_x[1] < bb_min[0])) &&
           !((prtl_range_y[0] > bb_max[1]) || (prtl_range_y[1] < bb_min[1])))
        //if(in_x && in_y)
        {
            return 1;
        }
        if(in_x && ((ABS(bb_min[1] - prtl_range_y[1]) <= 1.0) || (ABS(bb_max[1] - prtl_range_y[0]) <= 1.0)))
        {
            return 1;
        }
        if(in_y && ((ABS(bb_min[0] - prtl_range_x[1]) <= 1.0) || (ABS(bb_max[0] - prtl_range_x[0]) <= 1.0)))
        {
            return 1;
        }
    }

    return 0;
}


void Portal_Move(portal_p p, btScalar mv[3])
{
    btScalar *v = p->vertex;

    vec3_add(p->centre, p->centre, mv);
    for(uint16_t i=0;i<p->vertex_count;i++,v+=3)
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
int Portal_RayIntersect(portal_p p, btScalar dir[3], btScalar dot[3])
{
    btScalar t, u, v, E1[3], E2[3], P[3], Q[3], T[3], *vd;

    u = vec3_dot(p->norm, dir);
    if(ABS(u) < SPLIT_EPSILON)
    {
        return 0;                                                               // плоскость параллельна лучу
    }
    t = - p->norm[3] - vec3_dot(p->norm, dot);
    t /= u;
    if(0.0 > t)
    {
        return 0;                                                               // плоскость не с той стороны луча
    }

    vd = p->vertex;                                                             // Указатель на текущий треугольник
    vec3_sub(T, dot, vd)                                                        // Вектор который не меняется для всего полигона

    vec3_sub(E2, vd+3, vd)
    for(uint16_t i=0;i<p->vertex_count-2;i++,vd+=3)                             // Обход полигона веером, один из векторов остается прежним
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
            return 1;
        }
    }

    return 0;
}

/**
 * Просто генерация нормали к порталу
 */
void Portal_GenNormale(portal_p p)
{
    btScalar v1[3], v2[3];

    vec3_sub(v1, p->vertex+3, p->vertex)
    vec3_sub(v2, p->vertex+6, p->vertex+3)
    vec3_cross(p->norm, v1, v2)
    p->norm[3] = vec3_abs(p->norm);
    vec3_norm_plane(p->norm, p->vertex, p->norm[3])
}
