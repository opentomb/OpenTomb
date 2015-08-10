#pragma once

#include <memory>

#include <GL/glew.h>

#include "matrix4.h"
#include "vmath.h"

#define TR_CAM_MAX_SHAKE_DISTANCE   8192.0f
#define TR_CAM_DEFAULT_SHAKE_POWER  100.0f

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

struct Room;
struct Polygon;
struct Frustum;

struct Camera
{
    btVector3 m_pos{ 0,0,0 };                 // camera position
    btVector3 m_prevPos{ 0,0,0 };            // previous camera position
    btVector3 m_viewDir{ 0,0,1 };            // view cameradirection
    btVector3 m_upDir{ 0,1,0 };              // up vector
    btVector3 m_rightDir{ 1,0,0 };           // strafe vector
    btVector3 m_ang;                 // camera orientation

    matrix4 m_glViewMat = matrix4();
    matrix4 m_glProjMat = matrix4();
    matrix4 m_glViewProjMat = matrix4();

    Plane m_clipPlanes[4];        // frustum side clip planes
    std::shared_ptr<Frustum> frustum;               // camera frustum structure

    GLfloat m_distNear = 1;
    GLfloat m_distFar = 65536;

    GLfloat m_fov = 75;
    GLfloat m_aspect = 1;
    GLfloat m_f;
    GLfloat m_height;
    GLfloat m_width;

    GLfloat m_shakeValue = 0;
    GLfloat m_shakeTime = 0;

    int8_t m_targetDir = 1;//Target rotation direction (0 = Back, 1 = Front, 2 = Left, 3 = Right)

    Room* m_currentRoom = nullptr;

    Camera();
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    void apply();
    void setFovAspect(GLfloat fov, GLfloat aspect);
    void moveAlong(GLfloat dist);
    void moveStrafe(GLfloat dist);
    void moveVertical(GLfloat dist);
    void shake(GLfloat power, GLfloat time);
    void deltaRotation(const btVector3 &angles);
    void setRotation(const btVector3& angles);
    void recalcClipPlanes();
};

// Static camera / sink structure.
// In original engines, static cameras and sinks shared the same structure,
// albeit with different field meanings. In compiled level, it is unfortunately
// impossible to tell camera from sink, so the only way is to share the struct
// between these two types of objects.
// Thanks to b122251 for extra info describing this structure.

struct StatCameraSink
{
    GLfloat                     x;
    GLfloat                     y;
    GLfloat                     z;
    uint16_t                    room_or_strength;   // Room for camera, strength for sink.
    uint16_t                    flag_or_zone;       // Flag for camera, zone for sink.
};

// Flyby camera structure.

struct FlybyCamera
{
    GLfloat     cam_x;      // Camera position vector
    GLfloat     cam_y;
    GLfloat     cam_z;

    GLfloat     target_x;   // Target orientation vector
    GLfloat     target_y;
    GLfloat     target_z;

    GLfloat     fov;
    GLfloat     roll;
    GLfloat     speed;

    uint32_t    sequence;   // Sequence number to which camera belongs
    uint32_t    index;      // Index in sequence
    uint32_t    timer;      // How much to sit there
    uint32_t    room_id;

    uint16_t    flags;      // See TRLE manual
};
