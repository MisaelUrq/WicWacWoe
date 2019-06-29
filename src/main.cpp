#include <windows.h>

#include "common.h"

LRESULT WINAPI Win32WindowsProc(HWND, UINT, WPARAM, LPARAM);

static bool global_is_app_running;

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
            }

        }
        else
        {
            OutputDebugStringA("Could not create the window.");
        }
    }
    else
    {
        OutputDebugStringA("Could not register the window..");
    }

    return 0;
}

LRESULT WINAPI Win32WindowsProc(HWND handle, UINT msg, WPARAM w_param, LPARAM l_param)
{
    return DefWindowProc(handle, msg, w_param, l_param);
}
