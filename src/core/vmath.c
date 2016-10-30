
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "vmath.h"


spline_p Spline_Create(uint32_t base_points_count)
{
    spline_p ret = NULL;

    if(base_points_count >= 2)
    {
        ret = (spline_p)malloc(sizeof(spline_t));

        ret->base_points_count = base_points_count;
        ret->a = (float*)malloc(base_points_count * sizeof(float));
        ret->b = (float*)malloc(base_points_count * sizeof(float));
        ret->c = (float*)malloc(base_points_count * sizeof(float));
        ret->d = (float*)malloc(base_points_count * sizeof(float));
    }

    return ret;
}


void Spline_Clear(spline_p spline)
{
    if(spline && spline->base_points_count)
    {
        spline->base_points_count = 0;
        free(spline->a);
        free(spline->b);
        free(spline->c);
        free(spline->d);
    }
}


void Spline_BuildCubic(spline_p spline)
{
    long int n = spline->base_points_count - 2;
    long int i;
    float r, k;
    const float h = 1.0f;

    k = 3.0f / (h * h);
    spline->b[0] = 0.0f;

    for(i = 1; i <= n; i++)
    {
        r = spline->d[i + 1] - spline->d[i] - spline->d[i] + spline->d[i-1];
        r *= k;
        spline->b[i] = r;
        spline->a[i] = 4.0f;
    }

    for(i = 1; i < n; i++)
    {
        k = 1.0f / spline->a[i];
        spline->a[i + 1] -= k;
        spline->b[i+1] -= k * spline->b[i];
    }

    for(i = n; i > 1; i--)
    {
        spline->b[i - 1] -= (spline->b[i] / spline->a[i]);
        spline->b[i] /= spline->a[i];
    }

    spline->b[1] /= spline->a[1];

    for(i = 0; i < n; i++)
    {
        spline->a[i] = (spline->b[i + 1] - spline->b[i]) / (3.0f * h);
        spline->c[i] = (spline->d[i + 1] - spline->d[i]) / h;
        spline->c[i] -= h * (spline->b[i + 1] + spline->b[i] + spline->b[i]) / 3.0f;
    }

    spline->a[n] = -spline->b[n] / (3.0f * h);
    spline->c[n] = (spline->d[n + 1] - spline->d[n]) / h;
    spline->c[n] -= h * spline->b[n] * 2.0f / 3.0f;
}


void Spline_BuildLine(spline_p spline)
{
    uint32_t i;
    const float h = 1.0f;

    for(i = 0; i < spline->base_points_count - 1; i++)
    {
        spline->a[i] = 0.0f;
        spline->b[i] = 0.0f;
        spline->c[i] = (spline->d[i + 1] - spline->d[i]) / h;
    }
}


float Spline_Get(spline_p spline, float t)
{
    int32_t i = t;

    if((i < 0) || (i + 1 > spline->base_points_count))
    {
    	return 0.0f;
    }
    else if(i + 1 == spline->base_points_count)
    {
    	return spline->d[i];
    }
    else
    {
        float x = t - i;
        float summ = spline->d[i];

        t = x;
        summ += spline->c[i] * t;
        t *= x;
        summ += spline->b[i] * t;
        t *= x;
        summ += spline->a[i] * t;

        return summ;
    }
}


// Fast reciprocal square root (Quake 3 game code)
__inline float RSqrt(float number)
{
    int32_t i;
    float x2, y;
    const float threehalfs = 1.5f;

    x2 = number * 0.5f;
    y  = number;
    i  = *(int32_t *)&y;                    // evil floating point bit level hacking
    i  = 0x5f3759df - (i >> 1);             // what the fuck?
    y  = *(float *)&i;
    y  = y * (threehalfs - (x2 * y * y));   // 1st iteration

    return y;
}


/*
 * VECTOR FUNCTIONS
 */
void vec3_GetOZsincos(float sincos[2], const float v0[3], const float v1[3])
{
    float t = v0[0] * v0[0] + v0[1] * v0[1];
    t *= v1[0] * v1[0] + v1[1] * v1[1];
    if(t > 0.0001f)
    {
        t = 1.0f / sqrtf(t);
        sincos[0] = v0[0] * v1[1] - v0[1] * v1[0];
        sincos[0] *= t;
        sincos[1] = v0[0] * v1[0] + v0[1] * v1[1];
        sincos[1] *= t;
    }
    else
    {
        sincos[0] = 0.0f;
        sincos[1] = 1.0f;
    }
}


void vec3_GetOYsincos(float sincos[2], const float v0[3], const float v1[3])
{
    float t = v0[0] * v0[0] + v0[2] * v0[2];
    t *= v1[0] * v1[0] + v1[2] * v1[2];
    if(t > 0.0001f)
    {
        t = 1.0f / sqrtf(t);
        sincos[0] =-v0[0] * v1[2] + v0[2] * v1[0];
        sincos[0] *= t;
        sincos[1] = v0[0] * v1[0] + v0[2] * v1[2];
        sincos[1] *= t;
    }
    else
    {
        sincos[0] = 0.0f;
        sincos[1] = 1.0f;
    }
}


void vec3_GetOXsincos(float sincos[2], const float v0[3], const float v1[3])
{
    float t = v0[1] * v0[1] + v0[2] * v0[2];
    t *= v1[1] * v1[1] + v1[2] * v1[2];
    if(t > 0.0001f)
    {
        t = 1.0f / sqrtf(t);
        sincos[0] = v0[1] * v1[2] - v0[2] * v1[1];
        sincos[0] *= t;
        sincos[1] = v0[1] * v1[1] + v0[2] * v1[2];
        sincos[1] *= t;
    }
    else
    {
        sincos[0] = 0.0f;
        sincos[1] = 1.0f;
    }
}


void vec4_rev(float rev[4], float src[4])
{
    float module = vec4_abs(src);
    rev[3] = src[3] / module;
    rev[0] = - src[0] / module;
    rev[1] = - src[1] / module;
    rev[2] = - src[2] / module;
}


void vec4_div(float ret[4], float a[4], float b[4])
{
    float temp[4];
    float module;
    module = vec4_abs(b);
    vec4_sop(b, b)
    vec4_mul(temp, a, b);
    vec4_sop(b, b)
    ret[0] = temp[0] / module;
    ret[1] = temp[1] / module;
    ret[2] = temp[2] / module;
    ret[3] = temp[3] / module;
}


void vec4_rotate(float rot[4], float vec[4], float angle)
{
    float sin_t2, cos_t2, module;
    float t1[4], t2[4], t[4];

    angle /= 2.0;
    sin_t2 = sinf(angle);
    cos_t2 = cosf(angle);

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


void vec4_GetEilerOrientationTransform(float R[4], float ang[3])
{
    float t, Rt[4], T[4];

    t = ang[2] / 2.0;                                                           // ROLL
    R[0] = 0.0;                                                                 // -OZ
    R[1] = 0.0;
    R[2] = -sinf(t);
    R[3] = cosf(t);

    t = ang[0] / 2.0;                                                           // PITCH
    Rt[0] = sinf(t);                                                            // OX
    Rt[1] = 0.0;
    Rt[2] = 0.0;
    Rt[3] = cosf(t);
    vec4_mul(T, R, Rt)

    t = ang[1] / 2.0;                                                           // YAW
    Rt[0] = 0.0;                                                                // OY
    Rt[1] = sinf(t);
    Rt[2] = 0.0;
    Rt[3] = cosf(t);
    vec4_mul(R, T, Rt)
}


void vec4_GetQuaternionRotation(float q[4], float v0[3], float v1[3])
{
    float t;

    vec3_cross(q, v0, v1);
    q[3] = vec3_dot(v0, v1);
    q[3] += sqrtf(vec3_sqabs(v0) * vec3_sqabs(v1));
    t = vec4_abs(q);
    q[0] /= t;
    q[1] /= t;
    q[2] /= t;
    q[3] /= t;
}


void vec4_ClampQuaternionRotation(float q[4], float cos_abs)
{
    if(q[3] < cos_abs)
    {
        float k = sqrtf((1.0f - cos_abs * cos_abs) / (1.0f - q[3] * q[3]));
        q[0] *= k;
        q[1] *= k;
        q[2] *= k;
        q[3] = cos_abs;
    }
}


void vec3_GetPlaneEquation(float eq[4], float v0[3], float v1[3], float v2[3])
{
    float l1[3], l2[3], t;

    vec3_sub(l1, v1, v0);                                                       // get the first vector inside the plane
    vec3_sub(l2, v0, v2);                                                       // get the second vector inside the plane
    vec3_cross(eq, l1, l2);                                                     // get the normal vector to the plane

    t = sqrtf(eq[0]*eq[0] + eq[1]*eq[1] + eq[2]*eq[2]);                         // normalize vector
    eq[0] /= t;
    eq[1] /= t;
    eq[2] /= t;

    eq[3] = -(v0[0]*eq[0] + v0[1]*eq[1] + v0[2]*eq[2]);                         // distance from the plane to (0, 0, 0)
}


void vec3_RotateX(float res[3], float src[3], float ang)
{
    float t[2], sint, cost;

    ang *= M_PI / 180.0;
    sint = sinf(ang);
    cost = cosf(ang);
    res[0] = src[0];
    t[0] = src[1] * cost - src[2] * sint;
    t[1] = src[1] * sint + src[2] * cost;

    res[1] = t[0];
    res[2] = t[1];
}


void vec3_RotateY(float res[3], float src[3], float ang)
{
    float t[2], sint, cost;

    ang *= M_PI / 180.0;
    sint = sinf(ang);
    cost = cosf(ang);
    res[1] = src[1];
    t[0] = src[0] * cost + src[2] * sint;
    t[1] =-src[0] * sint + src[2] * cost;

    res[0] = t[0];
    res[2] = t[1];
}


void vec3_RotateZ(float res[3], float src[3], float ang)
{
    float t[2], sint, cost;

    ang *= M_PI / 180.0;
    sint = sinf(ang);
    cost = cosf(ang);
    res[2] = src[2];
    t[0] = src[0]*cost - src[1] * sint;
    t[1] = src[0]*sint + src[1] * cost;

    res[0] = t[0];
    res[1] = t[1];
}


void vec4_GetRotationOperators(float t1[4], float t2[4], const float v[3], float ang)
{
    float sin_t2, cos_t2, module;

    ang /= 2.0f;
    sin_t2 = sinf(ang);
    cos_t2 = cosf(ang);

    t1[3] = cos_t2;
    t1[0] = v[0] * sin_t2;
    t1[1] = v[1] * sin_t2;
    t1[2] = v[2] * sin_t2;
    module = vec4_abs(t1);
    t1[0] /= module;
    t1[1] /= module;
    t1[2] /= module;
    t1[3] /= module;

    t2[3] = t1[3];
    t2[0] = -t1[0];
    t2[1] = -t1[1];
    t2[2] = -t1[2];
}


void vec4_slerp(float ret[4], float q1[4], float q2[4], float t)
{
    float cos_fi, sin_fi, fi, k1, k2, sign;
    cos_fi = q1[3] * q2[3] + q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2];
    sign = (cos_fi < 0.0f) ? (-1.0f) : (1.0f);
    fi = acosf(sign * cos_fi);
    sin_fi = sinf(fi);

    if((fabs(sin_fi) > 0.00001f) && (t > 0.0001f) && (t < 1.0f))
    {
        k1 = sinf(fi * (1.0f - t)) / sin_fi;
        k2 = sinf(fi * t * sign) / sin_fi;
    }
    else
    {
        k1 = 1.0f - t;
        k2 = t;
    }

    ret[0] = k1 * q1[0] + k2 * q2[0];
    ret[1] = k1 * q1[1] + k2 * q2[1];
    ret[2] = k1 * q1[2] + k2 * q2[2];
    ret[3] = k1 * q1[3] + k2 * q2[3];

    fi = 1.0f / vec4_abs(ret);
    ret[0] *= fi;
    ret[1] *= fi;
    ret[2] *= fi;
    ret[3] *= fi;
}


void vec4_slerp_to(float ret[4], float q1[4], float q2[4], float max_step_rad)
{
    float cos_fi, sin_fi, fi, k1, k2, sign;
    float t = 1.0f;
    cos_fi = q1[3] * q2[3] + q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2];
    sign = (cos_fi < 0.0f) ? (-1.0f) : (1.0f);
    fi = acosf(sign * cos_fi);
    sin_fi = sinf(fi);

    t = fabs(fi);
    if(t > max_step_rad)
    {
        t = max_step_rad / t;
    }
    else
    {
        t = 1.0f;
    }

    if((fabs(sin_fi) > 0.00001f) && (t > 0.0001f) && (t < 1.0f))
    {
        k1 = sinf(fi * (1.0f - t)) / sin_fi;
        k2 = sinf(fi * t * sign) / sin_fi;
    }
    else
    {
        k1 = 1.0f - t;
        k2 = t;
    }

    ret[0] = k1 * q1[0] + k2 * q2[0];
    ret[1] = k1 * q1[1] + k2 * q2[1];
    ret[2] = k1 * q1[2] + k2 * q2[2];
    ret[3] = k1 * q1[3] + k2 * q2[3];

    fi = 1.0f / vec4_abs(ret);
    ret[0] *= fi;
    ret[1] *= fi;
    ret[2] *= fi;
    ret[3] *= fi;
}


void vec4_clampw(float q[4], float w)
{
    if((fabs(w) < 0.999f) && (fabs(q[3]) < 0.999f))
    {
        float k = 1.0f - w * w;
        k /= (vec3_sqabs(q));
        k = sqrtf(k);
        q[0] *= k;
        q[1] *= k;
        q[2] *= k;
        q[3] = w;
    }
    else
    {
        vec4_set_zero_angle(q);
    }
}


void vec4_SetZXYRotations(float v[4], float rot[3])
{
    float angle, sin_t2, cos_t2, qt[4], qX[4], qY[4], qZ[4];

    // OZ    Mat4_RotateZ(btag->transform, btag->rotate[2]);
    angle = M_PI * rot[2] / 360.0;
    sin_t2 = sinf(angle);
    cos_t2 = cosf(angle);

    qZ[3] = cos_t2;
    qZ[0] = 0.0 * sin_t2;
    qZ[1] = 0.0 * sin_t2;
    qZ[2] = 1.0 * sin_t2;

    // OX    Mat4_RotateX(btag->transform, btag->rotate[0]);
    angle = M_PI * rot[0] / 360.0;
    sin_t2 = sinf(angle);
    cos_t2 = cosf(angle);

    qX[3] = cos_t2;
    qX[0] = 1.0 * sin_t2;
    qX[1] = 0.0 * sin_t2;
    qX[2] = 0.0 * sin_t2;

    // OY    Mat4_RotateY(btag->transform, btag->rotate[1]);
    angle = M_PI * rot[1] / 360.0;
    sin_t2 = sinf(angle);
    cos_t2 = cosf(angle);

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

void Mat4_E(float mat[16])
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


void Mat4_Copy(float dst[16], const float src[16])
{
    vec4_copy(dst,    src);
    vec4_copy(dst+4,  src+4);
    vec4_copy(dst+8,  src+8);
    vec4_copy(dst+12, src+12);
}


void Mat4_Translate(float mat[16], const float v[3])
{
    mat[12] += mat[0] * v[0] + mat[4] * v[1] + mat[8]  * v[2];
    mat[13] += mat[1] * v[0] + mat[5] * v[1] + mat[9]  * v[2];
    mat[14] += mat[2] * v[0] + mat[6] * v[1] + mat[10] * v[2];
}


void Mat4_Scale(float mat[16], float x, float y, float z)
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


void Mat4_RotateX_SinCos(float mat[16], float sina, float cosa)
{
    float R[9];

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


void Mat4_RotateY_SinCos(float mat[16], float sina, float cosa)
{
    float R[9];

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


void Mat4_RotateZ_SinCos(float mat[16], float sina, float cosa)
{
    float R[9];

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


void Mat4_RotateAxis(float mat[16], float axis[3], float ang)
{
    if(ang != 0.0)
    {
        float t = ang * M_PI / 360.0;
        float sin_t2 = sinf(t);
        float cos_t2 = cosf(t);
        float R[4], Rt[4], *v, buf[4];

        R[0] = axis[0] * sin_t2;
        R[1] = axis[1] * sin_t2;
        R[2] = axis[2] * sin_t2;
        R[3] = cos_t2;
        vec4_sop(Rt, R);

        v = mat + 0;
        vec4_mul(buf, R, v);
        vec4_mul(v, buf, Rt);
        mat[0 + 3] = 0.0f;

        v = mat + 4;
        vec4_mul(buf, R, v);
        vec4_mul(v, buf, Rt);
        mat[4 + 3] = 0.0f;

        v = mat + 8;
        vec4_mul(buf, R, v);
        vec4_mul(v, buf, Rt);
        mat[8 + 3] = 0.0f;
    }
}


void Mat4_RotateQuaternion(float mat[16], float q[4])
{
    float qt[4], *v, buf[4];
    vec4_sop(qt, q);

    v = mat + 0;
    vec4_mul(buf, q, v);
    vec4_mul(v, buf, qt);
    mat[0 + 3] = 0.0f;

    v = mat + 4;
    vec4_mul(buf, q, v);
    vec4_mul(v, buf, qt);
    mat[4 + 3] = 0.0f;

    v = mat + 8;
    vec4_mul(buf, q, v);
    vec4_mul(v, buf, qt);
    mat[8 + 3] = 0.0f;
}


void Mat4_T(float mat[16])
{
    float t;
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
void Mat4_affine_inv(float mat[16])
{
    float v[3];

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

// Gauss method
int Mat4_inv(float mat[16], float inv[16])
{
    register int i, j, s, k;
    float f, r;

    Mat4_E_macro(inv);
    for(j = 0; j < 4 - 1; j++)      //direct run
    {
        f = fabs(mat[j * 4 + j]);   // f - max
        k = j;                      // k - index of max
        for(s = j + 1; s < 4; s++)
        {
            r = fabs(mat[s * 4 + j]);
            if(f < r)               // max < a[s, j]
            {
                f = r;
                k = s;
            }
        }
        s = k;
        if(s != j)
        {
            for(k = j; k < 4; k++)
            {
                SWAPT(mat[s * 4 + k], mat[j * 4 + k], r);
            }
            for(k = 0; k < 4; k++)
            {
                SWAPT(inv[s * 4 + k], inv[j * 4 + k], r);
            }
        }

        if(f < 0.000001f)
        {
            return 0;
        }

        for(i = j + 1; i < 4; i++)                                              //make  matrix a high triangle matrix - zero down elements
        {
            if(mat[i * 4 + j] == 0.0f)
            {
                continue;
            }
            f = mat[i * 4 + j] / mat[j * 4 + j];
            for(k = j + 1; k < 4; k++)                                          //sub matrix strings
            {
                mat[i * 4 + k] -= f * mat[j * 4 + k];
            }
            for(k = 0; k < 4; k++)                                              //sub extend strings
            {
                inv[i * 4 + k] -= f * inv[j * 4 + k];
            }
            mat[i * 4 + j] = 0.0f;
        }
    }

    for(j = 4 - 1; j >=0 ; j--)                                                 //run back
    {
        if(mat[j * 4 + j] == 0.0f)
        {
            return 0;
        }
        for(k = 0; k < 4; k++)
        {
            inv[j * 4 + k] /= mat[j * 4 + j];
        }
        mat[j * 4 + j] = 1.0f;
        for(i = j - 1; i >= 0; i--)
        {
            f = mat[i * 4 + j];
            for(k = 0; k < 4; k++)
            {
                inv[i * 4 + k] -= f * inv[j * 4 + k];
            }
            mat[i * 4 + j] = 0.0f;
        }
    }

    return 1;
}

/**
 * Matrix multiplication. result = src1 x src2.
 */
void Mat4_Mat4_mul(float result[16], const float src1[16], const float src2[16])
{
    // Store in temporary matrix so we don't overwrite anything if src1,2 alias result
    float t_res[16];

    t_res[0 * 4 + 0] = src1[0 * 4 + 0] * src2[0 * 4 + 0] + src1[1 * 4 + 0] * src2[0 * 4 + 1] + src1[2 * 4 + 0] * src2[0 * 4 + 2] + src1[3 * 4 + 0] * src2[0 * 4 + 3];
    t_res[0 * 4 + 1] = src1[0 * 4 + 1] * src2[0 * 4 + 0] + src1[1 * 4 + 1] * src2[0 * 4 + 1] + src1[2 * 4 + 1] * src2[0 * 4 + 2] + src1[3 * 4 + 1] * src2[0 * 4 + 3];
    t_res[0 * 4 + 2] = src1[0 * 4 + 2] * src2[0 * 4 + 0] + src1[1 * 4 + 2] * src2[0 * 4 + 1] + src1[2 * 4 + 2] * src2[0 * 4 + 2] + src1[3 * 4 + 2] * src2[0 * 4 + 3];
    t_res[0 * 4 + 3] = src1[0 * 4 + 3] * src2[0 * 4 + 0] + src1[1 * 4 + 3] * src2[0 * 4 + 1] + src1[2 * 4 + 3] * src2[0 * 4 + 2] + src1[3 * 4 + 3] * src2[0 * 4 + 3];

    t_res[1 * 4 + 0] = src1[0 * 4 + 0] * src2[1 * 4 + 0] + src1[1 * 4 + 0] * src2[1 * 4 + 1] + src1[2 * 4 + 0] * src2[1 * 4 + 2] + src1[3 * 4 + 0] * src2[1 * 4 + 3];
    t_res[1 * 4 + 1] = src1[0 * 4 + 1] * src2[1 * 4 + 0] + src1[1 * 4 + 1] * src2[1 * 4 + 1] + src1[2 * 4 + 1] * src2[1 * 4 + 2] + src1[3 * 4 + 1] * src2[1 * 4 + 3];
    t_res[1 * 4 + 2] = src1[0 * 4 + 2] * src2[1 * 4 + 0] + src1[1 * 4 + 2] * src2[1 * 4 + 1] + src1[2 * 4 + 2] * src2[1 * 4 + 2] + src1[3 * 4 + 2] * src2[1 * 4 + 3];
    t_res[1 * 4 + 3] = src1[0 * 4 + 3] * src2[1 * 4 + 0] + src1[1 * 4 + 3] * src2[1 * 4 + 1] + src1[2 * 4 + 3] * src2[1 * 4 + 2] + src1[3 * 4 + 3] * src2[1 * 4 + 3];

    t_res[2 * 4 + 0] = src1[0 * 4 + 0] * src2[2 * 4 + 0] + src1[1 * 4 + 0] * src2[2 * 4 + 1] + src1[2 * 4 + 0] * src2[2 * 4 + 2] + src1[3 * 4 + 0] * src2[2 * 4 + 3];
    t_res[2 * 4 + 1] = src1[0 * 4 + 1] * src2[2 * 4 + 0] + src1[1 * 4 + 1] * src2[2 * 4 + 1] + src1[2 * 4 + 1] * src2[2 * 4 + 2] + src1[3 * 4 + 1] * src2[2 * 4 + 3];
    t_res[2 * 4 + 2] = src1[0 * 4 + 2] * src2[2 * 4 + 0] + src1[1 * 4 + 2] * src2[2 * 4 + 1] + src1[2 * 4 + 2] * src2[2 * 4 + 2] + src1[3 * 4 + 2] * src2[2 * 4 + 3];
    t_res[2 * 4 + 3] = src1[0 * 4 + 3] * src2[2 * 4 + 0] + src1[1 * 4 + 3] * src2[2 * 4 + 1] + src1[2 * 4 + 3] * src2[2 * 4 + 2] + src1[3 * 4 + 3] * src2[2 * 4 + 3];

    t_res[3 * 4 + 0] = src1[0 * 4 + 0] * src2[3 * 4 + 0] + src1[1 * 4 + 0] * src2[3 * 4 + 1] + src1[2 * 4 + 0] * src2[3 * 4 + 2] + src1[3 * 4 + 0] * src2[3 * 4 + 3];
    t_res[3 * 4 + 1] = src1[0 * 4 + 1] * src2[3 * 4 + 0] + src1[1 * 4 + 1] * src2[3 * 4 + 1] + src1[2 * 4 + 1] * src2[3 * 4 + 2] + src1[3 * 4 + 1] * src2[3 * 4 + 3];
    t_res[3 * 4 + 2] = src1[0 * 4 + 2] * src2[3 * 4 + 0] + src1[1 * 4 + 2] * src2[3 * 4 + 1] + src1[2 * 4 + 2] * src2[3 * 4 + 2] + src1[3 * 4 + 2] * src2[3 * 4 + 3];
    t_res[3 * 4 + 3] = src1[0 * 4 + 3] * src2[3 * 4 + 0] + src1[1 * 4 + 3] * src2[3 * 4 + 1] + src1[2 * 4 + 3] * src2[3 * 4 + 2] + src1[3 * 4 + 3] * src2[3 * 4 + 3];

    memcpy(result, t_res, sizeof(t_res));
}


/**
 * OpenGL matrices multiplication. serult = (src1^-1) x src2.
 * Works only with affine transformation matrices!
 */
void Mat4_inv_Mat4_affine_mul(float result[16], float src1[16], float src2[16])
{
    float t_res[16], v[3];

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


void Mat4_vec3_mul(float v[3], const float mat[16], const float src[3])
{
    float ret[3];

    ret[0] = mat[0] * src[0] + mat[4] * src[1] + mat[8]  * src[2] + mat[12];
    ret[1] = mat[1] * src[0] + mat[5] * src[1] + mat[9]  * src[2] + mat[13];
    ret[2] = mat[2] * src[0] + mat[6] * src[1] + mat[10] * src[2] + mat[14];
    vec3_copy(v, ret);
}


void Mat4_vec3_mul_inv(float v[3], float mat[16], float src[3])
{
    float ret[3];

    ret[0]  = mat[0] * src[0] + mat[1] * src[1] + mat[2]  * src[2];             // (M^-1 * src).x
    ret[0] -= mat[0] * mat[12]+ mat[1] * mat[13]+ mat[2]  * mat[14];            // -= (M^-1 * mov).x
    ret[1]  = mat[4] * src[0] + mat[5] * src[1] + mat[6]  * src[2];             // (M^-1 * src).y
    ret[1] -= mat[4] * mat[12]+ mat[5] * mat[13]+ mat[6]  * mat[14];            // -= (M^-1 * mov).y
    ret[2]  = mat[8] * src[0] + mat[9] * src[1] + mat[10] * src[2];             // (M^-1 * src).z
    ret[2] -= mat[8] * mat[12]+ mat[9] * mat[13]+ mat[10] * mat[14];            // -= (M^-1 * mov).z
    vec3_copy(v, ret);
}


void Mat4_vec3_mul_T(float v[3], float mat[16], float src[3])
{
    float ret[3];

    ret[0] = mat[0] * src[0] + mat[1] * src[1] + mat[2]  * src[2] + mat[3];
    ret[1] = mat[4] * src[0] + mat[5] * src[1] + mat[6]  * src[2] + mat[7];
    ret[2] = mat[8] * src[0] + mat[9] * src[1] + mat[10] * src[2] + mat[11];
    vec3_copy(v, ret);
}


void Mat4_SetSelfOrientation(float mat[16], float ang[3])
{
    float R[4], Rt[4], temp[4];
    float sin_t2, cos_t2, t;

    sin_t2 = 0.0;
    cos_t2 = 1.0;

    if(ang[0] != 0.0)
    {
        t = ang[0] * M_PI / 180.0f;
        sin_t2 = sinf(t);
        cos_t2 = cosf(t);
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
        t = ang[1] * M_PI / 360.0f;                                             // UP - DOWN
        sin_t2 = sinf(t);
        cos_t2 = cosf(t);
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
        t = ang[2] * M_PI / 360.0f;                                             // ROLL
        sin_t2 = sinf(t);
        cos_t2 = cosf(t);
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


void Mat4_GetSelfOrientation(float ang[3], float mat[16])
{
    const float rad_to_deg = 180.0f / M_PI;
    
    ang[1] = rad_to_deg * asinf(mat[2 + 4 * 1]);
    if(ang[1] < 90.0f)
    {
        if(ang[1] > -90.0f)
        {
            ang[0] = rad_to_deg * atan2f(-mat[0 + 4 * 1], mat[1 + 4 * 1]);
            ang[2] = rad_to_deg * atan2f(-mat[2 + 4 * 0], mat[2 + 4 * 2]);
        }
        else
        {
            ang[0] =-rad_to_deg * atan2f(mat[0 + 4 * 2], mat[0 + 4 * 0]);
            ang[2] = 0.0f;
        }
    }
    else
    {
        ang[0] =-rad_to_deg * atan2f(mat[0 + 4 * 2], mat[0 + 4 * 0]);
        ang[2] = 0.0f;
    }
}


int ThreePlanesIntersection(float v[3], float n0[4], float n1[4], float n2[4])
{
    float d;
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
