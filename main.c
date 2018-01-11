#include <SDL2/SDL.h>

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
    // Vertice.
    int va, vb, vc;
    // Normals.
    int na, nb, nc;
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
    Vertex a, b, c;
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
    int x0, y0, x1, y1;
}
Box;

typedef struct
{
    float xt;
    float yt;
    float sens;
    const uint8_t* key;
}
Input;

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
    char* line = (char*) malloc(sizeof(char) * size);
    while((ch = getc(file)) != '\n' && ch != EOF)
    {
        line[reads++] = ch;
        if(reads + 1 == size)
            line = (char*) realloc(line, sizeof(char) * (size *= 2));
    }
    line[reads] = '\0';
    return line;
}

static Vertices vsload(FILE* const file, const int lines)
{
    const int max = 128;
    Vertices vs = { (Vertex*) malloc(sizeof(Vertex) * max), 0, max };
    for(int i = 0; i < lines; i++)
    {
        char* line = ureadln(file);
        if(line[0] == 'v' && line[1] != 't' && line[1] != 'n')
        {
            if(vs.count == vs.max)
                vs.vertex = (Vertex*) realloc(vs.vertex, sizeof(Vertex) * (vs.max *= 2));
            Vertex v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            vs.vertex[vs.count++] = v;
        }
        free(line);
    }
    rewind(file);
    return vs;
}

static Vertices vsnew(const int max)
{
    const Vertices vs = { (Vertex*) malloc(sizeof(Vertex) * max), 0, max };
    return vs;
}

static Vertices vnload(FILE* const file, const int lines)
{
    Vertices vs = vsnew(128);
    for(int i = 0; i < lines; i++)
    {
        char* line = ureadln(file);
        if(line[0] == 'v' && line[1] == 'n')
        {
            if(vs.count == vs.max)
                vs.vertex = (Vertex*) realloc(vs.vertex, sizeof(Vertex) * (vs.max *= 2));
            Vertex v;
            sscanf(line, "vn %f %f %f", &v.x, &v.y, &v.z);
            vs.vertex[vs.count++] = v;
        }
        free(line);
    }
    rewind(file);
    return vs;
}

static Faces fsnew(const int max)
{
    const Faces fs = { (Face*) malloc(sizeof(Face) * max), 0, max };
    return fs;
}

static Faces fsload(FILE* const file, const int lines)
{
    Faces fs = fsnew(128);
    for(int i = 0; i < lines; i++)
    {
        char* line = ureadln(file);
        if(line[0] == 'f')
        {
            if(fs.count == fs.max)
                fs.face = (Face*) realloc(fs.face, sizeof(Face) * (fs.max *= 2));
            int waste;
            Face f;
            sscanf(
                line,
                "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &f.va, &waste, &f.na,
                &f.vb, &waste, &f.nb,
                &f.vc, &waste, &f.nc);
            const Face indexed = {
                f.va - 1, f.vb - 1, f.vc - 1,
                f.na - 1, f.nb - 1, f.nc - 1,
            };
            fs.face[fs.count++] = indexed;
        }
        free(line);
    }
    rewind(file);
    return fs;
}

Triangles tsnew(const int count)
{
    const Triangles ts = { (Triangle*) malloc(sizeof(Triangle) * count), count };
    return ts;
}

static Triangles tsgen(const Vertices vs, const Faces fs)
{
    Triangles ts = tsnew(fs.count);
    for(int i = 0; i < ts.count; i++)
    {
        const Triangle t = {
            vs.vertex[fs.face[i].va],
            vs.vertex[fs.face[i].vb],
            vs.vertex[fs.face[i].vc],
        };
        ts.triangle[i] = t;
    }
    return ts;
}

static Triangles tngen(const Vertices vs, const Faces fs)
{
    Triangles ts = tsnew(fs.count);
    for(int i = 0; i < fs.count; i++)
    {
        const Triangle t = {
            vs.vertex[fs.face[i].na],
            vs.vertex[fs.face[i].nb],
            vs.vertex[fs.face[i].nc],
        };
        ts.triangle[i] = t;
    }
    return ts;
}

static void spresent(const Sdl sdl)
{
    SDL_RenderPresent(sdl.renderer);
}

static void schurn(const Sdl sdl)
{
    const SDL_Rect dst = {
        (sdl.xres - sdl.yres) / 2,
        (sdl.yres - sdl.xres) / 2,
        sdl.yres, sdl.xres
    };
    SDL_RenderCopyEx(sdl.renderer, sdl.canvas, NULL, &dst, -90, NULL, SDL_FLIP_NONE);
}

static Sdl ssetup(const int xres, const int yres)
{
    Sdl sdl;
    SDL_Init(SDL_INIT_VIDEO);
    sdl.window = SDL_CreateWindow("water", 0, 0, xres, yres, SDL_WINDOW_SHOWN);
    sdl.renderer = SDL_CreateRenderer(sdl.window, -1, SDL_RENDERER_ACCELERATED);
    // To improve CPU line drawing cache speed the xres and yres for the painting canvas is reversed.
    // This offsets the canvas by 90 degrees. When the finished canvas frame is presented it will be
    // quickly rotated 90 degrees by the GPU. See: schurn(Sdl)
    sdl.canvas = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, yres, xres);
    sdl.xres = xres;
    sdl.yres = yres;
    return sdl;
}

static void sunlock(const Sdl sdl)
{
    SDL_UnlockTexture(sdl.canvas);
}

static uint32_t* slock(const Sdl sdl)
{
    void* pixel;
    int pitch;
    SDL_LockTexture(sdl.canvas, NULL, &pixel, &pitch);
    return (uint32_t*) pixel;
}

static Triangle tviewport(const Triangle t, const Sdl sdl)
{
    const float w = sdl.xres / 2.0;
    const float h = sdl.yres / 2.0;
    const float z = 1.0 / 2.0;
    const Triangle v = {
        { w * (t.a.x + 1.0), h * (t.a.y + 1.0), z * (t.a.z + 1.0) },
        { w * (t.b.x + 1.0), h * (t.b.y + 1.0), z * (t.b.z + 1.0) },
        { w * (t.c.x + 1.0), h * (t.c.y + 1.0), z * (t.c.z + 1.0) },
    };
    return v;
}

static Triangle tpersp(const Triangle t)
{
    const float c = 3.0;
    const float za = 1.0 - t.a.z / c;
    const float zb = 1.0 - t.b.z / c;
    const float zc = 1.0 - t.c.z / c;
    const Triangle p = {
        { t.a.x / za, t.a.y / za, t.a.z / za },
        { t.b.x / zb, t.b.y / zb, t.b.z / zb },
        { t.c.x / zc, t.c.y / zc, t.c.z / zc },
    };
    return p;
}

// Vector subtraction.
static Vertex vs(const Vertex a, const Vertex b)
{
    const Vertex v = { a.x - b.x, a.y - b.y, a.z - b.z };
    return v;
}

// Vector cross product.
static Vertex vc(const Vertex a, const Vertex b)
{
    const Vertex c = { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
    return c;
}

// Vector scalar multiplication.
static Vertex vm(const Vertex v, const float n)
{
    const Vertex m = { v.x * n, v.y * n, v.z * n };
    return m;
}

// Vector dot product.
static float vd(const Vertex a, const Vertex b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Vector length.
static float vl(const Vertex v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Vector unit.
static Vertex vu(const Vertex v)
{
    return vm(v, 1.0 / vl(v));
}

// Triangle barycentric coordinates.
static Vertex tbc(const Triangle t, const int x, const int y)
{
    const Vertex p = { (float) x, (float) y, 0.0 };
    const Vertex v0 = vs(t.b, t.a);
    const Vertex v1 = vs(t.c, t.a);
    const Vertex v2 = vs(p, t.a);
    const float d00 = vd(v0, v0);
    const float d01 = vd(v0, v1);
    const float d11 = vd(v1, v1);
    const float d20 = vd(v2, v0);
    const float d21 = vd(v2, v1);
    const float v = (d11 * d20 - d01 * d21) / (d00 * d11 - d01 * d01);
    const float w = (d00 * d21 - d01 * d20) / (d00 * d11 - d01 * d01);
    const float u = 1.0 - v - w;
    const Vertex vertex = { v, w, u };
    return vertex;
}

// Draws a triangle in its viewport (v) with normal indices (n). A lighting vertex (l) determines triangle shade. zbuffer is modified.
static void tdraw(const int yres, uint32_t* const pixel, float* const zbuff, const Triangle v, const Triangle n, const Vertex l)
{
    const Box b = {
        (int) fminf(v.a.x, fminf(v.b.x, v.c.x)),
        (int) fminf(v.a.y, fminf(v.b.y, v.c.y)),
        (int) fmaxf(v.a.x, fmaxf(v.b.x, v.c.x)),
        (int) fmaxf(v.a.y, fmaxf(v.b.y, v.c.y)),
    };
    for(int x = b.x0; x <= b.x1; x++)
    for(int y = b.y0; y <= b.y1; y++)
    {
        const Vertex bc = tbc(v, x, y);
        // Remember that the canvas is rotated 90 degrees
        // so the everything  x and y are flipped here.
        if(bc.x >= 0.0 && bc.y >= 0.0 && bc.z >= 0.0)
        {
            // Notice the 90 degree rotation between a, b, and c such that the order is b, c, a.
            const Vertex varying = {
                vd(l, n.b),
                vd(l, n.c),
                vd(l, n.a),
            };
            const float brightness = vd(bc, varying);
            if(brightness > 0.0)
            {
                // Z-depth is triangle depth multiplied by barycenter weights.
                const float z = bc.x * v.a.z + bc.y * v.b.z + bc.z * v.c.z;
                // Notice the 90 degree rotation between x and y.
                if(z > zbuff[y + x * yres])
                {
                    zbuff[y + x * yres] = z;
                    pixel[y + x * yres] = brightness * 0xFF;
                }
            }
        }
    }
}

// Rotates, translates, and scales triangles.
static Triangle tlookatt(const Triangle t, const Vertex e, const Vertex c, const Vertex u)
{
    const Vertex z = vu(vs(e, c));
    const Vertex x = vu(vc(u, z));
    const Vertex y = vc(z, x);
    const float xe = vd(x, e);
    const float ye = vd(y, e);
    const float ze = vd(z, e);
    const Triangle o = {
        { vd(t.a, x) - xe, vd(t.a, y) - ye, vd(t.a, z) - ze },
        { vd(t.b, x) - xe, vd(t.b, y) - ye, vd(t.b, z) - ze },
        { vd(t.c, x) - xe, vd(t.c, y) - ye, vd(t.c, z) - ze },
    };
    return o;
}

static Triangle tu(const Triangle t)
{
    const Triangle u = { vu(t.a), vu(t.b), vu(t.c) };
    return u;
}

// Rotates normal vectors.
static Triangle tlookatn(const Triangle n, const Vertex e, const Vertex c, const Vertex u)
{
    const Vertex z = vu(vs(e, c));
    const Vertex x = vu(vc(u, z));
    const Vertex y = vc(z, x);
    const Triangle o = {
        { vd(n.a, x), vd(n.a, y), vd(n.a, z) },
        { vd(n.b, x), vd(n.b, y), vd(n.b, z) },
        { vd(n.c, x), vd(n.c, y), vd(n.c, z) },
    };
    // Normalize for good measure
    return tu(o);
}

static Input iinit()
{
    const Input input = { 0.0, 0.0, 0.005, SDL_GetKeyboardState(NULL) };
    SDL_SetRelativeMouseMode(SDL_FALSE);
    return input;
}

static Input ipump(Input input)
{
    int dx, dy;
    SDL_PumpEvents();
    SDL_GetRelativeMouseState(&dx, &dy);
    input.xt -= input.sens * dx;
    input.yt += input.sens * dy;
    return input;
}

static Vertex ieye(const Input input)
{
    const Vertex e = { sinf(input.xt), sinf(input.yt), cosf(input.xt) };
    return e;
}

static void reset(float* const zbuff, uint32_t* const pixel, const int size)
{
    for(int i = 0; i < size; i++) zbuff[i] = -FLT_MAX, pixel[i] = 0x0;
}

static float* znew(const int size)
{
    return (float*) malloc(sizeof(float) * size);
}

int main()
{
    const int xres = 600;
    const int yres = 480;
    const char* path = "obj/african_head.obj";
    FILE* const file = fopen(path, "r");
    if(!file)
    {
        printf("could not open %s\n", path);
        return 1;
    }
    const int lines = ulns(file);
    const Faces fs = fsload(file, lines);
    const Vertices vs = vsload(file, lines);
    const Vertices vn = vnload(file, lines);
    const Triangles ts = tsgen(vs, fs);
    const Triangles tn = tngen(vn, fs);
    const Sdl sdl = ssetup(xres, yres);
    const Vertex lights = { 0.0, 0.0, 1.0 }; // Must not change as backface culling relies on direction.
    const Vertex center = { 0.0, 0.0, 0.0 };
    const Vertex upward = { 0.0, 1.0, 0.0 };
    const int size = xres * yres;
    float* const zbuff = znew(size);
    for(Input input = iinit(); !input.key[SDL_SCANCODE_END]; input = ipump(input))
    {
        uint32_t* const pixel = slock(sdl);
        reset(zbuff, pixel, size);
        const Vertex eye = ieye(input);
        for(int i = 0; i < ts.count; i++)
        {
            const Triangle n = tlookatn(tn.triangle[i], eye, center, upward);
            const Triangle t = tlookatt(ts.triangle[i], eye, center, upward);
            const Triangle p = tpersp(t);
            const Triangle v = tviewport(p, sdl);
            tdraw(yres, pixel, zbuff, v, n, lights);
        }
        sunlock(sdl);
        schurn(sdl);
        spresent(sdl);
    }
}
