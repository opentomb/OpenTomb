#include "camera.h"

#include <cmath>
#include <cassert>

#include "frustum.h"
#include "vmath.h"

void Camera::apply()
{
    m_glProjMat[0][0] = m_f / m_aspect;
    m_glProjMat[0][1] = 0.0;
    m_glProjMat[0][2] = 0.0;
    m_glProjMat[0][3] = 0.0;

    m_glProjMat[1][0] = 0.0;
    m_glProjMat[1][1] = m_f;
    m_glProjMat[1][2] = 0.0;
    m_glProjMat[1][3] = 0.0;

    m_glProjMat[2][0] = 0.0;
    m_glProjMat[2][1] = 0.0;
    m_glProjMat[2][2] = (m_distNear + m_distFar) / (m_distNear - m_distFar);
    m_glProjMat[2][3] = -1.0;

    m_glProjMat[3][0] = 0.0;
    m_glProjMat[3][1] = 0.0;
    m_glProjMat[3][2] = 2.0f * m_distNear * m_distFar / (m_distNear - m_distFar);
    m_glProjMat[3][3] = 0.0;

    m_glViewMat[0][0] = m_rightDir[0];
    m_glViewMat[1][0] = m_rightDir[1];
    m_glViewMat[2][0] = m_rightDir[2];

    m_glViewMat[0][1] = m_upDir[0];
    m_glViewMat[1][1] = m_upDir[1];
    m_glViewMat[2][1] = m_upDir[2];

    m_glViewMat[0][2] = -m_viewDir[0];
    m_glViewMat[1][2] = -m_viewDir[1];
    m_glViewMat[2][2] = -m_viewDir[2];

    m_glViewMat[3][0] = -(m_glViewMat[0][0] * m_pos[0] + m_glViewMat[1][0] * m_pos[1] + m_glViewMat[2][0] * m_pos[2]);
    m_glViewMat[3][1] = -(m_glViewMat[0][1] * m_pos[0] + m_glViewMat[1][1] * m_pos[1] + m_glViewMat[2][1] * m_pos[2]);
    m_glViewMat[3][2] = -(m_glViewMat[0][2] * m_pos[0] + m_glViewMat[1][2] * m_pos[1] + m_glViewMat[2][2] * m_pos[2]);

    m_glViewMat[0][3] = 0.0;
    m_glViewMat[1][3] = 0.0;
    m_glViewMat[2][3] = 0.0;
    m_glViewMat[3][3] = 1.0;

    m_glViewProjMat = m_glProjMat * m_glViewMat;
}

void Camera::setFovAspect(GLfloat fov, GLfloat aspect)
{
    m_fov = fov;
    m_aspect = aspect;
    m_f = std::tan(m_fov * RadPerDeg / 2);
    m_height = 2.0f * m_distNear * m_f;
    m_width = m_height * aspect;
    m_f = 1.0f / m_f;
}

void Camera::moveAlong(GLfloat dist)
{
    m_pos += m_viewDir * dist;
}

void Camera::moveStrafe(GLfloat dist)
{
    m_pos += m_rightDir * dist;
}

void Camera::moveVertical(GLfloat dist)
{
    m_pos += m_upDir * dist;
}

void Camera::shake(GLfloat power, GLfloat time)
{
    m_shakeValue = power;
    m_shakeTime = time;
}

void Camera::deltaRotation(const btVector3& angles)                         //angles = {OX, OY, OZ}
{
    m_ang += angles;

    // Roll
    m_upDir = m_upDir.rotate(m_viewDir, angles.z());
    // Pitch
    m_viewDir = m_viewDir.rotate(m_upDir, angles.x());
    // Yaw
    m_viewDir = m_viewDir.rotate(m_rightDir, angles.y());
    m_upDir = m_viewDir.rotate(m_rightDir, angles.y());
}

void Camera::setRotation(const btVector3& angles)                          //angles = {OX, OY, OZ}
{
    m_ang = angles;

    m_upDir = { 0,0,1 };

    m_viewDir = btVector3(0, 1, 0).rotate(m_upDir, angles.x());
    m_rightDir = btVector3(1, 0, 0).rotate(m_upDir, angles.x());

    m_upDir = m_upDir.rotate(m_rightDir, angles.y());
    m_viewDir = m_viewDir.rotate(m_rightDir, angles.y());

    m_rightDir = m_rightDir.rotate(m_viewDir, angles.z());
    m_upDir = m_upDir.rotate(m_viewDir, angles.z());
}

void Camera::recalcClipPlanes()
{
    const btVector3 nearViewPoint = m_viewDir * m_distNear;

    //==========================================================================

    frustum->norm.assign(m_viewDir, m_pos);                               // Основная плоскость отсечения (что сзади - то не рисуем)

    //==========================================================================

    //   DOWN
    btVector3 LU;
    LU = nearViewPoint - m_height / 2.0f * m_upDir;                                       // вектор нижней плоскости отсечения
    m_clipPlanes[2].assign(m_rightDir, LU, m_pos);

    //   UP
    LU = nearViewPoint + m_height / 2.0f * m_upDir;                                       // вектор верхней плоскости отсечения
    m_clipPlanes[2].assign(m_rightDir, LU, m_pos);

    //==========================================================================

    //   LEFT
    LU = nearViewPoint - m_width / 2.0f * m_rightDir;                                    // вектор левой плоскости отсечения
    m_clipPlanes[2].assign(m_upDir, LU, m_pos);

    //   RIGHT
    LU = nearViewPoint + m_width / 2.0f * m_rightDir;                                    // вектор правой плоскости отсечения
    m_clipPlanes[2].assign(m_upDir, LU, m_pos);

    auto worldNearViewPoint = m_pos + m_viewDir * m_distNear;
    // Ensure that normals point outside
    for(int i = 0; i < 4; ++i)
        if(m_clipPlanes[i].distance(worldNearViewPoint) < 0.0)
            m_clipPlanes[i].mirrorNormal();

    assert(!frustum->vertices.empty());
    frustum->vertices[0] = m_pos + m_viewDir;
}

Camera::Camera()
{
    m_f = 1.0f / std::tan(m_fov * RadPerDeg / 2);
    m_height = 2.0f * m_distNear / m_f;
    m_width = m_height * m_aspect;

    frustum = std::make_shared<Frustum>();
    frustum->cam_pos = &m_pos;
    frustum->vertices.resize(3, { 0,0,0 });
    frustum->planes.assign(m_clipPlanes + 0, m_clipPlanes + 4);
}
