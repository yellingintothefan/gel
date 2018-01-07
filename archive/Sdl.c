#include "Sdl.h"

#include "util.h"

void xschurn(const Sdl sdl)
{
    const SDL_Rect dst = {
        (sdl.xres - sdl.yres) / 2,
        (sdl.yres - sdl.xres) / 2,
        sdl.yres, sdl.xres
    };
    SDL_RenderCopyEx(sdl.renderer, sdl.canvas, NULL, &dst, -90, NULL, SDL_FLIP_NONE);
}

void xspresent(const Sdl sdl)
{
    SDL_RenderPresent(sdl.renderer);
}

Sdl xssetup(const int xres, const int yres)
{
    SDL_Init(SDL_INIT_VIDEO);
    Sdl sdl;
    xuzero(sdl);
    sdl.window = SDL_CreateWindow("water", 0, 0, xres, yres, SDL_WINDOW_SHOWN);
    sdl.renderer = SDL_CreateRenderer(sdl.window, -1, SDL_RENDERER_ACCELERATED);
    // Notice the flip in xres and yres. This is to widescreen.
    sdl.canvas = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, yres, xres);
    sdl.xres = xres;
    sdl.yres = yres;
    return sdl;
}

void xsrelease(const Sdl sdl)
{
    SDL_DestroyTexture(sdl.canvas);
    SDL_Quit();
    SDL_DestroyWindow(sdl.window);
    SDL_DestroyRenderer(sdl.renderer);
}

void xsunlock(const Sdl sdl)
{
    SDL_UnlockTexture(sdl.canvas);
}
