#pragma once

#include "Triangle.h"
#include "Sdl.h"

#include <stdint.h>

typedef struct
{
    uint32_t* pixels;
    int width;
}
Display;

void xdtriangle(const Display d, const Triangle t, const uint32_t color);

Display xdlock(const Sdl sdl);
