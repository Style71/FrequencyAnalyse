/**
  ******************************************************************************
  * File Name          : SysTime.h
  * Description        : This file contains all the functions prototypes for 
  *                      the system time.  
  * Author			 		   : Qi Yang
  * Date							 : June, 08, 2020
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTIME_H
#define __SYSTIME_H

#ifdef __cplusplus
extern "C" {
#endif
	
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
#include "core_cm7.h"
#include "stm32f7xx_hal.h"

typedef struct
{
  uint64_t u64Count;
  uint32_t ulUs;
  uint32_t ulMs;
  uint32_t ulSecond;
  uint32_t ulMinite;
  uint32_t ulHour;
} Time;
/* Exported functions prototypes ---------------------------------------------*/
/**
* @brief This function handles System tick timer.
*/
void SystemClock_Config(void);

void SysTick_Handler(void);

uint64_t GetSysTicks();

uint64_t GetUs();

void Tick();

uint64_t Tock();

Time TimeConvert(uint64_t ullCounts);

Time Time_Substraction(const Time *psTimerA, const Time *psTimerB);

#ifdef __cplusplus
}
#endif

#endif /*__SYSTIME_H */
