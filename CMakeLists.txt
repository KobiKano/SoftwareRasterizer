cmake_minimum_required(VERSION 3.12)
project(RenderEngine VERSION 1.0.0)

if(NOT WIN32)
	message(FATAL_ERROR "Need Windows Operating System to Run!!!")
endif()

add_subdirectory(src/logger)
add_subdirectory(src/window)
add_subdirectory(src/rasterizer)
add_subdirectory(src)
