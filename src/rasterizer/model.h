#pragma once

#include <vector>
#include <cmath>
#include <stdio.h>
#include <string>
#include <memory>

constexpr float PI = 3.14159265358979323846;

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

	//operator overloads
	inline Vec3<t> operator +(const Vec3<t> &v) const { return Vec3<t>(x + v.x, y + v.y, z + v.z); } //add
	inline Vec3<t> operator -(const Vec3<t> &v) const { return Vec3<t>(x - v.x, y - v.y, z - v.z); } //subtract
	inline Vec3<t> operator -(float f) const { return Vec3<t>(x - f, y - f, z - f); } //const subtract
	inline Vec3<t> operator *(float f)          const { return Vec3<t>(x * f, y * f, z * f); } //const mult
	inline Vec3<t> operator *(const Vec3<t> &v) const { return Vec3<t>(x * v.x, y * v.y, z * v.z); } //vector mult
	inline Vec3<t> operator /(float f)          const { return Vec3<t>(x / f, y / f, z / f); } //const div

	//vector operations
	inline Vec3<t> cross(const Vec3<t>& v) const { return Vec3<t>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); } //cross product
	inline t dot(const Vec3<t>& v) const { return (x * v.x + y * v.y + z * v.z); } //dot product
	float value() const { return sqrt(x * x + y * y + z * z); }
	Vec3<t>& norm() const { return *this / value(); }

	//to_string
	std::string to_string() { return "0: " + std::to_string(x) + "\n1: " + std::to_string(y) + "\n2: " + std::to_string(z) + "\n"; }
};

typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;

typedef struct Triangle
{
	Vec3f A;
	Vec3f B;
	Vec3f C;

	Triangle() { A = Vec3f(); B = Vec3f(); C = Vec3f(); }
	Triangle(Vec3f _A, Vec3f _B, Vec3f _C) { A = _A; B = _B, C = _C; }
};

class Model
{
private:
	std::vector<Vec3f> vertices;
	std::vector<Vec3f> textures;
	std::vector<Vec3f> normals;
	std::vector<std::vector<Vec3i>> faces;
	inline Vec3f world_to_screen(Vec3f in, int width, int height);
	void normalize_verts(float largest, float smallest);
	void process_faces();
	bool is_valid_ear(Triangle t, int i, int a, int b, int c, Vec3f center);
	
public:
	Model(const char* filepath);
	~Model();
	void draw();
};