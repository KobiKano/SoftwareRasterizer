#include "proc.hpp"
#include "render.hpp"
#include "geom.hpp"
#include "../logger/logger.hpp"
#include "../window/window.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>

extern volatile bool g_alive;
extern volatile bool g_exit_error;
extern volatile bool g_resize;

/**
* Reads provided config file to determine layout of scene
* Constructor for this class
* Adds values to keep track of models for animating
* Don't do any checks on config file
*	assume correctly formatted
*	provided file is correctly formatted and very simple to understand
*	breaking the config file is a user error
* @param config: path of config file
*/
Proc::Proc(const char* config)
{
	//read config file to determine layout of scene
	std::string line;
	std::ifstream fstream(config, std::ifstream::in);
	scene = std::unique_ptr<Scene>(new Scene());
	while (std::getline(fstream, line))
	{
		std::istringstream s(line);
		std::string t;
		s >> t;
		//determine if line is comment
		if (!t.compare("#") || !t.compare(""))
		{
			//skip line
			continue;
		}
		else if (!t.compare("fov_deg"))
		{
			float fov_rad;
			s >> fov_rad;
			fov_rad = fov_rad * PI / 180.f;

			scene->set_fov(fov_rad);
		}
		else if (!t.compare("z_bound"))
		{
			float z_near, z_far;
			s >> z_near;
			s >> z_far;
			scene->set_z_bound(z_far, z_near);
		}
		else if (!t.compare("wireframe"))
		{
			bool wireframe;
			s >> wireframe;
			scene->set_wireframe(wireframe);
		}
		else if (!t.compare("cam_light"))
		{
			bool cam_light;
			s >> cam_light;
			scene->set_cam_light(cam_light);
		}
		else if (!t.compare("light"))
		{
			Vec3f light;
			for (int i = 0; i < 3; i++)
			{
				s >> light.raw[i];
			}
			scene->add_light(light);
		}
		else if (!t.compare("model"))
		{
			//get model path
			std::string model;
			s >> model;
			model = model + ".obj";
			std::string path = "Models/" + model;
			std::ifstream f(path.c_str());
			if (!f.good())
			{
				log(WARNING, "Invalid path, defaulting to cube");
				path = "Models/cube.obj";
			}
			f.close();

			//get color and scale and position of models
			COLOR color;
			uint32_t col_val;
			Vec3f pos;
			float scale;

			s >> std::hex >> col_val;
			color = col_val;
			for (int i = 0; i < 3; i++)
			{
				s >> pos.raw[i];
			}
			s >> scale;

			//add model to scene
			scene->reg_model(std::shared_ptr<Model>(new Model(path.c_str())), pos, scale, color);
			positions.push_back(pos);
		}
	}
}

/**
* Debug destructor for class
*/
Proc::~Proc()
{
	log(DEBUG1, "Ending Process");
}

/**
* Animates each model to bounce up and down while rotating
*/
void Proc::animate()
{
	//record start and previous time as static values
	static auto start_time = std::chrono::steady_clock::now();
	static auto prev_time = start_time;

	//get current time
	auto curr_time = std::chrono::steady_clock::now();

	//get change in time
	auto d_time = curr_time - prev_time;
	prev_time = curr_time;
	for (int i = 0; i < positions.size(); i++)
	{
		//going to use sin function for position
		positions[i].y = sinf((float)std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - start_time).count() / 1000.f);
		scene->set_pos(i, positions[i]);

		//rotate based on elaped time
		scene->add_yaw(i, (float)std::chrono::duration_cast<std::chrono::milliseconds>(d_time).count() / 1000.f);
	}
}

/**
* Draw loop for window
*/
void Proc::start()
{
	//start draw loop
	while (1)
	{
		//check if window still exists
		if (!g_alive)
		{
			//window closed
			log(DEBUG1, "window termined");
			window_remove();
			if (g_exit_error)
				exit(-1);
			exit(0);
		}

		//start sync
		window_sync_begin();

		//clear screen
		window_clear();

		//process inputs
		scene->process_inputs();

		//get next animation step
		animate();

		//lock the screen
		draw_lock();

		//check if screen size changed
		if (g_resize)
		{
			//change aspect ratio
			float w = (float)get_buf_width();
			float h = (float)get_buf_height();
			scene->set_aspect_ratio(h / w);
		}

		//draw scene
		scene->draw();

		//unlock the screen
		draw_unlock();

		//send draw call to window
		window_update();

		//end sync
		window_sync_end(0, true); //0 input uncaps fps
	}
}