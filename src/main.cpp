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

            DestroyWindow(window_handle);
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

static bool global_ctr_key_was_down;

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

                }
            }
        }


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
