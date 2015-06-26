
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <cstdlib>
#include <cmath>

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
    cam->dist_far = 65536.0;

    cam->f = tanf(M_PI * cam->fov / 360.0);
    cam->h = 2.0 * cam->dist_near * cam->f;
    cam->w = cam->h * cam->aspect;
    cam->f = 1.0 / cam->f;

    cam->gl_view_mat.setIdentity();
    cam->gl_proj_mat.setIdentity();
    cam->gl_view_proj_mat.setIdentity();

    cam->frustum = (frustum_p)malloc(sizeof(frustum_t));
    cam->frustum->cam_pos = &cam->pos;
    cam->frustum->vertices.clear();
    cam->frustum->vertices.resize(3);
    cam->frustum->next = NULL;
    cam->frustum->parent = NULL;
    cam->frustum->parents_count = 0;
    cam->frustum->planes.assign( cam->clip_planes+0, cam->clip_planes+4 );

    cam->pos = btVector3(0,0,0);

    cam->prev_pos = btVector3(0,0,0);

    cam->view_dir = btVector3(0,0,1);                                                     // OZ - view

    cam->right_dir = btVector3(1,0,0);                                                    // OX - right

    cam->up_dir = btVector3(0,1,0);                                                       // OY - up

    cam->shake_value    = 0.0;
    cam->shake_time     = 0.0;

    cam->current_room = NULL;
}

void Cam_Apply(camera_p cam)
{
    btMatrix3x3& M = cam->gl_proj_mat.getBasis();
    M.setIdentity();
    M[0][0] = cam->f / cam->aspect;
    M[1][1] = cam->f;
    M[2][2] = (cam->dist_near + cam->dist_far) / (cam->dist_near - cam->dist_far);
    M[2][3] =-1.0;
    M[3][2] = 2.0 * cam->dist_near * cam->dist_far / (cam->dist_near - cam->dist_far);

    btMatrix3x3& M2 = cam->gl_view_mat.getBasis();
    M2[0] = cam->right_dir;
    M2[1] = cam->up_dir;
    M2[2] = -cam->view_dir;
    M2 = M2.transpose();

    cam->gl_view_mat.getOrigin() = M2 * cam->pos;
    cam->gl_view_mat.getOrigin().setW(1);

    cam->gl_view_proj_mat = cam->gl_proj_mat * cam->gl_view_mat;
}

void Cam_SetFovAspect(camera_p cam, GLfloat fov, GLfloat aspect)
{
    cam->fov = fov;
    cam->aspect = aspect;
    cam->f = tanf(M_PI * cam->fov / 360.0);
    cam->h = 2.0 * cam->dist_near * cam->f;
    cam->w = cam->h * aspect;
    cam->f = 1.0 / cam->f;
}

void Cam_MoveAlong(camera_p cam, GLfloat dist)
{
    cam->pos += cam->view_dir * dist;
}

void Cam_MoveStrafe(camera_p cam, GLfloat dist)
{
    cam->pos += cam->right_dir * dist;
}

void Cam_MoveVertical(camera_p cam, GLfloat dist)
{
    cam->pos += cam->up_dir * dist;
}

void Cam_Shake(camera_p cam, GLfloat power, GLfloat time)
{
    cam->shake_value = power;
    cam->shake_time  = time;
}

void Cam_DeltaRotation(camera_p cam, const btVector3& angles)                         //angles = {OX, OY, OZ}
{
    GLfloat sin_t2, cos_t2, t;

    cam->ang = cam->ang + angles;

    t = -angles[2] / 2.0;                                                       // ROLL
    sin_t2 = sinf(t);
    cos_t2 = cosf(t);
    btVector3 R;
    R[3] = cos_t2;
    R[0] = cam->view_dir[0] * sin_t2;
    R[1] = cam->view_dir[1] * sin_t2;
    R[2] = cam->view_dir[2] * sin_t2;
    btVector3 Rt;
    Rt = -R;
    Rt[3] = -Rt[3];

    cam->up_dir = R * cam->up_dir * Rt;

    t = -angles[0] / 2.0;                                                       // LEFT - RIGHT
    sin_t2 = sinf(t);
    cos_t2 = cosf(t);
    R[3] = cos_t2;
    R[0] = cam->up_dir[0] * sin_t2;
    R[1] = cam->up_dir[1] * sin_t2;
    R[2] = cam->up_dir[2] * sin_t2;
    Rt = R;
    Rt[3] = -Rt[3];

    cam->view_dir = R * cam->view_dir * Rt;

    t = angles[1] / 2.0;                                                        // UP - DOWN
    sin_t2 = sinf(t);
    cos_t2 = cosf(t);
    R[3] = cos_t2;
    R[0] = cam->right_dir[0] * sin_t2;
    R[1] = cam->right_dir[1] * sin_t2;
    R[2] = cam->right_dir[2] * sin_t2;
    Rt = -R;
    Rt[3] = -Rt[3];

    cam->view_dir = R * cam->view_dir * Rt;
    cam->up_dir = R * cam->up_dir * Rt;
}

void Cam_SetRotation(camera_p cam, const btVector3& angles)                          //angles = {OX, OY, OZ}
{
    GLfloat sin_t2, cos_t2, t;

    cam->ang = angles;

    sin_t2 = sinf(angles[0]);
    cos_t2 = cosf(angles[0]);

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
    sin_t2 = sinf(t);
    cos_t2 = cosf(t);
    btVector3 R;
    R[3] = cos_t2;
    R[0] = cam->right_dir[0] * sin_t2;
    R[1] = cam->right_dir[1] * sin_t2;
    R[2] = cam->right_dir[2] * sin_t2;
    btVector3 Rt = -R;
    Rt[3] = -Rt[3];

    cam->up_dir = R * cam->up_dir * Rt;
    cam->view_dir = R * cam->view_dir * Rt;

    t = angles[2] / 2.0;                                                        // ROLL
    sin_t2 = sinf(t);
    cos_t2 = cosf(t);
    R = cam->view_dir * sin_t2;
    Rt = -R;
    R[3] = cos_t2;

    cam->right_dir = R * cam->right_dir * Rt;
    cam->up_dir = R * cam->up_dir * Rt;
}

void Cam_RecalcClipPlanes(camera_p cam)
{
    btVector3 V = cam->view_dir * cam->dist_near;
    btVector3 T = cam->pos + V;

    //==========================================================================

    cam->frustum->norm = cam->view_dir;                               // Основная плоскость отсечения (что сзади - то не рисуем)
    cam->frustum->norm[3] = -cam->view_dir.dot(cam->pos);                 // плоскость проекции проходит через наблюдателя.

    //==========================================================================

    //   DOWN
    T[3] = cam->h / 2.0;                                                        // половина высоты плоскости проекции
    btVector3 LU;
    LU = V - T[3] * cam->up_dir;                                       // вектор нижней плоскости отсечения

    cam->clip_planes[2] = cam->right_dir.cross(LU);
    LU[3] = cam->clip_planes[2].length();                                      // модуль нормали к левой / правой плоскостям
    vec3_norm_plane(cam->clip_planes[2], cam->pos, LU[3])

    //   UP
    LU = V + T[3] * cam->up_dir;                                       // вектор верхней плоскости отсечения

    cam->clip_planes[3] = cam->right_dir.cross(LU);
    vec3_norm_plane(cam->clip_planes[3], cam->pos, LU[3])

    //==========================================================================

    //   LEFT
    T[3] = cam->w / 2.0;                                                        // половина ширины плоскости проекции
    LU = V - T[3] * cam->right_dir;                                    // вектор левой плоскости отсечения

    cam->clip_planes[0] = cam->up_dir.cross(LU);
    LU[3] = cam->clip_planes[0].length();                                         // модуль нормали к левой / правой плоскостям
    vec3_norm_plane(cam->clip_planes, cam->pos, LU[3])

    //   RIGHT
    LU = V + T[3] * cam->right_dir;                                    // вектор правой плоскости отсечения

    cam->clip_planes[1] = cam->up_dir.cross(LU);
    vec3_norm_plane(cam->clip_planes[1], cam->pos, LU[3])

    V = cam->pos + cam->view_dir;
    for(int i=0; i<4; ++i)
        if(planeDist(cam->clip_planes[i], V) < 0.0)
            cam->clip_planes[i] = -cam->clip_planes[i];

    cam->frustum->vertices[0] = cam->pos + cam->view_dir;
}
