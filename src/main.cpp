#ifndef UNICODE
#define UNICODE
#endif 

#include "window.h"
#include "logger.h"
#include "funcs.h"
#include <iostream>

void main(int argc, char* argv[])
{
	//set logger level
	logger_set_level(DEBUG);

	//create window
	if (create_window("SoftwareRasterizer", 512, 512) != 0)
	{
		log(ERR, "Window creation failed, exiting");
		exit(-1);
	}

	//start window
	window_update();

	//run loop on this process
	proc_loop();

	//default exit(should never be reached)
	log(WARNING, "Loop exited without ending process...");
	exit(0);
}