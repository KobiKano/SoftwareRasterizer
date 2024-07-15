#include "funcs.h"
#include "../logger/logger.h"
#include "../window/window.h"
#include <iostream>

extern volatile bool g_alive;
extern volatile bool g_exit_error;

void proc_loop()
{
	while (1)
	{
		//check if window still exists
		if (!g_alive)
		{
			//window closed
			log(DEBUG, "window termined");
			window_remove();
			if (g_exit_error)
				exit(-1);
			exit(0);
		}

		//start sync
		window_sync_begin();

		//clear screen
		window_clear();

		//TODO: ADD DRAW FUNCTIONS
		draw_lock();

		//test draw triangles
		draw_line(30, 30, 30, 90, RED);
		draw_line(30, 90, 90, 90, BLUE);
		draw_line(90, 90, 30, 30, GREEN);

		draw_line(120, 120, 180, 120, RED);
		draw_line(180, 120, 180, 180, BLUE);
		draw_line(180, 180, 120, 120, GREEN);

		draw_unlock();

		//send draw call to window
		window_update();

		//end sync
		window_sync_end(0, true); //0 input uncaps fps
	}
}