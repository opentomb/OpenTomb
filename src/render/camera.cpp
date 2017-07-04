
#include <stdlib.h>
#include <math.h>

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
    cam->frustum->cam_pos = cam->gl_transform + 12;
    cam->frustum->vertex_count = 4;
    cam->frustum->next = NULL;
    cam->frustum->parent = NULL;
    cam->frustum->parents_count = 0;
    cam->frustum->vertex = NULL;
    cam->frustum->planes = cam->clip_planes;
    cam->frustum->vertex = (float*)malloc(3 * 4 * sizeof(float));

    cam->prev_pos[0] = 0.0f;
    cam->prev_pos[1] = 0.0f;
    cam->prev_pos[2] = 0.0f;

    cam->gl_transform[0 + 0] = 1.0f;                                            // OX - right
    cam->gl_transform[0 + 1] = 0.0f;
    cam->gl_transform[0 + 2] = 0.0f;
    cam->gl_transform[0 + 3] = 0.0f;

    cam->gl_transform[4 + 0] = 0.0f;                                            // OY - up
    cam->gl_transform[4 + 1] = 1.0f;
    cam->gl_transform[4 + 2] = 0.0f;
    cam->gl_transform[4 + 3] = 0.0f;

    cam->gl_transform[8 + 0] = 0.0f;                                            // OZ - view
    cam->gl_transform[8 + 1] = 0.0f;
    cam->gl_transform[8 + 2] = 1.0f;
    cam->gl_transform[8 + 3] = 0.0f;

    cam->gl_transform[12 + 0] = 0.0f;
    cam->gl_transform[12 + 1] = 0.0f;
    cam->gl_transform[12 + 2] = 0.0f;
    cam->gl_transform[12 + 3] = 1.0f;

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
    M[0 * 4 + 0] = cam->gl_transform[0 + 0];
    M[1 * 4 + 0] = cam->gl_transform[0 + 1];
    M[2 * 4 + 0] = cam->gl_transform[0 + 2];

    M[0 * 4 + 1] = cam->gl_transform[4 + 0];
    M[1 * 4 + 1] = cam->gl_transform[4 + 1];
    M[2 * 4 + 1] = cam->gl_transform[4 + 2];

    M[0 * 4 + 2] = -cam->gl_transform[8 + 0];
    M[1 * 4 + 2] = -cam->gl_transform[8 + 1];
    M[2 * 4 + 2] = -cam->gl_transform[8 + 2];

    M[3 * 4 + 0] = -(M[0] * cam->gl_transform[12 + 0] + M[4] * cam->gl_transform[12 + 1] + M[8]  * cam->gl_transform[12 + 2]);
    M[3 * 4 + 1] = -(M[1] * cam->gl_transform[12 + 0] + M[5] * cam->gl_transform[12 + 1] + M[9]  * cam->gl_transform[12 + 2]);
    M[3 * 4 + 2] = -(M[2] * cam->gl_transform[12 + 0] + M[6] * cam->gl_transform[12 + 1] + M[10] * cam->gl_transform[12 + 2]);

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
    cam->gl_transform[12 + 0] += cam->gl_transform[8 + 0] * dist;
    cam->gl_transform[12 + 1] += cam->gl_transform[8 + 1] * dist;
    cam->gl_transform[12 + 2] += cam->gl_transform[8 + 2] * dist;
}

void Cam_MoveStrafe(camera_p cam, GLfloat dist)
{
    cam->gl_transform[12 + 0] += cam->gl_transform[0 + 0] * dist;
    cam->gl_transform[12 + 1] += cam->gl_transform[0 + 1] * dist;
    cam->gl_transform[12 + 2] += cam->gl_transform[0 + 2] * dist;
}

void Cam_MoveVertical(camera_p cam, GLfloat dist)
{
    cam->gl_transform[12 + 0] += cam->gl_transform[4 + 0] * dist;
    cam->gl_transform[12 + 1] += cam->gl_transform[4 + 1] * dist;
    cam->gl_transform[12 + 2] += cam->gl_transform[4 + 2] * dist;
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
    R[0] = cam->gl_transform[8 + 0] * sin_t2;
    R[1] = cam->gl_transform[8 + 1] * sin_t2;
    R[2] = cam->gl_transform[8 + 2] * sin_t2;
    vec4_sop(Rt, R)

    vec4_mul(temp, R, cam->gl_transform + 0)
    vec4_mul(cam->gl_transform + 0, temp, Rt)
    vec4_mul(temp, R, cam->gl_transform + 4)
    vec4_mul(cam->gl_transform + 4, temp, Rt)

    t = -angles[0] / 2.0;                                                       // LEFT - RIGHT
    sin_t2 = sinf(t);
    cos_t2 = cosf(t);
    R[3] = cos_t2;
    R[0] = cam->gl_transform[4 + 0] * sin_t2;
    R[1] = cam->gl_transform[4 + 1] * sin_t2;
    R[2] = cam->gl_transform[4 + 2] * sin_t2;
    vec4_sop(Rt, R)

    vec4_mul(temp, R, cam->gl_transform + 0)
    vec4_mul(cam->gl_transform + 0, temp, Rt)
    vec4_mul(temp, R, cam->gl_transform + 8)
    vec4_mul(cam->gl_transform + 8, temp, Rt)

    t = angles[1] / 2.0;                                                        // UP - DOWN
    sin_t2 = sinf(t);
    cos_t2 = cosf(t);
    R[3] = cos_t2;
    R[0] = cam->gl_transform[0 + 0] * sin_t2;
    R[1] = cam->gl_transform[0 + 1] * sin_t2;
    R[2] = cam->gl_transform[0 + 2] * sin_t2;
    vec4_sop(Rt, R)

    vec4_mul(temp, R, cam->gl_transform + 8)
    vec4_mul(cam->gl_transform + 8, temp, Rt)
    vec4_mul(temp, R, cam->gl_transform + 4)
    vec4_mul(cam->gl_transform + 4, temp, Rt)
}

void Cam_SetRotation(camera_p cam, GLfloat angles[3])
{
    GLfloat R[4], Rt[4], temp[4];
    GLfloat sin_t2, cos_t2, t;

    vec3_copy(cam->ang, angles);

    sin_t2 = sinf(angles[0]);
    cos_t2 = cosf(angles[0]);

    /*
     * LEFT - RIGHT INIT
     */
    cam->gl_transform[8 + 0] =-sin_t2;                                          // OY - view
    cam->gl_transform[8 + 1] = cos_t2;
    cam->gl_transform[8 + 2] = 0.0;
    cam->gl_transform[8 + 3] = 0.0;

    cam->gl_transform[0 + 0] = cos_t2;                                          // OX - right
    cam->gl_transform[0 + 1] = sin_t2;
    cam->gl_transform[0 + 2] = 0.0;
    cam->gl_transform[0 + 3] = 0.0;

    cam->gl_transform[4 + 0] = 0.0;                                             // OZ - up
    cam->gl_transform[4 + 1] = 0.0;
    cam->gl_transform[4 + 2] = 1.0;
    cam->gl_transform[4 + 3] = 0.0;

    t = angles[1] / 2.0;                                                        // UP - DOWN
    sin_t2 = sinf(t);
    cos_t2 = cosf(t);
    R[3] = cos_t2;
    R[0] = cam->gl_transform[0 + 0] * sin_t2;
    R[1] = cam->gl_transform[0 + 1] * sin_t2;
    R[2] = cam->gl_transform[0 + 2] * sin_t2;
    vec4_sop(Rt, R);

    vec4_mul(temp, R, cam->gl_transform + 4);
    vec4_mul(cam->gl_transform + 4, temp, Rt);
    vec4_mul(temp, R, cam->gl_transform + 8);
    vec4_mul(cam->gl_transform + 8, temp, Rt);

    t = angles[2] / 2.0;                                                        // ROLL
    sin_t2 = sinf(t);
    cos_t2 = cosf(t);
    R[3] = cos_t2;
    R[0] = cam->gl_transform[8 + 0] * sin_t2;
    R[1] = cam->gl_transform[8 + 1] * sin_t2;
    R[2] = cam->gl_transform[8 + 2] * sin_t2;
    vec4_sop(Rt, R);

    vec4_mul(temp, R, cam->gl_transform + 0);
    vec4_mul(cam->gl_transform + 0, temp, Rt);
    vec4_mul(temp, R, cam->gl_transform + 4);
    vec4_mul(cam->gl_transform + 4, temp, Rt);
}


void Cam_LookTo(camera_p cam, GLfloat to[3])
{
    float d;

    vec3_sub(cam->gl_transform + 8, to, cam->gl_transform + 12);
    vec3_norm(cam->gl_transform + 8, d);

    if(fabs(cam->gl_transform[8 + 2]) < 0.999f)
    {
        cam->gl_transform[0 + 0] = cam->gl_transform[8 + 1];
        cam->gl_transform[0 + 1] =-cam->gl_transform[8 + 0];
    }
    cam->gl_transform[0 + 2] = 0.0f;
    vec3_norm(cam->gl_transform + 0, d);

    vec3_cross(cam->gl_transform + 4, cam->gl_transform + 0, cam->gl_transform + 8);
    vec3_norm(cam->gl_transform + 4, d);
}


void Cam_RecalcClipPlanes(camera_p cam)
{
    GLfloat T[4], LU[4], V[3], *n = cam->clip_planes;

    V[0] = cam->gl_transform[8 + 0] * cam->dist_near;
    V[1] = cam->gl_transform[8 + 1] * cam->dist_near;
    V[2] = cam->gl_transform[8 + 2] * cam->dist_near;

    T[0] = cam->gl_transform[12 + 0] + V[0];
    T[1] = cam->gl_transform[12 + 1] + V[1];
    T[2] = cam->gl_transform[12 + 2] + V[2];

    //==========================================================================

    vec3_copy(cam->frustum->norm, cam->gl_transform + 8);
    cam->frustum->norm[3] = -vec3_dot(cam->gl_transform + 8, cam->gl_transform + 12);

    //==========================================================================

    //   DOWN
    T[3] = cam->h / 2.0;
    LU[0] = V[0] - T[3] * cam->gl_transform[4 + 0];
    LU[1] = V[1] - T[3] * cam->gl_transform[4 + 1];
    LU[2] = V[2] - T[3] * cam->gl_transform[4 + 2];

    vec3_cross(cam->clip_planes+8, cam->gl_transform + 0, LU)
    LU[3] = vec3_abs(cam->clip_planes+8);
    vec3_norm_plane(cam->clip_planes+8, cam->gl_transform + 12, LU[3])

    //   UP
    LU[0] = V[0] + T[3] * cam->gl_transform[4 + 0];
    LU[1] = V[1] + T[3] * cam->gl_transform[4 + 1];
    LU[2] = V[2] + T[3] * cam->gl_transform[4 + 2];

    vec3_cross(cam->clip_planes+12, cam->gl_transform + 0, LU)
    vec3_norm_plane(cam->clip_planes+12, cam->gl_transform + 12, LU[3])

    //==========================================================================

    //   LEFT
    T[3] = cam->w / 2.0;
    LU[0] = V[0] - T[3] * cam->gl_transform[0 + 0];
    LU[1] = V[1] - T[3] * cam->gl_transform[0 + 1];
    LU[2] = V[2] - T[3] * cam->gl_transform[0 + 2];

    vec3_cross(cam->clip_planes, cam->gl_transform + 4, LU)
    LU[3] = vec3_abs(cam->clip_planes);
    vec3_norm_plane(cam->clip_planes, cam->gl_transform + 12, LU[3])

    //   RIGHT
    LU[0] = V[0] + T[3] * cam->gl_transform[0 + 0];
    LU[1] = V[1] + T[3] * cam->gl_transform[0 + 1];
    LU[2] = V[2] + T[3] * cam->gl_transform[0 + 2];

    vec3_cross(cam->clip_planes+4, cam->gl_transform + 4, LU)
    vec3_norm_plane(cam->clip_planes+4, cam->gl_transform + 12, LU[3])

    vec3_add(V, cam->gl_transform + 12, cam->gl_transform + 8)
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

    vec3_add(cam->frustum->vertex, cam->gl_transform + 12, cam->gl_transform + 8);
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
        ret->locked = 0x00;

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
            ret->fov->d[i] = start[i].fov / 256.0f;
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
        Spline_BuildCubic(ret->speed);
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
    uint32_t index = t;

    cam->gl_transform[12 + 0] = Spline_Get(s->pos_x, t);
    cam->gl_transform[12 + 1] = Spline_Get(s->pos_y, t);
    cam->gl_transform[12 + 2] = Spline_Get(s->pos_z, t);
    if(index < s->pos_x->base_points_count)
    {
        cam->current_room = s->start[index].room;
    }

    Cam_SetFovAspect(cam, Spline_Get(s->fov, t), cam->aspect);

    to[0] = Spline_Get(s->target_x, t);
    to[1] = Spline_Get(s->target_y, t);
    to[2] = Spline_Get(s->target_z, t);

    Cam_LookTo(cam, to);

    d = Spline_Get(s->roll, t);

    if(d != 0.0f)
    {
        float sin_t2, cos_t2, module;
        float t1[4], t2[4], t[4];

        d /= 2.0;
        sin_t2 = sin(d);
        cos_t2 = cos(d);

        t1[3] = cos_t2;
        t1[0] = cam->gl_transform[8 + 0] * sin_t2;
        t1[1] = cam->gl_transform[8 + 1] * sin_t2;
        t1[2] = cam->gl_transform[8 + 2] * sin_t2;
        module = vec4_abs(t1);
        t1[0] /= module;
        t1[1] /= module;
        t1[2] /= module;
        t1[3] /= module;

        vec4_sop(t2, t1);

        cam->gl_transform[4 + 3] = 0.0;
        vec4_mul(t, t1, cam->gl_transform + 4);
        vec4_mul(cam->gl_transform + 4, t, t2);

        cam->gl_transform[0 + 3] = 0.0;
        vec4_mul(t, t1, cam->gl_transform + 0);
        vec4_mul(cam->gl_transform + 0, t, t2);
    }
}
