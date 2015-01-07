
#ifndef PORTAL_H
#define PORTAL_H

#define PORTAL_NORMAL 0x00
#define PORTAL_FICTIVE 0x01

#define SPLIT_EMPTY 0x00
#define SPLIT_SUCCES 0x01

#include <stdint.h>

struct camera_s;
struct room_s;
struct polygon_s;
struct render_s;
struct frustum_s;


/*
 * пока геометрия текущего портала и портала назначения совпадают.
 * далее будет проведена привязка камеры к взаимоориентации порталов
 */
typedef struct portal_s                                                         
{
    uint16_t vertex_count;     
    btScalar *vertex;                                                           // Оригинальные вершины портала
    btScalar norm[4];                                                           // уравнение плоскости оригинальных вершин (оно же нормаль)
    btScalar centre[3];                                                         // центр портала
    struct room_s *dest_room;                                                   // куда ведет портал
    struct room_s *current_room;                                                // комната, где нааходится портал
    unsigned int flag;                                                          // хз, мб потом понадобится
}portal_t, *portal_p;



void Portal_InitGlobals();
portal_p Portal_Create(unsigned int vcount);
void Portal_Clear(portal_p p);

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

// тут пошли реальные нешуточные функции
int Portal_IsOnSectorTop(portal_p p, struct room_sector_s *sector);
int Portal_IsWayToSector(portal_p p, struct room_sector_s *sector);
void Portal_Move(portal_p p, btScalar mv[3]);
int Portal_RayIntersect(portal_p p, btScalar dir[3], btScalar dot[3]);              // проверка на пересечение луча и портала

void Portal_GenNormale(portal_p p);
struct frustum_s* Portal_FrustumIntersect(portal_p portal, struct frustum_s *emitter, struct render_s *render);         // Основная функция для работы с порталами.

#endif   // PORTAL_H