#include "render.h"
#include "geom.h"

Camera::Camera()
{
	//initialize camera to (0,0, -1) pointed to origin
	step = 0.1f;
	pos = Vec3f(0.f, 0.f, -1.f);
	dir = Vec3f(0.f, 0.f, 1.f);
	v_up = Vec3f(0.f, 1.f, 0.f);
}
Camera::Camera(float step)
{
	//initialize camera to (0,0, -1) pointed to origin
	this->step = step;
	pos = Vec3f(0.f, 0.f, -1.f);
	dir = Vec3f(0.f, 0.f, 1.f);
	v_up = Vec3f(0.f, 1.f, 0.f);
}

void Camera::set_step(float step)
{
	step = abs(step);
	this->step = (step > 1.0f) ? 1.0f : step;
}
Vec3f Camera::get_pos() const
{
	return pos;
}
Vec3f Camera::get_dir() const
{
	return dir;
}
void Camera::zoom_in()
{
	pos = pos + (dir*step);
}
void Camera::zoom_out()
{
	pos = pos - (dir * step);
}
void Camera::left()
{
	pos = pos - (dir.cross(v_up)*step);
}
void Camera::right()
{
	pos = pos + (dir.cross(v_up)*step);
}
void Camera::up()
{
	pos = pos + (v_up * step);
}
void Camera::down()
{
	pos = pos - (v_up * step);
}
void Camera::rot_left()
{
	//transform using pitch matrix
	dir.x = dir.x * cos(step) + dir.z * sin(step);
	dir.z = dir.x * -sin(step) + dir.z * cos(step);

	v_up.x = v_up.x * cos(step) + v_up.z * sin(step);
	v_up.z = v_up.x * -sin(step) + v_up.z * cos(step);
}
void Camera::rot_right()
{
	//transform using pitch matrix
	dir.x = dir.x * cos(-step) + dir.z * sin(-step);
	dir.z = dir.x * -sin(-step) + dir.z * cos(-step);

	v_up.x = v_up.x * cos(-step) + v_up.z * sin(-step);
	v_up.z = v_up.x * -sin(-step) + v_up.z * cos(-step);
}
void Camera::rot_up()
{
	//transform using roll matrix
	dir.y = dir.y * cos(step) + dir.z * -sin(step);
	dir.z = dir.y * sin(step) + dir.z * cos(step);

	v_up.y = v_up.y * cos(step) + v_up.z * -sin(step);
	v_up.z = v_up.y * sin(step) + v_up.z * cos(step);
}
void Camera::rot_down()
{
	//transform using roll matrix
	dir.y = dir.y * cos(-step) + dir.z * -sin(-step);
	dir.z = dir.y * sin(-step) + dir.z * cos(-step);

	v_up.y = v_up.y * cos(-step) + v_up.z * -sin(-step);
	v_up.z = v_up.y * sin(-step) + v_up.z * cos(-step);
}

