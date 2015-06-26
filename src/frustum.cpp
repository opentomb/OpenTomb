
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


frustumManager engine_frustumManager;

frustum_p frustumManager::createFrustum()
{
    frustum_p ret = new frustum_s();
    ret->vertices.clear();
    ret->parents_count = 0;
    ret->next = NULL;
    ret->parent = NULL;
    ret->planes.clear();
    ret->cam_pos = NULL;
    ret->norm.setZero();
    return ret;
}

void frustumManager::splitPrepare(frustum_p frustum, struct portal_s *p, frustum_p emitter)
{
    frustum->vertices = p->vertices;
    frustum->norm = -p->norm;
    frustum->parent = NULL;
}

int frustumManager::split_by_plane(frustum_p frustum, const btVector3 &n, std::vector<btVector3>* buf)
{
    btScalar t;

    btVector3* frontVertex = &frustum->vertices.front();
    btVector3* backVertex = &frustum->vertices.back();

    btScalar dist[2];
    dist[0] = planeDist(n, *backVertex);
    buf->clear();
    for(uint16_t i=0;i<frustum->vertices.size();i++)
    {
        dist[1] = planeDist(n, *frontVertex);

        if(dist[1] > SPLIT_EPSILON)
        {
            if(dist[0] < -SPLIT_EPSILON)
            {
                auto dir = *frontVertex - *backVertex;
                btVector3 v;
                rayPlaneIntersect(*backVertex, dir, n, &v, &t);                  // ищем точку пересечения
                buf->emplace_back(v);                                                         // сдвигаем
            }
            buf->emplace_back( *frontVertex );                                               // добавляем
        }
        else if(dist[1] < -SPLIT_EPSILON)
        {
            if(dist[0] > SPLIT_EPSILON)
            {
                auto dir = *frontVertex - *backVertex;
                btVector3 v;
                rayPlaneIntersect(*backVertex, dir, n, &v, &t);                  // ищем точку пересечения
                buf->emplace_back(v);
            }
        }
        else
        {
            buf->emplace_back(*frontVertex);                                               // добавляем
        }

        backVertex = frontVertex;
        ++frontVertex;
        dist[0] = dist[1];
    }

    if(buf->size() <= 2)                                                              // ничего не добавлено или вырождено
    {
        frustum->vertices.clear();
        return SPLIT_EMPTY;
    }

#if 0
    p->vertex_count = added;
    memcpy(p->vertex, buf, added*3*sizeof(btScalar));
#else
    // filter repeating (too closest) points
    frontVertex = &buf->front();
    backVertex = &buf->back();
    auto srcVertexIt = frustum->vertices.begin();
    size_t finalVertexCount = 0;
    for(uint16_t i=0; i<buf->size(); i++)
    {
        if(backVertex->distance2(*frontVertex) > SPLIT_EPSILON * SPLIT_EPSILON)
        {
            *srcVertexIt = *frontVertex;
            ++srcVertexIt;
            ++finalVertexCount;
        }
        backVertex = frontVertex;
        ++frontVertex;
    }

    if(finalVertexCount <= 2)
    {
        frustum->vertices.clear();
        return SPLIT_EMPTY;
    }

    frustum->vertices.resize(finalVertexCount);
#endif
    return SPLIT_SUCCES;
}

void frustumManager::genClipPlanes(frustum_p p, struct camera_s *cam)
{
    if(p->vertices.empty())
        return;

    p->planes.resize(4*p->vertices.size());

    auto next_v = &p->vertices.front();
    auto curr_v = &p->vertices.back();
    auto prev_v = curr_v - 1;

    //==========================================================================

    for(uint16_t i=0; i<p->vertices.size(); i++)
    {
        auto V1 = *prev_v - cam->pos;                                      // вектор от наблюдателя до вершины полигона
        auto V2 = *curr_v - *prev_v;                                        // вектор соединяющий соседние вершины полигона
        V1.normalize();
        V2.normalize();
        p->planes[4*i+0] = V1.cross(V2);
        p->planes[4*i+0].normalize();
        p->planes[4*i+3] = -(p->planes[4*i+0] * (*curr_v)[0]);
        p->planes[4*i+0] = -p->planes[4*i+0];

        prev_v = curr_v;
        curr_v = next_v;
        ++next_v;
    }

    *p->cam_pos = cam->pos;
}

/*
 * receiver - указатель на базовый фрустум рума, куда ведет портал - берется из портала!!!
 * возвращает указатель на свежесгенеренный фрустум
 */
frustum_p frustumManager::portalFrustumIntersect(portal_s *portal, frustum_p emitter, struct render_s *render)
{
    if(planeDist(portal->norm, render->cam->pos) < -SPLIT_EPSILON)    // non face or degenerate to the line portal
    {
        return NULL;
    }

    if((portal->dest_room->frustum != NULL) && Frustum_HaveParent(portal->dest_room->frustum, emitter))
    {
        return NULL;                                                        // abort infinite cycling!
    }

    bool in_dist = false, in_face = false;
    for(const btVector3& v : portal->vertices)
    {
        if(!in_dist && (planeDist(render->cam->frustum->norm, v) < render->cam->dist_far))
        {
            in_dist = true;
        }
        if(!in_face && (planeDist(emitter->norm, v) > 0.0))
        {
            in_face = true;
        }
        if(in_dist && in_face)
            break;
    }

    if(!in_dist || !in_face)
    {
        return NULL;
    }

    /*
     * Search for the first free room's frustum
     */
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

    this->splitPrepare(current_gen, portal, emitter);                       // prepare to the clipping

    std::vector<btVector3> tmp;
    tmp.reserve(current_gen->vertices.size() + emitter->vertices.size() + 4);
    if(this->split_by_plane(current_gen, emitter->norm, &tmp))               // splitting by main frustum clip plane
    {
        for(size_t i=0; i<emitter->vertices.size(); i++)
        {
            const auto& n = emitter->planes[i];
            if(!this->split_by_plane(current_gen, n, &tmp))
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
        }

        this->genClipPlanes(current_gen, render->cam);                      // all is OK, let us generate clipplanes

        current_gen->parent = emitter;                                      // add parent pointer
        current_gen->parents_count = emitter->parents_count + 1;
        portal->dest_room->active_frustums++;
        if(portal->dest_room->max_path < current_gen->parents_count)
        {
            portal->dest_room->max_path = current_gen->parents_count;       // maximum path to the room
        }
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
bool Frustum_IsPolyVisible(struct polygon_s *p, struct frustum_s *frustum)
{
    if(planeDist(p->plane, *frustum->cam_pos) < 0.0)
    {
        return false;
    }

    auto dir = frustum->vertices[0] - *frustum->cam_pos;                            // направление от позици камеры до произвольной вершины фрустума
    btScalar t;
    if(Polygon_RayIntersect(p, dir, *frustum->cam_pos, &t))                      // полигон вмещает фрустум портала (бреед, но проверить надо)
    {
        return true;
    }

    btVector3* next_n = &frustum->planes.front();                                                   // генерим очередь проверки
    btVector3* curr_n = &frustum->planes.back();                     // 3 соседних плоскости отсечения
    btVector3* prev_n = curr_n - 1;                                                        //
    bool ins = true;                                                                    // на случай если нет пересечений
    for(size_t i=0; i<frustum->vertices.size(); i++)                               // перебираем все плоскости текущего фрустума
    {
        vertex_s* curr_v = &p->vertices.front();                                                   // генерим очередь вершин под проверку
        vertex_s* prev_v = &p->vertices.back();                             //
        btScalar dist0 = planeDist(*curr_n, prev_v->position);                    // расстояние со знаком от текущей точки до предыдущей плоскости
        bool outs = true;
        for(size_t j=0; j<p->vertices.size(); j++)                                 // перебираем все вершины полигона
        {
            btScalar dist1 = planeDist(*curr_n, curr_v->position);
            if(std::fabs(dist0) < SPLIT_EPSILON)                                    // точка на плоскости отсечения
            {
                if((planeDist(*prev_n, prev_v->position) > -SPLIT_EPSILON) &&
                   (planeDist(*next_n, prev_v->position) > -SPLIT_EPSILON) &&
                   (planeDist(frustum->norm, prev_v->position) > -SPLIT_EPSILON))
                {
                    return true;                                                   // прошли проверку на пересечение вершины многоугльника и фрустума
                }
            }

            if((dist0 * dist1 < 0) && std::fabs(dist1) >= SPLIT_EPSILON)        // вершины с разных сторон плоскости (или на ней)
            {
                dir = curr_v->position - prev_v->position;               // вектор, соединяющий вершины
                btVector3 T;
                rayPlaneIntersect(prev_v->position, dir, *curr_n, &T, &t);   // ищем точку пересечения
                if((planeDist(*prev_n, T) > -SPLIT_EPSILON) && (planeDist(*next_n, T) > -SPLIT_EPSILON))
                {
                    return true;                                                   // прошли проверку на пересечение отрезка многоугльника и фрустума
                }
            }

            if(dist1 < -SPLIT_EPSILON)                                        // точка снаружи
            {
                ins = false;
            }
            else
            {
                outs = false;
            }

            prev_v = curr_v;                                                    // сдвинули очередь вершин полигона
            ++curr_v;                                                          //
            dist0 = dist1;                                                  // сдвинули очередь дистанций
        }                                                                       // закончили переборку вершин полигона

        if(outs)
        {
            return false;                                                           // все точки снаружи относительно текущей плоскости - однозначно выход
        }
        prev_n = curr_n;                                                        // сдвинули очередь плоскостей отсечения
        curr_n = next_n;
        ++next_n;
    }                                                                           // закончили перебирать все плоскости текущего фрустума
    if(ins)
    {
        return true;                                                               // все вершины внутренние - тест пройден
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
bool Frustum_IsAABBVisible(const btVector3& bbmin, const btVector3& bbmax, struct frustum_s *frustum)
{
    polygon_t poly;
    poly.vertices.resize(4);
    bool ins = true;

    /* X_AXIS */

    poly.plane[1] = 0.0;
    poly.plane[2] = 0.0;
    if((*frustum->cam_pos)[0] < bbmin[0])
    {
        poly.plane[0] = -1.0;
        poly.plane[3] = bbmin[0];
        poly.vertices[0].position[0] = bbmin[0];
        poly.vertices[0].position[1] = bbmax[1];
        poly.vertices[0].position[2] = bbmax[2];

        poly.vertices[1].position[0] = bbmin[0];
        poly.vertices[1].position[1] = bbmin[1];
        poly.vertices[1].position[2] = bbmax[2];

        poly.vertices[2].position[0] = bbmin[0];
        poly.vertices[2].position[1] = bbmin[1];
        poly.vertices[2].position[2] = bbmin[2];

        poly.vertices[3].position[0] = bbmin[0];
        poly.vertices[3].position[1] = bbmax[1];
        poly.vertices[3].position[2] = bbmin[2];

        if(Frustum_IsPolyVisible(&poly, frustum))
        {
            return true;
        }
        ins = false;
    }
    else if((*frustum->cam_pos)[0] > bbmax[0])
    {
        poly.plane[0] = 1.0;
        poly.plane[3] =-bbmax[0];
        poly.vertices[0].position[0] = bbmax[0];
        poly.vertices[0].position[1] = bbmax[1];
        poly.vertices[0].position[2] = bbmax[2];

        poly.vertices[1].position[0] = bbmax[0];
        poly.vertices[1].position[1] = bbmin[1];
        poly.vertices[1].position[2] = bbmax[2];

        poly.vertices[2].position[0] = bbmax[0];
        poly.vertices[2].position[1] = bbmin[1];
        poly.vertices[2].position[2] = bbmin[2];

        poly.vertices[3].position[0] = bbmax[0];
        poly.vertices[3].position[1] = bbmax[1];
        poly.vertices[3].position[2] = bbmin[2];

        if(Frustum_IsPolyVisible(&poly, frustum))
        {
            return true;
        }
        ins = false;
    }

    /* Y AXIS */

    poly.plane[0] = 0.0;
    poly.plane[2] = 0.0;
    if((*frustum->cam_pos)[1] < bbmin[1])
    {
        poly.plane[1] = -1.0;
        poly.plane[3] = bbmin[1];
        poly.vertices[0].position[0] = bbmax[0];
        poly.vertices[0].position[1] = bbmin[1];
        poly.vertices[0].position[2] = bbmax[2];

        poly.vertices[1].position[0] = bbmin[0];
        poly.vertices[1].position[1] = bbmin[1];
        poly.vertices[1].position[2] = bbmax[2];

        poly.vertices[2].position[0] = bbmin[0];
        poly.vertices[2].position[1] = bbmin[1];
        poly.vertices[2].position[2] = bbmin[2];

        poly.vertices[3].position[0] = bbmax[0];
        poly.vertices[3].position[1] = bbmin[1];
        poly.vertices[3].position[2] = bbmin[2];

        if(Frustum_IsPolyVisible(&poly, frustum))
        {
            return true;
        }
        ins = false;
    }
    else if((*frustum->cam_pos)[1] > bbmax[1])
    {
        poly.plane[1] = 1.0;
        poly.plane[3] = -bbmax[1];
        poly.vertices[0].position[0] = bbmax[0];
        poly.vertices[0].position[1] = bbmax[1];
        poly.vertices[0].position[2] = bbmax[2];

        poly.vertices[1].position[0] = bbmin[0];
        poly.vertices[1].position[1] = bbmax[1];
        poly.vertices[1].position[2] = bbmax[2];

        poly.vertices[2].position[0] = bbmin[0];
        poly.vertices[2].position[1] = bbmax[1];
        poly.vertices[2].position[2] = bbmin[2];

        poly.vertices[3].position[0] = bbmax[0];
        poly.vertices[3].position[1] = bbmax[1];
        poly.vertices[3].position[2] = bbmin[2];

        if(Frustum_IsPolyVisible(&poly, frustum))
        {
            return true;
        }
        ins = false;
    }

    /* Z AXIS */

    poly.plane[0] = 0.0;
    poly.plane[1] = 0.0;
    if((*frustum->cam_pos)[2] < bbmin[2])
    {
        poly.plane[2] = -1.0;
        poly.plane[3] = bbmin[2];
        poly.vertices[0].position[0] = bbmax[0];
        poly.vertices[0].position[1] = bbmax[1];
        poly.vertices[0].position[2] = bbmin[2];

        poly.vertices[1].position[0] = bbmin[0];
        poly.vertices[1].position[1] = bbmax[1];
        poly.vertices[1].position[2] = bbmin[2];

        poly.vertices[2].position[0] = bbmin[0];
        poly.vertices[2].position[1] = bbmin[1];
        poly.vertices[2].position[2] = bbmin[2];

        poly.vertices[3].position[0] = bbmax[0];
        poly.vertices[3].position[1] = bbmin[1];
        poly.vertices[3].position[2] = bbmin[2];

        if(Frustum_IsPolyVisible(&poly, frustum))
        {
            return true;
        }
        ins = false;
    }
    else if((*frustum->cam_pos)[2] > bbmax[2])
    {
        poly.plane[2] = 1.0;
        poly.plane[3] = -bbmax[2];
        poly.vertices[0].position[0] = bbmax[0];
        poly.vertices[0].position[1] = bbmax[1];
        poly.vertices[0].position[2] = bbmax[2];

        poly.vertices[1].position[0] = bbmin[0];
        poly.vertices[1].position[1] = bbmax[1];
        poly.vertices[1].position[2] = bbmax[2];

        poly.vertices[2].position[0] = bbmin[0];
        poly.vertices[2].position[1] = bbmin[1];
        poly.vertices[2].position[2] = bbmax[2];

        poly.vertices[3].position[0] = bbmax[0];
        poly.vertices[3].position[1] = bbmin[1];
        poly.vertices[3].position[2] = bbmax[2];

        if(Frustum_IsPolyVisible(&poly, frustum))
        {
            return true;
        }
        ins = false;
    }

    return ins;
}


bool Frustum_IsOBBVisible(struct obb_s *obb, struct frustum_s *frustum)
{
    bool ins = true;
    polygon_p p = obb->polygons;
    for(int i=0;i<6;i++,p++)
    {
        auto t = planeDist(p->plane, *frustum->cam_pos);
        if((t > 0.0) && Frustum_IsPolyVisible(p, frustum))
        {
            return true;
        }
        if(ins && (t > 0))
        {
            ins = false;
        }
    }

    return ins;
}

bool Frustum_IsOBBVisibleInRoom(struct obb_s *obb, std::shared_ptr<Room> room)
{
    auto frustum = room->frustum;
    if(frustum == NULL)                                                         // В комнате нет активного фрустума, значит применяем фрустум камеры
    {
        bool ins = true;                                                                // считаем, что камера внутри OBB
        auto p = obb->polygons;
        for(int i=0;i<6;i++,p++)
        {
            auto t = planeDist(p->plane, engine_camera.pos);
            if((t > 0.0) && Frustum_IsPolyVisible(p, engine_camera.frustum))
            {
                return true;
            }
            if(ins && (t > 0.0))                                                // проверка на принадлежность точки наблюдателя OBB
            {
                ins = false;                                                        // хоть один провал проверки - и камера не может быть внутри
            }
        }
        return ins;                                                             // если камера внутри OBB объекта, то объект виден
    }

    for(;frustum;frustum=frustum->next)                                         // Если хоть в одном активном фрустуме виден объект, то возвращаем 1
    {
        auto p = obb->polygons;
        for(int i=0;i<6;i++,p++)
        {
            auto t = planeDist(p->plane, *frustum->cam_pos);
            if((t > 0.0) && Frustum_IsPolyVisible(p, frustum))
            {
                return true;
            }
        }
    }

    return false;
}

