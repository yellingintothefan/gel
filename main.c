#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <math.h>
#include <SDL2/SDL.h>

#define utoss(t, n) ((t*) malloc((n) * sizeof(t)))

#define uretoss(ptr, t, n) (ptr = (t*) realloc((ptr), (n) * sizeof(t)))

#define uzero(a) (memset(&(a), 0, sizeof(a)))

#define umax(a, b) (((a) > (b)) ? (a) : (b))

#define umin(a, b) (((a) < (b)) ? (a) : (b))

typedef struct
{
    float x;
    float y;
    float z;
}
Vertex;

typedef struct
{
    Vertex* vertex;
    int count;
    int max;
}
Vertices;

typedef struct
{
    int vertex[3];
}
Face;

typedef struct
{
    Face* face;
    int count;
    int max;
}
Faces;

typedef struct
{
    Vertex a;
    Vertex b;
    Vertex c;
}
Triangle;

typedef struct
{
    Triangle* triangle;
    int count;
}
Triangles;

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* canvas;
    int xres;
    int yres;
}
Sdl;

typedef struct
{
    uint32_t* pixels;
    int width;
}
Display;

typedef struct
{
    float* z;
    int xres;
    int yres;
}
Zbuff;

static int ulns(FILE* const file)
{
    int ch = EOF;
    int lines = 0;
    int pc = '\n';
    while((ch = getc(file)) != EOF)
    {
        if(ch == '\n') lines++;
        pc = ch;
    }
    if(pc != '\n') lines++;
    rewind(file);
    return lines;
}

static char* ureadln(FILE* const file)
{
    int ch = EOF;
    int reads = 0;
    int size = 128;
    char* line = utoss(char, size);;
    while((ch = getc(file)) != '\n' && ch != EOF)
    {
        line[reads++] = ch;
        if(reads + 1 == size)
            uretoss(line, char, size *= 2);
    }
    line[reads] = '\0';
    return line;
}

static void ubomb(const char* const message, ...)
{
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    exit(1);
}

static Vertices vsnew(const int max)
{
    Vertices vs;
    uzero(vs);
    vs.vertex = utoss(Vertex, max);
    vs.max = max;
    return vs;
}

static Vertices vsload(FILE* const file)
{
    Vertices vs = vsnew(128);
    const int lines = ulns(file);
    for(int i = 0; i < lines; i++)
    {
        char* line = ureadln(file);
        if(line[0] == 'v'
        && line[1] != 't'
        && line[1] != 'n')
        {
            // Growing.
            if(vs.count == vs.max)
                uretoss(vs.vertex, Vertex, vs.max *= 2);
            // Appending.
            Vertex v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            vs.vertex[vs.count++] = v;
        }
        free(line);
    }
    rewind(file);
    return vs;
}

Faces fsnew(const int max)
{
    Faces fs;
    uzero(fs);
    fs.face = utoss(Face, max);
    fs.max = max;
    return fs;
}

Faces fsload(FILE* const file)
{
    Faces fs = fsnew(128);
    const int lines = ulns(file);
    for(int i = 0; i < lines; i++)
    {
        char* line = ureadln(file);
        if(line[0] == 'f')
        {
            // Growing.
            if(fs.count == fs.max)
                uretoss(fs.face, Face, fs.max *= 2);
            // Appending.
            int waste;
            Face f;
            sscanf(
                line,
                "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &f.vertex[0], &waste, &waste,
                &f.vertex[1], &waste, &waste,
                &f.vertex[2], &waste, &waste);
            const Face indexed = {{
                f.vertex[0] - 1,
                f.vertex[1] - 1,
                f.vertex[2] - 1,
            }};
            fs.face[fs.count++] = indexed;
        }
        free(line);
    }
    rewind(file);
    return fs;
}

Triangles tsgen(const Vertices vs, const Faces fs)
{
    const Triangles ts = { utoss(Triangle, fs.count), fs.count };
    for(int i = 0; i < fs.count; i++)
    {
        const Triangle t = {
            vs.vertex[fs.face[i].vertex[0]],
            vs.vertex[fs.face[i].vertex[1]],
            vs.vertex[fs.face[i].vertex[2]],
        };
        ts.triangle[i] = t;
    }
    return ts;
}

void schurn(const Sdl sdl)
{
    const SDL_Rect dst = {
        (sdl.xres - sdl.yres) / 2,
        (sdl.yres - sdl.xres) / 2,
        sdl.yres, sdl.xres
    };
    SDL_RenderCopyEx(sdl.renderer, sdl.canvas, NULL, &dst, -90, NULL, SDL_FLIP_NONE);
}

void spresent(const Sdl sdl)
{
    SDL_RenderPresent(sdl.renderer);
}

Sdl ssetup(const int xres, const int yres)
{
    SDL_Init(SDL_INIT_VIDEO);
    Sdl sdl;
    uzero(sdl);
    sdl.window = SDL_CreateWindow("water", 0, 0, xres, yres, SDL_WINDOW_SHOWN);
    sdl.renderer = SDL_CreateRenderer(sdl.window, -1, SDL_RENDERER_ACCELERATED);
    // Notice the flip in xres and yres. This is for widescreen.
    sdl.canvas = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, yres, xres);
    sdl.xres = xres;
    sdl.yres = yres;
    return sdl;
}

void sunlock(const Sdl sdl)
{
    SDL_UnlockTexture(sdl.canvas);
}

Display dlock(const Sdl sdl)
{
    void* screen;
    int pitch;
    SDL_LockTexture(sdl.canvas, NULL, &screen, &pitch);
    const int width = pitch / sizeof(uint32_t);
    const Display display = { (uint32_t*) screen, width };
    return display;
}

Triangle tscale(const Triangle t, const Sdl sdl)
{
    const Triangle triangle = {
        { (sdl.xres / 2) * (t.a.x + 1.0), (sdl.yres / 2) * (t.a.y + 1.0), 0 },
        { (sdl.xres / 2) * (t.b.x + 1.0), (sdl.yres / 2) * (t.b.y + 1.0), 0 },
        { (sdl.xres / 2) * (t.c.x + 1.0), (sdl.yres / 2) * (t.c.y + 1.0), 0 },
    };
    return triangle;
}

float vmag(const Vertex v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vertex vsub(const Vertex a, const Vertex b)
{
    const Vertex v = {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
    };
    return v;
}

Vertex vcross(const Vertex a, const Vertex b)
{
    const Vertex c = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
    return c;
}

float vdot(const Vertex a, const Vertex b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vertex vmul(const Vertex v, const float n)
{
    const Vertex m = { v.x * n, v.y * n, v.z * n };
    return m;
}

Vertex tbc(const Triangle t, const int x, const int y)
{
    const Vertex p = { x, y, 0.0 };
    const Vertex v0 = vsub(t.b, t.a);
    const Vertex v1 = vsub(t.c, t.a);
    const Vertex v2 = vsub(p, t.a);
    const float d00 = vdot(v0, v0);
    const float d01 = vdot(v0, v1);
    const float d11 = vdot(v1, v1);
    const float d20 = vdot(v2, v0);
    const float d21 = vdot(v2, v1);
    const float v = (d11 * d20 - d01 * d21) / (d00 * d11 - d01 * d01);
    const float w = (d00 * d21 - d01 * d20) / (d00 * d11 - d01 * d01);
    const float u = 1.0 - v - w;
    const Vertex vertex = { v, w, u };
    return vertex;
}

Vertex tnormal(const Triangle t)
{
    return vcross(vsub(t.b, t.a), vsub(t.c, t.a));
}

void dtriangle(const Display d, const Triangle t, const Zbuff zbuff, const int color)
{
    const int x0 = umin(t.a.x, umin(t.b.x, t.c.x));
    const int y0 = umin(t.a.y, umin(t.b.y, t.c.y));
    const int x1 = umax(t.a.x, umax(t.b.x, t.c.x));
    const int y1 = umax(t.a.y, umax(t.b.y, t.c.y));
    for(int y = x0; y <= x1; y++)
    for(int x = y0; x <= y1; x++)
    {
        const Vertex bc = tbc(t, y, x); // Notice the flip between x and y.
        if(bc.x >= 0.0 && bc.y >= 0.0 && bc.z >= 0.0)
        {
            const float z = bc.x * t.a.z + bc.y * t.b.z + bc.z * t.c.z;
            if(z < zbuff.z[y + x * zbuff.xres]) // Notice the flip between x and y.
            {
                zbuff.z[y + x * zbuff.xres] = z; // Notice the flip between x and y.
                d.pixels[x + y * d.width] = color;
            }
        }
    }
}

Vertex vnormalized(const Vertex v)
{
    return vmul(v, 1.0 / vmag(v));
}

Zbuff znew(const int xres, const int yres)
{
    const int size = xres * yres;
    Zbuff zbuff = { utoss(float, size), xres, yres };
    for(int i = 0; i < size; i++)
        zbuff.z[i] = INFINITY;
    return zbuff;
}

int main()
{
    const int xres = 800;
    const int yres = 600;
    const char* path = "obj/african_head.obj";
    FILE* const file = fopen(path, "r");
    if(!file)
        ubomb("could not open %s\n", path);
    const Vertices vs = vsload(file);
    const Faces fs = fsload(file);
    const Triangles ts = tsgen(vs, fs);
    const Sdl sdl = ssetup(xres, yres);
    const Vertex light = { 0.0, 0.0, 1.0 };
    const Zbuff zbuff = znew(xres, yres);
    for(int cycles = 0; cycles < 120; cycles++)
    {
        const Display d = dlock(sdl);
        for(int i = 0; i < ts.count; i++)
        {
            const Triangle triangle = ts.triangle[i];
            const Triangle scaled = tscale(triangle, sdl);
            const Vertex normal = vnormalized(tnormal(triangle));
            const float brightness = vdot(normal, light);
            if(brightness > 0.0)
                dtriangle(d, scaled, zbuff, 0x0000FF * fabs(brightness));
        }
        sunlock(sdl);
        schurn(sdl);
        spresent(sdl);
        printf("%d\n", cycles);
    }
    SDL_DestroyTexture(sdl.canvas);
    SDL_DestroyWindow(sdl.window);
    SDL_DestroyRenderer(sdl.renderer);
    SDL_Quit();
    free(zbuff.z);
    free(ts.triangle);
    free(vs.vertex);
    free(fs.face);
    fclose(file);
}
