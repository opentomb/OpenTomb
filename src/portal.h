#pragma once

#define PORTAL_NORMAL 0x00
#define PORTAL_FICTIVE 0x01

#define SPLIT_EMPTY 0x00
#define SPLIT_SUCCES 0x01

#include <cstdint>
#include <memory>
#include <vector>

#include "vmath.h"

struct Room;
struct RoomSector;

/*
 * пока геометрия текущего портала и портала назначения совпадают.
 * далее будет проведена привязка камеры к взаимоориентации порталов
 */
struct Portal
{
    std::vector<btVector3> vertices;                                                           // Оригинальные вершины портала
    btVector3 norm;                                                           // уравнение плоскости оригинальных вершин (оно же нормаль)
    btVector3 centre;                                                         // центр портала
    std::shared_ptr<Room> dest_room = nullptr;                                                   // куда ведет портал
    std::shared_ptr<Room> current_room;                                                // комната, где нааходится портал
    unsigned int flag = 0;                                                          // хз, мб потом понадобится

    Portal(size_t vcount = 0)
        : vertices(vcount)
    {
    }

    ~Portal() = default;

    bool isOnSectorTop(RoomSector *sector) const;
    bool isWayToSector(RoomSector *sector) const;
    void move(const btVector3 &mv);
    bool rayIntersect(const btVector3 &dir, const btVector3 &dot);              // проверка на пересечение луча и портала

    void genNormale();
};


/**
 * Draws wireframe of this portal.
 *
 * Expected state:
 *  - Vertex array is enabled, color, tex coord, normal disabled
 *  - No vertex buffer object is bound
 *  - Texturing is disabled
 *  - Alpha test is disabled
 *  - Blending is enabled
 *  - Lighting is disabled
 *  - Line width is set to desired width (typically 3.0)
 *  - Current color set to desired color (typically black)
 * Ignored state:
 *  - Currently bound texture.
 *  - Currently bound element buffer.
 *  - Vertex pointer (changes it)
 * Changed state:
 *  - Current position will be arbitrary.
 *  - Vertex pointer will be arbitray.
 */
