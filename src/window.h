#pragma once
#include <Windows.h>
#include <stdint.h>

//global defs

//typedefs
typedef uint32_t PIXEL;

//funtion defs
int create_window(const char* name, int width, int height);
void window_update();
int window_clear();
void window_remove();
void window_sync_begin();
void window_sync_end(int fps_cap);
