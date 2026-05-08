/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

#include "stm32l4xx_ll_crs.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_dma.h"
#include "stm32l4xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BUTTON_SW2_Pin LL_GPIO_PIN_4
#define BUTTON_SW2_GPIO_Port GPIOC
#define BUTTON_SW3_Pin LL_GPIO_PIN_5
#define BUTTON_SW3_GPIO_Port GPIOC
#define BUCK_MODE_Pin LL_GPIO_PIN_2
#define BUCK_MODE_GPIO_Port GPIOB
#define LCD_nSS_Pin LL_GPIO_PIN_12
#define LCD_nSS_GPIO_Port GPIOB
#define LCD_DC_Pin LL_GPIO_PIN_14
#define LCD_DC_GPIO_Port GPIOB
#define LCD_nPWR_Pin LL_GPIO_PIN_6
#define LCD_nPWR_GPIO_Port GPIOC
#define LCD_BL_Pin LL_GPIO_PIN_7
#define LCD_BL_GPIO_Port GPIOC
#define CHARGER_nCHARGE_Pin LL_GPIO_PIN_8
#define CHARGER_nCHARGE_GPIO_Port GPIOC
#define CHARGER_STANDBY_Pin LL_GPIO_PIN_9
#define CHARGER_STANDBY_GPIO_Port GPIOC
#define BUTTON_SW4_RIGHT_Pin LL_GPIO_PIN_15
#define BUTTON_SW4_RIGHT_GPIO_Port GPIOA
#define BUTTON_SW4_LEFT_Pin LL_GPIO_PIN_10
#define BUTTON_SW4_LEFT_GPIO_Port GPIOC
#define BUTTON_SW4_PUSH_Pin LL_GPIO_PIN_11
#define BUTTON_SW4_PUSH_GPIO_Port GPIOC
#define BUTTON_SW4_UP_Pin LL_GPIO_PIN_12
#define BUTTON_SW4_UP_GPIO_Port GPIOC
#define BUTTON_SW4_DOWN_Pin LL_GPIO_PIN_3
#define BUTTON_SW4_DOWN_GPIO_Port GPIOB
#define VMEAS_GND_Pin LL_GPIO_PIN_6
#define VMEAS_GND_GPIO_Port GPIOB
#define BUZZER_Pin LL_GPIO_PIN_7
#define BUZZER_GPIO_Port GPIOB
#define BUTTON_SW1_Pin LL_GPIO_PIN_3
#define BUTTON_SW1_GPIO_Port GPIOH

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
