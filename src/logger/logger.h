#pragma once
#include <string>

typedef enum level {
	DEBUG1 = 0,
	DEBUG2 = 1,
	WARNING = 2,
	ERR = 3
} LEVEL;

void logger_set_level(LEVEL level);
void log(LEVEL level, std::string msg);
