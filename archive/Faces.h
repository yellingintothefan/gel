#pragma once

#include "Face.h"

typedef struct
{
    Face* face;
    int count;
    int max;
}
Faces;

Faces xfsnew(const int max);

Faces xfsload(FILE* const file);

void xfskill(const Faces fs);
