#ifndef STM32F4XX_HAL_H
#define STM32F4XX_HAL_H

#include <stdint.h>
#include <string.h>

typedef uint32_t U32;

typedef enum
{
  HAL_OK = 0,
  HAL_ERROR
} HAL_StatusTypeDef;

void HAL_Init(void);
U32 HAL_GetTick(void);
U32 HAL_RNG_GetRandomNumber(void);

#endif