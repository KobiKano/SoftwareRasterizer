#pragma once

#include "geom.h"
#include "model.h"
#include <vector>

//very self explanitory camera class
typedef class Camera
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
typedef class Scene
{
public:
	Scene();
	void reg_model(std::unique_ptr<Model> m);
	void draw();
	void set_perspective(); //TODO finish
	void add_light(Vec3f p);
private:
	std::vector<std::unique_ptr<Model>> models;
	std::vector<Vec3f> lights;
	Mat4x4f translate;
	Mat4x4f scale;
	Mat4x4f perspective;
	Camera cam;

	bool cullable(const std::vector<Vec3i> &old);
	std::vector<Vec3f> translate(const Vec3f &old);
	std::vector<Vec3f> scale(const Vec3f &old);
	std::vector<Vec3f> perspective(const Vec3f &old);
};