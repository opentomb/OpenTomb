/*
 *  matrix4.cpp
 *  OpenTomb
 *
 *  Created by Torsten Kammer on 07.05.10.
 */

#include <iostream>

#include "matrix4.h"
#include "vmath.h"

matrix4 float4::transposeMult(const float4 &other) const
{
#ifdef __SSE__
	return matrix4(_mm_mul_ps(v, _mm_shuffle_ps(other.v, other.v, _MM_SHUFFLE(0, 0, 0, 0))),
				  _mm_mul_ps(v, _mm_shuffle_ps(other.v, other.v, _MM_SHUFFLE(1, 1, 1, 1))),
				  _mm_mul_ps(v, _mm_shuffle_ps(other.v, other.v, _MM_SHUFFLE(2, 2, 2, 2))),
				  _mm_mul_ps(v, _mm_shuffle_ps(other.v, other.v, _MM_SHUFFLE(3, 3, 3, 3))));
#else
	return matrix4(*this*other.x, *this*other.y, *this*other.z, *this*other.w);
#endif
}

matrix4 matrix4::affineInverse() const
{
	return matrix4(float4(x.x, y.x, z.x, 0),
				  float4(x.y, y.y, z.y, 0),
				  float4(x.z, y.z, z.z, 0),
				  float4(-w*x, -w*y, -w*z, 1.0));
}

matrix4 matrix4::transposed() const
{
#ifdef __SSE__
	__m128 tmp3, tmp2, tmp1, tmp0;
	tmp0 = _mm_unpacklo_ps(x.v, y.v);
	tmp2 = _mm_unpacklo_ps(z.v, w.v);
	tmp1 = _mm_unpackhi_ps(x.v, y.v);
	tmp3 = _mm_unpackhi_ps(z.v, w.v);

	return matrix4(_mm_movelh_ps(tmp0, tmp2), _mm_movehl_ps(tmp2, tmp0), _mm_movelh_ps(tmp1, tmp3), _mm_movehl_ps(tmp3, tmp1));
#else
	return matrix4(float4(x.x, y.x, z.x, w.x),
				  float4(x.y, y.y, z.y, w.y),
				  float4(x.z, y.z, z.z, w.z),
				  float4(x.w, y.w, z.w, w.w));
#endif
}

matrix4 matrix4::diagonal(const float4 &diagonal)
{
	return matrix4(float4(diagonal.x, 0.0f, 0.0f, 0.0f),
				  float4(0.0f, diagonal.y, 0.0f, 0.0f),
				  float4(0.0f, 0.0f, diagonal.z, 0.0f),
				  float4(0.0f, 0.0f, 0.0f, diagonal.w));
}

matrix4 matrix4::createLookAt(const float4 &eye, const float4 &center, const float4 &up)
{
    matrix4 result;
    float4 direction = center - eye;
    result.x = direction.cross(up).normalized();
    result.y = result.x.cross(direction).normalized();
    result.z = -direction.normalized();
    result.w = eye;

    return result.affineInverse();
}

std::ostream &operator<<(std::ostream &out, const float4 &vec)
{
    return out << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w;
}
