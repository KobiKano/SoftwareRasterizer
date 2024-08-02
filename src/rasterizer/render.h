#pragma once

#include "geom.h"
#include "model.h"
#include <vector>

//very self explanitory camera class
class Camera
{
public:
	Camera();
	Camera(float step);
	void set_step(float step);
	Vec3f get_pos() const;
	Vec3f get_dir() const;
	void zoom_in();
	void zoom_out();
	void left();
	void right();
	void up();
	void down();
	void rot_left();
	void rot_right();
	void rot_up();
	void rot_down();
private:
	float step;
	Vec3f pos;
	Vec3f dir;
	Vec3f v_up;
};

//scene class to hold all objects needed to render a scene
//i.e models, lights, and camera
//also defines perspective projection and translation for models
class Scene
{
public:
	Scene();
	~Scene();
	int reg_model(std::shared_ptr<Model> m);
	int reg_model(std::shared_ptr<Model> m, Vec3f &center, float scale);
	void draw();
	void set_pos(int index, Vec3f &center);
	void rot_left(int index, float rads);
	void rot_right(int index, float rads);
	void rot_up(int index, float rads);
	void rot_down(int index, float rads);
	void set_scale(int index, float scale);
	void set_projection(float fov_rad, float zfar, float znear, float aspect_r);
	void set_aspect_ratio(float aspect_r);
	void set_z_bound(float zfar, float znear);
	void set_fov(float fov_rad);
	void set_wireframe(bool b);
	int add_light(Vec3f &p);
private:
	struct ProjMat
	{
		Mat4x4f mat;
		float fov_rad, zfar, znear, aspect_r, f, q;

		ProjMat() { mat = Mat4x4f(); fov_rad = 0; zfar = 0; znear = 0; aspect_r = 0; }
		ProjMat(float fov_rad, float zfar, float znear, float aspect_r);  //defined in scene.cpp
	};
	std::vector<std::shared_ptr<Model>> models;
	std::vector<Vec3f> lights;
	std::vector<Mat4x4f> translates;
	std::vector<Mat4x4f> scales;
	ProjMat proj_mat;
	Camera cam;
	bool wireframe;

	bool cullable(int model_i, int face_i);
	void translate(Vec3f &old, int i);
	void scale(Vec3f &old, int i);
	void projection(Vec3f &old);
	void triangle_to_screen(Triangle &t_draw, Triangle &t_norm, PIXEL color);
};