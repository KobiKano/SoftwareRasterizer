/*
* Credit for a lot of this code goes to https://github.com/TanukiSharp/SoftwareRasterizer/blob/master/RenderLibrary/renderlib.c
*/

#include "window.h"
#include "logger.h"
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <mutex>

using namespace std;

//global defs
volatile bool g_alive = false;
volatile bool g_exit_error = false;
static mutex _draw_lock;

static HWND _handle;
static HDC _win_hDC;
static WNDCLASS _wnd_class;

static int _buf_width = -1;
static int _buf_height = -1;

static int _frames = -1;
static FILETIME _start_frame_time = { 0 };
static FILETIME _start_total_time = { 0 };

static PIXEL* _buf = NULL;  //heap allocated array of uint32 for each pixel on screen to hold color value
static BITMAPINFO _bmp_info;

/*
* Resize operations
*/
static bool resize(int width, int height)
{
    log(DEBUG, "resize");

    //aquire lock so buffer not modified or cleared while resizing
    _draw_lock.lock();

    //assume width and height are for buffer
    _buf_width = width;
    _buf_height = height;

    //reallocate buffer
    void* tmp = realloc((void*)_buf, (size_t)_buf_height * (size_t)_buf_width * sizeof(PIXEL));
    if (tmp == NULL)
    {
        log(ERR, "Failed to heap allocate window buffer");
        _draw_lock.unlock();
        return false;
    }
    _buf = (PIXEL*)tmp;

    //modify bitmap
    _bmp_info.bmiHeader.biHeight = -height;
    _bmp_info.bmiHeader.biWidth = width;

    //unlock and return success
    _draw_lock.unlock();
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
        _draw_lock.lock();
        if (_buf == NULL)
        {
            log(ERR, "buffer not allocated");
            g_exit_error = true;
            _draw_lock.unlock();
            SendMessage(_handle, WM_DESTROY, 0, 0);
            return 0;
        }

        RECT wsize;
        GetClientRect(hwnd, &wsize);
        StretchDIBits(_win_hDC, 0, 0, wsize.right, wsize.bottom, 0, 0, _buf_width, _buf_height, _buf, &_bmp_info, DIB_RGB_COLORS, SRCCOPY);
        ValidateRect(_handle, NULL);
        //release lock
        _draw_lock.unlock();
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
    log(DEBUG, "x: " + to_string(x) + "|y: " + to_string(y));

    //set rect buf
    rect.left = x;
    rect.top = y;
    rect.right = x + width;
    rect.bottom = y + height;
    log(DEBUG, "right: " + to_string(rect.right) + "|bottom: " + to_string(rect.bottom));

    //calculate actual window size
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);

    _buf_width = rect.right - rect.left;
    _buf_height = rect.bottom - rect.top;
    log(DEBUG, "width: " + to_string(_buf_width) + "|height: " + to_string(_buf_height));

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

    log(DEBUG, "Painting");
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
    _draw_lock.lock();
    log(DEBUG, "clearing window");
    if (_buf == NULL)
    {
        log(ERR, "buffer not allocated");
        g_exit_error = true;
        _draw_lock.unlock();
        SendMessage(_handle, WM_DESTROY, 0, 0);
        return;
    }

    memset(_buf, 0, (size_t)_buf_width * (size_t)_buf_height * sizeof(PIXEL));

    //unlock and return succeess
    _draw_lock.unlock();
}

/*
* Frees window memory
*/
void window_remove()
{
    log(DEBUG, "freeing window");
    ReleaseDC(_handle, _win_hDC);
    DestroyWindow(_handle);
    free(_buf);
}

/*
* Sync window with rasterizer
*/
void window_sync_begin()
{
    log(DEBUG, "sync begin");
    GetSystemTimePreciseAsFileTime(&_start_frame_time);

    if (_start_total_time.dwHighDateTime == 0 && _start_total_time.dwLowDateTime == 0)
        _start_total_time = _start_frame_time;
}

/*
* Sleep window to sync to max fps
*/
static void window_sleep(long long time)
{
    log(DEBUG, "sleeping window");
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
    log(DEBUG, "sync end");

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