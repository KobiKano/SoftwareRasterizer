#pragma once

#include "geom.h"
#include "model.h"
#include "quaternion.h"
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
	void roll_left();
	void roll_right();
	void raise();
	void lower();
	void zoom_in(float step);
	void zoom_out(float step);
	void left(float step);
	void right(float step);
	void up(float step);
	void down(float step);
	void rot_left(float step);
	void rot_right(float step);
	void rot_up(float step);
	void rot_down(float step);
	void roll_left(float step);
	void roll_right(float step);
	void raise(float step);
	void lower(float step);
	Mat4x4f gen_vert_mat();
	Mat4x4f gen_norm_mat();
private:
	float step;
	Vec3f pos;
	Vec3f dir;
	Vec3f v_up;
	Vec3f v_right;
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
	void process_inputs();
	void set_cam_step(float step);
	void set_pos(int index, Vec3f &center);
	void add_pitch(int index, float rads);
	void add_roll(int index, float rads);
	void add_yaw(int index, float rads);
	void set_rot_order(int index, int order[3]);
	void set_scale(int index, float scale);
	void set_projection(float fov_rad, float zfar, float znear, float aspect_r);
	void set_aspect_ratio(float aspect_r);
	void set_z_bound(float zfar, float znear);
	void set_fov(float fov_rad);
	void set_wireframe(bool b);
	void set_cam_light(bool b);
	int add_light(Vec3f &p);
private:
	struct ProjMat
	{
		Mat4x4f mat;
		float fov_rad, zfar, znear, aspect_r, f, q;

		ProjMat() { mat = Mat4x4f(); fov_rad = 0; zfar = 0; znear = 0; aspect_r = 0; f = 0; q = 0; }
		ProjMat(float fov_rad, float zfar, float znear, float aspect_r);  //defined in scene.cpp
	};
	std::vector<std::shared_ptr<Model>> models;
	std::vector<Vec3f> lights;
	std::vector<Mat4x4f> translates;
	std::vector<Mat4x4f> scales;
	std::vector<Rotation> rotates;  //[0] is x, [1] is y [2] is z
	ProjMat proj_mat;
	Camera cam;
	bool wireframe;
	bool cam_light;

	void cull(std::vector<Vec3f>& f_norms, std::vector<Triangle>& t_draws, std::vector<Triangle>& t_norms);
	void rotate(Vec3f& old, int i);
	void projection(std::vector<Triangle> &t_draws);
	void triangle_to_screen(Triangle &t_draw, Triangle &t_norm, Triangle& t_world, std::vector<Vec3f> lights, COLOR color) const;
};