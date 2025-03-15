/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
  typedef enum
  {
    FUNCTION_OFF,
    RELEASE_ACTIVATE,
    PRESS_ACTIVATE,
    TOUCH_ACTIVATE,
    PULSE_MODE,
    BREATHING_MODE,
    ADJUSTABLE_BRIGHTNESS,
    RGB_LED,
    POTENTIOMETER,
    TEMPERATURE,
    VOLUME_LED,
    VOLUME_LED_THRESHOLD,

    MAX_VALUE_MODE,

  }Mode_Select;

  typedef enum
  {
    BLACK,
    BLUE,
    GREEN,
    RED,
    WHITE,

    MAX_VALUE_COLOR
  }LED_Color;

  extern int key1_press_trigger_state;
  extern int key1_release_trigger_state;
  extern int key2_trigger_state;
  extern int mode_trigger_state;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MODE_Pin GPIO_PIN_3
#define MODE_GPIO_Port GPIOG
#define MODE_EXTI_IRQn EXTI3_IRQn
#define KEY2_Pin GPIO_PIN_4
#define KEY2_GPIO_Port GPIOG
#define KEY2_EXTI_IRQn EXTI4_IRQn
#define KEY1_Pin GPIO_PIN_5
#define KEY1_GPIO_Port GPIOG
#define KEY1_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
