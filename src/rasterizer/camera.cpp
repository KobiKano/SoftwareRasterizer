#include "render.h"
#include "quaternion.h"
#include "geom.h"

Camera::Camera()
{
	//initialize camera to (0,0, -1) pointed to origin
	step = 0.01f;
	pos = Vec3f(0.f, 0.f, -1.f);
	dir = Vec3f(0.f, 0.f, 1.f);
	v_up = Vec3f(0.f, 1.f, 0.f);
	v_right = Vec3f(1.f, 0.f, 0.f);
}
Camera::Camera(float step)
{
	//initialize camera to (0,0, -1) pointed to origin
	this->step = step;
	pos = Vec3f(0.f, 0.f, -1.f);
	dir = Vec3f(0.f, 0.f, 1.f);
	v_up = Vec3f(0.f, 1.f, 0.f);
	v_right = Vec3f(1.f, 0.f, 0.f);
}

/**
* Remember vectors are represented as column matrices
* Transforms point in word space to be in position as if camera defines cartesian axes
* Overall matrix is the sum of the camera translation matrix (which moves a point in space such that the origin of the coordinate system is the position of the camera)
*			and the camera projection matrix (which rotates a point in space such that the axis of the camera line up with the cartesian axis)
* @return: inverted point at matrix
*/
Mat4x4f Camera::gen_mat()
{
	//lookat rotation
	 float r[4][4] = 
	   {{v_right.x, v_right.y, v_right.z, -pos.dot(v_right)},
		{v_up.x, v_up.y, v_up.z,  -pos.dot(v_up)},
		{dir.x, dir.y, dir.z,  -pos.dot(dir)},
		{0.f, 0.f, 0.f, 1.f}};

	 //return matrix product of matrices for full transformation
	 return Mat4x4f(r);
}

/**
* Helper function to transform an angle over an axis by a specific angle
* @param old: vector to modify
* @param axis: axis of rotation
* @param angle: angle in rads to change by
*/
static void quaternion_mult(Vec3f& old, Vec3f &axis, float angle)
{
	Quaternion q = Quaternion(angle, axis);
	Quaternion q_neg = q.conjugate();
	old = (Vec3f)(q * Quaternion(old) * q_neg);
}

/**
* Aligns up and right vectors with forward direction in case of floating point error
* @param dir: direction of camera
* @param v_up: up direction of camera
* @param v_right: right direction of camera
*/
static void force_align(Vec3f &dir, Vec3f &v_up, Vec3f &v_right)
{
	if (dir.dot(v_up) != 0.f || dir.dot(v_right) != 0.f || v_up.dot(v_right) != 0.f)
	{
		//force align up and right to dir
		//up value based on how much up maps to dir
		v_up = v_up - (dir * v_up.dot(dir));
		//right is just cross product of up and dir
		v_right = v_up.cross(dir);
	}
}


/******************************************************************************************
* Not doc commenting rest of the file cause I am lazy, and everything is self explanitory *
******************************************************************************************/
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
	pos = pos - (v_right*step);
}
void Camera::right()
{
	pos = pos + (v_right*step);
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
	quaternion_mult(dir, v_up, -step);
	quaternion_mult(v_right, v_up, -step);
	force_align(dir, v_up, v_right);
}
void Camera::rot_right()
{
	quaternion_mult(dir, v_up, step);
	quaternion_mult(v_right, v_up, step);
	force_align(dir, v_up, v_right);
}
void Camera::rot_up()
{
	quaternion_mult(dir, v_right, -step);
	quaternion_mult(v_up, v_right, -step);
	force_align(dir, v_up, v_right);
}
void Camera::rot_down()
{
	quaternion_mult(dir, v_right, step);
	quaternion_mult(v_up, v_right, step);
	force_align(dir, v_up, v_right);
}
void Camera::roll_left()
{
	quaternion_mult(v_right, dir, -step);
	quaternion_mult(v_up, dir, -step);
	force_align(dir, v_up, v_right);
}
void Camera::roll_right()
{
	quaternion_mult(v_right, dir, step);
	quaternion_mult(v_up, dir, step);
	force_align(dir, v_up, v_right);
}
/**********************************************/
void Camera::zoom_in(float step)
{
	pos = pos + (dir * step);
}
void Camera::zoom_out(float step)
{
	pos = pos - (dir * step);
}
void Camera::left(float step)
{
	pos = pos - (v_right * step);
}
void Camera::right(float step)
{
	pos = pos + (v_right * step);
}
void Camera::up(float step)
{
	pos = pos + (v_up * step);
}
void Camera::down(float step)
{
	pos = pos - (v_up * step);
}
void Camera::rot_left(float step)
{
	quaternion_mult(dir, Vec3f(0.f, 1.f, 0.f), -step);
	quaternion_mult(v_right, Vec3f(0.f, 1.f, 0.f), -step);
	force_align(dir, v_up, v_right);
}
void Camera::rot_right(float step)
{
	quaternion_mult(dir, Vec3f(0.f, 1.f, 0.f), step);
	quaternion_mult(v_right, Vec3f(0.f, 1.f, 0.f), step);
	force_align(dir, v_up, v_right);
}
void Camera::rot_up(float step)
{
	quaternion_mult(dir, Vec3f(1.f, 0.f, 0.f), step);
	quaternion_mult(v_up, Vec3f(1.f, 0.f, 0.f), step);
	force_align(dir, v_up, v_right);
}
void Camera::rot_down(float step)
{
	quaternion_mult(dir, Vec3f(1.f, 0.f, 0.f), -step);
	quaternion_mult(v_up, Vec3f(1.f, 0.f, 0.f), -step);
	force_align(dir, v_up, v_right);
}
void Camera::roll_left(float step)
{
	quaternion_mult(v_right, dir, -step);
	quaternion_mult(v_up, dir, -step);
	force_align(dir, v_up, v_right);
}
void Camera::roll_right(float step)
{
	quaternion_mult(v_right, dir, step);
	quaternion_mult(v_up, dir, step);
	force_align(dir, v_up, v_right);
}

