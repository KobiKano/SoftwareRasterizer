#pragma once
#ifdef _WINDOWS
#include <Windows.h>
#include <stdint.h>
#include <vector>
#include <cmath>

//typedefs
struct COLOR
{
	union
	{
		struct { uint8_t R, G, B, alpha; };
		uint32_t val;
	};

	//constructors
	COLOR() { val = 0x0; }
	COLOR(uint32_t val) { this->val = val; }
	COLOR(uint8_t R, uint8_t G, uint8_t B) { this->R = R; this->G = G; this->B = B; this->alpha = 0x0; }
	COLOR(uint8_t R, uint8_t G, uint8_t B, uint8_t alpha) { this->R = R; this->G = G; this->B = B; this->alpha = alpha; }

	//casting
	inline operator float() const { return (float)val; }

	//operators
	inline COLOR operator +(const COLOR& c) const 
	{ 
		uint8_t n_R = (((int)R + (int)c.R) > 0xff) ? 0xff : R + c.R;
		uint8_t n_G = (((int)G + (int)c.G) > 0xff) ? 0xff : G + c.G;
		uint8_t n_B = (((int)B + (int)c.B) > 0xff) ? 0xff : B + c.B;
		return COLOR(n_R, n_G, n_B);
	}
	inline COLOR operator -(const COLOR& c) const
	{ 
		uint8_t n_R = (((int)R - (int)c.R) < 0x0) ? 0x0 : R - c.R;
		uint8_t n_G = (((int)G - (int)c.G) < 0x0) ? 0x0 : G - c.G;
		uint8_t n_B = (((int)B - (int)c.B) < 0x0) ? 0x0 : B - c.B;
		return COLOR(n_R, n_G, n_B);
	}
	inline COLOR operator /(const int& i)   const { return COLOR(floorf(R / i), floorf(G / i), floorf(B / i)); }
	inline COLOR operator *(const float& f) const { return COLOR(floorf(R * f), floorf(G * f), floorf(B * f)); }
};
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
COLOR* get_buf();
float* get_z_buf();

//window creation/deletion
int create_window(const char* name, int width, int height);
void window_remove();

//window update
void window_update();
void window_clear();

//buffer modification
PIX_RET get_pixel(int x, int y, COLOR& color, float& depth);
PIX_RET set_pixel(int x, int y, COLOR color, float depth);
void draw_lock();
void draw_unlock();

//window sync
void window_sync_begin();
void window_sync_end(int fps_cap, bool print_fps);

//draw.cpp
struct PIXEL //struct to hold pixel info
{
	int x, y;
	float z;
	COLOR color;

	PIXEL() { x = 0; y = 0; z = 0.f; color = 0; };
	PIXEL(int x, int y, float z, COLOR color)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->color = color;
	}
};
void draw_line(int x0, int y0, float z0, int x1, int y1, float z1, COLOR color0, COLOR color1);
void draw_line(int x0, int y0, float z0, int x1, int y1, float z1, COLOR color0, COLOR color1, std::vector<PIXEL> &line);
void draw_triangle(int x0, int y0, float z0,
	int x1, int y1, float z1,
	int x2, int y2, float z2, COLOR color);
void fill_triangle(int x0, int y0, float z0,
	int x1, int y1, float z1,
	int x2, int y2, float z2, 
	COLOR color0, COLOR color1, COLOR color2);
#endif