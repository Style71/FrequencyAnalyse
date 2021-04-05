/**
  ******************************************************************************
  * File Name       : SysTime.c
  * Description     : System time base realised by systick timer.
	* Author					: Qi Yang
	* Date						: June, 08, 2020
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "SysTime.h"
#include "main.h"

#define TICKS_PER_US	216  // Equals to SystemCoreClock/1M

static uint64_t ullSysTicks = 0;
static uint64_t ullLastSysTicks;

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART2;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

uint64_t GetSysTicks()
{
  return (ullSysTicks + ((SysTick->LOAD & SysTick_LOAD_RELOAD_Msk) - (SysTick->VAL & SysTick_VAL_CURRENT_Msk)));
}

uint64_t GetUs()
{
	return (ullSysTicks + ((SysTick->LOAD & SysTick_LOAD_RELOAD_Msk) - (SysTick->VAL & SysTick_VAL_CURRENT_Msk)))/TICKS_PER_US;
}

void Tick()
{
  ullLastSysTicks = ullSysTicks + ((SysTick->LOAD & SysTick_LOAD_RELOAD_Msk) - (SysTick->VAL & SysTick_VAL_CURRENT_Msk));
}

uint64_t Tock()
{
  return ((ullSysTicks + ((SysTick->LOAD & SysTick_LOAD_RELOAD_Msk) - (SysTick->VAL & SysTick_VAL_CURRENT_Msk))) - ullLastSysTicks);
}

Time TimeConvert(uint64_t ullCounts)
{
  Time sTemp;
  uint64_t u64Us;

  //1 count is equal to 1M/ulSystemClock us, thus the total microsecond passed is TotalCount*1M/ulSystemClock.
  u64Us = ullCounts / TICKS_PER_US;

  sTemp.ulUs = u64Us % 1000;
  u64Us /= 1000;
  sTemp.ulMs = u64Us % 1000;
  u64Us /= 1000;
  sTemp.ulSecond = u64Us % 60;
  u64Us /= 60;
  sTemp.ulMinite = u64Us % 60;
  u64Us /= 60;
  sTemp.ulHour = u64Us;

  return sTemp;
}

Time Time_Substraction(const Time *psTimerA, const Time *psTimerB)
{
  Time sTemp = {0, 0, 0, 0, 0, 0};
  uint64_t u64Us;

  //We do subtraction only when TimerA is larger than TimerB, otherwise return zero
  if (psTimerA->u64Count > psTimerB->u64Count)
  {
    u64Us = psTimerA->u64Count - psTimerB->u64Count;

    sTemp = TimeConvert(u64Us);
  }
  return sTemp;
}

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  //Update system time ticks.
  /* USER CODE BEGIN SysTick_IRQn 0 */
  ullSysTicks += (SysTick->LOAD & SysTick_LOAD_RELOAD_Msk) + 1;
  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}