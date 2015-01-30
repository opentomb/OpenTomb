
#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <stdint.h>
#include "bullet/LinearMath/btScalar.h"

struct room_s;
struct portal_s;
struct obb_s;

typedef struct frustum_s
{
    uint16_t            count;                                                  // количество вершин фрустума
    int8_t              active;                                                 // статус фрустума.

    btScalar            *planes;                                                // плоскости отсечения
    btScalar            *vertex;                                                // вершины портала
    btScalar            *cam_pos;
    btScalar            norm[4];                                                // нормаль фрустума

    uint16_t            parents_count;
    struct frustum_s    *parent;                                                // кем был сгенерирован фрустум. если NULL - то камерой.
    struct frustum_s    *next;                                                  // следующий по списку фрустум
}frustum_t, *frustum_p;


frustum_p Frustum_Create();                                                     // выделить память под фрустум
void Frustum_Delete(frustum_p p);                                               // удалить фрустум и его компоненты
void Frustum_Copy(frustum_p p, frustum_p src);                                  // скопировать фрустум и его компоненты

/**
 * Draws wireframe of this frustum.
 *
 * Expected state:
 *  - Vertex array is enabled, color, tex coord, normal disabled
 *  - No vertex buffer object is bound
 *  - Texturing is disabled
 *  - Alpha test is disabled
 *  - Blending is enabled
 *  - Lighting is disabled
 *  - Line width is set to desired width (typically 3.0)
 *  - Current color set to desired color (typically red)
 * Ignored state:
 *  - Currently bound texture.
 *  - Currently bound element buffer.
 *  - Vertex pointer (changes it)
 * Changed state:
 *  - Current position will be arbitrary.
 *  - Vertex pointer will be arbitray.
 */

int Frustum_GetFrustumsCount(struct frustum_s *f);
int Frustum_HaveParent(frustum_p parent, frustum_p frustum);                    // ести ли у фрустума хоть один родитель из цепочки фрустумов parent
void Frustum_SplitPrepare(frustum_p frustum, struct portal_s *p);               // подготовка фрустума к сплиту
int Frustum_Split(frustum_p p, btScalar n[4], btScalar *buf);                   // отсечение части портала плоскостью

void Frustum_GenClipPlanes(frustum_p p, struct camera_s *cam);                  // генерация плоскостей отсечения

int Frustum_IsPolyVisible(struct polygon_s *p, struct frustum_s *frustum);
int Frustum_IsAABBVisible(btScalar bbmin[3], btScalar bbmax[3], struct frustum_s *frustum);
int Frustum_IsOBBVisible(struct obb_s *obb, struct frustum_s *frustum);
int Frustum_IsOBBVisibleInRoom(struct obb_s *obb, struct room_s *room);

#endif
