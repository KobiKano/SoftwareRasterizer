#pragma once
#include <memory>
#include <vector>
#include "model.h"
#include "geom.h"
#include "render.h"
#include "../window/window.h"

constexpr auto RED   = 0xFF0000;
constexpr auto BLUE  = 0x0000FF;
constexpr auto GREEN = 0x00FF00;
constexpr auto WHITE = 0xFFFFFF;

//process class used to hold functionality for running backend of render engine
class Proc
{
public:
	//constructor for class
	Proc(const char* config);
	~Proc();
	void start();
private:
	std::unique_ptr<Scene> scene;
	std::vector<Vec3f> positions;

	void animate();
};
