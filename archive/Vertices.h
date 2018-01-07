#pragma once

#include "Vertex.h"

#include <stdio.h>

typedef struct
{
    Vertex* vertex;
    int count;
    int max;
}
Vertices;

Vertices xvsnew(const int max);

Vertices xvsload(FILE* const file);

void xvskill(const Vertices v);
