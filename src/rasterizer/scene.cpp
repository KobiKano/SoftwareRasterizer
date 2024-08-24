#include "proc.h"
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

	//default to wireframe and cam_light
	wireframe = true;
	cam_light = true;
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
	//default to no rotation and x-y-z rot order
	rotates.push_back(Rotation());

	return (int)models.size() - 1;
}
/**
* Adds model in scene
* @param m: model to add
* @param center: position of model
* @param scale: scale of model
* @return: index of model
*/
int Scene::reg_model(std::shared_ptr<Model> m, Vec3f &center, float scale, COLOR color)
{
	models.push_back(m);
	//init with defaults
	translates.push_back(Mat4x4f());
	scales.push_back(Mat4x4f());
	rotates.push_back(Rotation());

	//use build in functions to set matrix vals
	int i = (int)models.size() - 1;
	this->set_pos(i, center);
	this->set_scale(i, scale);
	this->set_color(i, color);

	return i;
}

/**
* Gets number of models
* @return number of models in scene
*/
int Scene::num_models()
{
	return models.size();
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
* Sets cam_light mode
* @param b: bool value to set
*/
void Scene::set_cam_light(bool b)
{
	cam_light = b;
}
/**
* Sets color of model at index
* @param index: index of model
* @param color: color to set
*/
void Scene::set_color(int index, COLOR color)
{
	models[index].get()->set_color(color);
}
/**
* Adds pitch to object rotation
* @param index: index of model
* @param rads: rads to change (can be negative)
*/
void Scene::add_pitch(int index, float rads)
{
	Quaternion q = rotates[index].x; //x-axis
	float curr_rads = q.get_angle();

	//do operations
	q = Quaternion(curr_rads + rads, Vec3f(1.f, 0.f, 0.f));
	rotates[index].x = q;
}
/**
* Adds yaw object rotation
* @param index: index of model
* @param rads: rads to change (can be negative)
*/
void Scene::add_yaw(int index, float rads)
{
	Quaternion q = rotates[index].y; //y-axis
	float curr_rads = q.get_angle();

	//do operations
	q = Quaternion(curr_rads + rads, Vec3f(0.f, 1.f, 0.f));
	rotates[index].y = q;
}
/**
* Adds roll object rotation
* @param index: index of model
* @param rads: rads to change (can be negative)
*/
void Scene::add_roll(int index, float rads)
{
	Quaternion q = rotates[index].z; //z-axis
	float curr_rads = q.get_angle();

	//do operations
	q = Quaternion(curr_rads + rads, Vec3f(0.f, 0.f, 1.f));
	rotates[index].z = q;
}
/**
* Sets order in which to rotate object about axis
* @param index: index of model
* @param order: new axis order ([0] is first, [2] is last)
*/
void Scene::set_rot_order(int index, int order[3])
{
	for (int i = 0; i < 3; i++)
	{
		rotates[index].order[i] = order[i];
	}
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
* Sets camera float step
* @param step: float value to set
*/
void Scene::set_cam_step(float step)
{
	cam.set_step(step);
}

extern volatile bool UP_KEY;
extern volatile bool DOWN_KEY;
extern volatile bool LEFT_KEY;
extern volatile bool RIGHT_KEY;
extern volatile bool W_KEY;
extern volatile bool A_KEY;
extern volatile bool S_KEY;
extern volatile bool D_KEY;
extern volatile bool Z_KEY;
extern volatile bool C_KEY;
extern volatile bool SHIFT_KEY;
extern volatile bool TAB_KEY;

/**
* Proceess keyboard inputs from window
* This is very rudimentary, I could've done a proper event system, however, that is just extra work for no gain
*/
void Scene::process_inputs()
{
	if (UP_KEY)
	{
		cam.rot_up();
		UP_KEY = false;
	}
	if (DOWN_KEY)
	{
		cam.rot_down();
		DOWN_KEY = false;
	}
	if (LEFT_KEY)
	{
		cam.rot_left();
		LEFT_KEY = false;
	}
	if (RIGHT_KEY)
	{
		cam.rot_right();
		RIGHT_KEY = false;
	}
	if (W_KEY)
	{
		cam.zoom_in();
		W_KEY = false;
	}
	if (A_KEY)
	{
		cam.left();
		A_KEY = false;
	}
	if (S_KEY)
	{
		cam.zoom_out();
		S_KEY = false;
	}
	if (D_KEY)
	{
		cam.right();
		D_KEY = false;
	}
	if (Z_KEY)
	{
		cam.roll_left();
		Z_KEY = false;
	}
	if (C_KEY)
	{
		cam.roll_right();
		C_KEY = false;
	}
	if (TAB_KEY)
	{
		cam.raise();
		TAB_KEY = false;
	}
	if (SHIFT_KEY)
	{
		cam.lower();
		SHIFT_KEY = false;
	}
}

/*******************************************************************************
* static helper functions for draw routine
*******************************************************************************/


/*
* Helper funtion to return point at which a line intersects a plane
* @param plane_p: position of center of plane
* @param plane_n: normal vector of plane
* @param p0: start of line
* @param p1: end of line
* @return: point of intersection
*/
static Vec3f intersect_plane(Vec3f plane_p, Vec3f plane_n, Vec3f p0, Vec3f p1)
{
	plane_n = plane_n.norm();
	float plane_d = -plane_n.dot(plane_p);
	float ad = p0.dot(plane_n);
	float bd = p1.dot(plane_n);
	float t = (-plane_d - ad) / (bd - ad);
	Vec3f line_start_end = p1 - p0;
	Vec3f intersect = line_start_end * t;
	return p0 + intersect;
}

/**
* Helper function to clip a triangle over a given plane
* This version doesnt modify a third pre-projection triangle
* @param plane_p position of plane
* @param plane_n normal vector of plane
* @param in: triangle to clip
* @param in_n: vertex normals of triangle
* @param out1: possible output triangle 1
* @param out2: possible output triangle 2
* @param out_n1: possible vertex normals of output 1
* @param out_n2: possible vertex normals of output 2
* @return: number of output triangles
*/
static int clip(Vec3f& plane_p, Vec3f& plane_n, Triangle& in, Triangle& in_n, Triangle& out1, Triangle& out2, Triangle& out_n1, Triangle& out_n2)
{
	//ensure plane normal is normalized
	plane_n = plane_n.norm();

	//auto funcntion to determine signed distance from point to plane
	//allows to determine if point on inside or outside of view box
	auto dist = [&](Vec3f& p)
		{
			return ((p - plane_p).dot(plane_n));
		};

	//tmp storage of inside and outside points in triangle
	Vec3f* in_p[3]; Vec3f* in_n_p[3]; int num_in_p = 0;
	Vec3f* out_p[3]; Vec3f* out_n_p[3]; int num_out_p = 0;

	//determine signed distance for each vertex of triangle
	float d[3] = { dist(in.raw[0]), dist(in.raw[1]), dist(in.raw[2]) };
	for (int i = 0; i < 3; i++)
	{
		if (d[i] >= 0.f)
		{
			in_n_p[num_in_p] = &in_n.raw[i];
			in_p[num_in_p++] = &in.raw[i];
		}
		else
		{
			out_n_p[num_out_p] = &in_n.raw[i];
			out_p[num_out_p++] = &in.raw[i];
		}
	}

	//determine which course of action to take dependent on num inside points
	switch (num_in_p)
	{
	case 0:
		return 0; //can discard triangle
	case 3:
	{
		out1 = in;
		out_n1 = in_n;
		return 1; //don't need to clip triangle
	}
	case 2:
	{
		//need to form two extra triangles
		//get intersections between points inside and outside
		Vec3f p1 = intersect_plane(plane_p, plane_n, *in_p[0], *out_p[0]);
		Vec3f p2 = intersect_plane(plane_p, plane_n, *in_p[1], *out_p[0]);

		out1 = Triangle(p1, *in_p[0], *in_p[1]);
		out2 = Triangle(p1, *in_p[1], p2);

		//push normal vector of clipped points towards inner points by ratio of distance clipped to distance
		float r_d1 = p1.dist(*in_p[0]) / in_p[0]->dist(*out_p[0]);  //ratio of distance
		float r_d2 = p2.dist(*in_p[1]) / in_p[1]->dist(*out_p[0]);

		Vec3f d_in_out1 = *in_n_p[0] - *out_n_p[0]; //change in normal
		Vec3f d_in_out2 = *in_n_p[1] - *out_n_p[0];

		p1 = *out_n_p[0] + (d_in_out1 * r_d1); //new normals
		p2 = *out_n_p[0] + (d_in_out2 * r_d2);

		out_n1 = Triangle(p1, *in_n_p[0], *in_n_p[1]);
		out_n2 = Triangle(p1, *in_n_p[1], p2);

		return 2;
	}
	case 1:
	{
		//simply set new end points on triangle
		//get intersections between points inside and outside
		Vec3f p1 = intersect_plane(plane_p, plane_n, *in_p[0], *out_p[0]);
		Vec3f p2 = intersect_plane(plane_p, plane_n, *in_p[0], *out_p[1]);

		out1 = Triangle(*in_p[0], p1, p2);

		//push normal vector of clipped points towards inner points by ratio of distance clipped to distance
		float r_d1 = p1.dist(*out_p[0]) / out_p[0]->dist(*in_p[0]); //ratio of distance
		float r_d2 = p2.dist(*out_p[1]) / out_p[1]->dist(*in_p[0]);

		Vec3f d_in_out1 = *in_n_p[0] - *out_n_p[0]; //change in normals
		Vec3f d_in_out2 = *in_n_p[0] - *out_n_p[1];

		p1 = *out_n_p[0] + (d_in_out1 * r_d1); //new normals
		p2 = *out_n_p[1] + (d_in_out2 * r_d2);

		out_n1 = Triangle(*in_n_p[0], p1, p2);

		return 1;
	}
	default:
	{
		log(ERR, "Error occured in clipping, should never reach this... Ignoring triangle");
		return 0;
	}
	}
}

/**
* Helper function to clip a triangle over a given plane
* @param plane_p position of plane
* @param plane_n normal vector of plane
* @param in: triangle to clip
* @param in_n: vertex normals of triangle
* @param in_w: pre-projection coords of in triangle
* @param out1: possible output triangle 1
* @param out2: possible output triangle 2
* @param out_n1: possible vertex normals of output 1
* @param out_n2: possible vertex normals of output 2
* @param out_w1: possible pre-projection output triangle 1
* @param out_w2: possible pre-projection output triangle 2
* @return: number of output triangles
*/
static int clip(Vec3f& plane_p, Vec3f& plane_n, Triangle& in, Triangle& in_n, Triangle& in_w, Triangle& out1, Triangle& out2, Triangle& out_n1, Triangle& out_n2, Triangle& out_w1, Triangle& out_w2)
{
	//ensure plane normal is normalized
	plane_n = plane_n.norm();

	//auto funcntion to determine signed distance from point to plane
	//allows to determine if point on inside or outside of view box
	auto dist = [&](Vec3f& p)
		{
			return ((p - plane_p).dot(plane_n));
		};

	//tmp storage of inside and outside points in triangle
	Vec3f* in_p[3]; Vec3f* in_n_p[3]; Vec3f* in_w_p[3]; int num_in_p = 0;
	Vec3f* out_p[3]; Vec3f* out_n_p[3]; Vec3f* out_w_p[3]; int num_out_p = 0;

	//determine signed distance for each vertex of triangle
	float d[3] = { dist(in.raw[0]), dist(in.raw[1]), dist(in.raw[2]) };
	for (int i = 0; i < 3; i++)
	{
		if (d[i] >= 0.f)
		{
			in_n_p[num_in_p] = &in_n.raw[i];
			in_w_p[num_in_p] = &in_w.raw[i];
			in_p[num_in_p++] = &in.raw[i];
		}
		else
		{
			out_n_p[num_out_p] = &in_n.raw[i];
			out_w_p[num_out_p] = &in_w.raw[i];
			out_p[num_out_p++] = &in.raw[i];
		}
	}

	//determine which course of action to take dependent on num inside points
	switch (num_in_p)
	{
	case 0:
		return 0; //can discard triangle
	case 3:
	{
		out1 = in;
		out_n1 = in_n;
		out_w1 = in_w;
		return 1; //don't need to clip triangle
	}
	case 2:
	{
		//need to form two extra triangles
		//get intersections between points inside and outside
		Vec3f p1 = intersect_plane(plane_p, plane_n, *in_p[0], *out_p[0]);
		Vec3f p2 = intersect_plane(plane_p, plane_n, *in_p[1], *out_p[0]);

		out1 = Triangle(p1, *in_p[0], *in_p[1]);
		out2 = Triangle(p1, *in_p[1], p2);

		//push normal vector of clipped points towards inner points by ratio of distance clipped to distance
		float r_d1 = p1.dist(*in_p[0]) / in_p[0]->dist(*out_p[0]);  //ratio of distance
		float r_d2 = p2.dist(*in_p[1]) / in_p[1]->dist(*out_p[0]);

		Vec3f d_in_out1 = *in_n_p[0] - *out_n_p[0]; //change in normal
		Vec3f d_in_out2 = *in_n_p[1] - *out_n_p[0];

		p1 = *out_n_p[0] + (d_in_out1 * r_d1); //new normals
		p2 = *out_n_p[0] + (d_in_out2 * r_d2);

		out_n1 = Triangle(p1, *in_n_p[0], *in_n_p[1]);
		out_n2 = Triangle(p1, *in_n_p[1], p2);

		//determine change in world space triangle
		d_in_out1 = *in_w_p[0] - *out_w_p[0]; //change in positions
		d_in_out2 = *in_w_p[1] - *out_w_p[0];

		p1 = *out_w_p[0] + (d_in_out1 * r_d1); //new positions
		p2 = *out_w_p[0] + (d_in_out2 * r_d2);

		out_w1 = Triangle(p1, *in_w_p[0], *in_w_p[1]);
		out_w2 = Triangle(p1, *in_w_p[1], p2);

		return 2;
	}
	case 1:
	{
		//simply set new end points on triangle
		//get intersections between points inside and outside
		Vec3f p1 = intersect_plane(plane_p, plane_n, *in_p[0], *out_p[0]);
		Vec3f p2 = intersect_plane(plane_p, plane_n, *in_p[0], *out_p[1]);

		out1 = Triangle(*in_p[0], p1, p2);

		//push normal vector of clipped points towards inner points by ratio of distance clipped to distance
		float r_d1 = p1.dist(*out_p[0]) / out_p[0]->dist(*in_p[0]); //ratio of distance
		float r_d2 = p2.dist(*out_p[1]) / out_p[1]->dist(*in_p[0]);

		Vec3f d_in_out1 = *in_n_p[0] - *out_n_p[0]; //change in normals
		Vec3f d_in_out2 = *in_n_p[0] - *out_n_p[1];

		p1 = *out_n_p[0] + (d_in_out1 * r_d1); //new normals
		p2 = *out_n_p[1] + (d_in_out2 * r_d2);

		out_n1 = Triangle(*in_n_p[0], p1, p2);

		//determine change in world space triangle
		d_in_out1 = *in_w_p[0] - *out_w_p[0]; //change in positions
		d_in_out2 = *in_w_p[0] - *out_w_p[1];

		p1 = *out_w_p[0] + (d_in_out1 * r_d1); //new positions
		p2 = *out_w_p[1] + (d_in_out2 * r_d2);

		out_w1 = Triangle(*in_w_p[0], p1, p2);

		return 1;
	}
	default:
	{
		log(ERR, "Error occured in clipping, should never reach this... Ignoring triangle");
		return 0;
	}
	}
}
/**
* Clips triangle over z bounds of perspective box if part of triangle outside of box
* @param t_draws: vector of triangles to draw
* @param t_norms: vector of vertex normals for each triangle
*/
static void clip_z(std::vector<Triangle>& t_draws, std::vector<Triangle>& t_norms, std::vector<Vec3f>& f_norms, float z_near, float z_far)
{
	std::vector<Triangle> new_draws;
	std::vector<Triangle> new_norms;
	std::vector<Vec3f> new_f_norms;

	//get near and far plane
	Vec3f plane_p_near = Vec3f(0.f, 0.f, z_near);
	Vec3f plane_p_far = Vec3f(0.f, 0.f, z_far);
	Vec3f plane_n_near = Vec3f(0.f, 0.f, 1.f);
	Vec3f plane_n_far = Vec3f(0.f, 0.f, -1.f);
	for (int i = 0; i < t_draws.size(); i++)
	{
		Triangle out_near[2], out_far[2];
		Triangle out_n_near[2], out_n_far[2];
		int num_near, num_far;

		/**************************************************************************************************
		* This is a faster but much harder to read way of doing this
		* This costs more memory but is significantly faster in some cases
		* The easiest way would be to iterate over every single face once per clip plane, but we can do it
		*	faster by doing it this way
		* We take the clipped(or unclipped) triangles from the near clip and see if they need to be
		*	clipped in the far plane.  After determining clipping on both they are added to our new list.
		**************************************************************************************************/

		//clip near plane
		num_near = clip(plane_p_near, plane_n_near, t_draws[i], t_norms[i], out_near[0], out_near[1], out_n_near[0], out_n_near[1]);
		for (int j = 0; j < num_near; j++)
		{
			//clip far plane
			num_far = clip(plane_p_far, plane_n_far, out_near[j], out_n_near[j], out_far[0], out_far[1], out_n_far[0], out_n_far[1]);
			for (int k = 0; k < num_far; k++)
			{
				//add to output
				new_draws.push_back(out_far[k]);
				new_norms.push_back(out_n_far[k]);
				new_f_norms.push_back(f_norms[i]);
			}
		}
	}

	//point to new data vectors
	t_draws.clear();
	t_norms.clear();
	f_norms.clear();
	t_draws = new_draws;
	t_norms = new_norms;
	f_norms = new_f_norms;
}
/**
* Clips triangle over x and y bounds of perspective box if part of triangle outside of box
* @param t_draws: vector of triangles to draw
* @param t_norms: vector of vertex normals for each triangle
*/
static void clip_xy(std::vector<Triangle> &t_draws, std::vector<Triangle> &t_norms, std::vector<Triangle> &t_world)
{
	std::vector<Triangle> new_draws;
	std::vector<Triangle> new_norms;
	std::vector<Triangle> new_world;

	//get near and far plane
	Vec3f plane_p_x0 = Vec3f(-0.9f, 0.f, 0.f);
	Vec3f plane_p_x1 = Vec3f(0.9f, 0.f, 0.f);
	Vec3f plane_n_x0 = Vec3f(0.9f, 0.f, 0.f);
	Vec3f plane_n_x1 = Vec3f(-0.9f, 0.f, 0.f);
	Vec3f plane_p_y0 = Vec3f(0.f, -0.9f, 0.f);
	Vec3f plane_p_y1 = Vec3f(0.f, 0.9f, 0.f);
	Vec3f plane_n_y0 = Vec3f(0.f, 0.9f, 0.f);
	Vec3f plane_n_y1 = Vec3f(0.f, -0.9f, 0.f);
	for (int i = 0; i < t_draws.size(); i++)
	{
		Triangle out_x0[2], out_x1[2], out_y0[2], out_y1[2];
		Triangle out_n_x0[2], out_n_x1[2], out_n_y0[2], out_n_y1[2];
		Triangle out_w_x0[2], out_w_x1[2], out_w_y0[2], out_w_y1[2];
		int num_x0, num_x1, num_y0, num_y1;

		/**************************************************************************************************
		* This is a faster but much harder to read way of doing this
		* This costs more memory but is significantly faster in some cases
		* The easiest way would be to iterate over every single face once per clip plane, but we can do it
		*	faster by doing it this way
		* We take the clipped(or unclipped) triangles from the each clip and see if they need to be
		*	clipped in the next plane.  After determining clipping on both they are added to our new list.
		**************************************************************************************************/

		//clip x0 plane
		num_x0 = clip(plane_p_x0, plane_n_x0, t_draws[i], t_norms[i], t_world[i], out_x0[0], out_x0[1], out_n_x0[0], out_n_x0[1], out_w_x0[0], out_w_x0[1]);
		for (int j = 0; j < num_x0; j++)
		{
			//clip x1 plane
			num_x1 = clip(plane_p_x1, plane_n_x1, out_x0[j], out_n_x0[j], out_w_x0[j], out_x1[0], out_x1[1], out_n_x1[0], out_n_x1[1], out_w_x1[0], out_w_x1[1]);
			for (int k = 0; k < num_x1; k++)
			{
				//clip y0 plane
				num_y0 = clip(plane_p_y0, plane_n_y0, out_x1[k], out_n_x1[k], out_w_x1[k], out_y0[0], out_y0[1], out_n_y0[0], out_n_y0[1], out_w_y0[0], out_w_y0[1]);
				for (int l = 0; l < num_y0; l++)
				{
					//clip y1 plane
					num_y1 = clip(plane_p_y1, plane_n_y1, out_y0[l], out_n_y0[l], out_w_y0[l], out_y1[0], out_y1[1], out_n_y1[0], out_n_y1[1], out_w_y1[0], out_w_y1[1]);
					for (int m = 0; m < num_y1; m++)
					{
						//add to output
						new_draws.push_back(out_y1[m]);
						new_norms.push_back(out_n_y1[m]);
						new_world.push_back(out_w_y1[m]);
					}
				}
			}
		}
	}

	//point to new data vectors
	t_draws.clear();
	t_norms.clear();
	t_world.clear();
	t_draws = new_draws;
	t_norms = new_norms;
	t_world = new_world;
}
/**
* Draws all models to the screen
*/
void Scene::draw()
{
	//get camera matrices
	Mat4x4f vert_cam_mat = cam.gen_vert_mat();
	Mat4x4f norm_cam_mat = cam.gen_norm_mat();

	//iterate through each model
	for (int i = 0; i < models.size(); i++)
	{
		//get faces, vertices, and normals
		std::vector<std::vector<Vec3i>> faces = models[i].get()->get_faces();
		std::vector<Vec3f> vertices = models[i].get()->get_vertices();
		std::vector<Vec3f> v_normals = models[i].get()->get_vert_normals();
		std::vector<Vec3f> f_normals = models[i].get()->get_face_normals();
		std::vector<Triangle> t_draws;
		std::vector<Triangle> t_norms;
		std::vector<Vec3f> f_norms;
		std::vector<Vec3f> lights = this->lights;
		Mat4x4f local_to_world = scales[i] * translates[i];
		for (int j = 0; j < faces.size(); j++)
		{
			//translate vertices
			Triangle t_draw;
			Triangle t_norm;
			Vec3f    f_norm = f_normals[j];
			for (int k = 0; k < 3; k++)
			{
				//all vertices should be within [-1, 1] range on all axis
				Vec3f vertex = vertices[faces[j][k].i_vert];
				Vec3f norm = v_normals[faces[j][k].i_norm];
				//rotate object
				rotate(vertex, i);
				rotate(norm, i);

				//translate to world coords
				vertex = Vec3f(local_to_world * Vec4f(vertex));

				//translate based on camera pos
				vertex = Vec3f(vert_cam_mat * Vec4f(vertex));
				norm = Vec3f(norm_cam_mat * Vec4f(norm));

				//add to draw list
				t_draw.raw[k] = vertex;
				t_norm.raw[k] = norm;
			}
			//get new face normal
			rotate(f_norm, i);
			f_norm = Vec3f(norm_cam_mat * Vec4f(f_norm));

			//otherwise add triangles to draw list
			t_draws.push_back(t_draw);
			t_norms.push_back(t_norm);
			f_norms.push_back(f_norm);
		}

		//translate lights to camera world coords
		for (int j = 0; j < lights.size(); j++)
		{
			lights[j] = Vec3f(vert_cam_mat * Vec4f(lights[j]));
		}
		//add cam pos to lights if cam_light set
		if (cam_light)
			lights.push_back(Vec3f(0.f, 0.f, 0.f)); //cam pos is origin after transform

		//clip over z bounds
		clip_z(t_draws, t_norms, f_norms, proj_mat.znear, proj_mat.zfar);

		//check if face can be culled (face is facing away from viewpoint)
		cull(f_norms, t_draws, t_norms);

		//project triangle to screen coords
		std::vector<Triangle> t_world = t_draws; //copy pre-projected values for lighting
		projection(t_draws);

		//clip over x and y bounds
		clip_xy(t_draws, t_norms, t_world);

		//draw all triangles
		if (t_draws.size() != t_norms.size())
		{
			log(ERR, "Error occured drawing model... Incorrect triangle or vertex normal count... skipping");
			continue;
		}
		for (int j = 0; j < t_draws.size(); j++)
			triangle_to_screen(t_draws[j], t_norms[j], t_world[j], lights, models[i].get()->get_color());
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
* @param t_draws: traingles to draw
* @param t_norms: triangle normals
*/
void Scene::cull(std::vector<Vec3f> &f_norms, std::vector<Triangle> &t_draws, std::vector<Triangle> &t_norms)
{
	//get the face normal to check
	std::vector<Triangle> new_draws;
	std::vector<Triangle> new_norms;
	std::vector<Vec3f>    new_f_norms;
	for (int i = 0; i < t_draws.size(); i++)
	{
		Vec3f face_n = f_norms[i];

		//determine the vector pointing from the camera to this face
		//get center of face
		Vec3f center;
		for (int j = 0; j < 3; j++)
		{
			center = center + t_draws[i].raw[j];
		}
		center = center / 3.f;
		Vec3f face_to_cam = Vec3f(0.f, 0.f, 0.f) - center;  //camera position is 0 relative to object because of transform

		//if the dot product between the camera and normal is less than 90 degrees, then we can draw
		if (face_to_cam.norm().dot(face_n.norm()) >= -0.9999f) //account for some error
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
	t_norms = new_norms;
}

/**
* Rotates an object based off rotation matrix
* Object should be centered around origin at this point
* @param old: vertex data to modify
* @param i: index of rotation matrix
*/
void Scene::rotate(Vec3f &old, int i)
{
	//iterate through each rotation on 
	for (int j = 0; j < 3; j++)
	{
		Quaternion q = rotates[i].raw[rotates[i].order[j]];
		Quaternion q_neg = q.conjugate();
		old = (Vec3f)(q * Quaternion(old) * q_neg);
	}
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
void Scene::triangle_to_screen(Triangle &t_draw, Triangle &t_norm, Triangle &t_world, std::vector<Vec3f> lights, COLOR color) const
{
	//assume z values to be between 0 and 1, values outside of range will just not be drawn
	//assume x and y values in range [-1, 1]

	//normalize triangle coordinates to fit to screen
	Triangle temp = t_draw; //copy values in t_draw in case need to be used later
	temp.A.x += 1.f;
	temp.B.x += 1.f;
	temp.C.x += 1.f;
	temp.A.y += 1.f;
	temp.B.y += 1.f;
	temp.C.y += 1.f;

	float w = (float)get_buf_width();
	float h = (float)get_buf_height();
	int x0, x1, x2, y0, y1, y2;

	x0 = (int)(temp.A.x * w / 2.f);
	x1 = (int)(temp.B.x * w / 2.f);
	x2 = (int)(temp.C.x * w / 2.f);
	y0 = (int)(temp.A.y * h / 2.f);
	y1 = (int)(temp.B.y * h / 2.f);
	y2 = (int)(temp.C.y * h / 2.f);

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
		//want to go over every light in the scene and determine color value of each vertex based on the vertex normals
		float color_r[3] = { 0.f, 0.f, 0.f }; //used to accumulate dot products
		for (auto light : lights)
		{
			for (int i = 0; i < 3; i++)
			{
				Vec3f light_vec = (light - t_world.raw[i]).norm();
				float dot = light_vec.dot(t_norm.raw[i].norm());
				color_r[i] += (dot < 0.f) ? 0.f : dot;
			}
		}

		//determine colors based on accumulated dot products
		COLOR colors[3];
		for (int i = 0; i < 3; i++)
		{
			if (color_r[i] >= 1.f)
				colors[i] = color;
			else if (color_r[i] <= 0.f)
				colors[i] = 0x0;
			else
				colors[i] = (color * color_r[i]);
		}

		//draw filled triangle based on these points
		fill_triangle(x0, y0, t_draw.A.z, x1, y1, t_draw.B.z, x2, y2, t_draw.C.z, colors[0], colors[1], colors[2]);
	}
}