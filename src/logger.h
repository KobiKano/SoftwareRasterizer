#pragma once
#include <string>

typedef enum level {
	DEBUG = 0,
	WARNING = 1,
	ERR = 2
} LEVEL;

void logger_set_level(LEVEL level);
void log(LEVEL level, std::string msg);
