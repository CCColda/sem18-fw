#include "sdl_backend.h"

static const Uint8* gKeys = NULL;

void SDL_Backend_Init(void)
{
  gKeys = SDL_GetKeyboardState(NULL);
}

void SDL_Backend_PollEvents(void)
{
  SDL_PumpEvents();
}

bool SDL_Backend_IsKeyDown(E_BUTTONS_INDEX idx)
{
  if (!gKeys) return false;

  switch (idx)
  {
    case BUTTON_SW1:         return gKeys[SDL_SCANCODE_1];
    case BUTTON_SW2:         return gKeys[SDL_SCANCODE_2];
    case BUTTON_SW3:         return gKeys[SDL_SCANCODE_3];

    case BUTTON_SW4_UP:      return gKeys[SDL_SCANCODE_UP];
    case BUTTON_SW4_DOWN:    return gKeys[SDL_SCANCODE_DOWN];
    case BUTTON_SW4_LEFT:    return gKeys[SDL_SCANCODE_LEFT];
    case BUTTON_SW4_RIGHT:   return gKeys[SDL_SCANCODE_RIGHT];
    case BUTTON_SW4_PUSH:    return gKeys[SDL_SCANCODE_SPACE];

    default: return false;
  }
}