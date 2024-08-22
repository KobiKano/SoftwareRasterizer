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
	//if not in debug mode do nothing
#ifdef _DEBUG
	//check if logger level is higher than given level
	if (level < _level)
		return; //do nothing

	//else print desired statement
	switch (level)
	{
	case DEBUG1:
		fprintf(stdout, "___DEBUG1___: %s\n", msg.c_str());
		return;
	case DEBUG2:
		fprintf(stdout, "___DEBUG2___: %s\n", msg.c_str());
		return;
	case WARNING:
		fprintf(stdout, "___WARNING___: %s\n", msg.c_str());
		return;
	case ERR:
		fprintf(stderr, "___ERROR___: %s\n", msg.c_str());
		return;
	}
#endif
}