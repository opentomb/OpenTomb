
#ifndef CAMERA_H
#define CAMERA_H

#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>

struct room_s;
struct frustum_s;

#define TR_CAM_MAX_SHAKE_DISTANCE   8192.0
#define TR_CAM_DEFAULT_SHAKE_POWER  100.0


#define TR_CAM_TARG_BACK  (0)
#define TR_CAM_TARG_FRONT (1)
#define TR_CAM_TARG_LEFT  (2)
#define TR_CAM_TARG_RIGHT (3)


#define CAMERA_STATE_NORMAL     (0x0000)
#define CAMERA_STATE_LOOK_TO    (0x0001)
#define CAMERA_STATE_FIXED      (0x0002)
#define CAMERA_STATE_FLYBY      (0x0003)


typedef struct camera_state_s
{
    uint32_t                        state;
    struct flyby_camera_sequence_s *flyby;
    struct static_camera_sink_s    *sink;
    
    GLfloat                         shake_value;
    GLfloat                         time;
}camera_state_t, camera_state_p;


typedef struct camera_s
{
    GLfloat                     pos[3];                 // camera position
    GLfloat                     prev_pos[3];            // previous camera position
    GLfloat                     view_dir[4];            // view cameradirection
    GLfloat                     up_dir[4];              // up vector
    GLfloat                     right_dir[4];           // strafe vector
    GLfloat                     ang[3];                 // camera orientation

    GLfloat                     gl_view_mat[16];
    GLfloat                     gl_proj_mat[16];
    GLfloat                     gl_view_proj_mat[16];

    GLfloat                     clip_planes[16];        // frustum side clip planes
    struct frustum_s           *frustum;                // camera frustum structure

    GLfloat                     dist_near;
    GLfloat                     dist_far;

    GLfloat                     fov;
    GLfloat                     aspect;
    GLfloat                     f;
    GLfloat                     h;
    GLfloat                     w;

    int8_t                      target_dir;//Target rotation direction (0 = Back, 1 = Front, 2 = Left, 3 = Right)

    struct room_s               *current_room;
}camera_t, *camera_p;

// Static camera / sink structure.
// In original engines, static cameras and sinks shared the same structure,
// albeit with different field meanings. In compiled level, it is unfortunately
// impossible to tell camera from sink, so the only way is to share the struct
// between these two types of objects.
// Thanks to b122251 for extra info describing this structure.

typedef struct static_camera_sink_s
{
    GLfloat                     x;
    GLfloat                     y;
    GLfloat                     z;
    uint16_t                    room_or_strength;   // Room for camera, strength for sink.
    uint16_t                    flag_or_zone;       // Flag for camera, zone for sink.
}static_camera_sink_t, *static_camera_sink_p;

typedef struct flyby_camera_state_s
{
    float                       pos[3];
    float                       target[3];

    float                       fov;
    float                       roll;
    float                       timer;
    float                       speed;
    
    int8_t                      sequence;
    int8_t                      index;
    uint16_t                    flags;
    uint32_t                    room_id;
}flyby_camera_state_t, *flyby_camera_state_p;

typedef struct flyby_camera_sequence_s
{
    struct flyby_camera_state_s    *start;
    uint32_t                        locked : 1;
    
    struct spline_s                *pos_x;
    struct spline_s                *pos_y;
    struct spline_s                *pos_z;
    struct spline_s                *target_x;
    struct spline_s                *target_y;
    struct spline_s                *target_z;
    struct spline_s                *fov;
    struct spline_s                *roll;
    struct spline_s                *speed;
    
    struct flyby_camera_sequence_s *next;
}flyby_camera_sequence_t, *flyby_camera_sequence_p;

void Cam_Init(camera_p cam);                                       // set default camera parameters + frustum initialization
void Cam_Apply(camera_p cam);                                      // set OpenGL projection matrix + model wiev matrix
void Cam_SetFovAspect(camera_p cam, GLfloat fov, GLfloat aspect);
void Cam_MoveAlong(camera_p cam, GLfloat dist);
void Cam_MoveStrafe(camera_p cam, GLfloat dist);
void Cam_MoveVertical(camera_p cam, GLfloat dist);
void Cam_DeltaRotation(camera_p cam, GLfloat angles[3]);           // rotate camera around current camera coordinate system
void Cam_SetRotation(camera_p cam, GLfloat angles[3]);             // set orientation by angles
void Cam_LookTo(camera_p cam, GLfloat to[3]);
void Cam_RecalcClipPlanes(camera_p cam);                           // recalculation of camera frustum clipplanes

flyby_camera_sequence_p FlyBySequence_Create(flyby_camera_state_p start, uint32_t count);
void FlyBySequence_Clear(flyby_camera_sequence_p s);
void FlyBySequence_SetCamera(flyby_camera_sequence_p s, camera_p cam, float t);

#endif
