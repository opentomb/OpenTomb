
#include <cmath>
#include <cstring>
#include "vmath.h"

void vec3_RotateZ(btScalar res[3], btScalar src[3], btScalar ang)
{
    btScalar t[2], sint, cost;

    ang *= M_PI / 180.0;
    sint = sin(ang);
    cost = cos(ang);
    res[2] = src[2];
    t[0] = src[0]*cost - src[1]*sint;
    t[1] = src[0]*sint + src[1]*cost;

    res[0] = t[0];
    res[1] = t[1];
}

btQuaternion vec4_slerp(const btQuaternion& q1, const btQuaternion& q2, btScalar t)
{
    btScalar cos_fi = q1.dot(q2);
    btScalar sign = (cos_fi < 0.0)?(-1.0):(1.0);
    btScalar fi = acos(sign * cos_fi);
    btScalar sin_fi = sin(fi);

    btScalar k1, k2;
    if((fabs(sin_fi) > 0.00001) && (t > 0.0001))
    {
        k1 = sin(fi * (1.0 - t)) / sin_fi;
        k2 = sin(fi * t * sign) / sin_fi;
    }
    else
    {
        k1 = 1.0 - t;
        k2 = t;
    }

    return (q1*k1 + q2*k2).normalized();
}

void vec4_SetTRRotations(btQuaternion& v, const btVector3& rot)
{
    btScalar angle, sin_t2, cos_t2;

    // OZ    Mat4_RotateZ(btag->transform, btag->rotate[2]);
    angle = M_PI * rot[2] / 360.0;
    sin_t2 = sin(angle);
    cos_t2 = cos(angle);

    btQuaternion qZ;
    qZ[0] = 0.0 * sin_t2;
    qZ[1] = 0.0 * sin_t2;
    qZ[2] = 1.0 * sin_t2;
    qZ[3] = cos_t2;

    // OX    Mat4_RotateX(btag->transform, btag->rotate[0]);
    angle = M_PI * rot[0] / 360.0;
    sin_t2 = sin(angle);
    cos_t2 = cos(angle);

    btQuaternion qX;
    qX[0] = 1.0 * sin_t2;
    qX[1] = 0.0 * sin_t2;
    qX[2] = 0.0 * sin_t2;
    qX[3] = cos_t2;

    // OY    Mat4_RotateY(btag->transform, btag->rotate[1]);
    angle = M_PI * rot[1] / 360.0;
    sin_t2 = sin(angle);
    cos_t2 = cos(angle);

    btQuaternion qY;
    qY[0] = 0.0 * sin_t2;
    qY[1] = 1.0 * sin_t2;
    qY[2] = 0.0 * sin_t2;
    qY[3] = cos_t2;

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
    mat.getBasis()[0][0] *= x;
    mat.getBasis()[0][1] *= y;
    mat.getBasis()[0][2] *= z;
}

void Mat4_RotateX(btTransform& mat, btScalar ang)
{
    btScalar tmp = ang * M_PI / 180.0;
    btScalar sina = sin(tmp);
    btScalar cosa = cos(tmp);

    btVector3 R[2];
    R[0] = mat.getBasis()[1] * cosa + mat.getBasis()[2] * sina;
    R[1] =-mat.getBasis()[1] * sina + mat.getBasis()[2] * cosa;

    mat.getBasis()[1] = R[0];
    mat.getBasis()[2] = R[1];
}

void Mat4_RotateY(btTransform& mat, btScalar ang)
{
    btScalar tmp = ang * M_PI / 180.0;
    btScalar sina = sin(tmp);
    btScalar cosa = cos(tmp);

    btVector3 R[2];
    R[0] = mat.getBasis()[0] * cosa - mat.getBasis()[2] * sina;
    R[1] = mat.getBasis()[0] * sina + mat.getBasis()[2] * cosa;

    mat.getBasis()[0] = R[0];
    mat.getBasis()[2] = R[1];
}

void Mat4_RotateZ(btTransform& mat, btScalar ang)
{
    btScalar tmp = ang * M_PI / 180.0;
    btScalar sina = sin(tmp);
    btScalar cosa = cos(tmp);

    btVector3 R[2];
    R[0] = mat.getBasis()[0] * cosa +  mat.getBasis()[1] * sina;
    R[1] =-mat.getBasis()[0] * sina +  mat.getBasis()[1] * cosa;

    mat.getBasis()[0] = R[0];
    mat.getBasis()[1] = R[1];
}
