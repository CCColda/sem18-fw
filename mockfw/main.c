#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

static void log_sdl(const char *msg) {
    printf("%s: %s\n", msg, SDL_GetError());
}

static void log_ttf(const char *msg) {
    printf("%s: %s\n", msg, TTF_GetError());
}

int main(int argc, char *argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *font = NULL;
    SDL_Surface *surface = NULL;
    SDL_Texture *texture = NULL;

    int result = 0;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        log_sdl("SDL_Init failed");
        return -1;
    }

    if (TTF_Init() != 0) {
        log_ttf("TTF_Init failed");
        result = -2;
        goto fail_sdl;
    }

    window = SDL_CreateWindow(
        "SDL2 Text",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, 0
    );
    if (!window) {
        log_sdl("CreateWindow failed");
        result = -3;
        goto fail_ttf;
    }

    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        log_sdl("CreateRenderer failed");
        result = -4;
        goto fail_window;
    }

    font = TTF_OpenFont("res/DejaVuSans.ttf", 24);
    if (!font) {
        log_ttf("TTF_OpenFont failed");
        result = -5;
        goto fail_renderer;
    }

    surface = TTF_RenderText_Solid(font, "Hello SDL2 Text", (SDL_Color){255,255,255,255});
    if (!surface) {
        log_ttf("RenderText failed");
        result = -6;
        goto fail_font;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        log_sdl("CreateTexture failed");
        result = -7;
        goto fail_surface;
    }

    SDL_FreeSurface(surface);
    surface = NULL;

    SDL_Rect dst;
    dst.x = 100;
    dst.y = 100;
    SDL_QueryTexture(texture, NULL, NULL, &dst.w, &dst.h);

    SDL_Event e;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, &dst);
        SDL_RenderPresent(renderer);
    }

fail_font:
    TTF_CloseFont(font);
fail_renderer:
    SDL_DestroyRenderer(renderer);
fail_window:
    SDL_DestroyWindow(window);
fail_ttf:
    TTF_Quit();
fail_sdl:
    SDL_Quit();
    return result;


fail_surface:
    SDL_FreeSurface(surface);
    goto fail_font;
}