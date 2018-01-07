#include "Faces.h"

#include "util.h"

Faces xfsnew(const int max)
{
    Faces fs = { 0, 0, 0 };
    fs.face = xutoss(Face, max);
    fs.max = max;
    return fs;
}

static int fscheck(const char* const line)
{
    return line[0] == 'f';
}

static Face fsparse(const char* const line)
{
    Face face;
    xuzero(face);
    int waste;
    sscanf(
        line,
        "f %d/%d/%d %d/%d/%d %d/%d/%d",
        &face.vertex[0], &waste, &waste,
        &face.vertex[1], &waste, &waste,
        &face.vertex[2], &waste, &waste);
    return face;
}

static Face fsindex(const Face f)
{
    const Face face = {{
        f.vertex[0] - 1,
            f.vertex[1] - 1,
            f.vertex[2] - 1,
    }};
    return face;
}

Faces xfsload(FILE* const file)
{
    Faces fs = xfsnew(1024);
    const int lines = xulns(file);
    for(int i = 0; i < lines; i++)
    {
        char* line = xureadln(file);
        if(fscheck(line))
        {
            // Growing.
            if(fs.count == fs.max)
                xuretoss(fs.face, Face, fs.max *= 2);
            // Appending.
            fs.face[fs.count++] = fsindex(fsparse(line));
        }
        free(line);
    }
    rewind(file);
    return fs;
}

void xfskill(const Faces fs)
{
    free(fs.face);
}
