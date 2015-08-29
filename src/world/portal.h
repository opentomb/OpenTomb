#pragma once

#define PORTAL_NORMAL 0x00
#define PORTAL_FICTIVE 0x01

#include <memory>
#include <vector>

#include "util/vmath.h"

namespace world
{
struct Room;
struct RoomSector;

/*
 * пока геометрия текущего портала и портала назначения совпадают.
 * далее будет проведена привязка камеры к взаимоориентации порталов
 */
struct Portal
{
    std::vector<btVector3> vertices;                                                           // Оригинальные вершины портала
    util::Plane normal;                                                           // уравнение плоскости оригинальных вершин (оно же нормаль)
    btVector3 centre = { 0,0,0 };                                                         // центр портала
    std::shared_ptr<Room> dest_room = nullptr;                                                   // куда ведет портал
    std::shared_ptr<Room> current_room;                                                // комната, где нааходится портал

    explicit Portal(size_t vcount = 0)
        : vertices(vcount)
    {
    }

    ~Portal() = default;

    void move(const btVector3 &mv);
    bool rayIntersect(const btVector3 &dir, const btVector3 &point);              // check the intersection of the beam and portal

    void genNormale();
};

} // namespace world
