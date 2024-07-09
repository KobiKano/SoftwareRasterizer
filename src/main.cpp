#ifndef UNICODE
#define UNICODE
#endif 

#include "window.h"
#include "logger.h"
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

	//run loop on this process(temporary solution)
	std::cout << "press y to exit";
	char in;
	while (1)
	{
		std::cin >> in;

		if (in == 'y')
		{
			exit(0);
		}
	}

	//default exit(should never be reached)
	log(WARNING, "Loop exited without ending process...");
	exit(0);
}