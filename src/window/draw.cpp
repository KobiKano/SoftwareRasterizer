#ifdef _WINDOWS
#include "window.h"
#include "../logger/logger.h"
#include "../rasterizer/geom.h"
#include <string>
#include <vector>


/********************************************************************************************************************************
* Doesn't check draw parameters or if locks are correct unless in debug mode
* These cases should never occur in this program, thus there is no point in slowing release mode
********************************************************************************************************************************/


/*
* Get pixel
* buf lock should be taken while drawing
* @param x: x coordinate on screen
* @param y: y coordinate on screen
* @param color: reference to color output, will be modified
* @param depth: reference to z depth, will be modified
* @return if in release always returns SUCCESS or may crash program
*           if in debug returns BOUNDS if attempt out of bounds
*           FAIL if lock not taken
*/
PIX_RET get_pixel(int x, int y, COLOR& color, float& depth)
{
#ifdef _DEBUG
    log(DEBUG1, "getting pixel");

    //make sure draw is locked
    if (!get_draw_locked())
    {
        log(ERR, "cannot write if not locked");
        return FAIL;
    }

    //do operation
    if (&color == NULL)
    {
        log(ERR, "null pointer given");
        return FAIL;
    }
    if (x < 0 || y < 0 || x >= get_buf_width() || y >= get_buf_height())
    {
        log(ERR, "out of bounds x: " + std::to_string(x) + "|y: " + std::to_string(y));
        return BOUNDS;
    }
#endif // _DEBUG

    color = get_buf()[y * get_buf_width() + x];
    depth = get_z_buf()[y * get_buf_width() + x];
    return SUCCESS;
}

/**
* Sets pixel on screen
* Assumes draw lock taken
* @param x: x coordinate on screen
* @param y: y coordinate on screen
* @param color: color to set
* @param depth: depth value to set
* @return if in release always returns SUCCESS or may crash program
*           if in debug returns BOUNDS if attempt out of bounds
*           DEPTH if depth is behind previous
*           FAIL if lock not taken
*/
PIX_RET set_pixel(int x, int y, COLOR color, float depth)
{
#ifdef _DEBUG
    log(DEBUG1, "writing pixel");

    //make sure bounds are valid
    if (x < 0 || y < 0 || x >= get_buf_width() || y >= get_buf_height())
    {
        log(DEBUG1, "out of bounds x: " + std::to_string(x) + "|y: " + std::to_string(y));
        return BOUNDS;
    }

    //immediately check if depth is valid
    if (depth < 0.f || depth > 1.f)
    {
        return DEPTH;
    }

    //make sure draw is locked
    if (!get_draw_locked())
    {
        log(ERR, "cannot write if not locked");
        return FAIL;
    }
#endif // _DEBUG

    //do operation
    if (depth > get_z_buf()[y * get_buf_width() + x])
        return DEPTH;
    get_buf()[y * get_buf_width() + x] = color;
    get_z_buf()[y * get_buf_width() + x] = depth;
    return SUCCESS;
}

/*
* Private function to check if all future points of a line being drawn will be outside bounds
*/
static bool check_bound(int x, int y)
{
    //check over right bound
    if (x > get_buf_width())
    {
        log(DEBUG1, "Over right bound, ending line draw");
        return false;
    }
    //check both y bounds
    else if (y > get_buf_height() || y < 0)
    {
        log(DEBUG1, "Over bottom or top bound, ending line draw");
        return false;
    }

    //defualt return (still over left bound, but can end up on screen still)
    log(DEBUG1, "Over left bound, continuing...");
    return true;
}

/*
* Draws a line into buffer from point0 to point1
* @param x0: start x coord
* @param y0: start y coord
* @param z0: start z depth
* @param x1: end x coord
* @param y1: end y coord
* @param z1: end z depth
* @param color0: start color
* @param color1: end color
*/
void draw_line(int x0, int y0, float z0, int x1, int y1, float z1, COLOR color0, COLOR color1)
{
#ifdef _DEBUG
    //quick on time check on bounds to make sure line is actually fully in bounds
    if ((x0 < 0 && (x1 <= x0)) || (x1 < 0 && (x0 <= x1)) || (x0 > get_buf_width() && (x0 <= x1)) || (x1 > get_buf_width() && (x1 <= x0)))
    {
        log(DEBUG1, "Line completely out of bounds, skipping");
        return;
    }
    if ((y0 < 0 && (y1 <= y0)) || (y1 < 0 && (y0 <= y1)) || (y0 > get_buf_height() && (y0 <= y1)) || (y1 > get_buf_height() && (y1 <= y0)))
    {
        log(DEBUG1, "Line completely out of bounds, skipping");
        return;
    }
#endif // _DEBUG

    //check steepness (if delta-y is greater than delta-x)
    bool steep = false;
    if (abs(x0 - x1) < abs(y0 - y1))
    {
        //transpose x and y
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }

    //ensure iterateing from left to right
    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
        std::swap(z0, z1);
        std::swap(color0, color1);
    }

    //calculate gradients
    int dx = x1 - x0;
    int dy = y1 - y0;
    float dz_dx = 0.f;
    int dyerror2 = abs(dy) * 2;
    int yerror2 = 0;
    COLOR color = color0;
    float R = 0.f, G = 0.f, B = 0.f;
    float dR_dx = 0.f, dG_dx = 0.f, dB_dx = 0.f;;
    int y = y0;
    float z = z0;
    if (dx != 0)
    {
        dz_dx = (z1 - z0) / ((float)dx);
        dR_dx = ((float)color1.R - (float)color0.R) / (float)dx;
        dG_dx = ((float)color1.G - (float)color0.G) / (float)dx;
        dB_dx = ((float)color1.B - (float)color0.B) / (float)dx;
    }

    //draw line
    for (int x = x0; x <= x1; x++)
    {
        //untranspose if needed and make sure not doing uneeded draws outside of bounds
        if (steep)
        {
            if (set_pixel(y, x, color, z) == BOUNDS && !check_bound(y, x))
                return;
        }
        else
        {
            if (set_pixel(x, y, color, z) == BOUNDS && !check_bound(x, y))
                return;
        }

        //calculate next y step using error
        yerror2 += dyerror2;
        if (yerror2 > dx)
        {
            y += (y1 > y0) ? 1 : -1;
            yerror2 -= dx * 2;
        }

        //calculate next z step
        z += dz_dx;
        
        //calculate next color step
        R += dR_dx;
        color.R = color0.R + floorf(R);
        G += dG_dx;
        color.G = color0.G + floorf(G);
        B += dB_dx;
        color.B = color0.B + floorf(B);
    }
}


/*
* Draws wireframe triangle over given points with given color
* @param x0: A x coord
* @param y0: A y coord
* @param z0: A z depth
* @param x1: B x coord
* @param y1: B y coord
* @param z1: B z depth
* @param x2: C x coord
* @param y2: C y coord
* @param z2: C z depth
* @param color: color to draw lines as
*/
void draw_triangle(int x0, int y0, float z0,
    int x1, int y1, float z1,
    int x2, int y2, float z2, COLOR color)
{
    draw_line(x0, y0, z0, x1, y1, z1, color, color);
    draw_line(x1, y1, z1, x2, y2, z2, color, color);
    draw_line(x2, y2, z2, x0, y0, z0, color, color);
}


/*
* Draws filled triangle over given points with given colors
* @param x0: A x coord
* @param y0: A y coord
* @param z0: A z depth
* @param x1: B x coord
* @param y1: B y coord
* @param z1: B z depth
* @param x2: C x coord
* @param y2: C y coord
* @param z2: C z depth
* @param color0: A color value
* @param color1: B color value
* @param color2: C color value
*/
void fill_triangle(int x0, int y0, float z0,
    int x1, int y1, float z1,
    int x2, int y2, float z2, 
    COLOR color0, COLOR color1, COLOR color2)
{

    //we are going to iterate over the bounds of the triangle and determine whether pixels are in or out of the triangle
    int x_min = min(min(x0, x1), x2); //want x to be furthest right value of highest y
    int x_max = max(max(x0, x1), x2);
    int x = x_min;

    int y_min = min(min(y0, y1), y2); //want x to be furthest right value of highest y
    int y_max = max(max(y0, y1), y2);
    int y = y_min;

    //get vectors and area of triangle for barycentric calcs
    Vec3i A = Vec3i(x0, y0, 0);
    Vec3i B = Vec3i(x1, y1, 0);
    Vec3i C = Vec3i(x2, y2, 0);
    Vec3i AB = B - A;
    Vec3i AC = C - A;
    float Area_ABC = AB.cross(AC).value() / 2.f;

    while (y != (y_max + 1))
    {
        //determine if point is in triangle using barycentric coordinate system
        Vec3i P = Vec3i(x, y, 0);
        Vec3i PA = A - P;
        Vec3i PB = B - P;
        Vec3i PC = C - P;

        float u = (PA.cross(PC).value() / 2.f) / Area_ABC;
        float v = (PA.cross(PB).value() / 2.f) / Area_ABC;
        float w = (PC.cross(PB).value() / 2.f) / Area_ABC;

        //if sum of u v and w does not equal 1.0, then point not in triangle
        //allow for small error
        if (((u + v + w) >= 0.99f) && ((u + v + w) <= 1.01f))
        {
            //point in triangle, determine color and depth val dependent on barycentric weights
            COLOR color = (color0 * w) + (color1 * u) + (color2 * v);
            float depth = (z0 * w) + (z1 * u) + (z2 * v);
            set_pixel(x, y, color, depth);
        }

        //increase x
        x++;
        if (x == (x_max + 1))
        {
            x = x_min;
            y++;
        }
    }
}
#endif