#include "funcs.h"
#include "geom.h"
#include "model.h"
#include "render.h"
#include "../window/window.h"
#include "../logger/logger.h"

/**
* Projection matrix for this renderer
* Idea is that as an object gets further from the camera, the more distorted it gets
* That distortion is based on the angle of the fov in radians
* If the depth of a point is too close or too far, they will end up ouside of the [0, 1] range and therefore won't be drawn when
*			passed to the pixel drawing function
* @param fov_rad: angle of fov in radians default 90
* @param zfar: z coordinate of far clip plane default 10
* @param znear z coordinate of near clip plane default 1
* @param aspect_r ratio of screen height and width
*/
Scene::ProjMat::ProjMat(float fov_rad, float zfar, float znear, float aspect_r)
{
	//set locals
	this->fov_rad = fov_rad;
	this->zfar = zfar;
	this->znear = znear;
	this->aspect_r = aspect_r;

	//initialize proj mat
	mat = Mat4x4f();
	float f = 1 / tan(fov_rad / 2); //scale factor for x and y axis (as angle increases, points get shifted further)
	float q = zfar / (zfar - znear);  //normalized view space
	this->f = f;
	this->q = q;
	mat.val[0][0] = aspect_r * f;
	mat.val[1][1] = f;
	mat.val[2][2] = q;
	mat.val[2][3] = -znear * q; //offset to make z values too close to camera less than zero
	mat.val[3][2] = 1; //isolate z value in w ouput for normalization
	mat.val[3][3] = 0;
}

Scene::Scene()
{
	//initialize camera with 0.1 step speed
	cam = Camera(0.1f);

	//initialize projection matrix with default vals
	float w = (float)get_buf_width();
	float h = (float)get_buf_height();
	this->set_projection(PI / 2, 10.f, 1.f, h / w);

	//default to wireframe
	wireframe = true;
}
Scene::~Scene()
{
	log(DEBUG1, "ending scene");
}
int Scene::reg_model(std::shared_ptr<Model> m)
{
	models.push_back(m);
	//default to center at (0,0,0) and scale of 1.0
	translates.push_back(Mat4x4f());
	scales.push_back(Mat4x4f());

	return (int)models.size() - 1;
}
int Scene::reg_model(std::shared_ptr<Model> m, Vec3f &center, float scale)
{
	models.push_back(m);
	//init with defaults
	translates.push_back(Mat4x4f());
	scales.push_back(Mat4x4f());

	//use build in functions to set matrix vals
	int i = (int)models.size() - 1;
	this->set_pos(i, center);
	this->set_scale(i, scale);

	return i;
}
void Scene::set_projection(float fov_rad, float zfar, float znear, float aspect_r)
{
	proj_mat = ProjMat(fov_rad, zfar, znear, aspect_r);
}
void Scene::set_aspect_ratio(float aspect_r)
{
	//clear current val
	proj_mat.mat.val[0][0] = proj_mat.f * aspect_r;
	proj_mat.aspect_r = aspect_r;
}
void Scene::set_z_bound(float zfar, float znear)
{
	//set new val
	float q = zfar / (zfar - znear);
	proj_mat.mat.val[2][2] = q;
	proj_mat.mat.val[2][3] = -znear * q;
	proj_mat.q = q;
	proj_mat.zfar = zfar;
	proj_mat.znear = znear;
}
void Scene::set_fov(float fov_rad)
{
	float f = 1 / tan(fov_rad / 2);
	//set new val
	proj_mat.mat.val[0][0] = proj_mat.aspect_r * f;
	proj_mat.mat.val[1][1] = f;
	proj_mat.fov_rad = fov_rad;
	proj_mat.f = f;
}
void Scene::set_wireframe(bool b)
{
	wireframe = b;
}
void Scene::rot_left(int index, float rads)
{

}
void Scene::rot_right(int index, float rads)
{

}
void Scene::rot_up(int index, float rads)
{

}
void Scene::rot_down(int index, float rads)
{

}
void Scene::set_pos(int index, Vec3f &center)
{
	translates[index].val[0][3] = center.x;
	translates[index].val[1][3] = center.y;
	translates[index].val[2][3] = center.z;
}
void Scene::set_scale(int index, float scale)
{
	scales[index].val[0][0] = scale;
	scales[index].val[1][1] = scale;
	scales[index].val[2][2] = scale;
}
int Scene::add_light(Vec3f &p)
{
	lights.push_back(p);
	return (int)lights.size() - 1;
}
void Scene::draw()
{
	//iterate through each model
	for (int i = 0; i < models.size(); i++)
	{
		//get faces, vertices, and normals
		std::vector<std::vector<Vec3i>> faces = models[i].get()->get_faces();
		std::vector<Vec3f> vertices = models[i].get()->get_vertices();
		std::vector<Vec3f> normals = models[i].get()->get_vert_normals();
		for (int j = 0; j < faces.size(); j++)
		{
			//check if face can be culled (face is facing away from viewpoint)
			if (this->cullable(i, j))
				continue;

			//else translate vertices
			Triangle t_draw;
			Triangle t_norm;
			for (int k = 0; k < 3; k++)
			{
				//all vertices should be within [-1, 1] range on all axis
				Vec3f vertex = vertices[faces[j][k].i_vert];
				//translate to world coords
				translate(vertex, i);
				//scale accordingly
				scale(vertex, i);
				//finally determine projection
				projection(vertex);
				t_draw.raw[k] = vertex;
				t_norm.raw[k] = normals[faces[j][k].i_norm];
			}

			//draw triangles
			triangle_to_screen(t_draw, t_norm, models[i].get()->get_color());
		}
	}
}

/********************************************************************
* Private Functions
********************************************************************/
bool Scene::cullable(int model_i, int face_i)
{
	//TODO
	return false;
}
void Scene::translate(Vec3f &old, int i)
{
	Mat4x4f t = translates[i];
	Vec4f v = Vec4f(old);
	old = (Vec3f)(t * v);
}
void Scene::scale(Vec3f &old, int i)
{
	Mat4x4f t = scales[i];
	Vec4f v = Vec4f(old);
	old = (Vec3f)(t * v);
}
void Scene::projection(Vec3f &old)
{
	Vec4f v = Vec4f(old);
	v = proj_mat.mat * v;
	old = (Vec3f)v;
	if (v.w != 0.f)
		old = old / v.w;
}
void Scene::triangle_to_screen(Triangle &t_draw, Triangle &t_norm, PIXEL color)
{
	//assume z values to be between 0 and 1, values outside of range will just not be drawn
	//assume x and y values in range [-1, 1]

	//normalize triangle coordinates to fit to screen
	t_draw.A.x += 1.f;
	t_draw.B.x += 1.f;
	t_draw.C.x += 1.f;
	t_draw.A.y += 1.f;
	t_draw.B.y += 1.f;
	t_draw.C.y += 1.f;

	float w = (float)get_buf_width();
	float h = (float)get_buf_height();
	int x0, x1, x2, y0, y1, y2;

	x0 = (int)(t_draw.A.x * w / 2.f);
	x1 = (int)(t_draw.B.x * w / 2.f);
	x2 = (int)(t_draw.C.x * w / 2.f);
	y0 = (int)(t_draw.A.y * h / 2.f);
	y1 = (int)(t_draw.B.y * h / 2.f);
	y2 = (int)(t_draw.C.y * h / 2.f);

	/////////////////////////////////////////////////////////////////////
	//actual drawing of triangles
	//check if wireframe
	if (wireframe)
	{
		draw_triangle(x0, y0, t_draw.A.z, x1, y2, t_draw.B.z, x2, y2, t_draw.C.z, color);
	}
	//else fill triangle using vertex normals to determine lighting
	else
	{
		//TODO Complete
	}
}