
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
#define CAMERA_STATE_LOOK_AT    (0x0001)
#define CAMERA_STATE_FIXED      (0x0002)
#define CAMERA_STATE_FLYBY      (0x0003)


typedef struct camera_flags_s
{
    uint16_t        smoothly_init : 1;
    uint16_t        subjective : 1;
    uint16_t        infinitely_loop : 1;
    uint16_t        track_player : 1;
    uint16_t        last_head_position : 1;
    uint16_t        track_head_position : 1;
    uint16_t        smoothly_end : 1;
    uint16_t        switch_to_other_camera : 1;   // see timer field for camera number
    uint16_t        pause_movement_camera : 1;    // see timer field for waiting time
    uint16_t        disable_breakout : 1;
    uint16_t        disable_controls : 1;
    uint16_t        enable_controls : 1;
    uint16_t        fade_in : 1;
    uint16_t        fade_out : 1;
    uint16_t        heavy_triggering : 1;
    uint16_t        : 1;
    //Bit 15 —  TR5 only TRLE for TR5 says this flag is used to make camera one-shot, but it’s not true. Actual one-shot flag is placed in extra uint16_t field at 0x0100 for flyby camera TrigAction.*/
}camera_flags_t, *camera_flags_p;

typedef struct camera_state_s
{
    uint32_t                        state;
    uint32_t                        target_id;
    struct flyby_camera_sequence_s *flyby;
    struct static_camera_sink_s    *sink;

    GLfloat                         shake_value;
    GLfloat                         time;
    int                             move;
    float                           cutscene_tr[16];
    float                           entity_offset_x;
    float                           entity_offset_z;
    int8_t                          target_dir; //Target rotation direction (0 = Back, 1 = Front, 2 = Left, 3 = Right)
}camera_state_t, camera_state_p;


typedef struct camera_s
{
    GLfloat                     gl_transform[16] __attribute__((packed, aligned(16)));
    GLfloat                     gl_view_mat[16] __attribute__((packed, aligned(16)));
    GLfloat                     gl_proj_mat[16] __attribute__((packed, aligned(16)));
    GLfloat                     gl_view_proj_mat[16] __attribute__((packed, aligned(16)));

    GLfloat                     clip_planes[16];        // frustum side clip planes
    GLfloat                     prev_pos[3];            // previous camera position
    GLfloat                     ang[3];                 // camera orientation ZXY
    struct frustum_s           *frustum;                // camera frustum structure
    GLfloat                     dist_near;
    GLfloat                     dist_far;

    GLfloat                     fov;
    GLfloat                     aspect;
    GLfloat                     f;
    GLfloat                     h;
    GLfloat                     w;

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
    GLfloat                     pos[3];
    uint16_t                    locked : 1;
    uint16_t                    room_or_strength : 15;   // Room for camera, strength for sink.
    uint16_t                    flag_or_zone;            // Flag for camera, zone for sink.
}static_camera_sink_t, *static_camera_sink_p;

typedef struct camera_frame_s
{
    float                       pos[3];
    float                       target[3];

    float                       fov;
    float                       roll;
    float                       timer;
    float                       speed;

    int8_t                      sequence;
    int8_t                      index;
    struct camera_flags_s       flags;
    struct room_s              *room;
}camera_frame_t, *camera_frame_p;

typedef struct flyby_camera_sequence_s
{
    struct camera_frame_s          *start;
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
void Cam_SetRoll(camera_p cam, GLfloat roll);                      // set roll only
void Cam_MoveTo(camera_p cam, GLfloat to[3], GLfloat max_dist);
void Cam_LookTo(camera_p cam, GLfloat to[3]);
void Cam_RecalcClipPlanes(camera_p cam);                           // recalculation of camera frustum clipplanes
void Cam_SetFrame(camera_p cam, camera_frame_p a, camera_frame_p b, float offset[3], float lerp);

flyby_camera_sequence_p FlyBySequence_Create(camera_frame_p start, uint32_t count);
void FlyBySequence_Clear(flyby_camera_sequence_p s);
void FlyBySequence_SetCamera(flyby_camera_sequence_p s, camera_p cam, float t);

#endif
