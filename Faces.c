#include "Faces.h"

Faces fsnew(const int max)
{
    Faces fs = { 0, 0, 0 };
    fs.face = xutoss(Face, max);
    fs.max = max;
    return fs;
}

int fscheck(const char* const line)
{
    return line[0] == 'f';
}

Face fparse(const char* const line)
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

Faces fsload(FILE* const file)
{
    Faces fs = fsnew(1024);
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
            fs.face[fs.count++] = zindex(fparse(line));
        }
        free(line);
    }
    rewind(file);
    return fs;
}

void fskill(const Faces fs)
{
    free(fs.face);
}
