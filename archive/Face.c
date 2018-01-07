#include "Face.h"

Vertex xfnormal(const Face f, const Vertices v)
{
    const Vertex a = v.vertex[f.vertex[0]];
    const Vertex b = v.vertex[f.vertex[1]];
    const Vertex c = v.vertex[f.vertex[2]];
    return xvcross(xvsub(b, a), xvsub(c, a));
}

Triangle xfproject(const Face f, const Vertices v)
{
    const Triangle triangle = {
        { v.vertex[f.vertex[0]].x, v.vertex[f.vertex[0]].y },
        { v.vertex[f.vertex[1]].x, v.vertex[f.vertex[1]].y },
        { v.vertex[f.vertex[2]].x, v.vertex[f.vertex[2]].y },
    };
    return triangle;
}
