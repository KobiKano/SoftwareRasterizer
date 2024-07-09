#include "window.h"
#include "logger.h"
#include <Windows.h>
#include <stdio.h>
#include <string>

using namespace std;

//global defs
static HWND _handle;
static HDC _win_hDC;
static WNDCLASS _wnd_class;

static int _buf_width = -1;
static int _buf_height = -1;
static int _wnd_width = -1;
static int _wnd_height = -1;

static PIXEL* _buf = NULL;  //heap allocated array of uint32 for each pixel on screen to hold color value
static BITMAPINFO _bmp_info;


/*
* Defines functionality for window events
*/
LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // All painting occurs here, between BeginPaint and EndPaint.

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
    }
    return 0;
    case WM_SIZE:
    {
        int width = LOWORD(lParam);  // Macro to get the low-order word.
        int height = HIWORD(lParam); // Macro to get the high-order word.

        // Respond to the message:
        _buf_width = width;
        _buf_height = height;
    }
    return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/*
* Creates window process
*/
int create_window(const char* name, int width, int height)
{
    int x;
    int y;
    RECT rect;
    string str;

    //define window class
    _wnd_class.style = CS_HREDRAW | CS_VREDRAW;
    _wnd_class.lpfnWndProc = (WNDPROC)window_proc;
    _wnd_class.cbWndExtra = 0;
    _wnd_class.cbClsExtra = 0;
    _wnd_class.hInstance = NULL;
    _wnd_class.hbrBackground = NULL;
    _wnd_class.lpszMenuName = NULL;
    _wnd_class.lpszClassName = name;

    //register class
    if (RegisterClass(&_wnd_class) == 0)
    {
        log(ERR, "Window Class failed to register");
        return 1; //failiure
    }

    //get top left of desired window
    x = (GetSystemMetrics(SM_CXSCREEN) - width) >> 1;
    y = (GetSystemMetrics(SM_CYSCREEN) - height) >> 1;
    _buf_width = width;
    _buf_height = height;
    log(DEBUG, "x: " + to_string(x) + "|y: " + to_string(y));

    //set rect buf
    rect.left = x;
    rect.top = y;
    rect.right = x + width;
    rect.bottom = y + height;
    log(DEBUG, "right: " + to_string(rect.right) + "|bottom: " + to_string(rect.bottom));

    //calculate actual window size
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);

    //get actual window values from modified rect
    _wnd_height = rect.right - rect.left;
    _wnd_width = rect.bottom - rect.top;
    log(DEBUG, "window height: " + to_string(_wnd_height) + "|window width: " + to_string(_wnd_width));

    //create window handle
    _handle = CreateWindowEx(
        0,
        name,
        name,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        x, y, _wnd_width, _wnd_height,
        NULL,
        NULL,
        NULL,
        NULL);
    if (_handle == NULL)
    {
        log(ERR, "Window Handle creation failed");
        return 1;
    }

    //allocate space for client area buffer
    _buf = (PIXEL*)malloc((size_t)_buf_height * (size_t)_buf_width * sizeof(PIXEL));
    if (_buf == NULL)
    {
        log(ERR, "Failed to heap allocate window buffer");
        return 1;
    }

    //allocate bitmap for buffer
    memset(&_bmp_info, 0, sizeof(BITMAPINFO));
    _bmp_info.bmiHeader.biBitCount = 32;
    _bmp_info.bmiHeader.biHeight = -height;
    _bmp_info.bmiHeader.biWidth = width;
    _bmp_info.bmiHeader.biPlanes = 1;
    _bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

    _win_hDC = GetDC(_handle);

    ShowWindow(_handle, SW_SHOW);
    UpdateWindow(_handle);

    return 0;

    //default return
    return 0;
}