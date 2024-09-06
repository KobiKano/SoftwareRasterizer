add_library(
	rasterizer
	geom.hpp
	model.hpp
	proc.hpp
	quaternion.hpp
	render.hpp
	camera.cpp
	model.cpp
	proc.cpp
	scene.cpp
)

target_include_directories(rasterizer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")