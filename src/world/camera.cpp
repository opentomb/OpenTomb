#include "camera.h"

#include "util/vmath.h"
#include "world/core/frustum.h"

#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

namespace world
{
void Camera::apply()
{
    m_glProjMat = glm::perspectiveFov(glm::radians(m_fov), m_width, m_height, m_distNear, m_distFar);

    m_glViewMat = glm::lookAt(m_pos, m_pos+getViewDir(), getUpDir());

    m_glViewProjMat = m_glProjMat * m_glViewMat;
}

void Camera::setFovAspect(glm::float_t fov, glm::float_t aspect)
{
    m_fov = fov;
    m_height = 2.0f * m_distNear * glm::tan(glm::radians(m_fov) / 2);
    m_width = m_height * aspect;
}

void Camera::moveAlong(glm::float_t dist)
{
    m_pos += getViewDir() * dist;
}

void Camera::moveStrafe(glm::float_t dist)
{
    m_pos += getRightDir() * dist;
}

void Camera::moveVertical(glm::float_t dist)
{
    m_pos += getUpDir() * dist;
}

void Camera::shake(glm::float_t power, glm::float_t time)
{
    m_shakeValue = power;
    m_shakeTime = time;
}

void Camera::setRotation(const glm::vec3& angles) //angles = {OX, OY, OZ}
{
    glm::quat q = glm::rotate(glm::quat(1, 0, 0, 0), angles.z, { 0,1,0 });
    q *= glm::rotate(glm::quat(1, 0, 0, 0), angles.x, { 0,0,1 });
    q *= glm::rotate(glm::quat(1, 0, 0, 0), angles.y, { 1,0,0 });
    m_axes = glm::mat3_cast(q);
}

void Camera::recalcClipPlanes()
{
    frustum.planes.resize(6);

    // Extract frustum planes from matrix
    // Planes are in format: normal(xyz), offset(w)

    auto matr = glm::transpose(m_glViewProjMat);

    // right
    frustum.planes[0].assign(matr[3] - matr[0]);
    // left
    frustum.planes[1].assign(matr[3] + matr[0]);
    // bottom
    frustum.planes[2].assign(matr[3] + matr[1]);
    // top
    frustum.planes[3].assign(matr[3] - matr[1]);
    // far
    frustum.planes[4].assign(matr[3] - matr[2]);
    // near
    frustum.planes[5].assign(matr[3] + matr[2]);
}

Camera::Camera()
{
    m_width = m_height = 2.0f * m_distNear * glm::tan(glm::radians(m_fov) / 2);

    recalcClipPlanes();
}
} // namespace world
