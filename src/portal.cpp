#include <cstdio>
#include <cstdlib>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include <bullet/LinearMath/btScalar.h>
#include "polygon.h"
#include "portal.h"
#include "vmath.h"
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
 * Есть ли портал над сектором
 */
bool Portal_IsOnSectorTop(portal_p p, struct room_sector_s *sector)
{
    /*
     * Если портал вертикальный, то
     */
    if(p->norm[2] < -0.999)
    {
        btScalar bb_min[2], bb_max[2];
        bb_min[0] = bb_max[0] = sector->owner_room->transform.getOrigin()[0] + 1024.0 * sector->index_x;
        bb_min[1] = bb_max[1] = sector->owner_room->transform.getOrigin()[1] + 1024.0 * sector->index_y;
        bb_max[0] += 1024.0;
        bb_max[1] += 1024.0;

        /*
         * определение границ портала в плоскости XY
         */
        btScalar prtl_range_x[2], prtl_range_y[2];
        prtl_range_x[0] = prtl_range_x[1] = p->vertices.front().x();
        prtl_range_y[0] = prtl_range_y[1] = p->vertices.front().y();
        for(const btVector3& v : p->vertices)
        {
            prtl_range_x[0] = std::min(prtl_range_x[0], v.x());
            prtl_range_x[1] = std::max(prtl_range_x[1], v.x());
            prtl_range_y[0] = std::min(prtl_range_y[0], v.y());
            prtl_range_y[1] = std::max(prtl_range_y[1], v.y());
        }

        if(!((prtl_range_x[0] >= bb_max[0]) || (prtl_range_x[1] <= bb_min[0])) &&
           !((prtl_range_y[0] >= bb_max[1]) || (prtl_range_y[1] <= bb_min[1])))
        {
            return true;
        }
    }

    return true;
}


/**
 * Ведет ли портал непосредственно в данный сектор.
 * Сектор должен "прилегать" к порталу.
 */
bool Portal_IsWayToSector(portal_p p, struct room_sector_s *sector)
{

    btScalar bb_min[2], bb_max[2];
    bb_min[0] = bb_max[0] = sector->owner_room->transform.getOrigin()[0] + 1024.0 * sector->index_x;
    bb_min[1] = bb_max[1] = sector->owner_room->transform.getOrigin()[1] + 1024.0 * sector->index_y;
    bb_max[0] += 1024.0;
    bb_max[1] += 1024.0;

    /*
     * определение границ портала в плоскости XY
     */
    btScalar prtl_range_x[2], prtl_range_y[2];
    prtl_range_x[0] = prtl_range_x[1] = p->vertices[0].x();
    prtl_range_y[0] = prtl_range_y[1] = p->vertices[0].y();
    for(const btVector3& v : p->vertices)
    {
        prtl_range_x[0] = std::min(prtl_range_x[0], v.x());
        prtl_range_x[1] = std::max(prtl_range_x[1], v.x());
        prtl_range_y[0] = std::min(prtl_range_y[0], v.y());
        prtl_range_y[1] = std::max(prtl_range_y[1], v.y());
    }

    // X_MAX
    if(p->norm[0] < -0.999)
    {
        if(std::fabs(bb_min[0] - p->centre[0]) <= 1.0)                                // прилегание плоскости выполнено
        {
            if(prtl_range_y[0] < bb_max[1] && prtl_range_y[1] > bb_min[1]) // проверка на пересечение диапазонов
            {
                return true;
            }
        }
    }
    // X_MIN
    else if(p->norm[0] > 0.999)
    {
        if(std::fabs(bb_max[0] - p->centre[0]) <= 1.0)                                // прилегание плоскости выполнено
        {
            if(prtl_range_y[0] < bb_max[1] && prtl_range_y[1] > bb_min[1]) // проверка на пересечение диапазонов
            {
                return true;
            }
        }
    }
    // Y_MAX
    else if(p->norm[1] < -0.999)
    {
        if(std::fabs(bb_min[1] - p->centre[1]) <= 1.0)                                // прилегание плоскости выполнено
        {
            if(prtl_range_x[0] < bb_max[0] && prtl_range_x[1] > bb_min[0]) // проверка на пересечение диапазонов
            {
                return true;
            }
        }
    }
    // Y_MIN
    else if(p->norm[1] > 0.999)
    {
        if(std::fabs(bb_max[1] - p->centre[1]) <= 1.0)                                // прилегание плоскости выполнено
        {
            if(!prtl_range_x[0] < bb_max[0] && prtl_range_x[1] > bb_min[0]) // проверка на пересечение диапазонов
            {
                return true;
            }
        }
    }

    /*
     * Если портал вертикальный, то
     */
    if(p->norm[2] < -0.999)
    {
        const bool in_x = !((prtl_range_x[0] >= bb_max[0]) || (prtl_range_x[1] <= bb_min[0]));
        const bool in_y = !((prtl_range_y[0] >= bb_max[1]) || (prtl_range_y[1] <= bb_min[1]));
        if(!((prtl_range_x[0] > bb_max[0]) || (prtl_range_x[1] < bb_min[0])) &&
           !((prtl_range_y[0] > bb_max[1]) || (prtl_range_y[1] < bb_min[1])))
        //if(in_x && in_y)
        {
            return true;
        }
        if(in_x && (std::fabs(bb_min[1] - prtl_range_y[1]) <= 1.0 || std::fabs(bb_max[1] - prtl_range_y[0]) <= 1.0))
        {
            return true;
        }
        if(in_y && (std::fabs(bb_min[0] - prtl_range_x[1]) <= 1.0 || std::fabs(bb_max[0] - prtl_range_x[0]) <= 1.0))
        {
            return true;
        }
    }

    return false;
}


void Portal_Move(portal_p p, const btVector3& mv)
{
    p->centre += mv;
    for(btVector3& v : p->vertices)
        v += mv;

    p->norm[3] = -p->norm.dot(p->vertices.front());
}


/**
 * Барицентрический метод определения пересечения луча и треугольника
 * p - tested portal
 * dir - ray directionа
 * dot - point of ray - plane intersection
 */
bool Portal_RayIntersect(portal_p p, const btVector3& dir, const btVector3& dot)
{
    btScalar u = p->norm.dot(dir);
    if(std::fabs(u) < SPLIT_EPSILON)
    {
        return false;                                                               // плоскость параллельна лучу
    }
    btScalar t = -(p->norm[3] + p->norm.dot(dot));
    t /= u;
    if(0.0 > t)
    {
        return false;                                                               // плоскость не с той стороны луча
    }

    auto* vd = &p->vertices.front();                                                             // Указатель на текущий треугольник
    auto T = dot - *vd;                                                        // Вектор который не меняется для всего полигона

    auto E2 = p->vertices[1]-p->vertices[0];
    for(size_t i=0; i<p->vertices.size()-2; i++, vd++)                             // Обход полигона веером, один из векторов остается прежним
    {
        auto E1 = E2;                                                       // PREV
        E2 = vd[2] - p->vertices[0];                                           // NEXT

        auto P = dir.cross(E2);
        auto Q = T.cross(E1);

        t = P.dot(E1);
        u = P.dot(T);
        u /= t;
        btScalar v = Q.dot(dir);
        v /= t;
        t = 1.0 - u - v;
        if((u <= 1.0) && (u >= 0.0) && (v <= 1.0) && (v >= 0.0) && (t <= 1.0) && (t >= 0.0))
        {
            return true;
        }
    }

    return false;
}

/**
 * Просто генерация нормали к порталу
 */
void Portal_GenNormale(portal_p p)
{
    auto v1 = p->vertices[1] - p->vertices[0];
    auto v2 = p->vertices[2] - p->vertices[1];
    p->norm = v1.cross(v2).normalized();
}
