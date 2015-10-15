
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <stdlib.h>
#include <math.h>

#include "../core/gl_util.h"
#include "../core/vmath.h"
#include "../core/polygon.h"
#include "camera.h"
#include "frustum.h"

/*
  ======================
        CAM TARGET
  ======================
            ^
           (N)
  ----------------------
  |  ??  |  01  |  ??  |
  ----------------------
  |  02  | LARA |  03  |
  ----------------------
  |  ??  |  00  |  ??  |
  ----------------------
*/

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

    Mat4_E_macro(cam->gl_view_mat);
    Mat4_E_macro(cam->gl_proj_mat);
    Mat4_E_macro(cam->gl_view_proj_mat);

    cam->frustum = (frustum_p)malloc(sizeof(frustum_t));
    cam->frustum->cam_pos = cam->pos;
    cam->frustum->vertex_count = 4;
    cam->frustum->next = NULL;
    cam->frustum->parent = NULL;
    cam->frustum->parents_count = 0;
    cam->frustum->vertex = NULL;
    cam->frustum->planes = cam->clip_planes;
    cam->frustum->vertex = (float*)malloc(3*4*sizeof(float));

    cam->pos[0] = 0.0;
    cam->pos[1] = 0.0;
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
    GLfloat *M;

    M = cam->gl_proj_mat;
    M[0 * 4 + 0] = cam->f / cam->aspect;
    M[0 * 4 + 1] = 0.0;
    M[0 * 4 + 2] = 0.0;
    M[0 * 4 + 3] = 0.0;

    M[1 * 4 + 0] = 0.0;
    M[1 * 4 + 1] = cam->f;
    M[1 * 4 + 2] = 0.0;
    M[1 * 4 + 3] = 0.0;

    M[2 * 4 + 0] = 0.0;
    M[2 * 4 + 1] = 0.0;
    M[2 * 4 + 2] = (cam->dist_near + cam->dist_far) / (cam->dist_near - cam->dist_far);
    M[2 * 4 + 3] =-1.0;

    M[3 * 4 + 0] = 0.0;
    M[3 * 4 + 1] = 0.0;
    M[3 * 4 + 2] = 2.0 * cam->dist_near * cam->dist_far / (cam->dist_near - cam->dist_far);
    M[3 * 4 + 3] = 0.0;

    M = cam->gl_view_mat;
    M[0 * 4 + 0] = cam->right_dir[0];
    M[1 * 4 + 0] = cam->right_dir[1];
    M[2 * 4 + 0] = cam->right_dir[2];

    M[0 * 4 + 1] = cam->up_dir[0];
    M[1 * 4 + 1] = cam->up_dir[1];
    M[2 * 4 + 1] = cam->up_dir[2];

    M[0 * 4 + 2] = -cam->view_dir[0];
    M[1 * 4 + 2] = -cam->view_dir[1];
    M[2 * 4 + 2] = -cam->view_dir[2];

    M[3 * 4 + 0] = -(M[0] * cam->pos[0] + M[4] * cam->pos[1] + M[8]  * cam->pos[2]);
    M[3 * 4 + 1] = -(M[1] * cam->pos[0] + M[5] * cam->pos[1] + M[9]  * cam->pos[2]);
    M[3 * 4 + 2] = -(M[2] * cam->pos[0] + M[6] * cam->pos[1] + M[10] * cam->pos[2]);

    M[0 * 4 + 3] = 0.0;
    M[1 * 4 + 3] = 0.0;
    M[2 * 4 + 3] = 0.0;
    M[3 * 4 + 3] = 1.0;

    Mat4_Mat4_mul(cam->gl_view_proj_mat, cam->gl_proj_mat, cam->gl_view_mat);
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
    cam->pos[0] += cam->view_dir[0] * dist;
    cam->pos[1] += cam->view_dir[1] * dist;
    cam->pos[2] += cam->view_dir[2] * dist;
}

void Cam_MoveStrafe(camera_p cam, GLfloat dist)
{
    cam->pos[0] += cam->right_dir[0] * dist;
    cam->pos[1] += cam->right_dir[1] * dist;
    cam->pos[2] += cam->right_dir[2] * dist;
}

void Cam_MoveVertical(camera_p cam, GLfloat dist)
{
    cam->pos[0] += cam->up_dir[0] * dist;
    cam->pos[1] += cam->up_dir[1] * dist;
    cam->pos[2] += cam->up_dir[2] * dist;
}

void Cam_Shake(camera_p cam, GLfloat power, GLfloat time)
{
    cam->shake_value = power;
    cam->shake_time  = time;
}

void Cam_DeltaRotation(camera_p cam, GLfloat angles[3])                         //angles = {OX, OY, OZ}
{
    GLfloat R[4], Rt[4], temp[4];
    GLfloat sin_t2, cos_t2, t;

    vec3_add(cam->ang, cam->ang, angles)

    t = -angles[2] / 2.0;                                                       // ROLL
    sin_t2 = sinf(t);
    cos_t2 = cosf(t);
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
    sin_t2 = sinf(t);
    cos_t2 = cosf(t);
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
    sin_t2 = sinf(t);
    cos_t2 = cosf(t);
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

void Cam_SetRotation(camera_p cam, float angles[3])
{
    GLfloat R[4], Rt[4], temp[4];
    GLfloat sin_t2, cos_t2, t;

    vec3_copy(cam->ang, angles);

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
    sin_t2 = sinf(t);
    cos_t2 = cosf(t);
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
    GLfloat T[4], LU[4], V[3], *n = cam->clip_planes;

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
    LU[3] = vec3_abs(cam->clip_planes+8);                                       // модуль нормали к левой / правой плоскостям
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

/*
 * FLYBY CAMERAS
 */
flyby_camera_sequence_p FlyBySequence_Create(flyby_camera_state_p start, uint32_t count)
{
    flyby_camera_sequence_p ret = NULL;

    if(count >= 2)
    {
        ret = (flyby_camera_sequence_p)malloc(sizeof(flyby_camera_sequence_t));

        ret->start = start;
        ret->next = NULL;

        ret->pos_x = Spline_Create(count);
        ret->pos_y = Spline_Create(count);
        ret->pos_z = Spline_Create(count);
        ret->target_x = Spline_Create(count);
        ret->target_y = Spline_Create(count);
        ret->target_z = Spline_Create(count);
        ret->fov = Spline_Create(count);
        ret->roll = Spline_Create(count);
        ret->speed = Spline_Create(count);

        for(uint32_t i = 0; i < count; i++)
        {
            ret->pos_x->d[i] = start[i].pos[0];
            ret->pos_y->d[i] = start[i].pos[1];
            ret->pos_z->d[i] = start[i].pos[2];
            ret->target_x->d[i] = start[i].target[0];
            ret->target_y->d[i] = start[i].target[1];
            ret->target_z->d[i] = start[i].target[2];
            ret->fov->d[i] = start[i].fov;
            ret->roll->d[i] = start[i].roll * M_PI / (180.0f * 2048.0f);
            ret->speed->d[i] = start[i].speed;
        }

        Spline_BuildCubic(ret->pos_x);
        Spline_BuildCubic(ret->pos_y);
        Spline_BuildCubic(ret->pos_z);
        Spline_BuildCubic(ret->target_x);
        Spline_BuildCubic(ret->target_y);
        Spline_BuildCubic(ret->target_z);
        Spline_BuildCubic(ret->fov);
        Spline_BuildCubic(ret->roll);
        Spline_BuildLine(ret->speed);
    }

    return ret;
}


void FlyBySequence_Clear(flyby_camera_sequence_p s)
{
    Spline_Clear(s->pos_x);
    free(s->pos_x);
    s->pos_x = NULL;
    Spline_Clear(s->pos_y);
    free(s->pos_y);
    s->pos_y = NULL;
    Spline_Clear(s->pos_z);
    free(s->pos_z);
    s->pos_z = NULL;
    Spline_Clear(s->target_x);
    free(s->target_x);
    s->target_x = NULL;
    Spline_Clear(s->target_y);
    free(s->target_y);
    s->target_y = NULL;
    Spline_Clear(s->target_z);
    free(s->target_z);
    s->target_z = NULL;
    Spline_Clear(s->roll);
    free(s->roll);
    s->roll = NULL;
    Spline_Clear(s->fov);
    free(s->fov);
    s->fov = NULL;
    Spline_Clear(s->speed);
    free(s->speed);
    s->speed = NULL;

    s->start = NULL;
    s->next = NULL;
}


void FlyBySequence_SetCamera(flyby_camera_sequence_p s, camera_p cam, float t)
{
    float to[3], d;

    cam->pos[0] = Spline_Get(s->pos_x, t);
    cam->pos[1] = Spline_Get(s->pos_y, t);
    cam->pos[2] = Spline_Get(s->pos_z, t);

    to[0] = Spline_Get(s->target_x, t);
    to[1] = Spline_Get(s->target_y, t);
    to[2] = Spline_Get(s->target_z, t);

    vec3_sub(cam->view_dir, to, cam->pos);
    vec3_norm(cam->view_dir, d);
    to[0] = 0.0f;
    to[1] = 0.0f;
    to[2] = 1.0f;

    cam->right_dir[0] = cam->view_dir[1];
    cam->right_dir[1] =-cam->view_dir[0];
    cam->right_dir[2] = 0.0f;
    vec3_norm(cam->right_dir, d);

    vec3_cross(cam->up_dir, cam->right_dir, cam->view_dir);

    d = Spline_Get(s->roll, t);

    if(d != 0.0f)
    {
        float sin_t2, cos_t2, module;
        float t1[4], t2[4], t[4];

        d /= 2.0;
        sin_t2 = sin(d);
        cos_t2 = cos(d);

        t1[3] = cos_t2;
        t1[0] = cam->view_dir[0] * sin_t2;
        t1[1] = cam->view_dir[1] * sin_t2;
        t1[2] = cam->view_dir[2] * sin_t2;

        module = vec4_abs(t1);
        t2[3] = t1[3] / module;
        t2[0] = -t1[0] / module;
        t2[1] = -t1[1] / module;
        t2[2] = -t1[2] / module;

        cam->up_dir[3] = 0.0;
        vec4_mul(t, t1, cam->up_dir);
        vec4_mul(cam->up_dir, t, t2);

        cam->right_dir[3] = 0.0;
        vec4_mul(t, t1, cam->right_dir);
        vec4_mul(cam->right_dir, t, t2);
    }
}
