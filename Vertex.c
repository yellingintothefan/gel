#include "Vertex.h"

#include <math.h>

Vertex xvsub(const Vertex a, const Vertex b)
{
    const Vertex c = { a.x - b.x, a.y - b.y, a.z - b.z };
    return c;
}

Vertex xvcross(const Vertex a, const Vertex b)
{
    const Vertex c = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
    return c;
}

float xvmag(const Vertex v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vertex xvmul(const Vertex v, const float n)
{
    const Vertex m = { v.x * n, v.y * n, v.z * n };
    return m;
}

float xvdot(const Vertex a, const Vertex b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vertex xvnormalized(const Vertex v)
{
    return xvmul(v, 1.0 / xvmag(v));
}
