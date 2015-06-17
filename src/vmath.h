
#ifndef VMATH_H
#define VMATH_H

#include <math.h>
#include "bullet/LinearMath/btScalar.h"

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#define PLANE_X        1
#define PLANE_Y        2
#define PLANE_Z        3

#define ABS(x) (((x)>0)?(x):(-(x)))
#define SWAPT(a, b, t) {(t) = (a); (a) = (b); (b) = (t);}

#define vec3_set_one(x) {(x)[0] = 1.0; (x)[1] = 1.0; (x)[2] = 1.0;}
#define vec3_set_zero(x) {(x)[0] = 0.0; (x)[1] = 0.0; (x)[2] = 0.0;}
#define vec3_copy(x, y) {(x)[0] = (y)[0]; (x)[1] = (y)[1]; (x)[2] = (y)[2];}
#define vec3_copy_inv(x, y) {(x)[0] =-(y)[0]; (x)[1] =-(y)[1]; (x)[2] =-(y)[2];}
#define vec3_add(r, x, y) {(r)[0] = (x)[0] + (y)[0]; (r)[1] = (x)[1] + (y)[1]; (r)[2] = (x)[2] + (y)[2];}
#define vec3_add_to(r, x) {(r)[0] += (x)[0]; (r)[1] += (x)[1]; (r)[2] += (x)[2];}
#define vec3_add_mul(r, x, y, k) {(r)[0] = (x)[0] + (y)[0] * (k); (r)[1] = (x)[1] + (y)[1] * (k); (r)[2] = (x)[2] + (y)[2] * (k);}
#define vec3_sub(r, x, y) {(r)[0] = (x)[0] - (y)[0]; (r)[1] = (x)[1] - (y)[1]; (r)[2] = (x)[2] - (y)[2];}
#define vec3_sub_mul(r, x, y, k) {(r)[0] = (x)[0] - (y)[0] * (k); (r)[1] = (x)[1] - (y)[1] * (k); (r)[2] = (x)[2] - (y)[2] * (k);}
#define vec3_inv(x) {(x)[0] = -(x)[0]; (x)[1] = -(x)[1]; (x)[2] = -(x)[2];}

#define vec3_dot(x, y) ((x)[0]*(y)[0] + (x)[1]*(y)[1] + (x)[2]*(y)[2])
#define vec3_norm(x, t) {(t) = vec3_abs(x); (x)[0] /= (t); (x)[1] /= (t); (x)[2] /= (t);}
#define vec3_sqabs(x) ((x)[0]*(x)[0] + (x)[1]*(x)[1] + (x)[2]*(x)[2])
#define vec3_abs(x) (sqrt((x)[0]*(x)[0] + (x)[1]*(x)[1] + (x)[2]*(x)[2]))
#define vec3_dist(x, y) sqrt(((x)[0] - (y)[0]) * ((x)[0] - (y)[0]) + ((x)[1] - (y)[1]) * ((x)[1] - (y)[1]) + ((x)[2] - (y)[2]) * ((x)[2] - (y)[2]))
#define vec3_dist_sq(x, y) (((x)[0] - (y)[0]) * ((x)[0] - (y)[0]) + ((x)[1] - (y)[1]) * ((x)[1] - (y)[1]) + ((x)[2] - (y)[2]) * ((x)[2] - (y)[2]))

#define vec3_mul_scalar(res, x, t) {\
                   (res)[0] = (x)[0] * (t);\
                   (res)[1] = (x)[1] * (t);\
                   (res)[2] = (x)[2] * (t);}

#define vec3_cross(res, x, y) {\
                   (res)[0] = (x)[1]*(y)[2] - (x)[2]*(y)[1]; \
                   (res)[1] = (x)[2]*(y)[0] - (x)[0]*(y)[2]; \
                   (res)[2] = (x)[0]*(y)[1] - (x)[1]*(y)[0]; }

#define vec3_cross_safe(res, x, y, tmp) {\
                   (tmp)[0] = (x)[1]*(y)[2] - (x)[2]*(y)[1]; \
                   (tmp)[1] = (x)[2]*(y)[0] - (x)[0]*(y)[2]; \
                   (tmp)[2] = (x)[0]*(y)[1] - (x)[1]*(y)[0]; \
                   vec3_copy(res, tmp)} 

#define vec3_interpolate_macro(res, x, y, lerp, t) {\
                   (res)[0] = (x)[0] * (t) + (y)[0] * (lerp);\
                   (res)[1] = (x)[1] * (t) + (y)[1] * (lerp);\
                   (res)[2] = (x)[2] * (t) + (y)[2] * (lerp);}

#define vec3_plane_dist(p, dot) ((p)[3] + (p)[0]*(dot)[0] + (p)[1]*(dot)[1] + (p)[2]*(dot)[2])

#define vec3_norm_plane(p, dot, a) {(p)[0] /= (a); (p)[1] /= (a); (p)[2] /= (a); \
                                    (p)[3] = -((p)[0]*(dot)[0] + (p)[1]*(dot)[1] + (p)[2]*(dot)[2]);}
/**
 * p - point of ray entrance                                                    in
 * v - ray direction                                                            in
 * n - plane equaton                                                            in
 * dot - ray and plane intersection                                             out
 * t - parametric intersection coordinate
 */
#define vec3_ray_plane_intersect(p, v, n, dot, t) {\
                   (t) = -((n)[3] + (n)[0]*(p)[0] + (n)[1]*(p)[1] + (n)[2]*(p)[2]);\
                   (t) /= ((n)[0]*(v)[0] + (n)[1]*(v)[1] + (n)[2]*(v)[2]); \
                   (dot)[0] = (p)[0] + (t) * (v)[0]; \
                   (dot)[1] = (p)[1] + (t) * (v)[1]; \
                   (dot)[2] = (p)[2] + (t) * (v)[2]; }

void vec3_GetPlaneEquation(btScalar eq[4], btScalar v0[3], btScalar v1[3], btScalar v2[3]);
void vec3_RotateX(btScalar res[3], btScalar src[3], btScalar ang);
void vec3_RotateY(btScalar res[3], btScalar src[3], btScalar ang);
void vec3_RotateZ(btScalar res[3], btScalar src[3], btScalar ang);

//vec4 - btScalar[4] (x, y, z, w)

#define vec4_set_one(x) {(x)[0] = 1.0; (x)[1] = 1.0; (x)[2] = 1.0; (x)[3] = 1.0;}
#define vec4_set_zero(x) {(x)[0] = 0.0; (x)[1] = 0.0; (x)[2] = 0.0; (x)[3] = 0.0;}
#define vec4_copy(x, y) {(x)[0] = (y)[0]; (x)[1] = (y)[1]; (x)[2] = (y)[2]; (x)[3] = (y)[3];}
#define vec4_copy_inv(x, y) {(x)[0] =-(y)[0]; (x)[1] =-(y)[1]; (x)[2] =-(y)[2]; (x)[3] =-(y)[3];}
#define vec4_add(r, x, y) {(r)[0] = (x)[0] + (y)[0]; (r)[1] = (x)[1] + (y)[1]; (r)[2] = (x)[2] + (y)[2]; (r)[3] = (x)[3] + (y)[3];}
#define vec4_sub(r, x, y) {(r)[0] = (x)[0] - (y)[0]; (r)[1] = (x)[1] - (y)[1]; (r)[2] = (x)[2] - (y)[2]; (r)[3] = (x)[3] - (y)[3];}
#define vec4_sop(x, y) {(x)[0] = -(y)[0]; (x)[1] = -(y)[1]; (x)[2] = -(y)[2]; (x)[3] = (y)[3];}
#define vec4_inv(x) {(x)[0] = -(x)[0]; (x)[1] = -(x)[1]; (x)[2] = -(x)[2]; (x)[3] = -(x)[3];}

#define vec4_norm(x) ((x)[0]*(x)[0] + (x)[1]*(x)[1] + (x)[2]*(x)[2] + (x)[3]*(x)[3])
#define vec4_abs(x) (sqrt((x)[0]*(x)[0] + (x)[1]*(x)[1] + (x)[2]*(x)[2] + (x)[3]*(x)[3]))

#define vec4_mul(res, x, y) {\
                   (res)[0] = (x)[3]*(y)[0] + (x)[0]*(y)[3] + (x)[1]*(y)[2] - (x)[2]*(y)[1]; \
                   (res)[1] = (x)[3]*(y)[1] + (x)[1]*(y)[3] + (x)[2]*(y)[0] - (x)[0]*(y)[2]; \
                   (res)[2] = (x)[3]*(y)[2] + (x)[2]*(y)[3] + (x)[0]*(y)[1] - (x)[1]*(y)[0]; \
                   (res)[3] = (x)[3]*(y)[3] - (x)[0]*(y)[0] - (x)[1]*(y)[1] - (x)[2]*(y)[2];} 

#define vec4_mul_safe(res, x, y, tmp) {\
                   (tmp)[0] = (x)[3]*(y)[0] + (x)[0]*(y)[3] + (x)[1]*(y)[2] - (x)[2]*(y)[1]; \
                   (tmp)[1] = (x)[3]*(y)[1] + (x)[1]*(y)[3] + (x)[2]*(y)[0] - (x)[0]*(y)[2]; \
                   (tmp)[2] = (x)[3]*(y)[2] + (x)[2]*(y)[3] + (x)[0]*(y)[1] - (x)[1]*(y)[0]; \
                   (tmp)[3] = (x)[3]*(y)[3] - (x)[0]*(y)[0] - (x)[1]*(y)[1] - (x)[2]*(y)[2]; \
                   vec4_copy(res, tmp)} 

#define vec4_dot(x, y) ((x)[0]*(y)[0] + (x)[1]*(y)[1] + (x)[2]*(y)[2] + (x)[3]*(y)[3])

void vec4_rev(btScalar rev[4], btScalar src[4]);
void vec4_div(btScalar ret[4], btScalar a[4], btScalar b[4]);
void vec4_rotate(btScalar rot[4], btScalar vec[4], btScalar angle);
void vec4_GetEilerOrientationTransform(btScalar R[4], btScalar ang[3]);
void vec4_GetPlaneEquation(btScalar eq[4], btScalar poly[12]);
void vec4_slerp(btScalar ret[4], btScalar q1[4], btScalar q2[4], btScalar t);
void vec4_SetTRRotations(btScalar v[4], btScalar rot[3]);
/*
 * Matrix transformation functions and macro
 */

#define Mat4_vec3_mul_macro(ret, tr, src)\
{ \
    (ret)[0] = (tr)[0] * (src)[0] + (tr)[4] * (src)[1] + (tr)[8]  * (src)[2] + (tr)[12];\
    (ret)[1] = (tr)[1] * (src)[0] + (tr)[5] * (src)[1] + (tr)[9]  * (src)[2] + (tr)[13];\
    (ret)[2] = (tr)[2] * (src)[0] + (tr)[6] * (src)[1] + (tr)[10] * (src)[2] + (tr)[14];\
}

#define Mat4_vec3_mul_inv_macro(ret, tr, src)\
{ \
    (ret)[0]  = (tr)[0] * (src)[0] + (tr)[1] * (src)[1] + (tr)[2]  * (src)[2];  /* (M^-1 * src).x*/\
    (ret)[0] -= (tr)[0] * (tr)[12]+ (tr)[1] * (tr)[13]+ (tr)[2]  * (tr)[14];    /* -= (M^-1 * mov).x*/\
    (ret)[1]  = (tr)[4] * (src)[0] + (tr)[5] * (src)[1] + (tr)[6]  * (src)[2];  /* (M^-1 * src).y*/\
    (ret)[1] -= (tr)[4] * (tr)[12]+ (tr)[5] * (tr)[13]+ (tr)[6]  * (tr)[14];    /* -= (M^-1 * mov).y*/\
    (ret)[2]  = (tr)[8] * (src)[0] + (tr)[9] * (src)[1] + (tr)[10] * (src)[2];  /* (M^-1 * src).z*/\
    (ret)[2] -= (tr)[8] * (tr)[12]+ (tr)[9] * (tr)[13]+ (tr)[10] * (tr)[14];    /* -= (M^-1 * mov).z*/\
}

#define Mat4_vec3_rot_macro(ret, tr, src)\
{ \
    (ret)[0] = (tr)[0] * (src)[0] + (tr)[4] * (src)[1] + (tr)[8]  * (src)[2];\
    (ret)[1] = (tr)[1] * (src)[0] + (tr)[5] * (src)[1] + (tr)[9]  * (src)[2];\
    (ret)[2] = (tr)[2] * (src)[0] + (tr)[6] * (src)[1] + (tr)[10] * (src)[2];\
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

#define Mat4_E_macro(mat)\
{\
    (mat)[0]  = 1.0;\
    (mat)[1]  = 0.0;\
    (mat)[2]  = 0.0;\
    (mat)[3]  = 0.0;\
    \
    (mat)[4]  = 0.0;\
    (mat)[5]  = 1.0;\
    (mat)[6]  = 0.0;\
    (mat)[7]  = 0.0;\
    \
    (mat)[8]  = 0.0;\
    (mat)[9]  = 0.0;\
    (mat)[10] = 1.0;\
    (mat)[11] = 0.0;\
    \
    (mat)[12] = 0.0;\
    (mat)[13] = 0.0;\
    (mat)[14] = 0.0;\
    (mat)[15] = 1.0;\
}

void Mat4_E(btScalar mat[16]);
void Mat4_Copy(btScalar dst[16], const btScalar src[16]);
void Mat4_Translate(btScalar mat[16], const btScalar v[3]);
void Mat4_Translate(btScalar mat[16], btScalar x, btScalar y, btScalar z);
void Mat4_Scale(btScalar mat[16], btScalar x, btScalar y, btScalar z);
void Mat4_RotateX(btScalar mat[16], btScalar ang);
void Mat4_RotateY(btScalar mat[16], btScalar ang);
void Mat4_RotateZ(btScalar mat[16], btScalar ang);
void Mat4_T(btScalar mat[16]);
void Mat4_affine_inv(btScalar mat[16]);
void Mat4_Mat4_mul(btScalar result[16], const btScalar src1[16], const btScalar src2[16]);
void Mat4_inv_Mat4_affine_mul(btScalar result[16], btScalar src1[16], btScalar src2[16]);
void Mat4_vec3_mul(btScalar v[3], const btScalar mat[16], const btScalar src[3]);
void Mat4_vec3_mul_inv(btScalar v[3], btScalar mat[16], btScalar src[3]);
void Mat4_vec3_mul_T(btScalar v[3], btScalar mat[16], btScalar src[3]);
void Mat4_SetSelfOrientation(btScalar mat[16], btScalar ang[3]);

int ThreePlanesIntersection(btScalar v[3], btScalar n0[4], btScalar n1[4], btScalar n2[4]);
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