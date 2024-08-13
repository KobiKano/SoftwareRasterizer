/*
* Credit for a lot of this code goes to https://github.com/TanukiSharp/SoftwareRasterizer/blob/master/RenderLibrary/renderlib.c
*/

#include "window.h"
#include "../logger/logger.h"
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <mutex>
#include <math.h>
#include <stdlib.h>

using namespace std;

//global defs
volatile bool UP_KEY = false;
volatile bool DOWN_KEY = false;
volatile bool LEFT_KEY = false;
volatile bool RIGHT_KEY = false;
volatile bool W_KEY = false;
volatile bool A_KEY = false;
volatile bool S_KEY = false;
volatile bool D_KEY = false;
volatile bool Z_KEY = false;
volatile bool C_KEY = false;
volatile bool g_alive = false;
volatile bool g_exit_error = false;
volatile bool g_resize = false;

static mutex _buf_lk;

static volatile bool _draw_locked = false;

static HWND _handle;
static HDC _win_hDC;
static WNDCLASS _wnd_class;

static int _buf_width = -1;
static int _buf_height = -1;

static int _frames = -1;
static FILETIME _start_frame_time = { 0 };
static FILETIME _start_total_time = { 0 };

static PIXEL* _buf = NULL;  //heap allocated array of uint32 for each pixel on screen to hold color value
static float* _z_buf = NULL;  //z buffer
static BITMAPINFO _bmp_info;

/*************************************************************************************
* Getters for private vals to be used in draw.cpp (avoiding file clutter)
*************************************************************************************/
bool get_draw_locked() { return _draw_locked; }
int get_buf_width() { return _buf_width; }
int get_buf_height() { return _buf_height; }
PIXEL* get_buf() { return _buf; }
float* get_z_buf() { return _z_buf; }

/*
* Resize operations
*/
static bool resize(int width, int height)
{
    log(DEBUG1, "resize");

    //aquire lock so buffer not modified or cleared while resizing
    _buf_lk.lock();

    //assume width and height are for buffer
    _buf_width = width;
    _buf_height = height;

    //reallocate buffer
    free(_buf);
    free(_z_buf);
    _buf = (PIXEL*)calloc((size_t)_buf_height * (size_t)_buf_width, sizeof(PIXEL));
    _z_buf = (float*)malloc((size_t)_buf_height * (size_t)_buf_width * sizeof(float));
    if (_buf == NULL || _z_buf == NULL)
    {
        log(ERR, "Failed to heap allocate window buffer");
        _buf_lk.unlock();
        return false;
    }

    //very annoying but have to manually set all values to float 1.0, but compiler will optimize
    for (size_t i = 0; i < (size_t)_buf_height * (size_t)_buf_width; i++)
    {
        _z_buf[i] = 1.f;
    }

    //modify bitmap
    _bmp_info.bmiHeader.biHeight = -height;
    _bmp_info.bmiHeader.biWidth = width;

    //unlock and return success
    _buf_lk.unlock();
    g_resize = true;
    return true;
}


/*
* Defines functionality for window events
*/
LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        g_alive = false;
    }
    return 0;

    case WM_PAINT:
    {
        //aquire lock
        _buf_lk.lock();
        if (_buf == NULL)
        {
            log(ERR, "buffer not allocated");
            g_exit_error = true;
            _buf_lk.unlock();
            SendMessage(_handle, WM_DESTROY, 0, 0);
            return 0;
        }

        RECT wsize;
        GetClientRect(hwnd, &wsize);
        StretchDIBits(_win_hDC, 0, 0, wsize.right, wsize.bottom, 0, 0, _buf_width, _buf_height, _buf, &_bmp_info, DIB_RGB_COLORS, SRCCOPY);
        ValidateRect(_handle, NULL);
        //release lock
        _buf_lk.unlock();
    }
    return 0;
    case WM_SIZE:
    {
        int width = LOWORD(lParam);  // Macro to get the low-order word.
        int height = HIWORD(lParam); // Macro to get the high-order word.

        //width and height are of client area (buffer)
        if (!resize(width, height))
        {
            g_exit_error = true;
            SendMessage(_handle, WM_DESTROY, 0, 0);
        }
    }
    return 0;
    case WM_KEYDOWN:
    {
        if (GetKeyState(VK_UP) & 0x8000)
        {
            UP_KEY = true;
        }
        if (GetKeyState(VK_DOWN) & 0x8000)
        {
            DOWN_KEY = true;
        }
        if (GetKeyState(VK_LEFT) & 0x8000)
        {
            LEFT_KEY = true;
        }
        if (GetKeyState(VK_RIGHT) & 0x8000)
        {
            RIGHT_KEY = true;
        }
        if (GetKeyState(0x57) & 0x8000) //W key
        {
            W_KEY = true;
        }
        if (GetKeyState(0x41) & 0x8000)  //A key
        {
            A_KEY = true;
        }
        if (GetKeyState(0x53) & 0x8000) //S key
        {
            S_KEY = true;
        }
        if (GetKeyState(0x44) & 0x8000) //D key
        {
            D_KEY = true;
        }
        if (GetKeyState(0x5A) & 0x8000) //Z key
        {
            Z_KEY = true;
        }
        if (GetKeyState(0x43) & 0x8000) //C key
        {
            C_KEY = true;
        }
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
    log(DEBUG1, "x: " + to_string(x) + "|y: " + to_string(y));

    //set rect buf
    rect.left = x;
    rect.top = y;
    rect.right = x + width;
    rect.bottom = y + height;
    log(DEBUG1, "right: " + to_string(rect.right) + "|bottom: " + to_string(rect.bottom));

    //calculate actual window size
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);

    _buf_width = rect.right - rect.left;
    _buf_height = rect.bottom - rect.top;
    log(DEBUG1, "width: " + to_string(_buf_width) + "|height: " + to_string(_buf_height));

    //create window handle
    _handle = CreateWindowEx(
        0,
        name,
        name,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        x, y, _buf_width, _buf_height,
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
    _buf = (PIXEL*)calloc((size_t)_buf_height * (size_t)_buf_width, sizeof(PIXEL));
    _z_buf = (float*)malloc((size_t)_buf_height * (size_t)_buf_width * sizeof(float));
    if (_buf == NULL || _z_buf == NULL)
    {
        log(ERR, "Failed to heap allocate window buffer");
        return 1;
    }
    //very annoying but have to manually set all values to float 1.0, but compiler will optimize
    for (size_t i = 0; i < (size_t)_buf_height * (size_t)_buf_width; i++)
    {
        _z_buf[i] = 1.f;
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

    //default return
    g_alive = true;
    return 0;
}

/*
* Sends paint message to window
*/
void window_update()
{
    MSG msg;

    log(DEBUG1, "Painting");
    SendMessage(_handle, WM_PAINT, 0, 0);

    while (PeekMessage(&msg, _handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

/*
* Clear buffer to only black
*/
void window_clear()
{
    //aquire lock to make sure resizing cannot occur during modification
    _buf_lk.lock();
    log(DEBUG1, "clearing window");
    if (_buf == NULL || _z_buf == NULL)
    {
        log(ERR, "buffer not allocated");
        g_exit_error = true;
        _buf_lk.unlock();
        SendMessage(_handle, WM_DESTROY, 0, 0);
        return;
    }

    memset((void*)_buf, 0, (size_t)_buf_width * (size_t)_buf_height * sizeof(PIXEL));
    //very annoying but have to manually set all values to float 1.0, but compiler will optimize
    for (size_t i = 0; i < (size_t)_buf_height * (size_t)_buf_width; i++)
    {
        _z_buf[i] = 1.f;
    }

    //unlock and return succeess
    _buf_lk.unlock();
}

/*
* Locks buffer from being changed while getting next frame
* It is up to user to use properly
*/
void draw_lock()
{
    log(DEBUG1, "locking for draw");
    if (!_draw_locked)
    {
        _buf_lk.lock();
        _draw_locked = true;
    }
    else
        log(WARNING, "attempted to lock locked draw lock");
}
void draw_unlock()
{
    log(DEBUG1, "unlocking for draw");
    if (_draw_locked)
    {
        _buf_lk.unlock();
        _draw_locked = false;
    }
    else
        log(WARNING, "attempted to unlock untaken draw lock");
}

/*
* Frees window memory
*/
void window_remove()
{
    log(DEBUG1, "freeing window");
    ReleaseDC(_handle, _win_hDC);
    DestroyWindow(_handle);
    free(_buf);
}

/*
* Sync window with rasterizer
*/
void window_sync_begin()
{
    log(DEBUG1, "sync begin");
    GetSystemTimePreciseAsFileTime(&_start_frame_time);

    if (_start_total_time.dwHighDateTime == 0 && _start_total_time.dwLowDateTime == 0)
        _start_total_time = _start_frame_time;
}

/*
* Sleep window to sync to max fps
*/
static void window_sleep(long long time)
{
    log(DEBUG1, "sleeping window");
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10 * time);

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (timer == 0)
    {
        printf("Call to CreateWaitableTimer failed.\n");
        exit(1);
    }

    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}

/*
* End window sync
*/
void window_sync_end(int fps_cap, bool print_fps)
{
    log(DEBUG1, "sync end");

    FILETIME ft;
    ULARGE_INTEGER start_frame_time;
    ULARGE_INTEGER current_frame_time;
    ULARGE_INTEGER start_total_time;

    GetSystemTimePreciseAsFileTime(&ft);

    start_frame_time.LowPart = _start_frame_time.dwLowDateTime;
    start_frame_time.HighPart = _start_frame_time.dwHighDateTime;

    current_frame_time.LowPart = ft.dwLowDateTime;
    current_frame_time.HighPart = ft.dwHighDateTime;

    start_total_time.LowPart = _start_total_time.dwLowDateTime;
    start_total_time.HighPart = _start_total_time.dwHighDateTime;

    //check if capping fps
    if (fps_cap != 0)
        window_sleep((1000 * 1000 / fps_cap) - ((current_frame_time.QuadPart - start_frame_time.QuadPart) / 10));
        
    if (!print_fps)
        return;

    if (_frames < 0)
        _frames = 1;
    else
        _frames++;

    if (current_frame_time.QuadPart - start_total_time.QuadPart >= 10000000) //1 second
    {
        printf("fps: %d\n", _frames);
        _start_total_time = ft;
        _frames = 0;
    }
}