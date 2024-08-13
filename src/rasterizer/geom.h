#pragma once

#include <vector>
#include <cmath>
#include <stdio.h>
#include <string>
#include <memory>
#include "geom.h"

template <class t> struct Vec3
{
	union
	{
		struct { t x, y, z; };
		struct { int i_vert, i_text, i_norm; };
		t raw[3];
	};

	//empty init
	Vec3() { x = 0; y = 0; z = 0; }
	//non-empty init
	Vec3(t _x, t _y, t _z) { x = _x; y = _y; z = _z; }
	//copy
	Vec3(const Vec3& v) { x = v.x; y = v.y; z = v.z; }

	//operator overloads
	inline Vec3<t> operator +(const Vec3<t>& v) const { return Vec3<t>(x + v.x, y + v.y, z + v.z); } //add
	inline Vec3<t> operator -(const Vec3<t>& v) const { return Vec3<t>(x - v.x, y - v.y, z - v.z); } //subtract
	inline Vec3<t> operator -(float f)          const { return Vec3<t>(x - f, y - f, z - f); } //const subtract
	inline Vec3<t> operator *(float f)          const { return Vec3<t>(x * f, y * f, z * f); } //const mult
	inline Vec3<t> operator *(const Vec3<t>& v) const { return Vec3<t>(x * v.x, y * v.y, z * v.z); } //vector mult
	inline Vec3<t> operator /(float f)          const { return Vec3<t>(x / f, y / f, z / f); } //const div

	//vector operations
	inline Vec3<t> cross(const Vec3<t>& v) const { return Vec3<t>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); } //cross product
	inline t dot(const Vec3<t>& v) const { return (x * v.x + y * v.y + z * v.z); } //dot product
	float dist(const Vec3<t> v) const { return sqrtf(powf((v.x - x), 2) + powf((v.y - y), 2) + powf((v.z - z), 2)); }
	float value() const { return sqrtf(x * x + y * y + z * z); }
	Vec3<t> norm() const { return *this / this->value(); }

	//to_string
	std::string to_string() { return "0: " + std::to_string(x) + "\n1: " + std::to_string(y) + "\n2: " + std::to_string(z) + "\n"; }
};

typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;

struct Vec4f
{
	union
	{
		struct { float x, y, z, w; };
		float raw[4];
	};

	//empty init
	Vec4f() { x = 0; y = 0; z = 0; w = 0; }
	//non-empty init
	Vec4f(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
	Vec4f(const Vec3f &v) { x = v.x; y = v.y; z = v.z; w = 1.f; }
	//copy
	Vec4f(const Vec4f &v) { x = v.x; y = v.y; z = v.z; w = v.w; }

	//cast to vec3
	inline operator Vec3f() const { return Vec3f(x, y, z); }

	//to_string
	std::string to_string() const { return "0: " + std::to_string(x) + "\n1: " + std::to_string(y) + "\n2: " + std::to_string(z) + "\n3:" + std::to_string(w) + "\n"; }
};

struct Mat4x4f
{
	float val[4][4] = { 0 };

	Mat4x4f() { val[0][0] = 1.f; val[1][1] = 1.f; val[2][2] = 1.f; val[3][3] = 1.f; };
	Mat4x4f(float in[4][4])
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				val[i][j] = in[i][j];
			}
		}
	}

	//copy
	Mat4x4f(const Mat4x4f& m)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				val[i][j] = m.val[i][j];
			}
		}
	}

	//4x4 matrix multiplication
	inline Mat4x4f operator *(const Mat4x4f& m) const
	{
		Mat4x4f out;

		//just gonna hard code this
		out.val[0][0] = val[0][0] * m.val[0][0] + val[0][1] * m.val[1][0] + val[0][2] * m.val[2][0] + val[0][3] * m.val[3][0];
		out.val[0][1] = val[0][0] * m.val[0][1] + val[0][1] * m.val[1][1] + val[0][2] * m.val[2][1] + val[0][3] * m.val[3][1];
		out.val[0][2] = val[0][0] * m.val[0][2] + val[0][1] * m.val[1][2] + val[0][2] * m.val[2][2] + val[0][3] * m.val[3][2];
		out.val[0][3] = val[0][0] * m.val[0][3] + val[0][1] * m.val[1][3] + val[0][2] * m.val[2][3] + val[0][3] * m.val[3][3];

		out.val[1][0] = val[1][0] * m.val[0][0] + val[1][1] * m.val[1][0] + val[1][2] * m.val[2][0] + val[1][3] * m.val[3][0];
		out.val[1][1] = val[1][0] * m.val[0][1] + val[1][1] * m.val[1][1] + val[1][2] * m.val[2][1] + val[1][3] * m.val[3][1];
		out.val[1][2] = val[1][0] * m.val[0][2] + val[1][1] * m.val[1][2] + val[1][2] * m.val[2][2] + val[1][3] * m.val[3][2];
		out.val[1][3] = val[1][0] * m.val[0][3] + val[1][1] * m.val[1][3] + val[1][2] * m.val[2][3] + val[1][3] * m.val[3][3];

		out.val[2][0] = val[2][0] * m.val[0][0] + val[2][1] * m.val[1][0] + val[2][2] * m.val[2][0] + val[2][3] * m.val[3][0];
		out.val[2][1] = val[2][0] * m.val[0][1] + val[2][1] * m.val[1][1] + val[2][2] * m.val[2][1] + val[2][3] * m.val[3][1];
		out.val[2][2] = val[2][0] * m.val[0][2] + val[2][1] * m.val[1][2] + val[2][2] * m.val[2][2] + val[2][3] * m.val[3][2];
		out.val[2][3] = val[2][0] * m.val[0][3] + val[2][1] * m.val[1][3] + val[2][2] * m.val[2][3] + val[2][3] * m.val[3][3];

		out.val[3][0] = val[3][0] * m.val[0][0] + val[3][1] * m.val[1][0] + val[3][2] * m.val[2][0] + val[3][3] * m.val[3][0];
		out.val[3][1] = val[3][0] * m.val[0][1] + val[3][1] * m.val[1][1] + val[3][2] * m.val[2][1] + val[3][3] * m.val[3][1];
		out.val[3][2] = val[3][0] * m.val[0][2] + val[3][1] * m.val[1][2] + val[3][2] * m.val[2][2] + val[3][3] * m.val[3][2];
		out.val[3][3] = val[3][0] * m.val[0][3] + val[3][1] * m.val[1][3] + val[3][2] * m.val[2][3] + val[3][3] * m.val[3][3];

		return out;
	}

	//matrix - vector multiplication for vertex transformations
	//vectors are treated as column vectors i.e. each component is in a different row of the same column (4x1) matrix
	inline Vec4f operator *(const Vec4f& v) const
	{ 
		return Vec4f(val[0][0] * v.x + val[0][1] * v.y + val[0][2] * v.z + val[0][3] * v.w,
			val[1][0] * v.x + val[1][1] * v.y + val[1][2] * v.z + val[1][3] * v.w,
			val[2][0] * v.x + val[2][1] * v.y + val[2][2] * v.z + val[2][3] * v.w,
			val[3][0] * v.x + val[3][1] * v.y + val[3][2] * v.z + val[3][3] * v.w);
	}
};

struct Triangle
{
	union {
		struct { Vec3f A, B, C; };
		Vec3f raw[3];
	};
	

	Triangle() { A = Vec3f(); B = Vec3f(); C = Vec3f(); }
	Triangle(Vec3f _A, Vec3f _B, Vec3f _C) { A = _A; B = _B, C = _C; }
	//copy
	Triangle(const Triangle& t) { A = Vec3f(t.A); B = Vec3f(t.B); C = Vec3f(t.C); }
};
