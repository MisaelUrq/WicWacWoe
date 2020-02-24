#include <windows.h>

#include "common.h"

#include <vector>

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

struct Line {
    v2f p1;
    v2f p2;
};

struct Renderer {
    PlatformScreenBuffer screen_buffer;
    std::vector<Line>   lines;

    Renderer(const u32 width, const u32 height) :
        screen_buffer(PlatformScreenBuffer(width, height))
    {
    }

    void StartFrame() {
        lines.clear();
        Clear(0xFF111111);
    }

    // TODO(Misael): This really should not be this, this is more of a
    // render call.
    void EndFrame() {
        for (auto iter = lines.begin(); iter != lines.end(); iter++) {
            DrawLine(*iter);
        }
    }

    void PushLine(v2f p1, v2f p2) {
        lines.push_back({p1, p2});
    }

    // TODO(Misael): This method is not that great, lock for a beter
    // one.
    void DrawLine(const Line& line) {
        PlatformScreenBuffer* buffer = &screen_buffer;
        Assert(buffer->memory);
        u32 *pixel = (u32*)buffer->memory;
        const v2f *p1 = &line.p1;
        const v2f *p2 = &line.p2;

        f32 m = (p1->x != p2->x) ? (p2 ->y - p1->y) / (p2->x - p1->x) : 0.0f;

        // TODO(Misael): Add color to this.
        if (p1->x != p2->x && std::abs(m) <= 1.0f) {
            if (p1->x > p2->x) {
                std::swap(p1, p2);
            }
            const f32 b = p1->y - m * p1->x;
            FOR_RANGE((u32)p1->x, (u32)p2->x) {
                f32 y = m * (f32)index + b;
                pixel[(u32)y*buffer->width+index] = 0xFF00FF00;
            }
        } else {
            if (p1->y > p2->y) {
                std::swap(p1, p2);
            }
            m = (p1->y != p2->y) ? (p2->x - p1->x) / (p2->y - p1->y) : 0.0f;
            const f32 b = p1->x - m * p1->y;
            FOR_RANGE((u32)p1->y, (u32)p2->y) {
                f32 x = m * (f32)index + b;
                pixel[index*buffer->width+(u32)x] = 0xFF00FF00;
            }
        }
    }

private:
    // TODO(Misael): Use SIMD instrucctions to accelerate this.
    void Clear(const u32 color) {
        u32* pixel = (u32*)screen_buffer.memory;
        for (u32 y = 0; y < screen_buffer.height; y++) {
            for (u32 x = 0; x < screen_buffer.width; x++) {
                *pixel = color;
                pixel++;
            }
        }
    }
};

struct Game {
    Renderer renderer;

    Game(u32 window_width, u32 window_height) :
        renderer(Renderer(window_width, window_height))
    {
    }

    void UpdateFrame() {
        renderer.StartFrame();
        PushBoard();
        renderer.EndFrame();
    }

    void PushBoard() {
        FOR_RANGE(0, 2) {
            f32 y = (renderer.screen_buffer.height / 3.0f) * (f32)(index+1);
            renderer.PushLine({0, y},{(f32)renderer.screen_buffer.width, y});
        }
        FOR_RANGE(0, 2) {
            f32 x = (renderer.screen_buffer.width / 3.0f) * (f32)(index+1);
            renderer.PushLine({(f32)x, 0} , {x, (f32)renderer.screen_buffer.height});
        }
    }
};

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
            global_offscreen_buffer = &game.renderer.screen_buffer;
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

                game.UpdateFrame();

                HDC dc = GetDC(window_handle);
                StretchDIBits(dc,
                              0, 0, 1280, 780,
                              0, 0, global_offscreen_buffer->width, global_offscreen_buffer->height,
                              global_offscreen_buffer->memory, &global_offscreen_buffer->info, DIB_RGB_COLORS, SRCCOPY);
                ReleaseDC(window_handle, dc);
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
