
#ifndef VMATH_H
#define VMATH_H

#include <cmath>
#include <array>
#include "bullet/LinearMath/btScalar.h"
#include "bullet/LinearMath/btVector3.h"
#include "bullet/LinearMath/btMatrix3x3.h"
#include "bullet/LinearMath/btQuaternion.h"
#include "bullet/LinearMath/btTransform.h"

#define PLANE_X        1
#define PLANE_Y        2
#define PLANE_Z        3

struct Plane
{
    btVector3 normal;
    btScalar dot;

    btScalar distance(const btVector3& pos) const
    {
        return dot + normal.dot(pos);
    }

    btVector3 rayIntersect(const btVector3& rayStart, const btVector3& rayDir, btScalar& lambda) const
    {
        lambda = -(dot + normal.dot(rayStart));
        lambda /= normal.dot(rayDir);
        return rayStart + lambda * rayDir;
    }

    btVector3 rayIntersect(const btVector3& rayStart, const btVector3& rayDir) const
    {
        btScalar t;
        return rayIntersect(rayStart, rayDir, t);
    }

    void assign(const btVector3& v1, const btVector3& v2, const btVector3& pos)
    {
        normal = v1.cross(v2).normalized();
        dot = -normal.dot( pos );
    }

    void assign(const btVector3& n, const btVector3& pos)
    {
        normal = n.normalized();
        dot = -normal.dot( pos );
    }

    void mirrorNormal()
    {
        normal = -normal;
        dot = -dot;
    }

    void moveTo(const btVector3& where)
    {
        dot = -normal.dot(where);
    }
};

void vec4_SetTRRotations(btQuaternion &v, const btVector3 &rot);
/*
 * Matrix transformation functions and macro
 */

void Mat4_Translate(btTransform &mat, const btVector3 &v);
void Mat4_Translate(btTransform &mat, btScalar x, btScalar y, btScalar z);
void Mat4_Scale(btTransform &mat, btScalar x, btScalar y, btScalar z);
void Mat4_RotateX(btTransform &mat, btScalar ang);
void Mat4_RotateY(btTransform &mat, btScalar ang);
void Mat4_RotateZ(btTransform &mat, btScalar ang);

#endif  // VMATH_H
