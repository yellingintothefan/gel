#include "Triangle.h"

int tinside(const Triangle t, const Point p)
{
    const Line a = { t.a, t.b }, b = { t.b, t.c }, c = { t.c, t.a };
    return (lcross(a, p) > 0.0 && lcross(b, p) > 0.0 && lcross(c, p) > 0.0)
        || (lcross(a, p) < 0.0 && lcross(b, p) < 0.0 && lcross(c, p) < 0.0);
}

Line tenclose(const Triangle t)
{
    const Line line = {
        { xumin(t.a.x, xumin(t.b.x, t.c.x)), xumin(t.a.y, xumin(t.b.y, t.c.y)) },
        { xumax(t.a.x, xumax(t.b.x, t.c.x)), xumax(t.a.y, xumax(t.b.y, t.c.y)) },
    };
    return line;
}

Triangle tscale(const Triangle t, const int xres, const int yres)
{
    const Triangle triangle = {
        { (xres / 2) * (t.a.x + 1.0), (yres / 2) * (t.a.y + 1.0) },
        { (xres / 2) * (t.b.x + 1.0), (yres / 2) * (t.b.y + 1.0) },
        { (xres / 2) * (t.c.x + 1.0), (yres / 2) * (t.c.y + 1.0) },
    };
    return triangle;
}
