#ifndef SDL_BACKEND_H
#define SDL_BACKEND_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "buttons.h"

// Keyboard polling used by LCD + Buttons layer
void SDL_Backend_Init(void);
void SDL_Backend_PollEvents(void);
bool SDL_Backend_IsKeyDown(E_BUTTONS_INDEX idx);

#endif