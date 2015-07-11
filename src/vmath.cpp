
#include <cmath>
#include <cstring>
#include "vmath.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void vec4_SetTRRotations(btQuaternion& v, const btVector3& rot)
{
    btQuaternion qZ;
    qZ.setRotation({0,0,1}, M_PI * rot[2] / 360.0);

    btQuaternion qX;
    qX.setRotation({1,0,0}, M_PI * rot[0] / 360.0);

    btQuaternion qY;
    qY.setRotation({0,1,0}, M_PI * rot[1] / 360.0);

    v = qZ * qX * qY;
}


void Mat4_Translate(btTransform& mat, const btVector3& v)
{
    mat.getOrigin() += mat.getBasis() * v;
}

void Mat4_Translate(btTransform& mat, btScalar x, btScalar y, btScalar z)
{
    Mat4_Translate(mat, btVector3(x,y,z));
}

void Mat4_Scale(btTransform& mat, btScalar x, btScalar y, btScalar z)
{
    mat.getBasis().getColumn(0)[0] *= x;
    mat.getBasis().getColumn(0)[1] *= y;
    mat.getBasis().getColumn(0)[2] *= z;
}

void Mat4_RotateX(btTransform& mat, btScalar ang)
{
    btScalar tmp = ang * M_PI / 180.0;
    btScalar sina = sin(tmp);
    btScalar cosa = cos(tmp);

    btVector3 R[2];
    R[0] = mat.getBasis().getColumn(1) * cosa + mat.getBasis().getColumn(2) * sina;
    R[1] =-mat.getBasis().getColumn(1) * sina + mat.getBasis().getColumn(2) * cosa;

    mat.getBasis().getColumn(1) = R[0];
    mat.getBasis().getColumn(2) = R[1];
}

void Mat4_RotateY(btTransform& mat, btScalar ang)
{
    btScalar tmp = ang * M_PI / 180.0;
    btScalar sina = sin(tmp);
    btScalar cosa = cos(tmp);

    btVector3 R[2];
    R[0] = mat.getBasis().getColumn(0) * cosa - mat.getBasis().getColumn(2) * sina;
    R[1] = mat.getBasis().getColumn(0) * sina + mat.getBasis().getColumn(2) * cosa;

    mat.getBasis().getColumn(0) = R[0];
    mat.getBasis().getColumn(2) = R[1];
}

void Mat4_RotateZ(btTransform& mat, btScalar ang)
{
    btScalar tmp = ang * M_PI / 180.0;
    btScalar sina = sin(tmp);
    btScalar cosa = cos(tmp);

    btVector3 R[2];
    R[0] = mat.getBasis().getColumn(0) * cosa +  mat.getBasis().getColumn(1) * sina;
    R[1] =-mat.getBasis().getColumn(0) * sina +  mat.getBasis().getColumn(1) * cosa;

    mat.getBasis().getColumn(0) = R[0];
    mat.getBasis().getColumn(1) = R[1];
}
