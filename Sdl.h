#pragma once

#include <SDL2/SDL.h>

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* canvas;
    int xres;
    int yres;
}
Sdl;

void xschurn(const Sdl sdl);

void xspresent(const Sdl sdl);

Sdl xssetup(const int xres, const int yres);

void xsrelease(const Sdl sdl);

void xsunlock(const Sdl sdl);
