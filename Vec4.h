#ifndef VEC4_H
#define VEC4_H

/*
 *  Vec4.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 07.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <cmath>
#include <cstring>
#include <limits>
#include <ostream>

#if defined(_WIN32) || defined(__SSE__)
#include <xmmintrin.h>
#include <pmmintrin.h>
#if defined(__SSE4_1__)
#include <smmintrin.h>
#endif
#endif

#if defined(_MSC_VER)
// Microsoft Visual C++
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#define M_PI 3.1415926536

static inline float fmaxf(float a, float b)
{
	return a > b ? a : b;
}
static inline float fminf(float a, float b)
{
	return b > a ? a : b;
}
#else
// Mac OS X or iPhone or something else (that hopefully has this)
static inline int _finite(float a)
{
	return std::isfinite(a);
}
#endif

union float4;
struct matrix;

union uint4
{
	struct
	{
		unsigned x;
		unsigned y;
		unsigned z;
		unsigned w;
	};
#ifdef __SSE__
	__m128 v; // The type for bitwise operations is the one for floating point
	// because integer operations and types were a later addition to SSE.
#endif
	
	uint4(bool a, bool b, bool c, bool d)
#if defined(__SSE__)
	: x(std::numeric_limits<unsigned>::max() * a), y(std::numeric_limits<unsigned>::max() * b), z(std::numeric_limits<unsigned>::max() * c), w(std::numeric_limits<unsigned>::max() * d) {}
#else
	: x(a), y(b), z(c), w(d) {}
#endif
#ifdef __SSE__
	uint4(__m128 vec) : v(vec) {}
#endif
	
	bool any() const
	{
#ifdef __SSE__
		return _mm_movemask_ps(v) != 0;
#else
		return x || y || z || w;
#endif
	}
	
	bool all() const
	{
#ifdef __SSE__
		return _mm_movemask_ps(v) == 0xF;
#else
		return x && y && z && w;
#endif
	}
	
	uint4 operator&&(const uint4 &other)
	{
#ifdef __SSE__
		return _mm_and_ps(v, other.v);
#else
		return uint4(x && other.x, y && other.y, z && other.z, w && other.w);
#endif
	}
	uint4 operator||(const uint4 &other)
	{
#ifdef __SSE__
		return _mm_or_ps(v, other.v);
#else
		return uint4(x || other.x, y || other.y, z || other.z, w || other.w);
#endif
	}
	uint4 operator!()
	{
#if defined(__VEC__)
        // !v = v NOR 0 = v = v NOR (x XOR x) = v NOR (v XOR v)
        return vec_nor(v, vec_xor(v, v));
#else
		return uint4(!x, !y, !z, !w);
#endif
	}
		
	float4 select(const float4 &a, const float4 &b) const;
};

union float4
{
	// Data storage
	struct
	{
		float x;
		float y;
		float z;
		float w;
	};
#if defined(__SSE__)
	__m128 v;
#endif /* SSE */ 
	float4() {}
	float4(float anX, float anY, float aZ, float aW = 1.0f) : x(anX), y(anY), z(aZ), w(aW) {}
	float4(float s) : x(s), y(s), z(s), w(s) {}
	float4(const float4 &other)
#if defined(__SSE__)
	: v(other.v)
#else
	: x(other.x), y(other.y), z(other.z), w(other.w)
#endif
	{}
	
#ifdef __SSE__
	float4(__m128 vec) : v(vec) {}
#endif
	
	// Arithmetic
	
	float4 operator+(const float4 &other) const
	{
#ifdef __SSE__
		return _mm_add_ps(v, other.v);
#else
		return float4(x+other.x, y+other.y, z+other.z, w+other.w);
#endif
	}
	
	void operator+=(const float4 &other)
	{
#ifdef __SSE__
		v = _mm_add_ps(v, other.v);
#else
		*this = *this + other;
#endif
	}
	
	float4 operator-(const float4 &other) const
	{
#ifdef __SSE__
		return _mm_sub_ps(v, other.v);
#else
		return float4(x-other.x, y-other.y, z-other.z, w-other.w);
#endif
	}
	float4 operator/(const float4 &other) const
	{
#ifdef __SSE__
		return _mm_div_ps(v, other.v);
#else
		return float4(x/other.x, y/other.y, z/other.z, w/other.w);
#endif
	}
	
	float4 operator-() const
	{
#ifdef __SSE__
		return _mm_sub_ps(_mm_setzero_ps(), v);
#else
		return float4(-x, -y, -z, -w);
#endif
	}
	
	float4 operator*(float s) const { return float4(x*s, y*s, z*s, w*s); }
	float4 operator/(float s) const { return *this * (1.0f/s); }
	
	float operator*(const float4 &other) const
	{
		return x*other.x + y*other.y + z*other.z + w*other.w;
	}
	
	//   Used for quaternion multiplication. For directions, will give the same
	//   result as operator* (and vdot).
	float dot3(const float4 &other) const
	{
		return x*other.x + y*other.y + z*other.z;
	}
	
	float4 prod(const float4 &other) const
	{
#ifdef __SSE__
		return _mm_mul_ps(v, other.v);
#else
		return float4(x*other.x, y*other.y, z*other.z, w*other.w);
#endif
	}
	
	//   Sets all elements of the result to the dot product.
	//   This allows to keep some algorithms float4-only.
	float4 vdot(const float4 &other) const
	{
#ifdef __SSE4_1__
		return _mm_dp_ps(v, other.v, 0xFF);
#elif defined(__SSE__)
		__m128 c = _mm_mul_ps(v, other.v);
		c = _mm_hadd_ps(c, c);
		c = _mm_hadd_ps(c, c);
		return _mm_shuffle_ps(c, c, _MM_SHUFFLE(0, 0, 0, 0));
#else
		float dot = *this * other;
		return float4(dot);
#endif
	}
	
	float4 cross(const float4 &o) const
	{
#ifdef __SSE__
		// Remember: _MM_SHUFFLE takes arguments in reverse order
		return _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 0, 2, 1)),
									 _mm_shuffle_ps(o.v, o.v, _MM_SHUFFLE(3, 1, 0, 2))),
						  _mm_mul_ps(_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 1, 0, 2)), 
									 _mm_shuffle_ps(o.v, o.v, _MM_SHUFFLE(3, 0, 2, 1))));
#else
		return float4(y*o.z - z*o.y,
					  z*o.x - x*o.z,
					  x*o.y - y*o.x,
					  0.0f);
#endif
	}
	
	matrix transposeMult(const float4 &other) const;
	
	float4 squared() const
	{
#ifdef __SSE__
		return _mm_mul_ps(v, v);
#else
		return float4(x*x, y*y, z*z, w*w);
#endif
	}
	
	void operator*=(float s)
	{
		*this = *this * s;
	}
	
	void operator/=(const float4 &other)
	{
#ifdef __SSE__
		v = _mm_div_ps(v, other.v);
#else
		*this = *this / other;
#endif
	}
	void operator-=(const float4 &other)
	{
#ifdef __SSE__
		v = _mm_sub_ps(v, other.v);
#else
		*this = *this - other;
#endif
	}
	
	float max() const { return fmaxf(fmaxf(x, y), fmaxf(z, w)); }
	float min() const { return fminf(fminf(x, y), fminf(z, w)); }
	
	float4 min(const float4 &other) const
	{
#ifdef __SSE__
		return _mm_min_ps(v, other.v);
#else
		return float4(fminf(x, other.x), fminf(y, other.y), fminf(z, other.z), fminf(w, other.w));
#endif
	}
	float4 max(const float4 &other) const
	{
#ifdef __SSE__
		return _mm_max_ps(v, other.v);
#else
		return float4(fmaxf(x, other.x), fmaxf(y, other.y), fmaxf(z, other.z), fmaxf(w, other.w));
#endif
	}
	float4 abs() const
	{
#ifdef __SSE__
		// Shift once to the left (losing sign bit), once to the right (highest
		// bit then gets set to 0). Trick found in Apple sample code.
		return (__m128) _mm_srli_epi32( _mm_slli_epi32( (__m128i) v, 1 ), 1 );
#else
		return float4(std::fabs(x), std::fabs(y), std::fabs(z), std::fabs(w));
#endif
	}
	
	float4 floor() const
	{
#if defined(__SSE4_1__)
		return _mm_floor_ps(v);
#elif defined(__SSE__)
		// Source: http://developer.apple.com/hardwaredrivers/ve/sse.html#Translation
		
		const __m128 twoTo23 = _mm_set1_ps(0x1.0p23f);
		__m128 b = (__m128) _mm_srli_epi32( _mm_slli_epi32( (__m128i) v, 1 ), 1 ); //fabs(v)
		__m128 d = _mm_sub_ps( _mm_add_ps( _mm_add_ps( _mm_sub_ps( v, twoTo23 ), twoTo23 ), twoTo23 ), twoTo23 ); //the meat of floor
		__m128 largeMaskE = (__m128) _mm_cmpgt_ps( b, twoTo23 ); //-1 if v >= 2**23
		__m128 g = (__m128) _mm_cmplt_ps( v, d ); //check for possible off by one error
		__m128 h = _mm_cvtepi32_ps( (__m128i) g ); //convert positive check result to -1.0, negative to 0.0
		__m128 t = _mm_add_ps( d, h ); //add in the error if there is one
		
		//Select between output result and input value based on v >= 2**23
		__m128 newV = _mm_and_ps( v, largeMaskE );
		t = _mm_andnot_ps( largeMaskE, t );
		
		return _mm_or_ps( t, newV );
#else
		return float4(std::floor(x), std::floor(y), std::floor(z), std::floor(w));
#endif
	}

	float length() const { return std::sqrt(*this * *this); }
	float4 normalized() const
	{
#ifdef __SSE__
		__m128 dot = v*v;
		dot = _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(dot, dot, _MM_SHUFFLE(0,0,0,0)), _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(1,1,1,1))), _mm_add_ps(_mm_shuffle_ps(dot, dot, _MM_SHUFFLE(2,2,2,2)), _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(3,3,3,3))));
		__m128 scale = _mm_rsqrt_ps(dot);
		// Newton-Rhapson-Iteration
		// scale *= 1.5 - 0.5 * dot * scale^2
		scale = _mm_mul_ps(scale, _mm_sub_ps(_mm_set1_ps(1.5f), _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f), dot), _mm_mul_ps(scale, scale))));
		return v * scale;
#else
		return *this / length();
#endif
	}
	
	// Array and pointer access
	const float *c_ptr() const { return &x; }
	float *c_ptr() { return &x; }
	
	const float &operator[](int i) const { return this->c_ptr()[i]; }
	float &operator[](int i) { return this->c_ptr()[i]; }
	
	// Comparison
	uint4 operator>(const float4 &other) const
	{
#ifdef __SSE__
		return _mm_cmpgt_ps(v, other.v);
#else
		return uint4(x > other.x, y > other.y, z > other.z, w > other.w);
#endif
	}
	uint4 operator<(const float4 &other) const
	{
#ifdef __SSE__
		return _mm_cmplt_ps(v, other.v);
#else
		return uint4(x < other.x, y < other.y, z < other.z, w < other.w);
#endif
	}
	uint4 operator>=(const float4 &other) const
	{
#ifdef __SSE__
		return _mm_cmpge_ps(v, other.v);
#else
		return uint4(x >= other.x, y >= other.y, z >= other.z, w >= other.w);
#endif
	}
	uint4 operator<=(const float4 &other) const
	{
#ifdef __SSE__
		return _mm_cmple_ps(v, other.v);
#else
		return uint4(x <= other.x, y <= other.y, z <= other.z, w <= other.w);
#endif
	}
	uint4 operator==(const float4 &other) const
	{
#ifdef __SSE__
		return _mm_cmpeq_ps(v, other.v);
#else
		return uint4(x == other.x, y == other.y, z == other.z, w == other.w);
#endif
	}
	uint4 operator!=(const float4 &other) const
	{
#ifdef __SSE__
		return _mm_cmpneq_ps(v, other.v);
#else
		return uint4(x != other.x, y != other.y, z != other.z, w != other.w);
#endif
	}
	
	uint4 is_finite() const { return uint4(_finite(x), _finite(y), _finite(z), _finite(w)); }
	
	bool isOnTriangle(const float4 *points) const;
	
	// Treat as quaternion
	// Note: We use the final component as w (not the first), mainly because it
	// already has that name in the struct.
	float4 quatMul(const float4 &otherQuat) const
	{
		float4 result(cross(otherQuat));
		result += otherQuat * w;
		result += *this * otherQuat.w;
		
		result.w = w*otherQuat.w - dot3(otherQuat);
		return result;
	}
	float4 quatConjugated() const
	{
		return float4(-x, -y, -z, w);
	}
	float4 quatRotate(const float4 &vec) const
	{
		// Assumes that quaternion is unit length and that vec.w == 0
		float4 result = quatMul(vec).quatMul(quatConjugated());
		return float4(result.x, result.y, result.z, 0.0f);
	}
	float4 quatNormalized() const
	{
		return normalized();
	}
};

inline float4 operator*(float s, float4 v)
{
	return v * s;
}

inline float4 uint4::select(const float4 &a, const float4 &b) const
{
#if defined(__SSE4_1__)
	return _mm_blendv_ps(a.v, b.v, v);
#elif defined(__SSE__)
	return _mm_or_ps(_mm_and_ps(v, a.v), _mm_andnot_ps(v, b.v));
#else
	return float4(x ? a.x : b.x, y ? a.y : b.y, z ? a.z : b.z, w ? a.w : b.w);
#endif
}

class ray4
{
	float4 s;
	float4 e;
	
public:
	ray4(const float4 &start, const float4 &end)
	: s(start), e(end) {}
	
	float4 &start() { return s; }
	const float4 &start() const { return s; }
	
	float4 &end() { return e; }
	const float4 &end() const { return e; }
	
	float4 direction() const { return e-s; };
	
	float4 point(float t) const { return s + t*direction(); }
	
	ray4 reverse() const { return ray4(end(), start()); }
	
	bool hitsAABB(const float4 &min, const float4 &max, float4 &point) const;
	
	ray4 operator/(float4 f) const { return ray4(s/f, e/f); }
	
	float planeIntersection(const float4 &planeNormal, const float4 &planePoint) const
	{
		return (planeNormal*(planePoint - start()))/(planeNormal*direction());
	}
	float4 planeIntersectionPoint(const float4 &planeNormal, const float4 &planePoint) const
	{
		return point(planeIntersection(planeNormal, planePoint));
	}
	bool hitsTriangle(const float4 *points, float &length) const;
};

struct matrix
{
	float4 x;
	float4 y;
	float4 z;
	float4 w;
	
	matrix() : x(1.0f, 0.0f, 0.0f, 0.0f), y(0.0f, 1.0f, 0.0f, 0.0f), z(0.0f, 0.0f, 1.0f, 0.0f), w(0.0f, 0.0f, 0.0f, 1.0f) {}
	matrix(float4 anX, float4 anY, float4 aZ, float4 aW) : x(anX), y(anY), z(aZ), w(aW) {}
	matrix(const float *matrix) { memcpy(c_ptr(), matrix, sizeof(float [16])); }
	
	static matrix position(const float4 &position) { return matrix(float4(1.f, 0.f, 0.f, 0.f), float4(0.f, 1.f, 0.f, 0.f), float4(0.f, 0.f, 1.f, 0.f), position); }
	static matrix rotation(const float4 vector, float angleInRadian);
	static matrix frustum(float angle, float aspect, float near, float far);
	static matrix inverseFrustum(float angle, float aspect, float near, float far);
	static matrix lookat(const float4 &eye, const float4 &center, const float4 &up);
	static matrix fromQuaternion(const float4 &quaternion);
	static matrix diagonal(const float4 &diagonal);
	
	matrix transposed() const;
	
	matrix operator+(const matrix &other) const
	{
#ifdef __SSE__
		return matrix(_mm_add_ps(x.v, other.x.v),
					  _mm_add_ps(y.v, other.y.v),
					  _mm_add_ps(z.v, other.z.v),
					  _mm_add_ps(w.v, other.w.v));
#else
		return matrix(x + other.x, y + other.y, z + other.z, w + other.w);
#endif
	}
	
	void operator+=(const matrix &other)
	{
#ifdef __SSE__
		x.v = _mm_add_ps(x.v, other.x.v);
		y.v = _mm_add_ps(y.v, other.y.v);
		z.v = _mm_add_ps(z.v, other.z.v);
		w.v = _mm_add_ps(w.v, other.w.v);
#else
		*this = *this + other;
#endif
	}
	matrix operator-(const matrix &other) const
	{
#ifdef __SSE__
		return matrix(_mm_sub_ps(x.v, other.x.v),
					  _mm_sub_ps(y.v, other.y.v),
					  _mm_sub_ps(z.v, other.z.v),
					  _mm_sub_ps(w.v, other.w.v));
#else
		return matrix(x - other.x, y - other.y, z - other.z, w - other.w);
#endif
	}
	
	float4 operator*(const float4 &vec) const
	{
#ifdef __SSE__
		// vec.x * x + vec.y * y + vec.z * z + vec.w * w
		return _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(vec.v, vec.v, _MM_SHUFFLE(0,0,0,0)), x.v), _mm_mul_ps(_mm_shuffle_ps(vec.v, vec.v, _MM_SHUFFLE(1,1,1,1)), y.v)), _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(vec.v, vec.v, _MM_SHUFFLE(2,2,2,2)), z.v), _mm_mul_ps(_mm_shuffle_ps(vec.v, vec.v, _MM_SHUFFLE(3,3,3,3)), w.v)));
#else
		return float4(vec.x * x.x + vec.y * y.x + vec.z * z.x + vec.w * w.x,
					  vec.x * x.y + vec.y * y.y + vec.z * z.y + vec.w * w.y,
					  vec.x * x.z + vec.y * y.z + vec.z * z.z + vec.w * w.z,
					  vec.x * x.w + vec.y * y.w + vec.z * z.w + vec.w * w.w);
#endif
	}
	ray4 operator*(const ray4 &ray) const
	{
		return ray4(*this * ray.start(), *this * ray.end());
	}
	matrix operator*(const matrix &other) const
	{
		return matrix(*this * other.x, *this * other.y, *this * other.z, *this * other.w);
	};
	matrix mulRotationOnly(const matrix &other) const
	{
		return matrix(*this * other.x, *this * other.y, *this * other.z, float4(0.0f, 0.0f, 0.0f, 1.0f));
	};
	void operator*=(const matrix &other)
	{
		*this = *this * other;
	}
	matrix affineInverse() const;
	
	const float *c_ptr() const { return x.c_ptr(); }
	float *c_ptr() { return x.c_ptr(); }
	
	const float4 &operator[](int i) const { return (&x)[i]; }
	float4 &operator[](int i) { return (&x)[i]; }
	
};

std::ostream &operator<<(std::ostream &out, const float4 &vec);

#endif /* VEC4_H */