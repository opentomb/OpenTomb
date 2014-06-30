#ifndef _GLMATH_H_
#define _GLMATH_H_

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef F_PI
#define F_PI 3.14159265358979323846f
#endif

class vec3_t {
      public:
	float x, y, z;

	vec3_t () {
		x = 0; y = 0; z = 0;
	};

	vec3_t (float a, float b, float c) {
		x = a; y = b; z = c;
	};

	float &operator [] (const long i) {
		return *((&x) + i);
	};

	const bool operator ==(const vec3_t &v) const {
		return (v.x == x && v.y == y && v.z == z);
	};

	const bool operator !=(const vec3_t &v) const {
		return !(v == *this);
	};

	const vec3_t operator -() const {
		return vec3_t (-x, -y, -z);
	};

	const vec3_t &operator =(const vec3_t &v) {
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	};

	const vec3_t &operator +=(const vec3_t &v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	};

	const vec3_t &operator -=(const vec3_t &v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	};

	const vec3_t &operator *=(const float &s) {
		x *= s;
		y *= s;
		z *= s;
		return *this;
	};

	const vec3_t &operator /=(const float &s) {
		const float r = 1 / s;

		x *= r;
		y *= r;
		z *= r;
		return *this;
	};

	const vec3_t operator +(const vec3_t &v) const {
		return vec3_t (x + v.x, y + v.y, z + v.z);
	};

	const vec3_t operator -(const vec3_t &v) const {
		return vec3_t (x - v.x, y - v.y, z - v.z);
	};

	const vec3_t operator *(const float &s) const {
		return vec3_t (x * s, y * s, z * s);
	};

	friend inline const vec3_t operator *(const float &s, const vec3_t &v) {
		return v * s;
	};

	const vec3_t operator /(float s) const {
		s = 1 / s;
		return vec3_t (s * x, s * y, s * z);
	};

	const vec3_t cross(const vec3_t &v) const {
		return vec3_t (y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	};

	const float dot(const vec3_t &v) const {
		return x * v.x + y * v.y + z * v.z;
	};
};

#endif // _GLMATH_H_
