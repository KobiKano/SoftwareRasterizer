#include "logger.h"
#include <stdio.h>
#include <string>

//global defs
static LEVEL _level = ERR; //default error level

void logger_set_level(LEVEL level)
{
	_level = level;
}

void log(LEVEL level, std::string msg)
{
	//check if logger level is higher than given level
	if (level > _level)
		return; //do nothing

	//else print desired statement
	switch (level)
	{
	case DEBUG:
		fprintf(stdout, "___DEBUG___: %s\n", msg.c_str());
		return;
	case WARNING:
		fprintf(stdout, "___WARNING___: %s\n", msg.c_str());
		return;
	case ERR:
		fprintf(stderr, "___ERROR___: %s\n", msg.c_str());
		return;
	}
}