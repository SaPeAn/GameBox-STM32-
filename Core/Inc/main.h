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

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SHUTDOWN_Pin GPIO_PIN_13
#define SHUTDOWN_GPIO_Port GPIOC
#define BAT_LVL_CTRL_Pin GPIO_PIN_0
#define BAT_LVL_CTRL_GPIO_Port GPIOA
#define DISP_RS_Pin GPIO_PIN_1
#define DISP_RS_GPIO_Port GPIOA
#define DISP_RSE_Pin GPIO_PIN_2
#define DISP_RSE_GPIO_Port GPIOA
#define JOYSTICK_BTN_Pin GPIO_PIN_3
#define JOYSTICK_BTN_GPIO_Port GPIOA
#define SOUND_OUT_Pin GPIO_PIN_4
#define SOUND_OUT_GPIO_Port GPIOA
#define JOYSTICK_OX_Pin GPIO_PIN_0
#define JOYSTICK_OX_GPIO_Port GPIOB
#define JOYSTICK_OY_Pin GPIO_PIN_1
#define JOYSTICK_OY_GPIO_Port GPIOB
#define DISP_SPI_CS_Pin GPIO_PIN_12
#define DISP_SPI_CS_GPIO_Port GPIOB
#define DISP_SPI_SCK_Pin GPIO_PIN_13
#define DISP_SPI_SCK_GPIO_Port GPIOB
#define DISP_SPI_MOSI_Pin GPIO_PIN_15
#define DISP_SPI_MOSI_GPIO_Port GPIOB
#define BTN_3_Pin GPIO_PIN_4
#define BTN_3_GPIO_Port GPIOB
#define BTN_4_Pin GPIO_PIN_5
#define BTN_4_GPIO_Port GPIOB
#define BTN_1_Pin GPIO_PIN_6
#define BTN_1_GPIO_Port GPIOB
#define BTN_2_Pin GPIO_PIN_7
#define BTN_2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
