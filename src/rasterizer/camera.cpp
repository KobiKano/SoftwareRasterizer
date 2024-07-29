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
	this->step = step;
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
	pos = pos + (dir.cross(v_up)*step);
}
void Camera::right()
{
	pos = pos - (dir.cross(v_up)*step);
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
	dir.x = dir.x - step;
}
void Camera::rot_right()
{
	dir.x = dir.x + step;
}
void Camera::rot_up()
{
	dir.y = dir.y + step;
}
void Camera::rot_down()
{
	dir.y = dir.y - step;
}

