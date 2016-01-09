#include "camera.h"

#include "util/vmath.h"
#include "world/core/frustum.h"

#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

namespace world
{
void Camera::apply()
{
    m_projection = glm::perspectiveFov(glm::radians(m_fov), m_width, m_height, m_nearClipping, m_farClipping);

    m_view = glm::lookAt(m_position, m_position+getViewDir(), getUpDir());

    m_viewProjection = m_projection * m_view;

    m_frustum.setFromMatrix(m_viewProjection);
}

void Camera::setFovAspect(glm::float_t fov, glm::float_t aspect)
{
    m_fov = fov;
    m_height = 2.0f * m_nearClipping * glm::tan(glm::radians(m_fov) / 2);
    m_width = m_height * aspect;
}

void Camera::moveAlong(glm::float_t dist)
{
    m_position += getViewDir() * dist;
}

void Camera::moveStrafe(glm::float_t dist)
{
    m_position += getRightDir() * dist;
}

void Camera::moveVertical(glm::float_t dist)
{
    m_position += getUpDir() * dist;
}

void Camera::shake(glm::float_t power, util::Duration time)
{
    m_shakeValue = power;
    m_shakeTime = time;
}

void Camera::applyRotation()
{
    glm::quat q = glm::rotate(glm::quat(1, 0, 0, 0), m_angles.z, { 0,1,0 });
    q *= glm::rotate(glm::quat(1, 0, 0, 0), m_angles.x, { 0,0,1 });
    q *= glm::rotate(glm::quat(1, 0, 0, 0), m_angles.y, { 1,0,0 });
    m_axes = glm::mat3_cast(q);
}

Camera::Camera()
{
    m_width = m_height = 2.0f * m_nearClipping * glm::tan(glm::radians(m_fov) / 2);

    apply();
}
} // namespace world
