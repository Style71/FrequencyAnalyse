/**
  ******************************************************************************
  * File Name          : ADC.h
  * Description        : This file provides code for the configuration
  *                      of the ADC instances.
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __adc_H
#define __adc_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define ADC_BUFFER_SIZE 1536 // ADC data DMA buffer size for both buffers.

// ADC sample frequency.
// This macro is used only for signal processing reference, change this macro will not affect the actual sample rate.
// If you want to change the sample frequency, consider change the .Prescaler and .Period of TIM2.
#define SAMPLE_FREQ 4000

extern ADC_HandleTypeDef hadc1;
extern uint16_t ADCResult0[ADC_BUFFER_SIZE];
extern uint16_t ADCResult1[ADC_BUFFER_SIZE];
/* USER CODE END Private defines */

void MX_ADC1_Init(void);

/* USER CODE BEGIN Prototypes */
/**
  * @brief  Return current DMA data transfer buffer in double buffer mode.
  * @retval return true if current buffer is buffer1, and false if current buffer is buffer0.
  */
inline bool getCurrentADCBuffer()
{
  return (HAL_IS_BIT_SET(hadc1.DMA_Handle->Instance->CR, DMA_SxCR_CT));
}
/* USER CODE END Prototypes */
#endif /*__ adc_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
