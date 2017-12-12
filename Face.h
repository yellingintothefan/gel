#pragma once

#include "Triangle.h"
#include "Vertices.h"

typedef struct
{
    int vertex[3];
}
Face;

Vertex xfnormal(const Face f, const Vertices v);

Triangle xfproject(const Face f, const Vertices v);
