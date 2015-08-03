#include <cmath>

#include "vmath.h"

void vec4_SetTRRotations(btQuaternion& v, const btVector3& rot)
{
    btQuaternion qZ;
    qZ.setRotation({ 0,0,1 }, rot[2] * RadPerDeg);

    btQuaternion qX;
    qX.setRotation({ 1,0,0 }, rot[0] * RadPerDeg);

    btQuaternion qY;
    qY.setRotation({ 0,1,0 }, rot[1] * RadPerDeg);

    v = qZ * qX * qY;
}

void Mat4_Translate(btTransform& mat, const btVector3& v)
{
    mat.getOrigin() += mat.getBasis() * v;
}

void Mat4_Translate(btTransform& mat, btScalar x, btScalar y, btScalar z)
{
    Mat4_Translate(mat, btVector3(x, y, z));
}

void Mat4_Scale(btTransform& mat, btScalar x, btScalar y, btScalar z)
{
    mat.getBasis() = mat.getBasis().scaled(btVector3(x, y, z));
}

void Mat4_RotateX(btTransform& mat, btScalar ang)
{
    btScalar tmp = ang * RadPerDeg;
    btScalar sina = std::sin(tmp);
    btScalar cosa = std::cos(tmp);

    auto m = mat.getBasis().transpose();
    m[1] = mat.getBasis().getColumn(1) * cosa + mat.getBasis().getColumn(2) * sina;
    m[2] = -mat.getBasis().getColumn(1) * sina + mat.getBasis().getColumn(2) * cosa;

    mat.getBasis() = m.transpose();
}

void Mat4_RotateY(btTransform& mat, btScalar ang)
{
    btScalar tmp = ang * RadPerDeg;
    btScalar sina = std::sin(tmp);
    btScalar cosa = std::cos(tmp);

    auto m = mat.getBasis().transpose();
    m[0] = mat.getBasis().getColumn(0) * cosa - mat.getBasis().getColumn(2) * sina;
    m[2] = mat.getBasis().getColumn(0) * sina + mat.getBasis().getColumn(2) * cosa;

    mat.getBasis() = m.transpose();
}

void Mat4_RotateZ(btTransform& mat, btScalar ang)
{
    btScalar tmp = ang * RadPerDeg;
    btScalar sina = std::sin(tmp);
    btScalar cosa = std::cos(tmp);

    auto m = mat.getBasis().transpose();
    m[0] = mat.getBasis().getColumn(0) * cosa + mat.getBasis().getColumn(1) * sina;
    m[1] = -mat.getBasis().getColumn(0) * sina + mat.getBasis().getColumn(1) * cosa;

    mat.getBasis() = m.transpose();
}