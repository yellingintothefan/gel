#include "Face.h"

Face zindex(const Face f)
{
    const Face face = {{
        f.vertex[0] - 1,
        f.vertex[1] - 1,
        f.vertex[2] - 1,
    }};
    return face;
}

Vertex fnormal(const Face f, const Vertices v)
{
    const Vertex a = v.vertex[f.vertex[0]];
    const Vertex b = v.vertex[f.vertex[1]];
    const Vertex c = v.vertex[f.vertex[2]];
    return vcross(vsub(b, a), vsub(c, a));
}

Triangle fproject(const Face f, const Vertices v)
{
    const Triangle triangle = {
        { v.vertex[f.vertex[0]].x, v.vertex[f.vertex[0]].y },
        { v.vertex[f.vertex[1]].x, v.vertex[f.vertex[1]].y },
        { v.vertex[f.vertex[2]].x, v.vertex[f.vertex[2]].y },
    };
    return triangle;
}
