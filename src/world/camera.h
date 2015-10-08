#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "world/core/frustum.h"

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

namespace world
{
struct Room;

namespace core
{
struct Polygon;
struct Frustum;
} // namespace core

enum class CameraTarget
{
    Back,
    Front,
    Left,
    Right
};

namespace
{
constexpr float MaxShakeDistance = 8192;
constexpr float DefaultShakePower = 100;
}

class Camera
{
    glm::vec3 m_pos{ 0,0,0 };                 // camera position
    glm::mat3 m_axes = glm::mat3(1.0f);       // coordinate system axes
public:
    const glm::vec3& getPosition() const
    {
        return m_pos;
    }

    void setPosition(const glm::vec3& pos)
    {
        m_pos = pos;
    }

    const glm::vec3& getViewDir() const
    {
        return m_axes[1];
    }

    const glm::vec3& getRightDir() const
    {
        return m_axes[0];
    }

    const glm::vec3& getUpDir() const
    {
        return m_axes[2];
    }

    glm::vec3 m_prevPos{ 0,0,0 };            // previous camera position

    glm::mat4 m_glViewMat = glm::mat4(1.0f);
    glm::mat4 m_glProjMat = glm::mat4(1.0f);
    glm::mat4 m_glViewProjMat = glm::mat4(1.0f);

    core::Frustum frustum;               // camera frustum structure

    glm::float_t m_distNear = 1;
    glm::float_t m_distFar = 65536;

    glm::float_t m_fov = 75;
    glm::float_t m_width;
    glm::float_t m_height;

    glm::float_t m_shakeValue = 0;
    glm::float_t m_shakeTime = 0;

    CameraTarget m_targetDir = CameraTarget::Front;//Target rotation direction (0 = Back, 1 = Front, 2 = Left, 3 = Right)

    Room* m_currentRoom = nullptr;

    Camera();
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    void apply();
    void setFovAspect(glm::float_t fov, glm::float_t aspect);
    void moveAlong(glm::float_t dist);
    void moveStrafe(glm::float_t dist);
    void moveVertical(glm::float_t dist);
    void shake(glm::float_t power, glm::float_t time);
    void setRotation(const glm::vec3& angles);
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
    glm::float_t                     x;
    glm::float_t                     y;
    glm::float_t                     z;
    uint16_t                    room_or_strength;   // Room for camera, strength for sink.
    uint16_t                    flag_or_zone;       // Flag for camera, zone for sink.
};

// Flyby camera structure.

struct FlybyCamera
{
    glm::float_t     cam_x;      // Camera position vector
    glm::float_t     cam_y;
    glm::float_t     cam_z;

    glm::float_t     target_x;   // Target orientation vector
    glm::float_t     target_y;
    glm::float_t     target_z;

    glm::float_t     fov;
    glm::float_t     roll;
    glm::float_t     speed;

    uint32_t    sequence;   // Sequence number to which camera belongs
    uint32_t    index;      // Index in sequence
    uint32_t    timer;      // How much to sit there
    uint32_t    room_id;

    uint16_t    flags;      // See TRLE manual
};

} // namespace world
