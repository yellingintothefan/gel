#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>

#include "util.h"

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
            if(brightness > 0.0) /* Back face culling. */
                dtriangle(
                    display,
                    scale(tproject(face, vertices), sdl.xres, sdl.yres),
                    xushade(brightness)); /* Shading. */
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
