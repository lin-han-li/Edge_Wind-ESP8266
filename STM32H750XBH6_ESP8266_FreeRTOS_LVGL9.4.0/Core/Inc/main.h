/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern osMutexId_t mutex_id;
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
#define ESP8266_RX_Pin GPIO_PIN_5
#define ESP8266_RX_GPIO_Port GPIOD
#define ESP8266_RST_Pin GPIO_PIN_4
#define ESP8266_RST_GPIO_Port GPIOD
#define ADS131A04_DRDY_Pin GPIO_PIN_11
#define ADS131A04_DRDY_GPIO_Port GPIOG
#define ESP8266_TX_Pin GPIO_PIN_6
#define ESP8266_TX_GPIO_Port GPIOD
#define ADS131A04_CS_Pin GPIO_PIN_2
#define ADS131A04_CS_GPIO_Port GPIOE
#define ADS131A04_DIN_Pin GPIO_PIN_5
#define ADS131A04_DIN_GPIO_Port GPIOE
#define ADS131A04_DONE_Pin GPIO_PIN_4
#define ADS131A04_DONE_GPIO_Port GPIOE
#define ADS131A04_DOUT_Pin GPIO_PIN_3
#define ADS131A04_DOUT_GPIO_Port GPIOE
#define ADS131A04_REST_Pin GPIO_PIN_6
#define ADS131A04_REST_GPIO_Port GPIOE
#define ADS131A04_SCLK_Pin GPIO_PIN_3
#define ADS131A04_SCLK_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
