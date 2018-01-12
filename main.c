#include <SDL2/SDL.h>

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
    int va;
    int vb;
    int vc;
    int ta;
    int tb;
    int tc;
    int na;
    int nb;
    int nc;
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
    float xt;
    float yt;
    float sens;
    const uint8_t* key;
}
Input;

typedef struct
{
    Triangle vew;
    Triangle nrm;
    Triangle tex;
    SDL_Surface* stex;
}
Target;

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

static Vertices vsnew(const int max)
{
    const Vertices vs = {
        (Vertex*) malloc(sizeof(Vertex) * max),
        0,
        max
    };
    return vs;
}

static Vertices vsload(FILE* const file, const int lines)
{
    Vertices vs = vsnew(128);
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

static Vertices vtload(FILE* const file, const int lines)
{
    Vertices vs = vsnew(128);
    for(int i = 0; i < lines; i++)
    {
        char* line = ureadln(file);
        if(line[0] == 'v' && line[1] == 't')
        {
            if(vs.count == vs.max)
                vs.vertex = (Vertex*) realloc(vs.vertex, sizeof(Vertex) * (vs.max *= 2));
            Vertex v;
            sscanf(line, "vt %f %f %f", &v.x, &v.y, &v.z);
            vs.vertex[vs.count++] = v;
        }
        free(line);
    }
    rewind(file);
    return vs;
}

static Faces fsnew(const int max)
{
    const Faces fs = {
        (Face*) malloc(sizeof(Face) * max),
        0,
        max
    };
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
            Face f;
            sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &f.va, &f.ta, &f.na, &f.vb, &f.tb, &f.nb, &f.vc, &f.tc, &f.nc);
            const Face indexed = {
                f.va - 1, f.vb - 1, f.vc - 1,
                f.ta - 1, f.tb - 1, f.tc - 1,
                f.na - 1, f.nb - 1, f.nc - 1,
            };
            fs.face[fs.count++] = indexed;
        }
        free(line);
    }
    rewind(file);
    return fs;
}

static Vertex vsub(const Vertex a, const Vertex b)
{
    const Vertex v = {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
    };
    return v;
}

static Vertex vcrs(const Vertex a, const Vertex b)
{
    const Vertex c = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
    return c;
}

static Vertex vmul(const Vertex v, const float n)
{
    const Vertex m = {
        v.x * n,
        v.y * n,
        v.z * n,
    };
    return m;
}

static float vdot(const Vertex a, const Vertex b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static float vlen(const Vertex v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static Vertex vunt(const Vertex v)
{
    return vmul(v, 1.0 / vlen(v));
}

Triangles tsnew(const int count)
{
    const Triangles ts = {
        (Triangle*) malloc(sizeof(Triangle) * count),
        count
    };
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

static Triangles ttgen(const Vertices vs, const Faces fs)
{
    Triangles ts = tsnew(fs.count);
    for(int i = 0; i < fs.count; i++)
    {
        const Triangle t = {
            vs.vertex[fs.face[i].ta],
            vs.vertex[fs.face[i].tb],
            vs.vertex[fs.face[i].tc],
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

static Vertex tbc(const Triangle t, const int x, const int y)
{
    const Vertex p = { (float) x, (float) y, 0.0 };
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

// Modulous modify a pixel. Discards alpha. Great for pixel shading
static uint32_t mod(const uint32_t pixel, const int shade)
{
    // Shift right by 0x08 is same as dividing by 256. Somehow -ofast was not catching this.
    const uint32_t r = (((pixel >> 0x10) /****/) * shade) >> 0x08;
    const uint32_t g = (((pixel >> 0x08) & 0xFF) * shade) >> 0x08;
    const uint32_t b = (((pixel /*****/) & 0xFF) * shade) >> 0x08;
    return r << 0x10 | g << 0x08 | b;
}

// Draws a triangle in its viewport (v) with normal indices (n). A lighting vertex (l) determines triangle shade. zbuffer is modified.
static void tdraw(const int yres, uint32_t* const pixel, float* const zbuff, const Vertex lit, const Target t)
{
    const int x0 = fminf(t.vew.a.x, fminf(t.vew.b.x, t.vew.c.x));
    const int y0 = fminf(t.vew.a.y, fminf(t.vew.b.y, t.vew.c.y));
    const int x1 = fmaxf(t.vew.a.x, fmaxf(t.vew.b.x, t.vew.c.x));
    const int y1 = fmaxf(t.vew.a.y, fmaxf(t.vew.b.y, t.vew.c.y));
    for(int x = x0; x <= x1; x++)
    for(int y = y0; y <= y1; y++)
    {
        const Vertex bc = tbc(t.vew, x, y);
        // Remember that the canvas is rotated 90 degrees so the everything  x and y are flipped here.
        if(bc.x >= 0.0 && bc.y >= 0.0 && bc.z >= 0.0)
        {
            // Notice the 90 degree rotation between a, b, and c such that the order is b, c, a.
            const Vertex varying = {
                vdot(lit, t.nrm.b),
                vdot(lit, t.nrm.c),
                vdot(lit, t.nrm.a),
            };
            const float intensity = vdot(bc, varying);
            if(intensity > 0.0)
            {
                const int shade = 0xFF * intensity;
                // Z-depth is triangle depth multiplied by barycenter weights.
                const float z = bc.x * t.vew.a.z + bc.y * t.vew.b.z + bc.z * t.vew.c.z;
                // Notice the 90 degree rotation between x and y.
                if(z > zbuff[y + x * yres])
                {
                    const uint32_t* const pixels = (uint32_t*) t.stex->pixels;
                    // Once again, a, b, and c are rotate here t.to b, c, a.
                    const int xx = t.stex->w * (1.0 - (bc.x * t.tex.b.x + bc.y * t.tex.c.x + bc.z * t.tex.a.x));
                    const int yy = t.stex->h * (1.0 - (bc.x * t.tex.b.y + bc.y * t.tex.c.y + bc.z * t.tex.a.y));
                    // y and x are flipped, but xx and yy are regular to image.
                    zbuff[y + x * yres] = z;
                    pixel[y + x * yres] = mod(pixels[xx + yy * t.stex->w], shade);
                }
            }
        }
    }
}

// Rotates, translates, and scales triangles.
static Triangle tviewt(const Triangle t, const Vertex x, const Vertex y, const Vertex z, const Vertex eye)
{
    const float xe = vdot(x, eye);
    const float ye = vdot(y, eye);
    const float ze = vdot(z, eye);
    const Triangle o = {
        { vdot(t.a, x) - xe, vdot(t.a, y) - ye, vdot(t.a, z) - ze },
        { vdot(t.b, x) - xe, vdot(t.b, y) - ye, vdot(t.b, z) - ze },
        { vdot(t.c, x) - xe, vdot(t.c, y) - ye, vdot(t.c, z) - ze },
    };
    return o;
}

static Triangle tu(const Triangle t)
{
    const Triangle u = {
        vunt(t.a),
        vunt(t.b),
        vunt(t.c),
    };
    return u;
}

// Rotates normal vectors.
static Triangle tviewn(const Triangle n, const Vertex x, const Vertex y, const Vertex z)
{
    const Triangle o = {
        { vdot(n.a, x), vdot(n.a, y), vdot(n.a, z) },
        { vdot(n.b, x), vdot(n.b, y), vdot(n.b, z) },
        { vdot(n.c, x), vdot(n.c, y), vdot(n.c, z) },
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

static void reset(float* const zbuff, uint32_t* const pixel, const int size)
{
    for(int i = 0; i < size; i++)
        zbuff[i] = -FLT_MAX, pixel[i] = 0x0;
}

static SDL_Surface* sload(const char* const path)
{
    SDL_Surface* const bmp = SDL_LoadBMP(path);
    if(bmp == NULL)
    {
        printf("%s: Could not open %s\n", SDL_GetError(), path);
        exit(1);
    }
    SDL_PixelFormat* const allocation = SDL_AllocFormat(SDL_PIXELFORMAT_RGB888);
    SDL_Surface* const converted = SDL_ConvertSurface(bmp, allocation, 0);
    SDL_FreeFormat(allocation);
    SDL_FreeSurface(bmp);
    return converted;
}

static FILE* oload(const char* const path)
{
    FILE* const file = fopen(path, "r");
    if(file == NULL)
    {
        printf("could not open %s\n", path);
        exit(1);
    }
    return file;
}

int main()
{
    FILE* const obj = oload("head.obj");
    SDL_Surface* const dif = sload("head_diffuse.bmp");
    const int xres = 600;
    const int yres = 480;
    const int lines = ulns(obj);
    const Faces fs = fsload(obj, lines);
    const Vertices vsv = vsload(obj, lines);
    const Vertices vst = vtload(obj, lines);
    const Vertices vsn = vnload(obj, lines);
    const Triangles ts = tsgen(vsv, fs);
    const Triangles tt = ttgen(vst, fs);
    const Triangles tn = tngen(vsn, fs);
    const Sdl sdl = ssetup(xres, yres);
    float* const zbuff = (float*) malloc(sizeof(float) * xres * yres);
    for(Input input = iinit(); !input.key[SDL_SCANCODE_END]; input = ipump(input))
    {
        uint32_t* const pixel = slock(sdl);
        reset(zbuff, pixel, xres * yres);
        const Vertex lit = { 0.0, 0.0, 1.0 };
        const Vertex ctr = { 0.0, 0.0, 0.0 };
        const Vertex ups = { 0.0, 1.0, 0.0 };
        const Vertex eye = {
            sinf(input.xt),
            sinf(input.yt),
            cosf(input.xt),
        };
        const Vertex z = vunt(vsub(eye, ctr));
        const Vertex x = vunt(vcrs(ups, z));
        const Vertex y = vcrs(z, x);
        for(int i = 0; i < ts.count; i++)
        {
            const Triangle nrm = tviewn(tn.triangle[i], x, y, z);
            const Triangle tex = tt.triangle[i];
            const Triangle tri = tviewt(ts.triangle[i], x, y, z, eye);
            const Triangle per = tpersp(tri);
            const Triangle vew = tviewport(per, sdl);
            const Target targ = { vew, nrm, tex, dif };
            tdraw(yres, pixel, zbuff, lit, targ);
        }
        sunlock(sdl);
        schurn(sdl);
        spresent(sdl);
    }
}
