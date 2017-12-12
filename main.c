#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>

typedef struct
{
    float x, y;
}
Point;

typedef struct
{
    Point a, b;
}
Line;

typedef struct
{
    Point a, b, c;
}
Triangle;

typedef struct
{
    float x, y, z;
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

#define toss(t, n) ((t*) malloc((n) * sizeof(t)))

#define retoss(ptr, t, n) (ptr = (t*) realloc((ptr), (n) * sizeof(t)))

#define zero(a) (memset(&(a), 0, sizeof(a)))

#define max(a, b) (((a) > (b)) ? (a) : (b))

#define min(a, b) (((a) < (b)) ? (a) : (b))

int lns(FILE* const file)
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

char* readln(FILE* const file)
{
    int ch = EOF;
    int reads = 0;
    int size = 128;
    char* line = toss(char, size);;
    while((ch = getc(file)) != '\n' && ch != EOF)
    {
        line[reads++] = ch;
        if(reads + 1 == size) retoss(line, char, size *= 2);
    }
    line[reads] = '\0';
    return line;
}

void schurn(const Sdl sdl)
{
    const SDL_Rect dst = {
        (sdl.xres - sdl.yres) / 2,
        (sdl.yres - sdl.xres) / 2,
        sdl.yres, sdl.xres
    };
    SDL_RenderCopyEx(
        sdl.renderer, sdl.canvas, NULL, &dst, -90, NULL, SDL_FLIP_NONE);
}

void spresent(const Sdl sdl)
{
    SDL_RenderPresent(sdl.renderer);
}

Sdl ssetup(const int xres, const int yres)
{
    SDL_Init(SDL_INIT_VIDEO);
    Sdl sdl;
    zero(sdl);
    sdl.window = SDL_CreateWindow("water", 0, 0, xres, yres, SDL_WINDOW_SHOWN);
    sdl.renderer = SDL_CreateRenderer(
        sdl.window, -1,
        SDL_RENDERER_ACCELERATED);
    sdl.canvas = SDL_CreateTexture(
        sdl.renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, yres, xres);
    sdl.xres = xres;
    sdl.yres = yres;
    return sdl;
}

void srelease(const Sdl sdl)
{
    SDL_DestroyTexture(sdl.canvas);
    SDL_Quit();
    SDL_DestroyWindow(sdl.window);
    SDL_DestroyRenderer(sdl.renderer);
}

void sunlock(const Sdl sdl)
{
    SDL_UnlockTexture(sdl.canvas);
}

Display slock(const Sdl sdl)
{
    void* screen;
    int pitch;
    SDL_LockTexture(sdl.canvas, NULL, &screen, &pitch);
    const int width = pitch / sizeof(uint32_t);
    const Display display = { (uint32_t*) screen, width };
    return display;
}

/* Technically a cross product with the Z removed. */
float lcross(const Line l, const Point p)
{
    return (l.b.x - l.a.x) * (p.y - l.a.y) - (l.b.y - l.a.y) * (p.x - l.a.x);
}

int tinside(const Triangle t, const Point p)
{
    const Line a = { t.a, t.b }, b = { t.b, t.c }, c = { t.c, t.a };
    return (lcross(a, p) > 0.0 && lcross(b, p) > 0.0 && lcross(c, p) > 0.0)
        || (lcross(a, p) < 0.0 && lcross(b, p) < 0.0 && lcross(c, p) < 0.0);
}

Line lencloset(const Triangle t)
{
    const Line line = {
        { min(t.a.x, min(t.b.x, t.c.x)), min(t.a.y, min(t.b.y, t.c.y)) },
        { max(t.a.x, max(t.b.x, t.c.x)), max(t.a.y, max(t.b.y, t.c.y)) },
    };
    return line;
}

void dtriangle(const Display d, const Triangle t, const uint32_t color)
{
    const Line l = lencloset(t);
    // Reverse <i> and <j> because SDL will churn 90 degrees with render.
    for(int j = l.a.x; j < l.b.x; j++)
    for(int i = l.a.y; i < l.b.y; i++)
    {
        const Point p = { j, i };
        if(tinside(t, p)) d.pixels[i + j * d.width] = color;
    }
}

Vertex vsub(const Vertex a, const Vertex b)
{
    const Vertex c = { a.x - b.x, a.y - b.y, a.z - b.z };
    return c;
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

Vertex vnormal(const Face face, const Vertices vertices)
{
    const Vertex a = vertices.vertex[face.vertex[0]];
    const Vertex b = vertices.vertex[face.vertex[1]];
    const Vertex c = vertices.vertex[face.vertex[2]];
    return vcross(vsub(b, a), vsub(c, a));
}

float vmag(const Vertex v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vertex vmul(const Vertex v, const float n)
{
    const Vertex m = { v.x * n, v.y * n, v.z * n };
    return m;
}

float vdot(const Vertex a, const Vertex b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vertex vnormalized(const Vertex v)
{
    return vmul(v, 1.0 / vmag(v));
}

Vertices vsnew(const int max)
{
    Vertices v;
    zero(v);
    v.vertex = toss(Vertex, max);
    v.max = max;
    return v;
}

int vertex(const char* const line)
{
    return line[0] == 'v' && line[1] != 't' && line[1] != 'n';
}

Vertex vparse(const char* const line)
{
    Vertex v = { 0.0, 0.0, 0.0 };
    sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
    return v;
}

Vertices vsload(FILE* const file)
{
    Vertices v = vsnew(1024);
    const int lines = lns(file);
    for(int i = 0; i < lines; i++)
    {
        char* line = readln(file);
        if(vertex(line))
        {
            // Growing.
            if(v.count == v.max)
                retoss(v.vertex, Vertex, v.max *= 2);
            // Appending.
            v.vertex[v.count++] = vparse(line);;
        }
        free(line);
    }
    rewind(file);
    return v;
}

void vskill(const Vertices v)
{
    free(v.vertex);
}

Faces fsnew(const int max)
{
    Faces fa = { 0, 0, 0 };
    fa.face = toss(Face, max);
    fa.max = max;
    return fa;
}

int face(const char* const line)
{
    return line[0] == 'f';
}

// Vertices of faces start counting from 1.
Face zindex(const Face f)
{
    const Face face = {{
        f.vertex[0] - 1,
        f.vertex[1] - 1,
        f.vertex[2] - 1,
    }};
    return face;
}

Face fparse(const char* const line)
{
    Face face;
    zero(face);
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
    Faces fa = fsnew(1024);
    const int lines = lns(file);
    for(int i = 0; i < lines; i++)
    {
        char* line = readln(file);
        if(face(line))
        {
            // Growing.
            if(fa.count == fa.max)
                retoss(fa.face, Face, fa.max *= 2);
            // Appending.
            fa.face[fa.count++] = zindex(fparse(line));
        }
        free(line);
    }
    rewind(file);
    return fa;
}

void fskill(const Faces fa)
{
    free(fa.face);
}

Triangle tget(const Face f, const Vertices v)
{
    const Triangle triangle = {
        { v.vertex[f.vertex[0]].x, v.vertex[f.vertex[0]].y },
        { v.vertex[f.vertex[1]].x, v.vertex[f.vertex[1]].y },
        { v.vertex[f.vertex[2]].x, v.vertex[f.vertex[2]].y },
    };
    return triangle;
}

Triangle scale(const Triangle t, const int xres, const int yres)
{
    const Triangle triangle = {
        { (xres / 2) * (t.a.x + 1.0), (yres / 2) * (t.a.y + 1.0) },
        { (xres / 2) * (t.b.x + 1.0), (yres / 2) * (t.b.y + 1.0) },
        { (xres / 2) * (t.c.x + 1.0), (yres / 2) * (t.c.y + 1.0) },
    };
    return triangle;
}

uint32_t shade(const float brightness)
{
    return
    (uint32_t) (255 * brightness) << 0x10 |
    (uint32_t) (255 * brightness) << 0x08 |
    (uint32_t) (255 * brightness) << 0x00;
}

int main()
{
    srand(time(0));
    // Load.
    FILE* const file = fopen("african_head.obj", "r");
    Vertices vertices = vsload(file);
    Faces faces = fsload(file);
    const Sdl sdl = ssetup(600, 600);
    // Render.
    const Vertex light = { 0.0, 0.0, 1.0 };
    const int frames = 60;
    const int t0 = SDL_GetTicks();
    for(int renders = 0; renders < frames; renders++)
    {
        const Display display = slock(sdl);
        for(int f = 0; f < faces.count; f++)
        {
            const Face face = faces.face[f];
            const Vertex normal = vnormalized(vnormal(face, vertices));
            const float brightness = vdot(normal, light);
            if(brightness > 0.0)
                dtriangle(
                    display,
                    scale(tget(face, vertices), sdl.xres, sdl.yres),
                    shade(brightness));
        }
        sunlock(sdl);
        schurn(sdl);
        spresent(sdl);
    }
    const int t1 = SDL_GetTicks();
    printf("%f\n", frames / ((t1 - t0) / 1000.0));
    SDL_Delay(3000);
    // Clean up.
    srelease(sdl);
    fskill(faces);
    vskill(vertices);
    fclose(file);
}
