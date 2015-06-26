
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include <cstdlib>
#include <cmath>

#include "gl_util.h"
#include "camera.h"
#include "vmath.h"
#include "polygon.h"
#include "frustum.h"

void Camera::apply()
{
    btMatrix3x3& M = m_glProjMat.getBasis();
    M.setIdentity();
    M[0][0] = m_f / m_aspect;
    M[1][1] = m_f;
    M[2][2] = (m_distNear + m_distFar) / (m_distNear - m_distFar);
    M[2][3] =-1.0;
    M[3][2] = 2.0 * m_distNear * m_distFar / (m_distNear - m_distFar);

    btMatrix3x3& M2 = m_glViewMat.getBasis();
    M2[0] = m_rightDir;
    M2[1] = m_upDir;
    M2[2] = -m_viewDir;
    M2 = M2.transpose();

    m_glViewMat.getOrigin() = M2 * m_pos;
    m_glViewMat.getOrigin().setW(1);

    m_glViewProjMat = m_glProjMat * m_glViewMat;
}

void Camera::setFovAspect(GLfloat fov, GLfloat aspect)
{
    m_fov = fov;
    m_aspect = aspect;
    m_f = tanf(M_PI * m_fov / 360.0);
    m_height = 2.0 * m_distNear * m_f;
    m_width = m_height * aspect;
    m_f = 1.0 / m_f;
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
    m_shakeTime  = time;
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

    m_upDir = {0,0,1};

    m_viewDir = btVector3(0,1,0).rotate(m_upDir, angles.x());
    m_rightDir = btVector3(1,0,0).rotate(m_upDir, angles.x());

    m_upDir = m_upDir.rotate(m_rightDir, angles.y());
    m_viewDir = m_viewDir.rotate(m_rightDir, angles.y());

    m_rightDir = m_rightDir.rotate(m_viewDir, angles.z());
    m_upDir = m_upDir.rotate(m_viewDir, angles.z());
}

void Camera::recalcClipPlanes()
{
    const btVector3 nearViewPoint = m_viewDir * m_distNear;

    //==========================================================================

    frustum->norm = m_viewDir;                               // Основная плоскость отсечения (что сзади - то не рисуем)
    frustum->norm[3] = -m_viewDir.dot(m_pos);                 // плоскость проекции проходит через наблюдателя.

    //==========================================================================

    //   DOWN
    btVector3 LU;
    LU = nearViewPoint - m_height / 2.0 * m_upDir;                                       // вектор нижней плоскости отсечения

    m_clipPlanes[2] = m_rightDir.cross(LU);
    LU[3] = m_clipPlanes[2].length();                                      // модуль нормали к левой / правой плоскостям
    m_clipPlanes[2].normalize();
    m_clipPlanes[2][3] = -m_clipPlanes[2].dot(m_pos);

    //   UP
    LU = nearViewPoint + m_height / 2.0 * m_upDir;                                       // вектор верхней плоскости отсечения

    m_clipPlanes[3] = m_rightDir.cross(LU);
    m_clipPlanes[3].normalize();
    m_clipPlanes[3][3] = -m_clipPlanes[2].dot(m_pos);

    //==========================================================================

    //   LEFT
    LU = nearViewPoint - m_width / 2.0 * m_rightDir;                                    // вектор левой плоскости отсечения

    m_clipPlanes[0] = m_upDir.cross(LU);
    LU[3] = m_clipPlanes[0].length();                                         // модуль нормали к левой / правой плоскостям
    m_clipPlanes[0].normalize();
    m_clipPlanes[0][3] = -m_clipPlanes[2].dot(m_pos);

    //   RIGHT
    LU = nearViewPoint + m_width / 2.0 * m_rightDir;                                    // вектор правой плоскости отсечения

    m_clipPlanes[1] = m_upDir.cross(LU);
    m_clipPlanes[1].normalize();
    m_clipPlanes[1][3] = -m_clipPlanes[2].dot(m_pos);

    auto worldNearViewPoint = m_pos + m_viewDir * m_distNear;
    for(int i=0; i<4; ++i)
        if(planeDist(m_clipPlanes[i], worldNearViewPoint) < 0.0)
            m_clipPlanes[i] = -m_clipPlanes[i];

    frustum->vertices[0] = m_pos + m_viewDir;
}


Camera::Camera()
{
    m_f = 1.0 / std::tan(M_PI * m_fov / 360.0);
    m_height = 2.0 * m_distNear / m_f;
    m_width = m_height * m_aspect;

    frustum = new frustum_s;
    frustum->cam_pos = &m_pos;
    frustum->vertices.resize(3);
    frustum->next = nullptr;
    frustum->parent = nullptr;
    frustum->parents_count = 0;
    frustum->planes.assign( m_clipPlanes+0, m_clipPlanes+4 );
}
