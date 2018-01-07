#include "Vertices.h"

#include "util.h"

Vertices xvsnew(const int max)
{
    Vertices v;
    xuzero(v);
    v.vertex = xutoss(Vertex, max);
    v.max = max;
    return v;
}

static int vscheck(const char* const line)
{
    return line[0] == 'v' && line[1] != 't' && line[1] != 'n';
}

static Vertex vparse(const char* const line)
{
    Vertex v = { 0.0, 0.0, 0.0 };
    sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
    return v;
}

Vertices xvsload(FILE* const file)
{
    Vertices v = xvsnew(1024); /* Arbitrary size. */
    const int lines = xulns(file);
    for(int i = 0; i < lines; i++)
    {
        char* line = xureadln(file);
        if(vscheck(line))
        {
            // Growing.
            if(v.count == v.max)
                xuretoss(v.vertex, Vertex, v.max *= 2);
            // Appending.
            v.vertex[v.count++] = vparse(line);;
        }
        free(line);
    }
    rewind(file);
    return v;
}

void xvskill(const Vertices v)
{
    free(v.vertex);
}
