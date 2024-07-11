#include "funcs.h"
#include "logger.h"
#include "window.h"
#include <iostream>

extern volatile bool g_alive;

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
			exit(0);
		}

		//start sync
		window_sync_begin();

		//clear screen
		if (!window_clear())
			exit(-1);

		//TODO: ADD DRAW FUNCTIONS

		//send draw call to window
		window_update();

		//end sync
		window_sync_end(0, true); //0 input uncaps fps
	}
}