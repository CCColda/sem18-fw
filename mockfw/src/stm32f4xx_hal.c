#include "stm32f4xx_hal.h"

#include <SDL2/SDL.h>

#include <stdlib.h>
#include <time.h>

static U32 g_u32StartupTicks = 0u;
static int g_iInitialized = 0;

void HAL_Init(void)
{
  if(0 == g_iInitialized)
  {
    SDL_Init(SDL_INIT_TIMER);

    g_u32StartupTicks = (U32)SDL_GetTicks();

    srand((unsigned int)time(NULL));

    g_iInitialized = 1;
  }
}

U32 HAL_GetTick(void)
{
  if(0 == g_iInitialized)
  {
    HAL_Init();
  }

  return ((U32)SDL_GetTicks() - g_u32StartupTicks);
}

U32 HAL_RNG_GetRandomNumber(void)
{
  U32 u32Random;

  if(0 == g_iInitialized)
  {
    HAL_Init();
  }

  u32Random = ((U32)(rand() & 0xFFFFu) << 16u);
  u32Random |= (U32)(rand() & 0xFFFFu);

  return u32Random;
}