#include "funcs.h"
#include "geom.h"
#include "model.h"
#include "render.h"
#include "../window/window.h"
#include "../logger/logger.h"

Scene::Scene()
{
	//initialize camera with 0.1 step speed
	cam = Camera(0.1f);

	//initialize translation, scale, and perspective matrices
	//TODO
}
void Scene::reg_model(std::unique_ptr<Model> m)
{
	models.push_back(m);
}
void Scene::set_perspective()
{
	//TODO
}
void Scene::add_light(Vec3f p)
{
	lights.push_back(p);
}
void Scene::draw()
{
	//iterate through each model
	for (int i = 0; i < models.size(); i++)
	{
		//get faces
		std::vector<std::vector<Vec3i>> faces = models[i].get()->get_faces();
		for (int j = 0; j < faces.size(); j++)
		{
			//check if face can be culled (face is facing away from viewpoint)
			if (cullable(faces[j]))
				continue;

			//else translate vertices
			//TODO
		}
	}
}

/********************************************************************
* Private Functions
********************************************************************/
bool Scene::cullable(const std::vector<Vec3i>& old)
{
	//TODO
}
std::vector<Vec3f> Scene::translate(const Vec3f &old)
{
	//TODO
}
std::vector<Vec3f> Scene::scale(const Vec3f &old)
{
	//TODO
}
std::vector<Vec3f> Scene::perspective(const Vec3f &old)
{
	//TODO
}