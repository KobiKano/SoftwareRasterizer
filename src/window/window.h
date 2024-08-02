#pragma once
#include <Windows.h>
#include <stdint.h>

//typedefs
typedef uint32_t PIXEL;
enum PIX_RET
{
	SUCCESS,
	FAIL,
	BOUNDS,
	DEPTH
};

//funtion defs
//window.cpp
// private global getters
bool get_draw_locked();
int get_buf_width();
int get_buf_height();
PIXEL* get_buf();
float* get_z_buf();

//window creation/deletion
int create_window(const char* name, int width, int height);
void window_remove();

//window update
void window_update();
void window_clear();

//buffer modification
PIX_RET get_pixel(int x, int y, PIXEL& color, float& depth);
PIX_RET set_pixel(int x, int y, PIXEL color, float depth);
void draw_lock();
void draw_unlock();

//window sync
void window_sync_begin();
void window_sync_end(int fps_cap, bool print_fps);

//draw.cpp
void draw_line(int x0, int y0, float z0, int x1, int y1, float z1, PIXEL color);
void draw_triangle(int x0, int y0, float z0,
	int x1, int y1, float z1,
	int x2, int y2, float z2, PIXEL color);
void fill_triangle(int x0, int y0, float z0,
	int x1, int y1, float z1,
	int x2, int y2, float z2, 
	PIXEL color0, PIXEL color1, PIXEL color2);