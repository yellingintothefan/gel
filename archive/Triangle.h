#pragma once

#include "Point.h"
#include "Line.h"

typedef struct
{
    Point a, b, c;
}
Triangle;

int xtinside(const Triangle t, const Point p);

Line xtenclose(const Triangle t);

Triangle xtscale(const Triangle t, const int xres, const int yres);
