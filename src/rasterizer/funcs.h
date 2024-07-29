#pragma once
#include <memory>
#include <vector>
#include "model.h"
#include "../window/window.h"

constexpr auto RED   = 0xFF0000;
constexpr auto BLUE  = 0x0000FF;
constexpr auto GREEN = 0x00FF00;

//function defs to be used in rasterizer loop
//proc.cpp
void proc_loop();

//model.cpp
std::unique_ptr<Model> get_model();

//geom.cpp
void draw(Model &m);
