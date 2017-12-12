#include "Display.h"

void dtriangle(const Display d, const Triangle t, const uint32_t color)
{
    const Line l = lencloset(t);
    // Reverse <i> and <j> because SDL will churn 90 degrees with render.
    for(int j = l.a.x; j < l.b.x; j++)
        for(int i = l.a.y; i < l.b.y; i++)
        {
            const Point p = { j, i };
            if(tinside(t, p)) d.pixels[i + j * d.width] = color;
        }
}
