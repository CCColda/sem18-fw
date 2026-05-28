#ifndef SDL_BACKEND_H
#define SDL_BACKEND_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "buttons.h"
#include "types.h"

//--------------------------------------------------------------------------------------------------
// Error handling
//--------------------------------------------------------------------------------------------------
#define SDL_Backend_Fatal(msg)                                                     \
  do                                                                               \
  {                                                                                \
    fprintf(stderr, "\n[FATAL] " msg);                                             \
    fprintf(stderr, "SDL Error: %s\n", SDL_GetError());                            \
                                                                                   \
    TTF_Quit();                                                                    \
    SDL_Quit();                                                                    \
                                                                                   \
    abort();                                                                       \
  } while(0)

#define SDL_Backend_Check(condition)                                               \
  do                                                                               \
  {                                                                                \
    if(!(condition))                                                               \
    {                                                                              \
      fprintf(stderr, "\n[CHECK FAILED] " #condition "\n");                        \
      SDL_Backend_Fatal(#condition);                                               \
    }                                                                              \
  } while(0)

//--------------------------------------------------------------------------------------------------
// Init / Shutdown
//--------------------------------------------------------------------------------------------------
void SDL_Backend_Init(void);

//--------------------------------------------------------------------------------------------------
// Events / Timing
//--------------------------------------------------------------------------------------------------
void SDL_Backend_PollEvents(void);
U32 SDL_Backend_GetTicks(void);

//--------------------------------------------------------------------------------------------------
// Keyboard
//--------------------------------------------------------------------------------------------------
bool SDL_Backend_IsKeyDown(E_BUTTONS_INDEX idx);

//--------------------------------------------------------------------------------------------------
// Window / Renderer
//--------------------------------------------------------------------------------------------------
SDL_Window* SDL_Backend_GetWindow(void);
SDL_Renderer* SDL_Backend_GetRenderer(void);

//--------------------------------------------------------------------------------------------------
// Font
//--------------------------------------------------------------------------------------------------
TTF_Font* SDL_Backend_LoadFont(const char* path, int ptsize);

#endif