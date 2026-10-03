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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_BLUE_Pin GPIO_PIN_13
#define LED_BLUE_GPIO_Port GPIOC
#define PWM_L_Pin GPIO_PIN_0
#define PWM_L_GPIO_Port GPIOA
#define PWM_R_Pin GPIO_PIN_1
#define PWM_R_GPIO_Port GPIOA
#define K_4_Pin GPIO_PIN_4
#define K_4_GPIO_Port GPIOA
#define K_3_Pin GPIO_PIN_5
#define K_3_GPIO_Port GPIOA
#define K_2_Pin GPIO_PIN_0
#define K_2_GPIO_Port GPIOB
#define K_1_Pin GPIO_PIN_1
#define K_1_GPIO_Port GPIOB
#define MyI2C_SCL_Pin GPIO_PIN_10
#define MyI2C_SCL_GPIO_Port GPIOB
#define MyI2C_SDA_Pin GPIO_PIN_11
#define MyI2C_SDA_GPIO_Port GPIOB
#define AIN_1_Pin GPIO_PIN_12
#define AIN_1_GPIO_Port GPIOB
#define AIN_2_Pin GPIO_PIN_13
#define AIN_2_GPIO_Port GPIOB
#define BIN_1_Pin GPIO_PIN_14
#define BIN_1_GPIO_Port GPIOB
#define BIN_2_Pin GPIO_PIN_15
#define BIN_2_GPIO_Port GPIOB
#define OLED_SCL_Pin GPIO_PIN_8
#define OLED_SCL_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_9
#define OLED_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
