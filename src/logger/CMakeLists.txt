add_library(
	logger
	logger.hpp
	logger.cpp
)

target_include_directories(logger PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")