#include "Line.h"

/* Technically a cross product with the Z removed. */
float lcross(const Line l, const Point p)
{
    return (l.b.x - l.a.x) * (p.y - l.a.y) - (l.b.y - l.a.y) * (p.x - l.a.x);
}
