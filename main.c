/*           _____                    _____                    _____   */
/*          /\    \                  /\    \                  /\    \  */
/*         /::\    \                /::\    \                /::\____\ */
/*        /::::\    \              /::::\    \              /:::/    / */
/*       /::::::\    \            /::::::\    \            /:::/    /  */
/*      /:::/\:::\    \          /:::/\:::\    \          /:::/    /   */
/*     /:::/  \:::\    \        /:::/__\:::\    \        /:::/    /    */
/*    /:::/    \:::\    \      /::::\   \:::\    \      /:::/    /     */
/*   /:::/    / \:::\    \    /::::::\   \:::\    \    /:::/    /      */
/*  /:::/    /   \:::\ ___\  /:::/\:::\   \:::\    \  /:::/    /       */
/* /:::/____/  ___\:::|    |/:::/__\:::\   \:::\____\/:::/____/        */
/* \:::\    \ /\  /:::|____|\:::\   \:::\   \::/    /\:::\    \        */
/*  \:::\    /::\ \::/    /  \:::\   \:::\   \/____/  \:::\    \       */
/*   \:::\   \:::\ \/____/    \:::\   \:::\    \       \:::\    \      */
/*    \:::\   \:::\____\       \:::\   \:::\____\       \:::\    \     */
/*     \:::\  /:::/    /        \:::\   \::/    /        \:::\    \    */
/*      \:::\/:::/    /          \:::\   \/____/          \:::\    \   */
/*       \::::::/    /            \:::\    \               \:::\    \  */
/*        \::::/    /              \:::\____\               \:::\____\ */
/*         \::/____/                \::/    /                \::/    / */
/*                                   \/____/                  \/____/  */
/*                                                                     */
/*                      Graphical Emluation Layer                      */
/*          GEL aims to simulate primitive graphics rendering          */
/*                  to the likes of the N64, including                 */
/*            zbuffering, texture mapping, Gouraud shading,            */
/*                     rotation and translatation.                     */

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
    int x0;
    int y0;
    int x1;
    int y1;
}
Enclosure;

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
    Triangle vew;
    Triangle nrm;
    Triangle tex;
    SDL_Surface* dif;
}
Target;

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

static Vertices vsvload(FILE* const file, const int lines)
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

static Vertices vsnload(FILE* const file, const int lines)
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

static Vertices vstload(FILE* const file, const int lines)
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
    return
        a.x * b.x +
        a.y * b.y +
        a.z * b.z;
}

static float vlen(const Vertex v)
{
    return sqrtf(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z);
}

static Vertex vunt(const Vertex v)
{
    return vmul(v, 1.0 / vlen(v));
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

Triangles tsnew(const int count)
{
    const Triangles ts = {
        (Triangle*) malloc(sizeof(Triangle) * count),
        count
    };
    return ts;
}

static Triangles tsgen(const Vertices vs, const Faces fs, const float scale)
{
    Triangles ts = tsnew(fs.count);
    for(int i = 0; i < ts.count; i++)
    {
        const Triangle t = {
            vmul(vs.vertex[fs.face[i].va], 1.0 / scale),
            vmul(vs.vertex[fs.face[i].vb], 1.0 / scale),
            vmul(vs.vertex[fs.face[i].vc], 1.0 / scale),
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
    sdl.canvas = SDL_CreateTexture(sdl.renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        // To improve CPU line drawing cache speed the xres and yres for the painting canvas is reversed.
        // This offsets the canvas by 90 degrees. When the finished canvas frame is presented it will be
        // quickly rotated 90 degrees by the GPU. See: schurn(Sdl)
        yres, xres);
    sdl.xres = xres;
    sdl.yres = yres;
    return sdl;
}

// Unlocks the display once a frame is drawn.
static void sunlock(const Sdl sdl)
{
    SDL_UnlockTexture(sdl.canvas);
}

// Locks the display for pixel drawing.
static uint32_t* slock(const Sdl sdl)
{
    void* pixel;
    int pitch;
    SDL_LockTexture(sdl.canvas, NULL, &pixel, &pitch);
    return (uint32_t*) pixel;
}

// Triangle viewport.
static Triangle tviewport(const Triangle t, const Sdl sdl)
{
    const float w = sdl.yres / 1.5;
    const float h = sdl.yres / 1.5;
    const float x = sdl.xres / 2.0;
    const float y = sdl.yres / 4.0;
    const Triangle v = {
        { w * t.a.x + x, h * t.a.y + y, (t.a.z + 1.0) / 2.0 },
        { w * t.b.x + x, h * t.b.y + y, (t.b.z + 1.0) / 2.0 },
        { w * t.c.x + x, h * t.c.y + y, (t.c.z + 1.0) / 2.0 },
    };
    return v;
}

// Triangle perspective.
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

// Triangle barycenter.
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

// Shade a pixel. Discards alpha.
static uint32_t shade(const uint32_t pixel, const int shading)
{
    // Shift right by 0x08 is same as dividing by 256. Somehow -ofast was not catching this.
    const uint32_t r = (((pixel >> 0x10) /****/) * shading) >> 0x08;
    const uint32_t g = (((pixel >> 0x08) & 0xFF) * shading) >> 0x08;
    const uint32_t b = (((pixel /*****/) & 0xFF) * shading) >> 0x08;
    return r << 0x10 | g << 0x08 | b;
}

static Enclosure enew(const Triangle t)
{
    const Enclosure e = {
        fminf(t.a.x, fminf(t.b.x, t.c.x)),
        fminf(t.a.y, fminf(t.b.y, t.c.y)),
        fmaxf(t.a.x, fmaxf(t.b.x, t.c.x)),
        fmaxf(t.a.y, fmaxf(t.b.y, t.c.y)),
    };
    return e;
}

// Draw triangle. All the magic happens here.
static void tdraw(const int yres, uint32_t* const pixel, float* const zbuff, const Target t)
{
    // Create a square enclosure around the triangle for indexing.
    const Enclosure e = enew(t.vew);
    for(int x = e.x0; x <= e.x1; x++)
    for(int y = e.y0; y <= e.y1; y++)
    {
        // Determine the barycenter coordinates of the triangle.
        const Vertex bc = tbc(t.vew, x, y);
        // If within the barycenter of triangle then fill the triangle.
        if(bc.x >= 0.0 && bc.y >= 0.0 && bc.z >= 0.0)
        {
            // The light from the camera must face the object.
            const Vertex light = { 0.0, 0.0, 1.0 };
            // The varying light intensity across polygons is interpolated using gourard shading.
            // Note: Due to the 90 degree rotation the renderer will perform at the end of the frame
            // triangle coordinates are put on their side. That is, triangle vertices A, B, and C
            // must be rotated 90 degrees such that the order is B, C, A.
            const Vertex varying = {
                vdot(light, t.nrm.b), // B
                vdot(light, t.nrm.c), // C
                vdot(light, t.nrm.a), // A
            };
            // The intensity of light reaching the camera is then the vector dot product between
            // the varying intensity of a polygon and the polygon barycenter coordinate.
            const float intensity = vdot(bc, varying);
            // A shading value maps the intensity [0.0 - 1.0] to [0x00 - 0xFF].
            const int shading = 0xFF * (intensity < 0.0 ? 0.0 : intensity);
            // The distance from the triangle point to the camera is a multiplcation of the barycenter weights
            // by the triangle Z-coordinates.
            const float z =
                bc.x * t.vew.b.z + // B
                bc.y * t.vew.c.z + // C
                bc.z * t.vew.a.z;  // A
            // If the Z-depth is greater than a previous Z-depth, a new pixel must be placed,
            // and the old Z-depth must be updated.
            // Note: The zbuffer is also on its side like the render frame. X and Y are flipped for the index.
            if(z > zbuff[y + x * yres])
            {
                const uint32_t* const pixels = (uint32_t*) t.dif->pixels;
                // For texture mapping, the position of the texture is a multiplication of the barycenter weights
                // by the triangle X-coordinates.
                // Note: Once again, a, b, and c are rotate here to b, c, a.
                const int xx = t.dif->w * (
                    bc.x * t.tex.b.x + // B
                    bc.y * t.tex.c.x + // C
                    bc.z * t.tex.a.x); // A
                // Same goes for the Y-coordinates.
                // The image coordinate system starts bottom left, and renderer starts top left, so the
                // final weight becomes 1.0 - weight.
                const int yy = t.dif->h * (1.0 - (
                    bc.x * t.tex.b.y +  // B
                    bc.y * t.tex.c.y +  // C
                    bc.z * t.tex.a.y)); // A
                // Shade away.
                const uint32_t shaded = shade(pixels[xx + yy * t.dif->w], shading);
                // Y and X flipped again but XX and YY are not as the image is not on its side.
                zbuff[y + x * yres] = z;
                pixel[y + x * yres] = shaded;
            }
        }
    }
}

// Rotates, translates, and scales triangles.
static Triangle tviewt(const Triangle t, const Vertex x, const Vertex y, const Vertex z, const Vertex eye)
{
    const Triangle o = {
        { vdot(t.a, x) - vdot(x, eye), vdot(t.a, y) - vdot(y, eye), vdot(t.a, z) - vdot(z, eye) },
        { vdot(t.b, x) - vdot(x, eye), vdot(t.b, y) - vdot(y, eye), vdot(t.b, z) - vdot(z, eye) },
        { vdot(t.c, x) - vdot(x, eye), vdot(t.c, y) - vdot(y, eye), vdot(t.c, z) - vdot(z, eye) },
    };
    return o;
}

// Rotates normal vectors.
// Normally done with hte inverse transpose of the matrix defined in tviewt(),
// but approximated well enough by removing translation (eye dot products) from said matrix.
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
    const Input input = {
        0.0,
        0.0,
        0.005,
        SDL_GetKeyboardState(NULL)
    };
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
    {
        zbuff[i] = -FLT_MAX;
        pixel[i] = 0x0;
    }
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

static float vmaxlen(const Vertices vsv)
{
    float max = 0;
    for(int i = 0; i < vsv.count; i++)
        if(vlen(vsv.vertex[i]) > max)
            max = vlen(vsv.vertex[i]);
    return max;
}

int main()
{
    FILE* const obj = oload("model/anju.obj");
    SDL_Surface* const dif = sload("model/anju.bmp");
    const int xres = 800;
    const int yres = 600;
    const int lines = ulns(obj);
    const Faces fs = fsload(obj, lines);
    const Vertices vsv = vsvload(obj, lines);
    const Vertices vst = vstload(obj, lines);
    const Vertices vsn = vsnload(obj, lines);
    const Triangles ts = tsgen(vsv, fs, vmaxlen(vsv));
    const Triangles tt = ttgen(vst, fs);
    const Triangles tn = tngen(vsn, fs);
    const Sdl sdl = ssetup(xres, yres);
    float* const zbuff = (float*) malloc(sizeof(float) * xres * yres);
    for(Input input = iinit(); !input.key[SDL_SCANCODE_END]; input = ipump(input))
    {
        uint32_t* const pixel = slock(sdl);
        reset(zbuff, pixel, xres * yres);
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
            const Triangle vew = tviewport(input.key[SDL_SCANCODE_Q] ? tri : per, sdl);
            const Target targ = { vew, nrm, tex, dif };
            tdraw(yres, pixel, zbuff, targ);
        }
        sunlock(sdl);
        schurn(sdl);
        spresent(sdl);
    }
    // Let the OS free hoisted memory for a quick exit.
}
