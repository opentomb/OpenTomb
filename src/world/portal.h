#pragma once

#include "util/vmath.h"

#include <array>
#include <memory>

namespace world
{
struct Room;

/*
 * пока геометрия текущего портала и портала назначения совпадают.
 * далее будет проведена привязка камеры к взаимоориентации порталов
 */
struct Portal
{
    std::array<glm::vec3, 4> vertices;                                                           // Оригинальные вершины портала
    glm::vec3 normal;                                                           // уравнение плоскости оригинальных вершин (оно же нормаль)
    glm::vec3 center = { 0,0,0 };                                                         // центр портала
    std::shared_ptr<Room> dest_room = nullptr;                                                   // куда ведет портал
    std::shared_ptr<Room> current_room;                                                // комната, где нааходится портал
};

} // namespace world
