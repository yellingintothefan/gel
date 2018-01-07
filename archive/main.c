#include "Faces.h"
#include "Display.h"
#include "util.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

int main()
{
    srand(time(0));
    // Load.
    FILE* const file = fopen("obj/african_head.obj", "r");
    if(!file)
        xubomb("file not found\n");
    const Vertices vertices = xvsload(file);
    const Faces faces = xfsload(file);
    const Sdl sdl = xssetup(600, 600);
    // Render.
    const Vertex light = { 0.0, 0.0, 1.0 };
    const int frames = 60;
    const int t0 = SDL_GetTicks();
    for(int renders = 0; renders < frames; renders++)
    {
        const Display display = xdlock(sdl);
        for(int f = 0; f < faces.count; f++)
        {
            const Face face = faces.face[f];
            const Vertex normal = xvnormalized(xfnormal(face, vertices));
            const float brightness = xvdot(normal, light);
            if(brightness > 0.0) /* Back face culling. */
                xdtriangle(
                    display,
                    xtscale(xfproject(face, vertices), sdl.xres, sdl.yres),
                    xushade(brightness)); /* Shading. */
        }
        xsunlock(sdl);
        xschurn(sdl);
        xspresent(sdl);
    }
    const int t1 = SDL_GetTicks();
    printf("%f\n", frames / ((t1 - t0) / 1000.0));
    SDL_Delay(3000);
    // Clean up.
    xsrelease(sdl);
    xfskill(faces);
    xvskill(vertices);
    fclose(file);
}
