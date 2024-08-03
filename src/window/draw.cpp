#include "window.h"
#include "../logger/logger.h"
#include <string>

/*
* Get and draw pixel
* buf lock should be taken while drawing
*/
PIX_RET get_pixel(int x, int y, PIXEL& color, float& depth)
{
    log(DEBUG1, "getting pixel");

    //make sure draw is locked
    if (!get_draw_locked())
    {
        log(ERR, "cannot write if not locked");
        return FAIL;
    }

    //do operation
    if (color == NULL)
    {
        log(ERR, "null pointer given");
        return FAIL;
    }
    if (x < 0 || y < 0 || x >= get_buf_width() || y >= get_buf_height())
    {
        log(ERR, "out of bounds x: " + std::to_string(x) + "|y: " + std::to_string(y));
        return BOUNDS;
    }

    color = get_buf()[y * get_buf_width() + x];
    depth = get_z_buf()[y * get_buf_width() + x];
    return SUCCESS;
}
PIX_RET set_pixel(int x, int y, PIXEL color, float depth)
{
    log(DEBUG1, "writing pixel");

    //make sure bounds are valid
    if (x < 0 || y < 0 || x >= get_buf_width() || y >= get_buf_height())
    {
        log(DEBUG1, "out of bounds x: " + std::to_string(x) + "|y: " + std::to_string(y));
        return BOUNDS;
    }

    //immediately check if depth is valid
    if (depth > get_z_buf()[y * get_buf_width() + x] || depth < 0.f || depth > 1.f)
    {
        return DEPTH;
    }

    //make sure draw is locked
    if (!get_draw_locked())
    {
        log(ERR, "cannot write if not locked");
        return FAIL;
    }

    //do operation
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
*/
void draw_line(int x0, int y0, float z0, int x1, int y1, float z1, PIXEL color)
{
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
    }

    //calculate gradients
    int dx = x1 - x0;
    int dy = y1 - y0;
    float dz_dx = (z1 - z0)/((float)dx);
    int dyerror2 = abs(dy) * 2;
    int yerror2 = 0;
    int y = y0;
    float z = z0;

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
    }
}


/*
* Draws wireframe triangle over given points with given color
*/
void draw_triangle(int x0, int y0, float z0,
    int x1, int y1, float z1,
    int x2, int y2, float z2, PIXEL color)
{
    draw_line(x0, y0, z0, x1, y1, z1, color);
    draw_line(x1, y1, z1, x2, y2, z2, color);
    draw_line(x2, y2, z2, x0, y0, z0, color);
}


/*
* Draws triangle over given points with given color
*/
void fill_triangle(int x0, int y0, float z0,
    int x1, int y1, float z1,
    int x2, int y2, float z2, 
    PIXEL color0, PIXEL color1, PIXEL color2)
{
    //TODO: complete
}