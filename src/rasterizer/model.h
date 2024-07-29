#pragma once

#include <vector>
#include <cmath>
#include <stdio.h>
#include <string>
#include <memory>
#include "geom.h"

constexpr float PI = 3.14159265358979323846;

class Model
{
private:
	std::vector<Vec3f> vertices;
	std::vector<Vec3f> textures;
	std::vector<Vec3f> vert_normals;
	std::vector<Vec3f> face_normals;
	std::vector<std::vector<Vec3i>> faces;
	void normalize_verts(float largest, float smallest);
	void add_normals();
	void process_faces();
	bool is_valid_ear(Triangle t, int i, int a, int b, int c, Vec3f center);
	
public:
	Model(const char* filepath);
	~Model();
	std::vector<Vec3f>& get_vertices();
	std::vector<Vec3f>& get_textures();
	std::vector<Vec3f>& get_vert_normals();
	std::vector<Vec3f>& get_face_normals();
	std::vector<std::vector<Vec3i>>& get_faces();
};