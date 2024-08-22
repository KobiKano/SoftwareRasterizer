#include "funcs.h"
#include "render.h"
#include "geom.h"
#include "../logger/logger.h"
#include "../window/window.h"
#include <iostream>

extern volatile bool g_alive;
extern volatile bool g_exit_error;
extern volatile bool g_resize;

/**
* Draw loop for window
*/
void proc_loop()
{
	/*******************************************************************************************************************
	* can modify the following to alter scene
	*******************************************************************************************************************/
	std::unique_ptr<Scene> scene(new Scene());
	scene.get()->reg_model(get_model());
	scene.get()->add_light(Vec3f(-1, -1, -1));
	scene.get()->set_pos(0, Vec3f(0, 0, 3));
	scene.get()->set_wireframe(false);
	//scene.get()->set_perspective()
	/******************************************************************************************************************/

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
		scene.get()->process_inputs();

		//lock the screen
		draw_lock();

		//check if screen size changed
		if (g_resize)
		{
			//change aspect ratio
			float w = (float)get_buf_width();
			float h = (float)get_buf_height();
			scene.get()->set_aspect_ratio(h / w);
		}

		//draw scene
		scene.get()->draw();

		//unlock the screen
		draw_unlock();

		//send draw call to window
		window_update();

		//end sync
		window_sync_end(0, true); //0 input uncaps fps
	}
}