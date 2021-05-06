/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

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
//Macro for some common command.
#define insert(a, b, c) (a) = (((a) < (b)) ? (b) : (((a) > (c)) ? c : a))
#define judge_in(a, b, c) (((b) <= (a) && (a) <= (c)) ? 1 : 0)
#define insert_in(a, b, c) (((a) < (b)) ? (b) : (((a) > (c)) ? c : a))
#define maxab(a, b) ((a) < (b) ? b : a)
#define minab(a, b) ((a) > (b) ? b : a)
  /* USER CODE END EM */

  /* Exported functions prototypes ---------------------------------------------*/
  void Error_Handler(void);

/* USER CODE BEGIN EFP */
#ifdef USE_FULL_ASSERT
  /**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
  void assert_failed(uint8_t *file, uint32_t line);
#endif /* USE_FULL_ASSERT */
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */
#define S1_Pin GPIO_PIN_6
#define S1_GPIO_Port GPIOB
#define S2_Pin GPIO_PIN_7
#define S2_GPIO_Port GPIOB

#define BT_RST_Pin GPIO_PIN_5
#define BT_RST_GPIO_Port GPIOA
#define BT_CLR_Pin GPIO_PIN_5
#define BT_CLR_GPIO_Port GPIOB

/* USER CODE END Private defines */
#ifdef __cplusplus
}
#endif
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
