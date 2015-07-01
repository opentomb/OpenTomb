
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

inline btScalar planeDist(const btVector3& p, const btVector3& dot)
{
    return p[3] + p[0]*dot[0] + p[1]*dot[1] + p[2]*dot[2];
}

/**
 * p - point of ray entrance                                                    in
 * v - ray direction                                                            in
 * n - plane equaton                                                            in
 * dot - ray and plane intersection                                             out
 * t - parametric intersection coordinate
 */
inline void rayPlaneIntersect(const btVector3& p, const btVector3& v, const btVector3& n, btVector3* dot, btScalar* t)
{
    *t = -(n[3] + n[0]*p[0] + n[1]*p[1] + n[2]*p[2]);
    *t /= (n[0]*v[0] + n[1]*v[1] + n[2]*v[2]);
    *dot = p + *t * v;
}

//vec4 - btScalar[4] (x, y, z, w)

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
