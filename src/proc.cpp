#include "funcs.h"
#include "logger.h"
#include "window.h"
#include <iostream>

void proc_loop()
{
	while (1)
	{
		//start sync
		window_sync_begin();

		//clear screen
		if (!window_clear())
			exit(-1);

		//TODO: ADD DRAW FUNCTIONS

		//send draw call to window
		window_update();

		//end sync
		window_sync_end(0); //0 input uncaps fps
	}
}