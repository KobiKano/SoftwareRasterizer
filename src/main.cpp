#ifndef UNICODE
#define UNICODE
#endif 

#ifdef _WINDOWS
#include "window/window.h"
#include "logger/logger.h"
#include "rasterizer/proc.h"
#include <iostream>
#endif

void main(int argc, char* argv[])
{
#ifndef _WINDOWS
	//this program requires windows to run if windows not being used end process
	fprintf(stderr, "Program requires Windows OS to run, exiting...\n");
	exit(1);
#else
	//set logger level
	logger_set_level(DEBUG2);

	//create window
	if (create_window("SoftwareRasterizer", 512, 512) != 0)
	{
		log(ERR, "Window creation failed, exiting");
		exit(-1);
	}

	//start window
	window_update();

	//run loop on this process
	Proc proc("../config.txt");
	proc.start();

	//default exit(should never be reached)
	log(WARNING, "Loop exited without ending process...");
	exit(0);
#endif
}