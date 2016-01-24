#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "util/helpers.h"
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
class Frustum;
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
const glm::float_t CameraRotationSpeed = glm::radians(1.0f);
} // anonymous namespace

constexpr float MaxShakeDistance = 8192;
constexpr float DefaultShakePower = 100;

class Camera
{
private:
    glm::vec3 m_position{ 0,0,0 };
    glm::mat3 m_axes = glm::mat3(1.0f);       // coordinate system axes

    glm::vec3 m_prevPos{ 0,0,0 };            // previous camera position

    glm::mat4 m_view = glm::mat4(1.0f);
    glm::mat4 m_projection = glm::mat4(1.0f);
    glm::mat4 m_viewProjection = glm::mat4(1.0f);

    core::Frustum m_frustum;

    glm::float_t m_nearClipping = 1;
    glm::float_t m_farClipping = 65536;

    glm::float_t m_fov = 75;
    glm::float_t m_width;
    glm::float_t m_height;

    glm::float_t m_shakeValue = 0;
    util::Duration m_shakeTime{ 0 };

    CameraTarget m_targetDir = CameraTarget::Front; //Target rotation direction

    Room* m_currentRoom = nullptr;

    glm::vec3 m_angles{ 0,0,0 };

public:
    const glm::vec3& getPosition() const noexcept
    {
        return m_position;
    }

    glm::vec3 getMovement() const noexcept
    {
        return m_position - m_prevPos;
    }

    void resetMovement() noexcept
    {
        m_prevPos = m_position;
    }

    void setPosition(const glm::vec3& pos) noexcept
    {
        m_position = pos;
    }

    const glm::vec3& getViewDir() const noexcept
    {
        return m_axes[1];
    }

    const glm::vec3& getRightDir() const noexcept
    {
        return m_axes[0];
    }

    const glm::vec3& getUpDir() const noexcept
    {
        return m_axes[2];
    }

    const glm::mat4& getProjection() const noexcept
    {
        return m_projection;
    }

    const glm::mat4& getView() const noexcept
    {
        return m_view;
    }

    const glm::mat4& getViewProjection() const noexcept
    {
        return m_viewProjection;
    }

    Room* getCurrentRoom() noexcept
    {
        return m_currentRoom;
    }

    void setCurrentRoom(Room* room) noexcept
    {
        m_currentRoom = room;
    }

    const core::Frustum& getFrustum() const noexcept
    {
        return m_frustum;
    }

    CameraTarget getTargetDir() const noexcept
    {
        return m_targetDir;
    }

    void setTargetDir(CameraTarget target) noexcept
    {
        m_targetDir = target;
    }

    glm::float_t getShakeValue() const noexcept
    {
        return m_shakeValue;
    }

    const util::Duration& getShakeTime() const noexcept
    {
        return m_shakeTime;
    }

    void setShakeTime(const util::Duration& d) noexcept
    {
        m_shakeTime = d;
    }

    const glm::vec3& getAngles() const noexcept
    {
        return m_angles;
    }

    void shake(glm::float_t currentAngle, glm::float_t targetAngle, const util::Duration& frameTime)
    {
        constexpr glm::float_t RotationSpeed = 2.0; //Speed of rotation

        glm::float_t d_angle = m_angles[0] - targetAngle;
        if(d_angle > util::Rad90)
        {
            d_angle -= CameraRotationSpeed;
        }
        if(d_angle < -util::Rad90)
        {
            d_angle += CameraRotationSpeed;
        }
        m_angles[0] = glm::mod(m_angles[0] + glm::atan(glm::sin(currentAngle - d_angle), glm::cos(currentAngle + d_angle)) * util::toSeconds(frameTime) * RotationSpeed, util::Rad360);
    }

    Camera();
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    void apply();
    void setFovAspect(glm::float_t fov, glm::float_t aspect);
    void moveAlong(glm::float_t dist);
    void moveStrafe(glm::float_t dist);
    void moveVertical(glm::float_t dist);
    void move(const glm::vec3& v)
    {
        moveAlong(v.z);
        moveStrafe(v.x);
        moveVertical(v.y);
    }

    void shake(glm::float_t power, util::Duration time);
    void applyRotation();
    void rotate(const glm::vec3& v)
    {
        m_angles += v;
    }
};

// Static camera / sink structure.
// In original engines, static cameras and sinks shared the same structure,
// albeit with different field meanings. In compiled level, it is unfortunately
// impossible to tell camera from sink, so the only way is to share the struct
// between these two types of objects.
// Thanks to b122251 for extra info describing this structure.

struct StatCameraSink
{
    glm::vec3 position;
    uint16_t room_or_strength;   // Room for camera, strength for sink.
    uint16_t flag_or_zone;       // Flag for camera, zone for sink.
};

// Flyby camera structure.

struct FlybyCamera
{
    glm::vec3 position;
    glm::vec3 rotation;

    glm::float_t fov;
    glm::float_t roll;
    glm::float_t speed;

    uint32_t    sequence;   // Sequence number to which camera belongs
    uint32_t    index;      // Index in sequence
    uint32_t    timer;      // How much to sit there
    uint32_t    room_id;

    uint16_t    flags;      // See TRLE manual
};
} // namespace world
