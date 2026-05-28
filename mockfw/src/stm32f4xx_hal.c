#include "stm32f4xx_hal.h"

#include "sdl_backend.h"

#include <stdlib.h>

void HAL_Init(void)
{
  SDL_Backend_Init();
}

U32 HAL_GetTick(void)
{
  return SDL_Backend_GetTicks();
}

U32 HAL_RNG_GetRandomNumber(void)
{
  U32 u32Random;

  u32Random = ((U32)(rand() & 0xFFFFu) << 16u);
  u32Random |= (U32)(rand() & 0xFFFFu);

  return u32Random;
}
