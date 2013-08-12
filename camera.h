
#ifndef CAMERA_H
#define CAMERA_H

struct room_s;
struct polygon_s;
struct frustum_s;

#include "bullet/LinearMath/btScalar.h"

typedef struct camera_s
{
    btScalar                    pos[3];                                         // положение камеры
    btScalar                    view_dir[4];                                    // вектор направления камеры
    btScalar                    up_dir[4];                                      // вектор направления вверх
    btScalar                    right_dir[4];                                   // вектор направления вбок
    btScalar                    ang[3];                                         // ориентация камеры
    
    btScalar                    clip_planes[16];                                // плоскости отсечения фрустума
    struct frustum_s            *frustum;                                       // указатель
    
    btScalar                    dist_near;
    btScalar                    dist_far;
    
    btScalar                    fov;
    btScalar                    aspect;
    btScalar                    h;
    btScalar                    w;
    
    struct room_s               *current_room;
}camera_t, *camera_p;

void Cam_Init(camera_p cam);                                                    // сброс параметров + пересчет клиппланов
void Cam_Apply(camera_p cam);                                                   // применение камеры к пространству OpenGL
void Cam_SetFovAspect(camera_p cam, btScalar fov, btScalar aspect);
void Cam_MoveAlong(camera_p cam, btScalar dist);
void Cam_MoveStrafe(camera_p cam, btScalar dist);
void Cam_MoveVertical(camera_p cam, btScalar dist);
void Cam_DeltaRotation(camera_p cam, btScalar angles[3]);                         // поворот из текущего состояния на углы из массива angles
void Cam_SetRotation(camera_p cam, btScalar angles[3]);                           // задание начальной ориентации
void Cam_RecalcClipPlanes(camera_p cam);                                        // пересчет плоскостей отсечения

#endif
