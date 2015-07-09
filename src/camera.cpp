#include "camera.h"

#include <cstdlib>
#include <cmath>

#include "gl_util.h"
#include "vmath.h"
#include "polygon.h"
#include "frustum.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    m_glProjMat[2][3] =-1.0;
    
    m_glProjMat[3][0] = 0.0;
    m_glProjMat[3][1] = 0.0;
    m_glProjMat[3][2] = 2.0 * m_distNear * m_distFar / (m_distNear - m_distFar);
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
    
    m_glViewMat[3][0] = -(m_glViewMat[0][0] * m_pos[0] + m_glViewMat[1][0] * m_pos[1] + m_glViewMat[2][0]  * m_pos[2]);
    m_glViewMat[3][1] = -(m_glViewMat[0][1] * m_pos[0] + m_glViewMat[1][1] * m_pos[1] + m_glViewMat[2][1]  * m_pos[2]);
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

    frustum = std::make_shared<Frustum>();
    frustum->cam_pos = &m_pos;
    frustum->vertices.resize(3);
    frustum->planes.assign( m_clipPlanes+0, m_clipPlanes+4 );
}
