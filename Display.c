#include "Display.h"

void xdtriangle(const Display d, const Triangle t, const uint32_t color)
{
    const Line l = xtenclose(t);
    // Reverse <i> and <j> because SDL will churn 90 degrees with render.
    for(int j = l.a.x; j < l.b.x; j++)
    for(int i = l.a.y; i < l.b.y; i++)
    {
        const Point p = { j, i };
        if(xtinside(t, p)) d.pixels[i + j * d.width] = color;
    }
}

Display xdlock(const Sdl sdl)
{
    void* screen;
    int pitch;
    SDL_LockTexture(sdl.canvas, NULL, &screen, &pitch);
    const int width = pitch / sizeof(uint32_t);
    const Display display = { (uint32_t*) screen, width };
    return display;
}
