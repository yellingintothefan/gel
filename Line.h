#pragma once

#include "Point.h"

typedef struct
{
    Point a, b;
}
Line;

float xlcross(const Line l, const Point p);
