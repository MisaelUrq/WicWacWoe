#include <windows.h>

#include <vector>
#include "common.h"
#include "math.cpp"

LRESULT WINAPI Win32WindowsProc(HWND, UINT, WPARAM, LPARAM);

#define BYTES_PER_PIXEL 4

struct PlatformInput {
    v2f mouse_position;
    u32 mouse_events;

    bool is_down;
    bool is_up;
    bool is_left;
    bool is_right;
    bool spacebar;
    bool restart;

    PlatformInput() {
        memset(this, 0, sizeof(PlatformInput));
    }
};

struct PlatformScreenBuffer
{
    void* memory;
    u32   width;
    u32   height;
    u32   pitch;

    PlatformScreenBuffer(const u32 width, const u32 height)
    {
        this->width  = width;
        this->height = height;
        this->pitch  = width*BYTES_PER_PIXEL;
        this->memory = operator new(width*height*BYTES_PER_PIXEL);
    }

    ~PlatformScreenBuffer() {
        operator delete(this->memory);
    }

    virtual void Resize(const u32 w, const u32 h) {
        this->width  = w;
        this->height = h;
        this->pitch  = width*BYTES_PER_PIXEL;
        operator delete(this->memory);
        this->memory = operator new(width*height*BYTES_PER_PIXEL);
    }
};

struct Win32ScreenBuffer : PlatformScreenBuffer
{
    BITMAPINFO info;

    Win32ScreenBuffer(const u32 width, const u32 height) :
    PlatformScreenBuffer(width, height)
    {
        info.bmiHeader.biSize = sizeof(info.bmiHeader);
        info.bmiHeader.biWidth  = width;
        info.bmiHeader.biHeight = -(i32)height;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
    }

    // TODO(Misael): Does this actually works?
    void Resize(const u32 w, const u32 h) override {
        PlatformScreenBuffer::Resize(w, h);
        info.bmiHeader.biWidth  = w;
        info.bmiHeader.biHeight = -(i32)h;
    }
};

struct Line {
    v2f p1;
    v2f p2;
    u32 color;
};

struct Renderer {
    PlatformScreenBuffer* screen_buffer;
    std::vector<Line>     lines;

    Renderer(PlatformScreenBuffer* screen_buffer)
    {
        this->screen_buffer = screen_buffer;;
    }

    void StartFrame() {
        lines.clear();
        Clear(0xFF111111);
    }

    // TODO(Misael): This really should not be name like this, this is
    // more of a render call. But maybe here it's when sorting or some
    // other stuff can happen
    void EndFrame() {
        for (auto iter = lines.begin(); iter != lines.end(); iter++) {
            DrawLine(*iter);
        }
    }

    void PushSquare(v2f p, u32 d, u32 color) {
        PushLine(v2f(p.x - d, p.y - d), v2f(p.x - d, p.y + d), color);
        PushLine(v2f(p.x - d, p.y + d), v2f(p.x + d, p.y + d), color);
        PushLine(v2f(p.x + d, p.y + d), v2f(p.x + d, p.y - d), color);
        PushLine(v2f(p.x + d, p.y - d), v2f(p.x - d, p.y - d), color);
    }

    void PushCross(v2f p, u32 d, u32 color) {
        PushLine(v2f(p.x - d, p.y - d), v2f(p.x + d, p.y + d), color);
        PushLine(v2f(p.x - d, p.y + d), v2f(p.x + d, p.y - d), color);
    }

    void PushLine(v2f p1, v2f p2, u32 color) {
        lines.push_back({p1, p2, color});
    }

    void DrawPoint(f32 x, f32 y, u32 color) {
        PlatformScreenBuffer* buffer = screen_buffer;
        u32 *pixel = (u32*)buffer->memory;
        if (x < buffer->width && x >= 0 && y >= 0 && y < buffer->height) {
            pixel[(u32)y*buffer->width+(u32)x] = color;
        }
    }

    // TODO(Misael): This method is not that great, lock for a beter
    // one.
    void DrawLine(const Line& line) {
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
                DrawPoint((f32)index, y, line.color);
            }
        } else {
            if (p1->y > p2->y) {
                std::swap(p1, p2);
            }
            m = (p1->y != p2->y) ? (p2->x - p1->x) / (p2->y - p1->y) : 0.0f;
            const f32 b = p1->x - m * p1->y;
            FOR_RANGE((u32)p1->y, (u32)p2->y) {
                f32 x = m * (f32)index + b;
                DrawPoint(x, (f32)index, line.color);
            }
        }
    }

    private:
    // TODO(Misael): Use SIMD instrucctions to accelerate this.
    void Clear(const u32 color) {
        u32* pixel = (u32*)screen_buffer->memory;
        for (u32 y = 0; y < screen_buffer->height; y++) {
            for (u32 x = 0; x < screen_buffer->width; x++) {
                *pixel = color;
                pixel++;
            }
        }
    }
};

struct Game {
    Renderer renderer;
    PlatformInput* input;
    u32      board_pieces[9];
    i32      current_position;
    u32      current_piece;
    bool     is_game_done;
    Game(PlatformScreenBuffer* screen_buffer, PlatformInput* input) : renderer(Renderer(screen_buffer))
    {
        this->input = input;
        this->current_position =  4;
        this->current_piece    =  1;
        this->is_game_done     =  false;
        memset(board_pieces, 0, sizeof(board_pieces));
    }

    v2f GetPositionFromBoard(u32 position) {
        f32 w = (f32)renderer.screen_buffer->width;
        f32 h = (f32)renderer.screen_buffer->height;
        f32 x = w / 2.0f;
        f32 y = h / 2.0f;
        if (position % 3 != 1) {
            x /= 2.0f;
            x += position % 3 * x;
        }
        if (position < 3 || position > 5) {
            y /= 2.0f;
        }
        if (position > 5) {
            y += h / 2.0f;
        }
        return v2f(x, y);
    }

    void AddSquare(u32 position) {
        renderer.PushSquare(GetPositionFromBoard(position), 50, 0xFF0000FF);
    }

    void AddCross(u32 position) {
        renderer.PushCross(GetPositionFromBoard(position), 50, 0xFFFF0000);
    }

    void AddPieceToBoard(u32 position, u32 type) {
        board_pieces[position] = type;
    }

    void UpdateLogic() {
        if (input->restart) {
            memset(board_pieces, 0, sizeof(board_pieces));
            current_piece = 1;
            current_position = 4;
            is_game_done   = false;
            input->restart = false;
        }

        if (is_game_done == false) {
            if (input->is_left) {
                if (current_position > 0) {
                    for (i32 index = current_position-1; index >= 0; index--) {
                        if (board_pieces[index] == 0)  {
                            current_position = index;
                            break;
                        }
                    }
                }
                input->is_left = false;
            }
            if (input->is_right) {
                if (current_position < 8) {
                    FOR_RANGE(current_position+1, ArrayCount(board_pieces)) {
                        if (board_pieces[index] == 0)  {
                            current_position = index;
                            break;
                        }
                    }
                }
                input->is_right = false;
            }

            if (input->is_up) {
                if (current_position > 2) {
                    if (board_pieces[current_position-3] == 0) {
                        current_position = current_position-3;
                    } else if (current_position > 6 && board_pieces[current_position-6] == 0) {
                        current_position = current_position-6;
                    }
                }
                input->is_up = false;
            }
            if (input->is_down) {
                if (current_position < 6) {
                    if (board_pieces[current_position+3] == 0) {
                        current_position = current_position+3;
                    } else if (current_position < 3 && board_pieces[current_position+6] == 0) {
                        current_position = current_position+6;
                    }
                }
                input->is_down = false;
            }

            if (input->spacebar) {
                board_pieces[current_position] = current_piece;
                current_piece = (current_piece == 1) ? 2 : 1;
                if (current_position < 8) {
                    FOR_RANGE(current_position+1, ArrayCount(board_pieces)) {
                        if (board_pieces[index] == 0)  {
                            current_position = index;
                            break;
                        }
                    }
                }
                if (board_pieces[current_position] != 0) {
                    if (current_position > 0) {
                        for (i32 index = current_position-1; index >= 0; index--) {
                            if (board_pieces[index] == 0)  {
                                current_position = index;
                                break;
                            }
                        }
                    }
                }
                input->spacebar = false;
            }
        }
        if (CheckForWinCases()) {
            is_game_done = true;
        }
    }

    bool CheckForWinCases() {
        FOR_RANGE(0, 3) {
            u32 a = index * 3;
            u32 b = a + 1;
            u32 c = a + 2;
            u32 bit = (board_pieces[a] & board_pieces[b] & board_pieces[c]);
            if (bit < 3 && bit > 0) {
                renderer.PushLine(GetPositionFromBoard(index*3), GetPositionFromBoard(index*3+2), 0xFF00FF00);
                return true;
            }
        }
        FOR_RANGE(0, 3) {
            u32 a = index;
            u32 b = index + 3;
            u32 c = index + 6;
            u32 bit = (board_pieces[a] & board_pieces[b] & board_pieces[c]);
            if (bit < 3 && bit > 0) {
                renderer.PushLine(GetPositionFromBoard(index), GetPositionFromBoard(index+6), 0xFF00FF00);
                return true;
            }
        }
        {
            u32 bit = (board_pieces[0] & board_pieces[4] & board_pieces[8]);
            if (bit < 3 && bit > 0) {
                renderer.PushLine(GetPositionFromBoard(0), GetPositionFromBoard(8), 0xFF00FF00);
                return true;
            }
        }
        {
            u32 bit = (board_pieces[2] & board_pieces[4] & board_pieces[6]);
            if (bit < 3 && bit > 0) {
                renderer.PushLine(GetPositionFromBoard(2), GetPositionFromBoard(6), 0xFF00FF00);
                return true;
            }
        }
        FOR_RANGE(0, ArrayCount(board_pieces)) {
            if (board_pieces[index] == 0) {
                return false;
            }
        }
        return true;
    }

    void UpdateFrame() {
        renderer.StartFrame();
        PushBoard();
        if (current_position >= 0 && current_position < 9) {
            if (is_game_done == false) {
                if (current_piece == 1) {
                    AddCross(current_position);
                } else if (current_piece == 2){
                    AddSquare(current_position);
                }
            }
        }
        FOR_RANGE(0, ArrayCount(board_pieces)) {
            if (board_pieces[index] == 1) {
                AddCross(index);
            } else if (board_pieces[index] == 2){
                AddSquare(index);
            }
        }
        renderer.PushCross(input->mouse_position, 100, 0xFF00FF00);
        renderer.EndFrame();
    }

    void PushBoard() {
        FOR_RANGE(0, 2) {
            f32 y = (renderer.screen_buffer->height / 3.0f) * (f32)(index+1);
            renderer.PushLine({0, y},{(f32)renderer.screen_buffer->width, y}, 0xFFAAAAAA);
        }
        FOR_RANGE(0, 2) {
            f32 x = (renderer.screen_buffer->width / 3.0f) * (f32)(index+1);
            renderer.PushLine({(f32)x, 0} , {x, (f32)renderer.screen_buffer->height}, 0xFFAAAAAA);
        }
    }
};

global_var Win32ScreenBuffer *global_offscreen_buffer;
global_var bool global_is_app_running;
global_var bool global_ctr_key_was_down;
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
            global_offscreen_buffer = new Win32ScreenBuffer(window_width, window_height);
            PlatformInput* input = new PlatformInput();
            Game game = Game(global_offscreen_buffer, input);
            global_is_app_running = true;
            ShowWindow(window_handle, SW_SHOWDEFAULT);
            UpdateWindow(window_handle);

            while (global_is_app_running)
            {
                {
                    MSG msg = {};
                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                    {
                        switch (msg.message)
                        {

                            case WM_QUIT:
                            {
                                global_is_app_running = false;
                            } break;
                            case WM_SYSKEYDOWN:
                            case WM_SYSKEYUP:
                            case WM_KEYDOWN:
                            case WM_KEYUP:
                            {
                                u32 vkcode = (u32)msg.wParam;
                                b32 was_down = ((msg.lParam & (1 << 30)) != 0);
                                b32 is_down  = ((msg.lParam & (1 << 31)) == 0);
                                bool alt_key_was_down = (msg.lParam & (1 << 29));

                                if (!alt_key_was_down && vkcode == VK_CONTROL) {
                                    MSG next_msg;
                                    if (PeekMessage(&next_msg, NULL, 0, 0, PM_NOREMOVE) && next_msg.message == msg.message) {
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
                                                    input->is_up = is_down;
                                                }
                                                break;
                                                case VK_DOWN:
                                                {
                                                    input->is_down = is_down;
                                                }
                                                break;
                                                case VK_RIGHT:
                                                {
                                                    input->is_right = is_down;
                                                }
                                                break;
                                                case VK_LEFT:
                                                {
                                                    input->is_left = is_down;
                                                }
                                                break;
                                                case VK_SPACE:
                                                {
                                                    input->spacebar = is_down;
                                                }
                                                break;
                                                case VK_ESCAPE:
                                                {
                                                    input->restart = is_down;
                                                }
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    POINT MouseP;
                    GetCursorPos(&MouseP);
                    ScreenToClient(window_handle, &MouseP);
                    input->mouse_position = {(f32)MouseP.x, (f32)MouseP.y};
                }

                game.UpdateLogic();
                game.UpdateFrame();

                HDC dc = GetDC(window_handle);
                StretchDIBits(dc,
                              0, 0, 1280, 780,
                              0, 0, global_offscreen_buffer->width, global_offscreen_buffer->height,
                              global_offscreen_buffer->memory, &global_offscreen_buffer->info, DIB_RGB_COLORS, SRCCOPY);
                ReleaseDC(window_handle, dc);
            }

            delete input;
            delete global_offscreen_buffer;
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


LRESULT WINAPI Win32WindowsProc(HWND handle, UINT msg, WPARAM w_param, LPARAM l_param)
{
    switch (msg)
    {
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
    case WM_SETCURSOR:
    {
        SetCursor(0);
    } break;
    case WM_CLOSE:
        global_is_app_running = false;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(handle, msg, w_param, l_param);
}
