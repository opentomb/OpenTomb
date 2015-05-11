
#include <math.h>
#include "vmath.h"

void vec4_rev(btScalar rev[4], btScalar src[4])
{
    btScalar module;
    module = vec4_abs(src);
    rev[3] = src[3] / module;                                                   // w
    rev[0] = - src[0] / module;                                                 // x
    rev[1] = - src[1] / module;                                                 // y
    rev[2] = - src[2] / module;                                                 // z
}

void vec4_div(btScalar ret[4], btScalar a[4], btScalar b[4])
{
    btScalar temp[4];
    btScalar module;
    module = vec4_abs(b);
    vec4_sop(b, b)
    vec4_mul(temp, a, b);
    vec4_sop(b, b)
    ret[0] = temp[0] / module;
    ret[1] = temp[1] / module;
    ret[2] = temp[2] / module;
    ret[3] = temp[3] / module;
}

void vec4_rotate(btScalar rot[4], btScalar vec[4], btScalar angle)
{
    btScalar sin_t2, cos_t2, module;
    btScalar t1[4], t2[4], t[4];

    angle /= 2.0;
    sin_t2 = sin(angle);
    cos_t2 = cos(angle);

    t1[3] = cos_t2;
    t1[0] = vec[0] * sin_t2;
    t1[1] = vec[1] * sin_t2;
    t1[2] = vec[2] * sin_t2;

    module = vec4_abs(t1);
    t2[3] = t1[3] / module;
    t2[0] = -t1[0] / module;
    t2[1] = -t1[1] / module;
    t2[2] = -t1[2] / module;

    rot[3] = 0.0;
    vec4_mul(t, t1, rot);
    vec4_mul(rot, t, t2);
}

void vec4_GetEilerOrientationTransform(btScalar R[4], btScalar ang[3])
{
    btScalar t, Rt[4], T[4];

    t = ang[2] / 2.0;                                                           // ROLL
    R[0] = 0.0;                                                                 // -OZ
    R[1] = 0.0;
    R[2] = -sin(t);
    R[3] = cos(t);

    t = ang[0] / 2.0;                                                           // PITCH
    Rt[0] = sin(t);                                                             // OX
    Rt[1] = 0.0;
    Rt[2] = 0.0;
    Rt[3] = cos(t);
    vec4_mul(T, R, Rt)

    t = ang[1] / 2.0;                                                           // YAW
    Rt[0] = 0.0;                                                                // OY
    Rt[1] = sin(t);
    Rt[2] = 0.0;
    Rt[3] = cos(t);
    vec4_mul(R, T, Rt)
}

void vec4_GetPlaneEquation(btScalar eq[4], btScalar poly[12])
{
    btScalar v1[3], v2[3], t;

    v1[0] = poly[1*4+0] - poly[0*4+0];                                          // get the first vector inside the plane
    v1[1] = poly[1*4+1] - poly[0*4+1];
    v1[2] = poly[1*4+2] - poly[0*4+2];

    v2[0] = poly[0*4+0] - poly[2*4+0];                                          // get the second vector inside the plane
    v2[1] = poly[0*4+1] - poly[2*4+1];
    v2[2] = poly[0*4+2] - poly[2*4+2];

    eq[0] = v1[1]*v2[2] - v1[2]*v2[1];                                          // get the normal vector to the plane
    eq[0] = v1[2]*v2[0] - v1[0]*v2[2];
    eq[2] = v1[0]*v2[1] - v1[1]*v2[0];

    t = sqrt(eq[0]*eq[0] + eq[1]*eq[1] + eq[2]*eq[2]);                          // normalize vector
    eq[0] /= t;
    eq[1] /= t;
    eq[2] /= t;

    eq[3] = -(poly[0]*eq[0] + poly[1]*eq[1] + poly[2]*eq[2]);                   // distance from the plane to (0, 0, 0)
}

void vec3_GetPlaneEquation(btScalar eq[4], btScalar v0[3], btScalar v1[3], btScalar v2[3])
{
    btScalar l1[3], l2[3], t;

    vec3_sub(l1, v1, v0);                                                       // get the first vector inside the plane
    vec3_sub(l2, v0, v2);                                                       // get the second vector inside the plane
    vec3_cross(eq, l1, l2);                                                     // get the normal vector to the plane

    t = sqrt(eq[0]*eq[0] + eq[1]*eq[1] + eq[2]*eq[2]);                          // normalize vector
    eq[0] /= t;
    eq[1] /= t;
    eq[2] /= t;

    eq[3] = -(v0[0]*eq[0] + v0[1]*eq[1] + v0[2]*eq[2]);                         // distance from the plane to (0, 0, 0)
}

void vec3_RotateX(btScalar res[3], btScalar src[3], btScalar ang)
{
    btScalar t[2], sint, cost;

    ang *= M_PI / 180.0;
    sint = sin(ang);
    cost = cos(ang);
    res[0] = src[0];
    t[0] = src[1]*cost - src[2]*sint;
    t[1] = src[1]*sint + src[2]*cost;

    res[1] = t[0];
    res[2] = t[1];
}

void vec3_RotateY(btScalar res[3], btScalar src[3], btScalar ang)
{
    btScalar t[2], sint, cost;

    ang *= M_PI / 180.0;
    sint = sin(ang);
    cost = cos(ang);
    res[1] = src[1];
    t[0] = src[0]*cost + src[2]*sint;
    t[1] =-src[0]*sint + src[2]*cost;

    res[0] = t[0];
    res[2] = t[1];
}

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

void vec4_slerp(btScalar ret[4], btScalar q1[4], btScalar q2[4], btScalar t)
{
    btScalar cos_fi, sin_fi, fi, k1, k2, sign;
    cos_fi = q1[3] * q2[3] + q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2];
    sign = (cos_fi < 0.0)?(-1.0):(1.0);
    fi = acos(sign * cos_fi);
    sin_fi = sin(fi);

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

    ret[0] = k1 * q1[0] + k2 * q2[0];
    ret[1] = k1 * q1[1] + k2 * q2[1];
    ret[2] = k1 * q1[2] + k2 * q2[2];
    ret[3] = k1 * q1[3] + k2 * q2[3];

    fi = vec4_abs(ret);
    ret[0] /= fi;
    ret[1] /= fi;
    ret[2] /= fi;
    ret[3] /= fi;
}

void vec4_SetTRRotations(btScalar v[4], btScalar rot[3])
{
    btScalar angle, sin_t2, cos_t2, qt[4], qX[4], qY[4], qZ[4];

    // OZ    Mat4_RotateZ(btag->transform, btag->rotate[2]);
    angle = M_PI * rot[2] / 360.0;
    sin_t2 = sin(angle);
    cos_t2 = cos(angle);

    qZ[3] = cos_t2;
    qZ[0] = 0.0 * sin_t2;
    qZ[1] = 0.0 * sin_t2;
    qZ[2] = 1.0 * sin_t2;

    // OX    Mat4_RotateX(btag->transform, btag->rotate[0]);
    angle = M_PI * rot[0] / 360.0;
    sin_t2 = sin(angle);
    cos_t2 = cos(angle);

    qX[3] = cos_t2;
    qX[0] = 1.0 * sin_t2;
    qX[1] = 0.0 * sin_t2;
    qX[2] = 0.0 * sin_t2;

    // OY    Mat4_RotateY(btag->transform, btag->rotate[1]);
    angle = M_PI * rot[1] / 360.0;
    sin_t2 = sin(angle);
    cos_t2 = cos(angle);

    qY[3] = cos_t2;
    qY[0] = 0.0 * sin_t2;
    qY[1] = 1.0 * sin_t2;
    qY[2] = 0.0 * sin_t2;

    vec4_mul(qt, qZ, qX);
    vec4_mul(v, qt, qY);
}


/*
 * Matrix operations:
 */

void Mat4_E(btScalar mat[16])
{
    mat[0]  = 1.0;
    mat[1]  = 0.0;
    mat[2]  = 0.0;
    mat[3]  = 0.0;

    mat[4]  = 0.0;
    mat[5]  = 1.0;
    mat[6]  = 0.0;
    mat[7]  = 0.0;

    mat[8]  = 0.0;
    mat[9]  = 0.0;
    mat[10] = 1.0;
    mat[11] = 0.0;

    mat[12] = 0.0;
    mat[13] = 0.0;
    mat[14] = 0.0;
    mat[15] = 1.0;
}

void Mat4_Copy(btScalar dst[16], const btScalar src[16])
{
    vec4_copy(dst,    src);
    vec4_copy(dst+4,  src+4);
    vec4_copy(dst+8,  src+8);
    vec4_copy(dst+12, src+12);
}

void Mat4_Translate(btScalar mat[16], const btScalar v[3])
{
    mat[12] += mat[0] * v[0] + mat[4] * v[1] + mat[8]  * v[2];
    mat[13] += mat[1] * v[0] + mat[5] * v[1] + mat[9]  * v[2];
    mat[14] += mat[2] * v[0] + mat[6] * v[1] + mat[10] * v[2];
}

void Mat4_Translate(btScalar mat[16], btScalar x, btScalar y, btScalar z)
{
    btScalar offset[] = { x, y, z };
    Mat4_Translate(mat, offset);
}

void Mat4_Scale(btScalar mat[16], btScalar x, btScalar y, btScalar z)
{
    mat[ 0] *= x;
    mat[ 1] *= x;
    mat[ 2] *= x;
    
    mat[ 4] *= y;
    mat[ 5] *= y;
    mat[ 6] *= y;
    
    mat[ 8] *= z;
    mat[ 9] *= z;
    mat[ 10] *= z;
}

void Mat4_RotateX(btScalar mat[16], btScalar ang)
{
    btScalar sina, cosa, R[9];

    R[0] = ang * M_PI / 180.0;
    sina = sin(R[0]);
    cosa = cos(R[0]);

    R[0] = mat[0];
    R[1] = mat[1];
    R[2] = mat[2];

    R[3] = mat[4] * cosa + mat[8] * sina;
    R[4] = mat[5] * cosa + mat[9] * sina;
    R[5] = mat[6] * cosa + mat[10] * sina;

    R[6] =-mat[4] * sina + mat[8] * cosa;
    R[7] =-mat[5] * sina + mat[9] * cosa;
    R[8] =-mat[6] * sina + mat[10] * cosa;

    vec3_copy(mat, R);
    vec3_copy(mat+4, R+3);
    vec3_copy(mat+8, R+6);
}

void Mat4_RotateY(btScalar mat[16], btScalar ang)
{
    btScalar sina, cosa, R[9];

    R[0] = ang * M_PI / 180.0;
    sina = sin(R[0]);
    cosa = cos(R[0]);

    R[0] = mat[0] * cosa - mat[8] * sina;
    R[1] = mat[1] * cosa - mat[9] * sina;
    R[2] = mat[2] * cosa - mat[10] * sina;

    R[3] = mat[4];
    R[4] = mat[5];
    R[5] = mat[6];

    R[6] = mat[0] * sina + mat[8] * cosa;
    R[7] = mat[1] * sina + mat[9] * cosa;
    R[8] = mat[2] * sina + mat[10] * cosa;

    vec3_copy(mat, R);
    vec3_copy(mat+4, R+3);
    vec3_copy(mat+8, R+6);
}

void Mat4_RotateZ(btScalar mat[16], btScalar ang)
{
    btScalar sina, cosa, R[9];

    R[0] = ang * M_PI / 180.0;
    sina = sin(R[0]);
    cosa = cos(R[0]);

    R[0] = mat[0] * cosa +  mat[4] * sina;
    R[1] = mat[1] * cosa +  mat[5] * sina;
    R[2] = mat[2] * cosa +  mat[6] * sina;

    R[3] =-mat[0] * sina +  mat[4] * cosa;
    R[4] =-mat[1] * sina +  mat[5] * cosa;
    R[5] =-mat[2] * sina +  mat[6] * cosa;

    R[6] = mat[8];
    R[7] = mat[9];
    R[8] = mat[10];

    vec3_copy(mat, R);
    vec3_copy(mat+4, R+3);
    vec3_copy(mat+8, R+6);
}

void Mat4_T(btScalar mat[16])
{
    btScalar t;
    SWAPT(mat[1], mat[4], t);
    SWAPT(mat[2], mat[8], t);
    SWAPT(mat[3], mat[12], t);

    SWAPT(mat[6], mat[9], t);
    SWAPT(mat[7], mat[13], t);
    SWAPT(mat[11], mat[14], t);
}


/*
 * OpenGL matrix inversing. Not an usual matrix inversing.
 * Works only with OpenGL transformation matrices!
 */
void Mat4_inv(btScalar mat[16])
{
    btScalar v[3];

    SWAPT(mat[1], mat[4], v[0]);
    SWAPT(mat[2], mat[8], v[0]);
    SWAPT(mat[6], mat[9], v[0]);

    v[0] = mat[0] * mat[12] + mat[4] * mat[13] + mat[8] * mat[14];
    v[1] = mat[1] * mat[12] + mat[5] * mat[13] + mat[9] * mat[14];
    v[2] = mat[2] * mat[12] + mat[6] * mat[13] + mat[10] * mat[14];

    mat[12] = -v[0];
    mat[13] = -v[1];
    mat[14] = -v[2];
}


/**
 * OpenGL matrices multiplication. serult = src1 x src2.
 * Works only with OpenGL transformation matrices!
 */
void Mat4_Mat4_mul(btScalar result[16], const btScalar src1[16], const btScalar src2[16])
{
    btScalar t_res[16];

    t_res[0] = src1[0] * src2[0] + src1[4] * src2[1] + src1[8] * src2[2];
    t_res[1] = src1[1] * src2[0] + src1[5] * src2[1] + src1[9] * src2[2];
    t_res[2] = src1[2] * src2[0] + src1[6] * src2[1] + src1[10] * src2[2];
    t_res[3] = 0.0;

    t_res[4] = src1[0] * src2[4] + src1[4] * src2[5] + src1[8] * src2[6];
    t_res[5] = src1[1] * src2[4] + src1[5] * src2[5] + src1[9] * src2[6];
    t_res[6] = src1[2] * src2[4] + src1[6] * src2[5] + src1[10] * src2[6];
    t_res[7] = 0.0;

    t_res[8] = src1[0] * src2[8] + src1[4] * src2[9] + src1[8] * src2[10];
    t_res[9] = src1[1] * src2[8] + src1[5] * src2[9] + src1[9] * src2[10];
    t_res[10] = src1[2] * src2[8] + src1[6] * src2[9] + src1[10] * src2[10];
    t_res[11] = 0.0;

    t_res[12] = src1[12] + src1[0] * src2[12] + src1[4] * src2[13] + src1[8] * src2[14];
    t_res[13] = src1[13] + src1[1] * src2[12] + src1[5] * src2[13] + src1[9] * src2[14];
    t_res[14] = src1[14] + src1[2] * src2[12] + src1[6] * src2[13] + src1[10] * src2[14];
    t_res[15] = 1.0;

    vec4_copy(result  , t_res);
    vec4_copy(result+4, t_res+4);
    vec4_copy(result+8, t_res+8);
    vec4_copy(result+12, t_res+12);
}


/**
 * OpenGL matrices multiplication. serult = (src1^-1) x src2.
 * Works only with OpenGL transformation matrices!
 */
void Mat4_inv_Mat4_mul(btScalar result[16], btScalar src1[16], btScalar src2[16])
{
    btScalar t_res[16], v[3];

    v[0] = -(src1[0] * src1[12] + src1[1] * src1[13] + src1[2] * src1[14]);
    v[1] = -(src1[4] * src1[12] + src1[5] * src1[13] + src1[6] * src1[14]);
    v[2] = -(src1[8] * src1[12] + src1[9] * src1[13] + src1[10] * src1[14]);

    t_res[0] = src1[0] * src2[0] + src1[1] * src2[1] + src1[2] * src2[2];
    t_res[1] = src1[4] * src2[0] + src1[5] * src2[1] + src1[6] * src2[2];
    t_res[2] = src1[8] * src2[0] + src1[9] * src2[1] + src1[10] * src2[2];
    t_res[3] = 0.0;

    t_res[4] = src1[0] * src2[4] + src1[1] * src2[5] + src1[2] * src2[6];
    t_res[5] = src1[4] * src2[4] + src1[5] * src2[5] + src1[6] * src2[6];
    t_res[6] = src1[8] * src2[4] + src1[9] * src2[5] + src1[10] * src2[6];
    t_res[7] = 0.0;

    t_res[8] = src1[0] * src2[8] + src1[1] * src2[9] + src1[2] * src2[10];
    t_res[9] = src1[4] * src2[8] + src1[5] * src2[9] + src1[6] * src2[10];
    t_res[10] = src1[8] * src2[8] + src1[9] * src2[9] + src1[10] * src2[10];
    t_res[11] = 0.0;

    t_res[12] = v[0] + src1[0] * src2[12] + src1[1] * src2[13] + src1[2] * src2[14];
    t_res[13] = v[1] + src1[4] * src2[12] + src1[5] * src2[13] + src1[6] * src2[14];
    t_res[14] = v[2] + src1[8] * src2[12] + src1[9] * src2[13] + src1[10] * src2[14];
    t_res[15] = 1.0;

    vec4_copy(result  , t_res);
    vec4_copy(result+4, t_res+4);
    vec4_copy(result+8, t_res+8);
    vec4_copy(result+12, t_res+12);
}


/**
 * OpenGL matrices multiplication. serult = src1 x (src2^-1).
 * Works only with OpenGL transformation matrices!
 */
void Mat4_Mat4_inv_mul(btScalar result[16], btScalar src1[16], btScalar src2[16])
{
    btScalar t_res[16], v[3];

    v[0] = -(src2[0] * src2[12] + src2[1] * src2[13] + src2[2] * src2[14]);
    v[1] = -(src2[4] * src2[12] + src2[5] * src2[13] + src2[6] * src2[14]);
    v[2] = -(src2[8] * src2[12] + src2[9] * src2[13] + src2[10] * src2[14]);

    t_res[0] = src1[0] * src2[0] + src1[4] * src2[4] + src1[8] * src2[8];
    t_res[1] = src1[1] * src2[0] + src1[5] * src2[4] + src1[9] * src2[8];
    t_res[2] = src1[2] * src2[0] + src1[6] * src2[4] + src1[10] * src2[8];
    t_res[3] = 0.0;

    t_res[4] = src1[0] * src2[1] + src1[4] * src2[5] + src1[8] * src2[9];
    t_res[5] = src1[1] * src2[1] + src1[5] * src2[5] + src1[9] * src2[9];
    t_res[6] = src1[2] * src2[1] + src1[6] * src2[5] + src1[10] * src2[9];
    t_res[7] = 0.0;

    t_res[8] = src1[0] * src2[2] + src1[4] * src2[6] + src1[8] * src2[10];
    t_res[9] = src1[1] * src2[2] + src1[5] * src2[6] + src1[9] * src2[10];
    t_res[10] = src1[2] * src2[2] + src1[6] * src2[6] + src1[10] * src2[10];
    t_res[11] = 0.0;

    t_res[12] = src1[12] + src1[0] * v[0] + src1[4] * v[1] + src1[8] * v[2];
    t_res[13] = src1[13] + src1[1] * v[0] + src1[5] * v[1] + src1[9] * v[2];
    t_res[14] = src1[14] + src1[2] * v[0] + src1[6] * v[1] + src1[10] * v[2];
    t_res[15] = 1.0;

    vec4_copy(result  , t_res);
    vec4_copy(result+4, t_res+4);
    vec4_copy(result+8, t_res+8);
    vec4_copy(result+12, t_res+12);
}


void Mat4_vec3_mul(btScalar v[3], btScalar mat[16], btScalar src[3])
{
    btScalar ret[3];

    ret[0] = mat[0] * src[0] + mat[4] * src[1] + mat[8]  * src[2] + mat[12];
    ret[1] = mat[1] * src[0] + mat[5] * src[1] + mat[9]  * src[2] + mat[13];
    ret[2] = mat[2] * src[0] + mat[6] * src[1] + mat[10] * src[2] + mat[14];
    vec3_copy(v, ret);
}

void Mat4_vec3_mul_inv(btScalar v[3], btScalar mat[16], btScalar src[3])
{
    btScalar ret[3];

    ret[0]  = mat[0] * src[0] + mat[1] * src[1] + mat[2]  * src[2];             // (M^-1 * src).x
    ret[0] -= mat[0] * mat[12]+ mat[1] * mat[13]+ mat[2]  * mat[14];            // -= (M^-1 * mov).x
    ret[1]  = mat[4] * src[0] + mat[5] * src[1] + mat[6]  * src[2];             // (M^-1 * src).y
    ret[1] -= mat[4] * mat[12]+ mat[5] * mat[13]+ mat[6]  * mat[14];            // -= (M^-1 * mov).y
    ret[2]  = mat[8] * src[0] + mat[9] * src[1] + mat[10] * src[2];             // (M^-1 * src).z
    ret[2] -= mat[8] * mat[12]+ mat[9] * mat[13]+ mat[10] * mat[14];            // -= (M^-1 * mov).z
    vec3_copy(v, ret);
}

void Mat4_vec3_mul_T(btScalar v[3], btScalar mat[16], btScalar src[3])
{
    btScalar ret[3];

    ret[0] = mat[0] * src[0] + mat[1] * src[1] + mat[2]  * src[2] + mat[3];
    ret[1] = mat[4] * src[0] + mat[5] * src[1] + mat[6]  * src[2] + mat[7];
    ret[2] = mat[8] * src[0] + mat[9] * src[1] + mat[10] * src[2] + mat[11];
    vec3_copy(v, ret);
}


void Mat4_SetSelfOrientation(btScalar mat[16], btScalar ang[3])
{
    btScalar R[4], Rt[4], temp[4];
    btScalar sin_t2, cos_t2, t;

    sin_t2 = 0.0;
    cos_t2 = 1.0;

    if(ang[0] != 0.0)
    {
        t = ang[0] * M_PI / 180.0;
        sin_t2 = sin(t);
        cos_t2 = cos(t);
    }

    /*
     * LEFT - RIGHT INIT
     */

    mat[0] = cos_t2;                                                            // OX - strafe
    mat[1] = sin_t2;
    mat[2] = 0.0;
    mat[3] = 0.0;

    mat[4] =-sin_t2;                                                            // OY - view
    mat[5] = cos_t2;
    mat[6] = 0.0;
    mat[7] = 0.0;

    mat[8] = 0.0;                                                               // OZ - up / down
    mat[9] = 0.0;
    mat[10] = 1.0;
    mat[11] = 0.0;

    if(ang[1] != 0.0)
    {
        t = ang[1] * M_PI / 360.0;                                              // UP - DOWN
        sin_t2 = sin(t);
        cos_t2 = cos(t);
        R[3] = cos_t2;
        R[0] = mat[0] * sin_t2;                                                 // strafe sxis
        R[1] = mat[1] * sin_t2;
        R[2] = mat[2] * sin_t2;
        vec4_sop(Rt, R);

        vec4_mul(temp, R, mat+4);
        vec4_mul(mat+4, temp, Rt);
        vec4_mul(temp, R, mat+8);
        vec4_mul(mat+8, temp, Rt);
        mat[7] = 0.0;
        mat[11] = 0.0;
    }

    if(ang[2] != 0.0)
    {
        t = ang[2] * M_PI / 360.0;                                              // ROLL
        sin_t2 = sin(t);
        cos_t2 = cos(t);
        R[3] = cos_t2;
        R[0] = mat[4] * sin_t2;                                                 // view axis
        R[1] = mat[5] * sin_t2;
        R[2] = mat[6] * sin_t2;
        vec4_sop(Rt, R);

        vec4_mul(temp, R, mat+0);
        vec4_mul(mat+0, temp, Rt);
        vec4_mul(temp, R, mat+8);
        vec4_mul(mat+8, temp, Rt);
        mat[3] = 0.0;
        mat[11] = 0.0;
    }
}

int ThreePlanesIntersection(btScalar v[3], btScalar n0[4], btScalar n1[4], btScalar n2[4])
{
    btScalar d;
    /*
     * Solve system of the linear equations by Kramer method!
     * I know - It may be slow, but it has a good precision!
     * The root is point of 3 planes intersection.
     */

    d =-n0[0] * (n1[1] * n2[2] - n1[2] * n2[1]) +
        n1[0] * (n0[1] * n2[2] - n0[2] * n2[1]) -
        n2[0] * (n0[1] * n1[2] - n0[2] * n1[1]);

    if(fabs(d) < 0.001)                                                         // if d == 0, then something wrong
    {
        return 0;
    }

    v[0] = n0[3] * (n1[1] * n2[2] - n1[2] * n2[1]) -
           n1[3] * (n0[1] * n2[2] - n0[2] * n2[1]) +
           n2[3] * (n0[1] * n1[2] - n0[2] * n1[1]);
    v[0] /= d;

    v[1] = n0[0] * (n1[3] * n2[2] - n1[2] * n2[3]) -
           n1[0] * (n0[3] * n2[2] - n0[2] * n2[3]) +
           n2[0] * (n0[3] * n1[2] - n0[2] * n1[3]);
    v[1] /= d;

    v[2] = n0[0] * (n1[1] * n2[3] - n1[3] * n2[1]) -
           n1[0] * (n0[1] * n2[3] - n0[3] * n2[1]) +
           n2[0] * (n0[1] * n1[3] - n0[3] * n1[1]);
    v[2] /= d;

    return 1;
}


