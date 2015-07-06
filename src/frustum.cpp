
#include <cstdio>
#include <cstdlib>
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


void Frustum::splitPrepare(struct Portal *p)
{
    vertices = p->vertices;
    norm = -p->norm;
    parent.reset();
}

int Frustum::split_by_plane(const btVector3 &n, std::vector<btVector3>* buf)
{
    btScalar t;

    btVector3* frontVertex = &vertices.front();
    btVector3* backVertex = &vertices.back();

    btScalar dist[2];
    dist[0] = planeDist(n, *backVertex);
    buf->clear();
    for(uint16_t i=0;i<vertices.size();i++)
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
        vertices.clear();
        return SPLIT_EMPTY;
    }

#if 0
    p->vertex_count = added;
    memcpy(p->vertex, buf, added*3*sizeof(btScalar));
#else
    // filter repeating (too closest) points
    frontVertex = &buf->front();
    backVertex = &buf->back();
    auto srcVertexIt = vertices.begin();
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
        vertices.clear();
        return SPLIT_EMPTY;
    }

    vertices.resize(finalVertexCount);
#endif
    return SPLIT_SUCCES;
}

void Frustum::genClipPlanes(Camera *cam)
{
    if(vertices.empty())
        return;

    planes.resize(4*vertices.size());

    auto next_v = &vertices.front();
    auto curr_v = &vertices.back();
    auto prev_v = curr_v - 1;

    //==========================================================================

    for(uint16_t i=0; i<vertices.size(); i++)
    {
        auto V1 = *prev_v - cam->m_pos;                                      // вектор от наблюдателя до вершины полигона
        auto V2 = *curr_v - *prev_v;                                        // вектор соединяющий соседние вершины полигона
        V1.normalize();
        V2.normalize();
        planes[4*i+0] = V1.cross(V2).normalized();
        planes[4*i+3] = -(planes[4*i+0] * (*curr_v)[0]);
        planes[4*i+0] = -planes[4*i+0];

        prev_v = curr_v;
        curr_v = next_v;
        ++next_v;
    }

    *cam_pos = cam->m_pos;
}

/*
 * receiver - указатель на базовый фрустум рума, куда ведет портал - берется из портала!!!
 * возвращает указатель на свежесгенеренный фрустум
 */
std::shared_ptr<Frustum> Frustum::portalFrustumIntersect(Portal *portal, const std::shared_ptr<Frustum>& emitter, Render *render)
{
    if(!portal->dest_room)
        return nullptr;

    if(planeDist(portal->norm, render->camera()->m_pos) < -SPLIT_EPSILON)    // non face or degenerate to the line portal
    {
        return nullptr;
    }

    if(!portal->dest_room->frustum.empty() && emitter->hasParent(portal->dest_room->frustum.front()))
    {
        return nullptr;                                                        // abort infinite cycling!
    }

    bool in_dist = false, in_face = false;
    for(const btVector3& v : portal->vertices)
    {
        if(!in_dist && (planeDist(render->camera()->frustum->norm, v) < render->camera()->m_distFar))
            in_dist = true;
        if(!in_face && (planeDist(emitter->norm, v) > 0.0))
            in_face = true;
        if(in_dist && in_face)
            break;
    }

    if(!in_dist || !in_face)
        return nullptr;

    /*
     * Search for the first free room's frustum
     */
    portal->dest_room->frustum.emplace_back(std::make_shared<Frustum>());
    auto current_gen = portal->dest_room->frustum.back();

    current_gen->splitPrepare(portal);                       // prepare to the clipping

    std::vector<btVector3> tmp;
    tmp.reserve(current_gen->vertices.size() + emitter->vertices.size() + 4);
    if(current_gen->split_by_plane(emitter->norm, &tmp))               // splitting by main frustum clip plane
    {
        for(size_t i=0; i<emitter->vertices.size(); i++)
        {
            const auto& n = emitter->planes[i];
            if(!current_gen->split_by_plane(n, &tmp))
            {
                portal->dest_room->frustum.pop_back();
                return NULL;
            }
        }

        current_gen->genClipPlanes(render->camera());                      // all is OK, let us generate clipplanes

        current_gen->parent = emitter;                                      // add parent pointer
        current_gen->parents_count = emitter->parents_count + 1;
        portal->dest_room->active_frustums++;
        if(portal->dest_room->max_path < current_gen->parents_count)
        {
            portal->dest_room->max_path = current_gen->parents_count;       // maximum path to the room
        }
        return current_gen;
    }

    portal->dest_room->frustum.pop_back();

    return nullptr;
}

/*
 ************************* END FRUSTUM MANAGER IMPLEMENTATION*******************
 */

/**
 * ф-я разрыватель замкнутых реккурсий
 * если в комнате есть фрустум, породивший текущий, то возвращаем 1
 * и тогда порочный цикл рвется
 */
bool Frustum::hasParent(const std::shared_ptr<Frustum>& parent)
{
    auto frustum = this;
    while(frustum) {
        if(parent.get() == frustum)
            return true;
        frustum = frustum->parent.lock().get();
    }
    return false;
}


/**
 * Проверка полигона на видимость через портал.
 * данный метод НЕ для реалтайма, т.к. проверка в общем случае выходит дороже отрисовки...
 */
bool Frustum::isPolyVisible(struct Polygon *p)
{
    if(planeDist(p->plane, *cam_pos) < 0.0)
    {
        return false;
    }

    auto dir = vertices[0] - *cam_pos;                            // направление от позици камеры до произвольной вершины фрустума
    btScalar t;
    if(p->rayIntersect(dir, *cam_pos, &t))                      // полигон вмещает фрустум портала (бреед, но проверить надо)
    {
        return true;
    }

    btVector3* next_n = &planes.front();                                                   // генерим очередь проверки
    btVector3* curr_n = &planes.back();                     // 3 соседних плоскости отсечения
    btVector3* prev_n = curr_n - 1;                                                        //
    bool ins = true;                                                                    // на случай если нет пересечений
    for(size_t i=0; i<vertices.size(); i++)                               // перебираем все плоскости текущего фрустума
    {
        Vertex* curr_v = &p->vertices.front();                                                   // генерим очередь вершин под проверку
        Vertex* prev_v = &p->vertices.back();                             //
        btScalar dist0 = planeDist(*curr_n, prev_v->position);                    // расстояние со знаком от текущей точки до предыдущей плоскости
        bool outs = true;
        for(size_t j=0; j<p->vertices.size(); j++)                                 // перебираем все вершины полигона
        {
            btScalar dist1 = planeDist(*curr_n, curr_v->position);
            if(std::fabs(dist0) < SPLIT_EPSILON)                                    // точка на плоскости отсечения
            {
                if((planeDist(*prev_n, prev_v->position) > -SPLIT_EPSILON) &&
                   (planeDist(*next_n, prev_v->position) > -SPLIT_EPSILON) &&
                   (planeDist(norm, prev_v->position) > -SPLIT_EPSILON))
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
bool Frustum::isAABBVisible(const btVector3& bbmin, const btVector3& bbmax)
{
    struct Polygon poly;
    poly.vertices.resize(4);
    bool ins = true;

    /* X_AXIS */

    poly.plane[1] = 0.0;
    poly.plane[2] = 0.0;
    if((*cam_pos)[0] < bbmin[0])
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

        if(isPolyVisible(&poly))
        {
            return true;
        }
        ins = false;
    }
    else if((*cam_pos)[0] > bbmax[0])
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

        if(isPolyVisible(&poly))
        {
            return true;
        }
        ins = false;
    }

    /* Y AXIS */

    poly.plane[0] = 0.0;
    poly.plane[2] = 0.0;
    if((*cam_pos)[1] < bbmin[1])
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

        if(isPolyVisible(&poly))
        {
            return true;
        }
        ins = false;
    }
    else if((*cam_pos)[1] > bbmax[1])
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

        if(isPolyVisible(&poly))
        {
            return true;
        }
        ins = false;
    }

    /* Z AXIS */

    poly.plane[0] = 0.0;
    poly.plane[1] = 0.0;
    if((*cam_pos)[2] < bbmin[2])
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

        if(isPolyVisible(&poly))
        {
            return true;
        }
        ins = false;
    }
    else if((*cam_pos)[2] > bbmax[2])
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

        if(isPolyVisible(&poly))
        {
            return true;
        }
        ins = false;
    }

    return ins;
}


bool Frustum::isOBBVisible(OBB *obb)
{
    bool ins = true;
    struct Polygon *p = obb->polygons;
    for(int i=0;i<6;i++,p++)
    {
        auto t = planeDist(p->plane, *cam_pos);
        if((t > 0.0) && isPolyVisible(p))
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

bool Frustum::isOBBVisibleInRoom(OBB *obb, const Room& room)
{
    if(room.frustum.empty())                                                         // В комнате нет активного фрустума, значит применяем фрустум камеры
    {
        bool ins = true;                                                                // считаем, что камера внутри OBB
        auto p = obb->polygons;
        for(int i=0;i<6;i++,p++)
        {
            auto t = planeDist(p->plane, engine_camera.m_pos);
            if((t > 0.0) && engine_camera.frustum->isPolyVisible(p))
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

    for(const auto& frustum : room.frustum) {
        auto p = obb->polygons;
        for(int i=0;i<6;i++,p++) {
            auto t = planeDist(p->plane, *frustum->cam_pos);
            if((t > 0.0) && frustum->isPolyVisible(p))
            {
                return true;
            }
        }
    }

    return false;
}

