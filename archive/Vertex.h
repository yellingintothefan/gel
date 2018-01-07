#pragma once

typedef struct
{
    float x, y, z;
}
Vertex;

Vertex xvsub(const Vertex a, const Vertex b);

Vertex xvcross(const Vertex a, const Vertex b);

float xvmag(const Vertex v);

Vertex xvmul(const Vertex v, const float n);

float xvdot(const Vertex a, const Vertex b);

Vertex xvnormalized(const Vertex v);
