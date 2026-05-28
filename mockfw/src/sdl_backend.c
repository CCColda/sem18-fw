#include "sdl_backend.h"
#include "lcd.h"

#include <time.h>

static int gInitialized = 0;

static SDL_Window* gWindow = NULL;
static SDL_Renderer* gRenderer = NULL;

static const Uint8* gKeys = NULL;

static U32 gStartupTicks = 0u;

//--------------------------------------------------------------------------------------------------
static void SDL_Backend_CreateWindowInternal(
  const char* title)
{
  gWindow = SDL_CreateWindow(
    title,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    LCD_WIDTH,
    LCD_HEIGHT,
    SDL_WINDOW_SHOWN
  );

  SDL_Backend_Check(gWindow != NULL);

  gRenderer = SDL_CreateRenderer(
    gWindow,
    -1,
    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
  );

  SDL_Backend_Check(gRenderer != NULL);
}

//--------------------------------------------------------------------------------------------------
void SDL_Backend_Init(void)
{
  if(gInitialized)
  {
    return;
  }

  SDL_Backend_Check(
    0 == SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)
  );

  SDL_Backend_Check(
    0 == TTF_Init()
  );

  gKeys = SDL_GetKeyboardState(NULL);

  gStartupTicks = (U32)SDL_GetTicks();

  srand((unsigned int)time(NULL));

  gInitialized = 1;
}

//--------------------------------------------------------------------------------------------------
void SDL_Backend_PollEvents(void)
{
  SDL_PumpEvents();
}

//--------------------------------------------------------------------------------------------------
U32 SDL_Backend_GetTicks(void)
{
  SDL_Backend_Init();

  return ((U32)SDL_GetTicks() - gStartupTicks);
}

//--------------------------------------------------------------------------------------------------
bool SDL_Backend_IsKeyDown(E_BUTTONS_INDEX idx)
{
  if(!gKeys)
  {
    return false;
  }

  switch(idx)
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

//--------------------------------------------------------------------------------------------------
SDL_Renderer* SDL_Backend_GetRenderer(void)
{
  SDL_Backend_Init();

  if(!gRenderer)
  {
    SDL_Backend_CreateWindowInternal("mockfw");
  }

  return gRenderer;
}

//--------------------------------------------------------------------------------------------------
SDL_Window* SDL_Backend_GetWindow(void)
{
  SDL_Backend_GetRenderer();

  return gWindow;
}

//--------------------------------------------------------------------------------------------------
TTF_Font* SDL_Backend_LoadFont(const char* path, int ptsize)
{
  TTF_Font* font = TTF_OpenFont(path, ptsize);

  SDL_Backend_Check(font != NULL);

  return font;
}
