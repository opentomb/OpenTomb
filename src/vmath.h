
#ifndef VMATH_H
#define VMATH_H

#include <cmath>
#include <array>
#include <bullet/LinearMath/btScalar.h>
#include <bullet/LinearMath/btVector3.h>
#include <bullet/LinearMath/btMatrix3x3.h>
#include <bullet/LinearMath/btQuaternion.h>
#include <bullet/LinearMath/btTransform.h>

#define PLANE_X        1
#define PLANE_Y        2
#define PLANE_Z        3

#define vec3_interpolate_macro(res, x, y, lerp, t) {\
                   (res)[0] = (x)[0] * (t) + (y)[0] * (lerp);\
                   (res)[1] = (x)[1] * (t) + (y)[1] * (lerp);\
                   (res)[2] = (x)[2] * (t) + (y)[2] * (lerp);}

inline btScalar planeDist(const btVector3& p, const btVector3& dot)
{
    return p[3] + p[0]*dot[0] + p[1]*dot[1] + p[2]*dot[2];
}

#define vec3_norm_plane(p, dot, a) {(p)[0] /= (a); (p)[1] /= (a); (p)[2] /= (a); \
                                    (p)[3] = -((p)[0]*(dot)[0] + (p)[1]*(dot)[1] + (p)[2]*(dot)[2]);}
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
    (*dot)[0] = p[0] + *t * v[0];
    (*dot)[1] = p[1] + *t * v[1];
    (*dot)[2] = p[2] + *t * v[2];
}

void vec3_RotateZ(btScalar res[3], btScalar src[3], btScalar ang);

//vec4 - btScalar[4] (x, y, z, w)

btQuaternion vec4_slerp(const btQuaternion &q1, const btQuaternion &q2, btScalar t);
void vec4_SetTRRotations(btQuaternion &v, const btVector3 &rot);
/*
 * Matrix transformation functions and macro
 */

#define Mat4_vec3_mul_inv_macro(ret, tr, src)\
{ \
    (ret)[0]  = (tr)[0] * (src)[0] + (tr)[1] * (src)[1] + (tr)[2]  * (src)[2];  /* (M^-1 * src).x*/\
    (ret)[0] -= (tr)[0] * (tr)[12]+ (tr)[1] * (tr)[13]+ (tr)[2]  * (tr)[14];    /* -= (M^-1 * mov).x*/\
    (ret)[1]  = (tr)[4] * (src)[0] + (tr)[5] * (src)[1] + (tr)[6]  * (src)[2];  /* (M^-1 * src).y*/\
    (ret)[1] -= (tr)[4] * (tr)[12]+ (tr)[5] * (tr)[13]+ (tr)[6]  * (tr)[14];    /* -= (M^-1 * mov).y*/\
    (ret)[2]  = (tr)[8] * (src)[0] + (tr)[9] * (src)[1] + (tr)[10] * (src)[2];  /* (M^-1 * src).z*/\
    (ret)[2] -= (tr)[8] * (tr)[12]+ (tr)[9] * (tr)[13]+ (tr)[10] * (tr)[14];    /* -= (M^-1 * mov).z*/\
}

#define Mat4_vec3_rot_inv_macro(ret, tr, src)\
{ \
    (ret)[0]  = (tr)[0] * (src)[0] + (tr)[1] * (src)[1] + (tr)[2]  * (src)[2];  /* (M^-1 * src).x*/\
    (ret)[1]  = (tr)[4] * (src)[0] + (tr)[5] * (src)[1] + (tr)[6]  * (src)[2];  /* (M^-1 * src).y*/\
    (ret)[2]  = (tr)[8] * (src)[0] + (tr)[9] * (src)[1] + (tr)[10] * (src)[2];  /* (M^-1 * src).z*/\
}

/**
 * That macro does not touch mat transformation
 * @param mat
 */

#define Mat4_set_qrotation(mat, q)\
{\
    (mat)[0] = 1.0 - 2.0 * ((q)[1] * (q)[1] + (q)[2] * (q)[2]);\
    (mat)[1] =       2.0 * ((q)[0] * (q)[1] + (q)[3] * (q)[2]);\
    (mat)[2] =       2.0 * ((q)[0] * (q)[2] - (q)[3] * (q)[1]);\
    (mat)[3] = 0.0;\
    \
    (mat)[4] =       2.0 * ((q)[0] * (q)[1] - (q)[3] * (q)[2]);\
    (mat)[5] = 1.0 - 2.0 * ((q)[0] * (q)[0] + (q)[2] * (q)[2]);\
    (mat)[6] =       2.0 * ((q)[1] * (q)[2] + (q)[3] * (q)[0]);\
    (mat)[7] = 0.0;\
    \
    (mat)[8] =       2.0 * ((q)[0] * (q)[2] + (q)[3] * (q)[1]);\
    (mat)[9] =       2.0 * ((q)[1] * (q)[2] - (q)[3] * (q)[0]);\
    (mat)[10]= 1.0 - 2.0 * ((q)[0] * (q)[0] + (q)[1] * (q)[1]);\
    (mat)[11]= 0.0;\
}

void Mat4_Translate(btTransform &mat, const btVector3 &v);
void Mat4_Translate(btTransform &mat, btScalar x, btScalar y, btScalar z);
void Mat4_Scale(btTransform &mat, btScalar x, btScalar y, btScalar z);
void Mat4_RotateX(btTransform &mat, btScalar ang);
void Mat4_RotateY(btTransform &mat, btScalar ang);
void Mat4_RotateZ(btTransform &mat, btScalar ang);

#define ThreePlanesIntersection_macro(v, n0, n1, n2, d)\
{\
    (d) =-(n0)[0] * ((n1)[1] * (n2)[2] - (n1)[2] * (n2)[1]) + \
          (n1)[0] * ((n0)[1] * (n2)[2] - (n0)[2] * (n2)[1]) - \
          (n2)[0] * ((n0)[1] * (n1)[2] - (n0)[2] * (n1)[1]);\
\
    (v)[0] = (n0)[3] * ((n1)[1] * (n2)[2] - (n1)[2] * (n2)[1]) - \
             (n1)[3] * ((n0)[1] * (n2)[2] - (n0)[2] * (n2)[1]) + \
             (n2)[3] * ((n0)[1] * (n1)[2] - (n0)[2] * (n1)[1]);\
    (v)[0] /= d;\
\
    (v)[1] = (n0)[0] * ((n1)[3] * (n2)[2] - (n1)[2] * (n2)[3]) - \
             (n1)[0] * ((n0)[3] * (n2)[2] - (n0)[2] * (n2)[3]) + \
             (n2)[0] * ((n0)[3] * (n1)[2] - (n0)[2] * (n1)[3]);\
    (v)[1] /= d;\
\
    (v)[2] = (n0)[0] * ((n1)[1] * (n2)[3] - (n1)[3] * (n2)[1]) - \
             (n1)[0] * ((n0)[1] * (n2)[3] - (n0)[3] * (n2)[1]) + \
             (n2)[0] * ((n0)[1] * (n1)[3] - (n0)[3] * (n1)[1]);\
    (v)[2] /= d;    \
}

#endif  // VMATH_H
