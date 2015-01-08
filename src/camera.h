
#ifndef CAMERA_H
#define CAMERA_H

struct room_s;
struct polygon_s;
struct frustum_s;

#include "bullet/LinearMath/btScalar.h"

#define TR_CAM_MAX_SHAKE_DISTANCE   8192.0
#define TR_CAM_DEFAULT_SHAKE_POWER  100.0

typedef struct camera_s
{
    btScalar                    pos[3];                                         // camera position
    btScalar                    prev_pos[3];                                    // previous camera position
    btScalar                    view_dir[4];                                    // view cameradirection
    btScalar                    up_dir[4];                                      // up vector
    btScalar                    right_dir[4];                                   // strafe vector
    btScalar                    ang[3];                                         // camera orientation
    
    btScalar                    clip_planes[16];                                // frustum side clip planes
    struct frustum_s            *frustum;                                       // camera frustum structure
    
    btScalar                    dist_near;
    btScalar                    dist_far;
    
    btScalar                    fov;
    btScalar                    aspect;
    btScalar                    h;
    btScalar                    w;
    
    btScalar                    shake_value;
    btScalar                    shake_time;
    
    struct room_s               *current_room;
}camera_t, *camera_p;

void Cam_InitGlobals(camera_p cam);
void Cam_Init(camera_p cam);                                                    // set default camera parameters + frustum initialization
void Cam_Apply(camera_p cam);                                                   // set OpenGL projection matrix + model wiev matrix
void Cam_SetFovAspect(camera_p cam, btScalar fov, btScalar aspect);
void Cam_MoveAlong(camera_p cam, btScalar dist);
void Cam_MoveStrafe(camera_p cam, btScalar dist);
void Cam_MoveVertical(camera_p cam, btScalar dist);
void Cam_Shake(camera_p cam, btScalar power, btScalar time);                    // make camera shake
void Cam_DeltaRotation(camera_p cam, btScalar angles[3]);                       // rotate camera around current camera coordinate system
void Cam_SetRotation(camera_p cam, btScalar angles[3]);                         // set orientation by angles
void Cam_RecalcClipPlanes(camera_p cam);                                        // recalculation of camera frustum clipplanes 

#endif
