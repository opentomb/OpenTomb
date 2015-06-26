
#ifndef CAMERA_H
#define CAMERA_H

struct Room;
struct polygon_s;
struct frustum_s;

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

#include <memory>
#include <vector>
#include "vmath.h"

#define TR_CAM_MAX_SHAKE_DISTANCE   8192.0
#define TR_CAM_DEFAULT_SHAKE_POWER  100.0

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

#define TR_CAM_TARG_BACK  (0)
#define TR_CAM_TARG_FRONT (1)
#define TR_CAM_TARG_LEFT  (2)
#define TR_CAM_TARG_RIGHT (3)

typedef struct camera_s
{
    btVector3                   pos;                 // camera position
    btVector3                   prev_pos;            // previous camera position
    btVector3                   view_dir;            // view cameradirection
    btVector3 up_dir;              // up vector
    btVector3 right_dir;           // strafe vector
    btVector3 ang;                 // camera orientation

    btTransform gl_view_mat;
    btTransform gl_proj_mat;
    btTransform gl_view_proj_mat;

    btVector3 clip_planes[4];        // frustum side clip planes
    struct frustum_s            *frustum;               // camera frustum structure

    GLfloat                     dist_near;
    GLfloat                     dist_far;

    GLfloat                     fov;
    GLfloat                     aspect;
    GLfloat                     f;
    GLfloat                     h;
    GLfloat                     w;

    GLfloat                     shake_value;
    GLfloat                     shake_time;

    int8_t                      target_dir;//Target rotation direction (0 = Back, 1 = Front, 2 = Left, 3 = Right)

    std::shared_ptr<Room> current_room;
}camera_t, *camera_p;

// Static camera / sink structure.
// In original engines, static cameras and sinks shared the same structure,
// albeit with different field meanings. In compiled level, it is unfortunately
// impossible to tell camera from sink, so the only way is to share the struct
// between these two types of objects.
// Thanks to b122251 for extra info describing this structure.

typedef struct stat_camera_sink_s
{
    GLfloat                     x;
    GLfloat                     y;
    GLfloat                     z;
    uint16_t                    room_or_strength;   // Room for camera, strength for sink.
    uint16_t                    flag_or_zone;       // Flag for camera, zone for sink.

}stat_camera_sink_t, *stat_camera_sink_p;

void Cam_Init(camera_p cam);                                       // set default camera parameters + frustum initialization
void Cam_Apply(camera_p cam);                                      // set OpenGL projection matrix + model wiev matrix
void Cam_SetFovAspect(camera_p cam, GLfloat fov, GLfloat aspect);
void Cam_MoveAlong(camera_p cam, GLfloat dist);
void Cam_MoveStrafe(camera_p cam, GLfloat dist);
void Cam_MoveVertical(camera_p cam, GLfloat dist);
void Cam_Shake(camera_p cam, GLfloat power, GLfloat time);         // make camera shake
void Cam_DeltaRotation(camera_p cam, const btVector3 &angles);           // rotate camera around current camera coordinate system
void Cam_SetRotation(camera_p cam, const btVector3& angles);             // set orientation by angles
void Cam_RecalcClipPlanes(camera_p cam);                           // recalculation of camera frustum clipplanes

#endif
