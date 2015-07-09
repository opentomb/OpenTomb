/*
 *  Vec4.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 07.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Vec4.h"

bool float4::isOnTriangle(const float4 *points) const
{
	const float4 zero(0.0f);
	const float4 one(1.0f);
	
	const float4 p0p1(points[1] - points[0]);
	const float4 p0p2(points[2] - points[0]);
	
	const float4 normal(p0p1.cross(p0p2));
	if (fabsf((normal * *this) - (normal * points[0])) >= 0.001f)
		return false;
	
	const float4 p0hp(*this - points[0]);
	
	//	dot00 = dot(v0, v0)
	//	dot01 = dot(v0, v1)
	//	dot02 = dot(v0, v2)
	//	dot11 = dot(v1, v1)
	//	dot12 = dot(v1, v2)
	//	
	//	// Compute barycentric coordinates
	//	invDenom = 1 / (dot00 * dot11 - dot01 * dot01)
	//	u = (dot11 * dot02 - dot01 * dot12) * invDenom
	//	v = (dot00 * dot12 - dot01 * dot02) * invDenom
	//	
	//	// Check if point is in triangle
	//	return (u > 0) && (v > 0) && (u + v < 1)
	
	const float4 dot00(p0p1.vdot(p0p1));
	const float4 dot01(p0p1.vdot(p0p2));
	const float4 dot02(p0p1.vdot(p0hp));
	const float4 dot11(p0p2.vdot(p0p2));
	const float4 dot12(p0p2.vdot(p0hp));
	
	const float4 invDenom = one / (dot00.prod(dot11) - dot01.prod(dot01));
	const float4 u = (dot11.prod(dot02) - dot01.prod(dot12)).prod(invDenom);
	const float4 v = (dot00.prod(dot12) - dot01.prod(dot02)).prod(invDenom);
	
	return (u >= zero).all() && (v >= zero).all() && (u+v <= one).all();
}

matrix float4::transposeMult(const float4 &other) const
{
#ifdef __SSE__
	return matrix(_mm_mul_ps(v, _mm_shuffle_ps(other.v, other.v, _MM_SHUFFLE(0, 0, 0, 0))),
				  _mm_mul_ps(v, _mm_shuffle_ps(other.v, other.v, _MM_SHUFFLE(1, 1, 1, 1))),
				  _mm_mul_ps(v, _mm_shuffle_ps(other.v, other.v, _MM_SHUFFLE(2, 2, 2, 2))),
				  _mm_mul_ps(v, _mm_shuffle_ps(other.v, other.v, _MM_SHUFFLE(3, 3, 3, 3))));
#else
	return matrix(*this*other.x, *this*other.y, *this*other.z, *this*other.w);
#endif
}

matrix matrix::affineInverse() const
{	
	return matrix(float4(x.x, y.x, z.x, 0),
				  float4(x.y, y.y, z.y, 0),
				  float4(x.z, y.z, z.z, 0),
				  float4(-w*x, -w*y, -w*z, 1.0));
}


matrix matrix::transposed() const
{
#ifdef __SSE__
	__m128 tmp3, tmp2, tmp1, tmp0;
	tmp0 = _mm_unpacklo_ps(x.v, y.v);
	tmp2 = _mm_unpacklo_ps(z.v, w.v);
	tmp1 = _mm_unpackhi_ps(x.v, y.v);
	tmp3 = _mm_unpackhi_ps(z.v, w.v);
	
	return matrix(_mm_movelh_ps(tmp0, tmp2), _mm_movehl_ps(tmp2, tmp0), _mm_movelh_ps(tmp1, tmp3), _mm_movehl_ps(tmp3, tmp1));
#else
	return matrix(float4(x.x, y.x, z.x, w.x),
				  float4(x.y, y.y, z.y, w.y),
				  float4(x.z, y.z, z.z, w.z),
				  float4(x.w, y.w, z.w, w.w));
#endif
}

matrix matrix::diagonal(const float4 &diagonal)
{
	return matrix(float4(diagonal.x, 0.0f, 0.0f, 0.0f),
				  float4(0.0f, diagonal.y, 0.0f, 0.0f),
				  float4(0.0f, 0.0f, diagonal.z, 0.0f),
				  float4(0.0f, 0.0f, 0.0f, diagonal.w));
}

matrix matrix::rotation(float4 axis, float angle)
{
	matrix result;
	
	float sin = std::sin(angle);
	float cos = std::cos(angle);
	
	axis.w = 0.0f;
	float4 u(axis.normalized());
	
	/*
	 * According to Redbook:
	 *
	 * u = axis/||axis||
	 *
	 *     |  0 -z  y |
	 * S = |  z  0 -x |
	 *     | -y  x  0 |
	 *
	 * M = uu^t + cos(a)(I - uu^t) + sin(a)*S
	 *
	 * That is: M.x = (uu^t).x + cos(a)((1 0 0)^t - uu^t.x) + sin(a) (0 -z y)^t
	 * uu^t.x = u.x * u
	 * And so on for the others
	 */
	
	result.x = u.x*u + cos*(float4(1, 0, 0, 0) - u.x*u) + sin * float4( 0.0,  u.z, -u.y, 0);
	result.y = u.y*u + cos*(float4(0, 1, 0, 0) - u.y*u) + sin * float4(-u.z,  0.0,  u.x, 0);
	result.z = u.z*u + cos*(float4(0, 0, 1, 0) - u.z*u) + sin * float4( u.y, -u.x,  0.0, 0);
	result.w = float4(0, 0, 0, 1);
	
	return result;
}

matrix matrix::frustum(float angle, float aspect, float near, float far)
{
	float ymax = near * tanf(angle * float(M_PI) / 180.0f) * 0.5;
	float xmax = ymax * aspect;
	
	matrix result;
	
	result[0][0] = near/xmax;
	result[1][1] = near/ymax;
	
	result[2][2] = -(far + near) / (far - near);
	result[3][2] = -(2.0f * far * near) / (far - near);
	
	result[2][3] = -1.0f;
	result[3][3] = 0.0f;
	
	return result;
}

matrix matrix::inverseFrustum(float angle, float aspect, float near, float far)
{
	float ymax = near * tanf(angle * float(M_PI) / 360.0f);
	float xmax = ymax * aspect;
	
	matrix result;
	
	result[0][1] = 0.0f;
	result[1][0] = 0.0f;
	result[1][1] = ymax / near;
	result[0][0] = xmax / near;
	
	result[2][2] = 0.0f;
	result[2][3] = -(far - near) / (2.0f * near * far);
	result[3][3] = (far + near) / (2.0f * near * far);
	result[3][2] = -1.0f;
	
	return result;	
}

matrix matrix::lookat(const float4 &eye, const float4 &center, const float4 &up)
{
	matrix result;
	float4 direction = center - eye;
	result.x = direction.cross(up).normalized();
	result.y = result.x.cross(direction).normalized();
	result.z = -direction.normalized();
	result.w = eye;
	
	return result.affineInverse();
}

matrix matrix::fromQuaternion(const float4 &quat)
{
	matrix result;
	
	result[0][0] = 1.0f - 2.0f*(quat.y*quat.y + quat.z*quat.z);
	result[0][1] = 2.f*quat.w*quat.z + 2.f*quat.x*quat.y;
	result[0][2] = -2.f*quat.w*quat.y + 2.f*quat.x*quat.z;
	
	result[1][0] = -2.f*quat.w*quat.z + 2.f*quat.x*quat.y;
	result[1][1] = 1.0f - 2.0f*(quat.z*quat.z + quat.x*quat.x);
	result[1][2] = 2.f*quat.w*quat.x + 2.f*quat.y*quat.z;
	
	result[2][0] = 2.f*quat.w*quat.y + 2.f*quat.x*quat.z;
	result[2][1] = -2.f*quat.w*quat.x + 2.f*quat.y*quat.z;
	result[2][2] = 1.0f - 2.0f*(quat.x*quat.x + quat.y*quat.y);
	
	return result;
}

bool ray4::hitsAABB(const float4 &min, const float4 &max, float4 &position) const
{
	uint4 startSmallerMin = (start() < min);
	uint4 startBiggerMax = (start() > max);
	
	if (!(startSmallerMin || startBiggerMax).any())
	{
		position = start();
		return true;
	}
	
	float4 dir = direction();
	uint4 dirNotNull = (dir != 0);
	float4 maxT(-1.0f);
	
	position = startSmallerMin.select(min, position);
	position = startBiggerMax.select(max, position);
	maxT = (startSmallerMin && dirNotNull).select((min - start())/dir, maxT);
	maxT = (startBiggerMax && dirNotNull).select((max - start())/dir, maxT);
	
	if (maxT.max() < 0.0f)
		return false;
	
	position = point(maxT.max());
	
	if (((position > max) || (position < min)).any())
		return false;
	
	return true;
}

bool ray4::hitsTriangle(const float4 *points, float &length) const
{	
	const float4 p0p1(points[1] - points[0]);
	const float4 p0p2(points[2] - points[0]);
	
	const float4 normal = float4(p0p1.cross(p0p2));
	
	//	n * p = n * (s + t*d) = n*s + n*d*t
	//	n(p-s) = ndt
	//	(n(p-s))/(n*d) = t
	
	float4 dir = direction();
	float4 outFactor = float4(-1.0f);
	
	float4 normalTimesDirection = normal.prod(dir);
	
	outFactor = (normalTimesDirection != float4(0.0f)).select(normal.prod(points[0] - start()) / normalTimesDirection, outFactor);
	
	if ((outFactor < float4(0.0f)).all())
		return false;
	if (outFactor.max() > 1.0f)
		return false;
	
	const float4 location = this->point(outFactor.max());
	
	length = outFactor.max();
	
	return location.isOnTriangle(points);
}

std::ostream &operator<<(std::ostream &out, const float4 &vec)
{
	return out << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w;
}
