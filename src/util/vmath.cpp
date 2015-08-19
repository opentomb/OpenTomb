#include "vmath.h"

#include <cmath>

namespace util
{

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

btQuaternion Quat_Slerp(const btQuaternion& q1, const btQuaternion& q2, const btScalar& t)
{
    const btScalar magnitude = btSqrt(q1.length2() * q2.length2());
    btAssert(magnitude > btScalar(0));

    const btScalar product = q1.dot(q2) / magnitude;
    const btScalar absproduct = btFabs(product);

    if(absproduct < btScalar(1.0 - SIMD_EPSILON))
    {
        const btScalar theta = btAcos(absproduct);
        const btScalar d = btSin(theta);
        btAssert(d > btScalar(0))

        const btScalar sign = (product < 0) ? btScalar(-1) : btScalar(1);
        const btScalar s0 = btSin((btScalar(1.0) - t) * theta) / d;
        const btScalar s1 = btSin(sign * t * theta) / d;

        return btQuaternion(
            (q1.x() * s0 + q2.x() * s1),
            (q1.y() * s0 + q2.y() * s1),
            (q1.z() * s0 + q2.z() * s1),
            (q1.w() * s0 + q2.w() * s1));
    }
    else
    {
        return btQuaternion(q1);
    }
}

} // namespace util
