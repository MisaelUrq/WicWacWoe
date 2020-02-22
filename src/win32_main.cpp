#include <windows.h>

#include "common.h"

LRESULT WINAPI Win32WindowsProc(HWND, UINT, WPARAM, LPARAM);

#define BYTES_PER_PIXEL 4

struct Win32ScreenBuffer
{
    BITMAPINFO info;
    void *memory;
    u32   width;
    u32   height;
    u32   pitch;

    Win32ScreenBuffer(const u32 width, const u32 height) {
        this->width  = width;
        this->height = height;
        this->pitch  = width*BYTES_PER_PIXEL;

        info.bmiHeader.biSize = sizeof(info.bmiHeader);
        info.bmiHeader.biWidth  = width;
        info.bmiHeader.biHeight = -(i32)height;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;

        this->memory = operator new(width*height*BYTES_PER_PIXEL);
    }

    ~Win32ScreenBuffer() {
        operator delete(this->memory);
    }

    void Resize(const u32 w, const u32 h) {
        this->width  = w;
        this->height = h;
        this->pitch  = width*BYTES_PER_PIXEL;

        info.bmiHeader.biWidth  = w;
        info.bmiHeader.biHeight = -(i32)height;

        operator delete(this->memory);
        this->memory = operator new(width*height*BYTES_PER_PIXEL);
    }
};

struct PlatformScreenBuffer : Win32ScreenBuffer {
    PlatformScreenBuffer(const u32 width, const u32 height) :
        Win32ScreenBuffer(width, height)
    {
    }
};

struct Game {
    PlatformScreenBuffer screen_buffer;

    Game(u32 window_width, u32 window_height) :
        screen_buffer(PlatformScreenBuffer(window_width, window_height))
    {
    }
};

void PlatformDrawLine(PlatformScreenBuffer* buffer, f32 x1, f32 y1, f32 x2, f32 y2)
{
    u32 *pixel = (u32*)buffer->memory;

    const f32 m = (y2 - y1) / (x2 - x1);
    const f32 b = y1 - m * x1;
    FOR_RANGE((u32)x1, (u32)x2) {
        f32 y = m * (f32)index + b;
        pixel[(u32)y*buffer->width+index] = 0xFF00FF00;
    }
}

global_var Win32ScreenBuffer *global_offscreen_buffer;
global_var bool global_is_app_running;
int WinMain(HINSTANCE instance, HINSTANCE ,
            LPSTR     , int      )
{
    WNDCLASSEX wc;

    wc.cbSize         = sizeof(WNDCLASSEX);
    wc.style          = CS_CLASSDC;
    wc.lpfnWndProc    = Win32WindowsProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hInstance      = instance;
    wc.hIcon          = NULL;
    wc.hCursor        = NULL;
    wc.hbrBackground  = NULL;
    wc.lpszMenuName   = NULL;
    wc.lpszClassName  = "WicWacWoe_window_class";
    wc.hIconSm        = NULL;

    if (RegisterClassEx(&wc) != 0)
    {
        u32 window_width  = 1280;
        u32 window_height = 780;
        HWND window_handle = CreateWindowEx(0, wc.lpszClassName, "WicWacWoe",
                                            WS_OVERLAPPEDWINDOW, 0, 0, window_width, window_height,
                                            NULL, NULL, instance, NULL);

        if (window_handle)
        {
            Game game = Game(window_width, window_height);
            global_offscreen_buffer = &game.screen_buffer;
            global_is_app_running = true;
            ShowWindow(window_handle, SW_SHOWDEFAULT);
            UpdateWindow(window_handle);

            while (global_is_app_running)
            {
                {
                    MSG msg = {};
                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                PlatformDrawLine(&game.screen_buffer, 0, 0,
                                 game.screen_buffer.width, game.screen_buffer.height);

                PlatformDrawLine(&game.screen_buffer, 0, game.screen_buffer.height,
                                 game.screen_buffer.width, 0);

                HDC dc = GetDC(window_handle);
                StretchDIBits(dc,
                              0, 0, 1280, 780,
                              0, 0, global_offscreen_buffer->width, global_offscreen_buffer->height,
                              global_offscreen_buffer->memory, &global_offscreen_buffer->info, DIB_RGB_COLORS, SRCCOPY);
                ReleaseDC(window_handle, dc);
            }

            DestroyWindow(window_handle);
            delete global_offscreen_buffer;
        }
        else
        {
            OutputDebugStringA("Could not create the window.");
        }
        UnregisterClass(wc.lpszClassName, instance);
    }
    else
    {
        OutputDebugStringA("Could not register the window..");
    }


    return 0;
}


global_var bool global_ctr_key_was_down;
LRESULT WINAPI Win32WindowsProc(HWND handle, UINT msg, WPARAM w_param, LPARAM l_param)
{
    switch (msg)
    {
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        u32 vkcode = (u32)w_param;
        b32 was_down = ((l_param & (1 << 30)) != 0);
        b32 is_down  = ((l_param & (1 << 31)) == 0);
        bool alt_key_was_down = (l_param & (1 << 29));

        if (!alt_key_was_down && vkcode == VK_CONTROL) {
            MSG next_msg;
            if (PeekMessage(&next_msg, NULL, 0, 0, PM_NOREMOVE) && next_msg.message == msg) {
                // Nothing?
                global_ctr_key_was_down = false;
            } else if (global_ctr_key_was_down && !(is_down)) {
                global_ctr_key_was_down = false;
            }
            else {
                global_ctr_key_was_down = true;
            }
        }

        if(was_down != is_down)
        {
            if (is_down)
            {
                if (global_ctr_key_was_down)
                {
                    // TODO(Misael): Code with ctr presses....
                }
                else if (alt_key_was_down)
                {
                    // TODO(Misael): Alt was down....
                }
                else
                {
                    // TODO(Misael): Normal keys....
                    switch (vkcode)
                    {
                    case VK_UP:
                    {
                    }
                    break;
                    case VK_DOWN:
                    {
                    }
                    break;
                    case VK_RIGHT:
                    {
                    }
                    break;
                    case VK_LEFT:
                    {
                    }
                    break;
                    }
                }
            }
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        HDC dc = BeginPaint(handle, &paint);
        StretchDIBits(dc,
                      0, 0, 1280, 780,
                      0, 0, global_offscreen_buffer->width, global_offscreen_buffer->height,
                      global_offscreen_buffer->memory, &global_offscreen_buffer->info, DIB_RGB_COLORS, SRCCOPY);
        EndPaint(handle, &paint);
    }
    break;
    case WM_CLOSE:
        global_is_app_running = false;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(handle, msg, w_param, l_param);
}
