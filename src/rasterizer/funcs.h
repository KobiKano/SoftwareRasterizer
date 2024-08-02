#pragma once
#include <memory>
#include <vector>
#include "model.h"
#include "../window/window.h"

constexpr auto RED   = 0xFF0000;
constexpr auto BLUE  = 0x0000FF;
constexpr auto GREEN = 0x00FF00;
constexpr auto WHITE = 0xFFFFFF;

//function defs to be used in rasterizer loop
//proc.cpp
void proc_loop();

//model.cpp
std::shared_ptr<Model> get_model();
