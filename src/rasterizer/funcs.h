#pragma once
#include <memory>
#include "model.h"

constexpr auto RED   = 0xFF0000;
constexpr auto BLUE  = 0x0000FF;
constexpr auto GREEN = 0x00FF00;

//function defs to be used in rasterizer loop
void proc_loop();
std::unique_ptr<Model> get_model();