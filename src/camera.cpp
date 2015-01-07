
#include <SDL2/SDL_opengl.h>
#include <stdlib.h>
#include <math.h>

#include "gl_util.h"
#include "camera.h"
#include "vmath.h"
#include "polygon.h"
#include "frustum.h"


void Cam_Init(camera_p cam)
{
    cam->aspect = 1.0;
    cam->fov = 75.0;
    cam->dist_near = 1.0;
    cam->dist_far = 16384.0;

    cam->h = 2.0 * cam->dist_near * tan( M_PI * cam->fov / 360.0);
    cam->w = cam->h * cam->aspect;

    cam->frustum = Frustum_Create();
    cam->frustum->active = 1;
    cam->frustum->cam_pos = cam->pos;
    cam->frustum->count = 4;
    cam->frustum->next = NULL;
    cam->frustum->parent = NULL;
    cam->frustum->parents_count = 0;
    cam->frustum->vertex = NULL;
    cam->frustum->planes = cam->clip_planes;
    cam->frustum->vertex = (btScalar*)malloc(3*4*sizeof(btScalar));

    cam->pos[0] = 0.0;
    cam->pos[1] = 0.0;http://i.imgur.com/RYc9qWE.png
    cam->pos[2] = 0.0;

    cam->prev_pos[0] = 0.0;
    cam->prev_pos[1] = 0.0;
    cam->prev_pos[2] = 0.0;

    cam->view_dir[0] = 0.0;                                                     // OZ - view
    cam->view_dir[1] = 0.0;
    cam->view_dir[2] = 1.0;
    cam->view_dir[3] = 0.0;

    cam->right_dir[0] = 1.0;                                                    // OX - right
    cam->right_dir[1] = 0.0;
    cam->right_dir[2] = 0.0;
    cam->right_dir[3] = 0.0;

    cam->up_dir[0] = 0.0;                                                       // OY - up
    cam->up_dir[1] = 1.0;
    cam->up_dir[2] = 0.0;
    cam->up_dir[3] = 0.0;

    cam->shake_value    = 0.0;
    cam->shake_time     = 0.0;

    cam->current_room = NULL;
}

void Cam_Apply(camera_p cam)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(cam->fov, cam->aspect, cam->dist_near, cam->dist_far);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(cam->pos[0], cam->pos[1], cam->pos[2],                                                             // eye
              cam->pos[0]+cam->view_dir[0], cam->pos[1]+cam->view_dir[1], cam->pos[2]+cam->view_dir[2],          // centre
              cam->up_dir[0], cam->up_dir[1], cam->up_dir[2]);                                                   // UP dir
}

void Cam_SetFovAspect(camera_p cam, btScalar fov, btScalar aspect)
{
    cam->fov = fov;
    cam->aspect = aspect;
    cam->h = 2.0 * cam->dist_near * tan( M_PI * cam->fov / 360.0);
    cam->w = cam->h * aspect;
}

void Cam_MoveAlong(camera_p cam, btScalar dist)
{
    cam->pos[0] += cam->view_dir[0] * dist;
    cam->pos[1] += cam->view_dir[1] * dist;
    cam->pos[2] += cam->view_dir[2] * dist;
}

void Cam_MoveStrafe(camera_p cam, btScalar dist)
{
    cam->pos[0] += cam->right_dir[0] * dist;
    cam->pos[1] += cam->right_dir[1] * dist;
    cam->pos[2] += cam->right_dir[2] * dist;
}

void Cam_MoveVertical(camera_p cam, btScalar dist)
{
    cam->pos[0] += cam->up_dir[0] * dist;
    cam->pos[1] += cam->up_dir[1] * dist;
    cam->pos[2] += cam->up_dir[2] * dist;
}

void Cam_Shake(camera_p cam, btScalar power, btScalar time)
{
    cam->shake_value = power;
    cam->shake_time  = time;
}

void Cam_DeltaRotation(camera_p cam, btScalar angles[3])                        //angles = {OX, OY, OZ}
{
    btScalar R[4], Rt[4], temp[4];
    btScalar sin_t2, cos_t2, t;

    vec3_add(cam->ang, cam->ang, angles)

    t = -angles[2] / 2.0;                                                       // ROLL
    sin_t2 = sin(t);
    cos_t2 = cos(t);
    R[3] = cos_t2;
    R[0] = cam->view_dir[0] * sin_t2;
    R[1] = cam->view_dir[1] * sin_t2;
    R[2] = cam->view_dir[2] * sin_t2;
    vec4_sop(Rt, R)

    vec4_mul(temp, R, cam->right_dir)
    vec4_mul(cam->right_dir, temp, Rt)
    vec4_mul(temp, R, cam->up_dir)
    vec4_mul(cam->up_dir, temp, Rt)

    t = -angles[0] / 2.0;                                                       // LEFT - RIGHT
    sin_t2 = sin(t);
    cos_t2 = cos(t);
    R[3] = cos_t2;
    R[0] = cam->up_dir[0] * sin_t2;
    R[1] = cam->up_dir[1] * sin_t2;
    R[2] = cam->up_dir[2] * sin_t2;
    vec4_sop(Rt, R)

    vec4_mul(temp, R, cam->right_dir)
    vec4_mul(cam->right_dir, temp, Rt)
    vec4_mul(temp, R, cam->view_dir)
    vec4_mul(cam->view_dir, temp, Rt)

    t = angles[1] / 2.0;                                                        // UP - DOWN
    sin_t2 = sin(t);
    cos_t2 = cos(t);
    R[3] = cos_t2;
    R[0] = cam->right_dir[0] * sin_t2;
    R[1] = cam->right_dir[1] * sin_t2;
    R[2] = cam->right_dir[2] * sin_t2;
    vec4_sop(Rt, R)

    vec4_mul(temp, R, cam->view_dir)
    vec4_mul(cam->view_dir, temp, Rt)
    vec4_mul(temp, R, cam->up_dir)
    vec4_mul(cam->up_dir, temp, Rt)
}

void Cam_SetRotation(camera_p cam, btScalar angles[3])                          //angles = {OX, OY, OZ}
{
    btScalar R[4], Rt[4], temp[4];
    btScalar sin_t2, cos_t2, t;

    vec3_copy(cam->ang, angles);

    sin_t2 = sin(angles[0]);
    cos_t2 = cos(angles[0]);

    /*
     * LEFT - RIGHT INIT
     */
    cam->view_dir[0] =-sin_t2;                                                  // OY - view
    cam->view_dir[1] = cos_t2;
    cam->view_dir[2] = 0.0;
    cam->view_dir[3] = 0.0;

    cam->right_dir[0] = cos_t2;                                                 // OX - right
    cam->right_dir[1] = sin_t2;
    cam->right_dir[2] = 0.0;
    cam->right_dir[3] = 0.0;

    cam->up_dir[0] = 0.0;                                                       // OZ - up
    cam->up_dir[1] = 0.0;
    cam->up_dir[2] = 1.0;
    cam->up_dir[3] = 0.0;

    t = angles[1] / 2.0;                                                        // UP - DOWN
    sin_t2 = sin(t);
    cos_t2 = cos(t);
    R[3] = cos_t2;
    R[0] = cam->right_dir[0] * sin_t2;
    R[1] = cam->right_dir[1] * sin_t2;
    R[2] = cam->right_dir[2] * sin_t2;
    vec4_sop(Rt, R);

    vec4_mul(temp, R, cam->up_dir);
    vec4_mul(cam->up_dir, temp, Rt);
    vec4_mul(temp, R, cam->view_dir);
    vec4_mul(cam->view_dir, temp, Rt);

    t = angles[2] / 2.0;                                                        // ROLL
    sin_t2 = sin(t);
    cos_t2 = cos(t);
    R[3] = cos_t2;
    R[0] = cam->view_dir[0] * sin_t2;
    R[1] = cam->view_dir[1] * sin_t2;
    R[2] = cam->view_dir[2] * sin_t2;
    vec4_sop(Rt, R);

    vec4_mul(temp, R, cam->right_dir);
    vec4_mul(cam->right_dir, temp, Rt);
    vec4_mul(temp, R, cam->up_dir);
    vec4_mul(cam->up_dir, temp, Rt);
}

void Cam_RecalcClipPlanes(camera_p cam)
{
    btScalar T[4], LU[4], V[3], *n = cam->clip_planes;

    V[0] = cam->view_dir[0] * cam->dist_near;                                   // вектор - от позиции камеры до центра плоскости проекции
    V[1] = cam->view_dir[1] * cam->dist_near;
    V[2] = cam->view_dir[2] * cam->dist_near;

    T[0] = cam->pos[0] + V[0];                                                  // центр плоскости проекции
    T[1] = cam->pos[1] + V[1];
    T[2] = cam->pos[2] + V[2];

    //==========================================================================

    vec3_copy(cam->frustum->norm, cam->view_dir);                               // Основная плоскость отсечения (что сзади - то не рисуем)
    cam->frustum->norm[3] = -vec3_dot(cam->view_dir, cam->pos);                 // плоскость проекции проходит через наблюдателя.

    //==========================================================================

    //   DOWN
    T[3] = cam->h / 2.0;                                                        // половина высоты плоскости проекции
    LU[0] = V[0] - T[3] * cam->up_dir[0];                                       // вектор нижней плоскости отсечения
    LU[1] = V[1] - T[3] * cam->up_dir[1];
    LU[2] = V[2] - T[3] * cam->up_dir[2];

    vec3_cross(cam->clip_planes+8, cam->right_dir, LU)
    LU[3] = vec3_abs(cam->clip_planes+8);                                      // модуль нормали к левой / правой плоскостям
    vec3_norm_plane(cam->clip_planes+8, cam->pos, LU[3])

    //   UP
    LU[0] = V[0] + T[3] * cam->up_dir[0];                                       // вектор верхней плоскости отсечения
    LU[1] = V[1] + T[3] * cam->up_dir[1];
    LU[2] = V[2] + T[3] * cam->up_dir[2];

    vec3_cross(cam->clip_planes+12, cam->right_dir, LU)
    vec3_norm_plane(cam->clip_planes+12, cam->pos, LU[3])

    //==========================================================================

    //   LEFT
    T[3] = cam->w / 2.0;                                                        // половина ширины плоскости проекции
    LU[0] = V[0] - T[3] * cam->right_dir[0];                                    // вектор левой плоскости отсечения
    LU[1] = V[1] - T[3] * cam->right_dir[1];
    LU[2] = V[2] - T[3] * cam->right_dir[2];

    vec3_cross(cam->clip_planes, cam->up_dir, LU)
    LU[3] = vec3_abs(cam->clip_planes);                                         // модуль нормали к левой / правой плоскостям
    vec3_norm_plane(cam->clip_planes, cam->pos, LU[3])

    //   RIGHT
    LU[0] = V[0] + T[3] * cam->right_dir[0];                                    // вектор правой плоскости отсечения
    LU[1] = V[1] + T[3] * cam->right_dir[1];
    LU[2] = V[2] + T[3] * cam->right_dir[2];

    vec3_cross(cam->clip_planes+4, cam->up_dir, LU)
    vec3_norm_plane(cam->clip_planes+4, cam->pos, LU[3])

    vec3_add(V, cam->pos, cam->view_dir)
    if(vec3_plane_dist(n, V) < 0.0)
    {
        vec4_inv(n);
    }
    n += 4;
    if(vec3_plane_dist(n, V) < 0.0)
    {
        vec4_inv(n);
    }
    n += 4;
    if(vec3_plane_dist(n, V) < 0.0)
    {
        vec4_inv(n);
    }
    n += 4;
    if(vec3_plane_dist(n, V) < 0.0)
    {
        vec4_inv(n);
    }

    vec3_add(cam->frustum->vertex, cam->pos, cam->view_dir);
}
