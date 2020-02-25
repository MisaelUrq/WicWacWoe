#pragma once

struct v2u {
    u32 x;
    u32 y;

    v2u(u32 x, u32 y)  {
        this->x = x;
        this->y = y;
    }
};

struct v2f {
    f32 x;
    f32 y;

    v2f() {
        memset(this, 0, sizeof(v2f));
    }
    v2f(f32 x, f32 y)  {
        this->x = x;
        this->y = y;
    }
};
