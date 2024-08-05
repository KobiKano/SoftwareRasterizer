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

/**
* Constructor for scene object
*/
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
/**
* Adds model in scene
* @param m: model to add
* @return: index of model
*/
int Scene::reg_model(std::shared_ptr<Model> m)
{
	models.push_back(m);
	//default to center at (0,0,0) and scale of 1.0
	translates.push_back(Mat4x4f());
	scales.push_back(Mat4x4f());

	return (int)models.size() - 1;
}
/**
* Adds model in scene
* @param m: model to add
* @param center: position of model
* @param scale: scale of model
* @return: index of model
*/
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
/**
* Sets projection matrix
* @param fov_rad: angle of field of view
* @param zfar: position of far plane
* @param znear: position of near plane
* @param aspect_r: aspect ratio to scale points to
*/
void Scene::set_projection(float fov_rad, float zfar, float znear, float aspect_r)
{
	proj_mat = ProjMat(fov_rad, zfar, znear, aspect_r);
}
/**
* Sets aspect ration of perspective transform
* @param aspect_r: aspect ratio to scale points to
*/
void Scene::set_aspect_ratio(float aspect_r)
{
	//clear current val
	proj_mat.mat.val[0][0] = proj_mat.f * aspect_r;
	proj_mat.aspect_r = aspect_r;
}
/**
* Sets z bounds of perspective transform
* @param zfar: position of far plane
* @param znear: position of near plane
*/
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
/**
* Sets fov of perspective transform
* @param fov_rad: angle of field of view
*/
void Scene::set_fov(float fov_rad)
{
	float f = 1 / tan(fov_rad / 2);
	//set new val
	proj_mat.mat.val[0][0] = proj_mat.aspect_r * f;
	proj_mat.mat.val[1][1] = f;
	proj_mat.fov_rad = fov_rad;
	proj_mat.f = f;
}
/**
* Sets wireframe mode
* @param b: bool value to set
*/
void Scene::set_wireframe(bool b)
{
	wireframe = b;
}
/**
* Rotates model left
* @param index: index of model
* @param rads: angle to rotate in radians
*/
void Scene::rot_left(int index, float rads)
{

}
/**
* Rotates model right
* @param index: index of model
* @param rads: angle to rotate in radians
*/
void Scene::rot_right(int index, float rads)
{

}
/**
* Rotates model up
* @param index: index of model
* @param rads: angle to rotate in radians
*/
void Scene::rot_up(int index, float rads)
{

}
/**
* Rotates model down
* @param index: index of model
* @param rads: angle to rotate in radians
*/
void Scene::rot_down(int index, float rads)
{

}
/**
* Sets position of model
* @param index: index of model
* @param center: center of model (assumes vertices in model centered around (0,0,0)
*/
void Scene::set_pos(int index, Vec3f &center)
{
	translates[index].val[0][3] = center.x;
	translates[index].val[1][3] = center.y;
	translates[index].val[2][3] = center.z;
}
/**
* Sets the scale of a specific model
* @param index: index of model
* @param scale: scale of model on all axes
*/
void Scene::set_scale(int index, float scale)
{
	scales[index].val[0][0] = scale;
	scales[index].val[1][1] = scale;
	scales[index].val[2][2] = scale;
}
/**
* Adds light to scene
* @param p: position of light source
* @return: index of light in storage vector
*/
int Scene::add_light(Vec3f &p)
{
	lights.push_back(p);
	return (int)lights.size() - 1;
}
/**
* Draws all models to the screen
*/
void Scene::draw()
{
	//iterate through each model
	for (int i = 0; i < models.size(); i++)
	{
		//get faces, vertices, and normals
		std::vector<std::vector<Vec3i>> faces = models[i].get()->get_faces();
		std::vector<Vec3f> vertices = models[i].get()->get_vertices();
		std::vector<Vec3f> normals = models[i].get()->get_vert_normals();
		std::vector<Triangle> t_draws;
		std::vector<Triangle> t_norms;
		std::vector<int> face_i;
		for (int j = 0; j < faces.size(); j++)
		{
			//translate vertices
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
				//translate based on camera pos
				t_draw.raw[k] = vertex;
				t_norm.raw[k] = normals[faces[j][k].i_norm];
			}

			//otherwise add triangles to draw list
			t_draws.push_back(t_draw);
			t_norms.push_back(t_norm);
			face_i.push_back(j);
		}

		//clip over z bounds
		clip_z(t_draws, t_norms, face_i);

		//project triangle to screen coords
		projection(t_draws);

		//check if face can be culled (face is facing away from viewpoint)
		cull(i, face_i, t_draws, t_norms);

		//clip over x and y bounds
		clip_xy(t_draws, t_norms);


		//draw all triangles
		if (t_draws.size() != t_norms.size())
		{
			log(ERR, "Error occured drawing model... Incorrect triangle or vertex normal count... skipping");
			continue;
		}
		for (int j = 0; j < t_draws.size(); j++)
			triangle_to_screen(t_draws[j], t_norms[j], models[i].get()->get_color());
	}
}

/********************************************************************
* Private Functions
********************************************************************/
/**
* Check if face can be culled
* If the face a facing completely away from the camera, then we ignore it
* Otherwise proceed with draw
* @param model_i: index of model
* @param face_i: index of face
* @param face: face to check
* @param vertices: vertices of model
* @return true if we can discard face, false otherwise
*/
void Scene::cull(int model_i, std::vector<int> &face_i, std::vector<Triangle> &t_draws, std::vector<Triangle> &t_norms)
{
	//get the face normal to check
	std::vector<Triangle> new_draws;
	std::vector<Triangle> new_norms;
	for (int i = 0; i < t_draws.size(); i++)
	{
		Vec3f face_n = models[model_i].get()->get_face_normals()[face_i[i]];

		//determine the vector pointing from the camera to this face
		//get center of face
		Vec3f center;
		for (int j = 0; j < 3; j++)
		{
			center = center + t_draws[i].raw[j];
		}
		center = center / 3.f;
		Vec3f face_to_cam = cam.get_pos() - center;

		//if the dot product between the camera and normal is less than 90 degrees, then we can draw
		if (face_to_cam.norm().dot(face_n) > 0.f)
		{
			new_draws.push_back(t_draws[i]);
			new_norms.push_back(t_norms[i]);
		}

		//else we don't need to draw since face can't be seen
	}
	//redirect t_draws and t_norms to new vectors
	t_draws.clear();
	t_norms.clear();
	t_draws = new_draws;
	t_norms = new_draws;
}
/**
* Translates input vertex
* @param old: vertex data to modify
* @param i: index of transform matrix
*/
void Scene::translate(Vec3f &old, int i)
{
	Mat4x4f t = translates[i];
	Vec4f v = Vec4f(old);
	old = (Vec3f)(t * v);
}
/**
* Scales input vertex
* @param old: vertex data to modify
* @param i: index of scale matrix
*/
void Scene::scale(Vec3f &old, int i)
{
	Mat4x4f t = scales[i];
	Vec4f v = Vec4f(old);
	old = (Vec3f)(t * v);
}
/**
* Clips triangle over z bounds of perspective box if part of triangle outside of box
* @param t_draws: vector of triangles to draw
* @param t_norms: vector of vertex normals for each triangle
*/
void Scene::clip_z(std::vector<Triangle>& t_draws, std::vector<Triangle>& t_norms, std::vector<int>& face_i)
{

}
/**
* Clips triangle over x and y bounds of perspective box if part of triangle outside of box
* @param t_draws: vector of triangles to draw
* @param t_norms: vector of vertex normals for each triangle
*/
void Scene::clip_xy(std::vector<Triangle>& t_draws, std::vector<Triangle>& t_norms)
{

}
/**
* Projects input vertices to screen space(adds for depth)
* @param old: vertex data to modify
*/
void Scene::projection(std::vector<Triangle>& t_draws)
{
	for (int i = 0; i < t_draws.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			Vec4f v = Vec4f(t_draws[i].raw[j]);
			v = proj_mat.mat * v;
			t_draws[i].raw[j] = (Vec3f)v;
			if (v.w != 0.f)
				t_draws[i].raw[j] = t_draws[i].raw[j] / v.w;
		}
	}
}
/**
* Draws a triangle to the screen
* Assumes vertices are normalized between [-1, 1]
* Uses wireframe setting to draw in wireframe or solid block
* @param t_draw: triangle to draw
* @param t_norm: normal vectors of vertices in triangle
* @param color: color to use in draw
*/
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
		draw_triangle(x0, y0, t_draw.A.z, x1, y1, t_draw.B.z, x2, y2, t_draw.C.z, color);
	}
	//else fill triangle using vertex normals to determine lighting
	else
	{
		//TODO Complete
	}
}