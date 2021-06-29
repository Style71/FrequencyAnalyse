/**
  ******************************************************************************
  * File Name          : USART.cpp
  * Description        : This file provides function definition for the USART instances. 
  *                       The code use UASRT2 and configure GPIOD5 for USART2_TX, GPIOD6 for USART2_RX,
  *                       DMA1 is used to transfer received/transmitted data to USART buffer/register.
  * Author             : Qi Yang
  * Date               : 2021-03-16
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
Queue<unsigned char, USART_TXRX_BUFFER_SIZE> USART2_TX_Stream;
Queue<unsigned char, USART_TXRX_BUFFER_SIZE> USART2_RX_Stream;
/* USER CODE END 0 */

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;

/* USART2 init function */

void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_UART_Receive_DMA(&huart2, USART2_RX_Stream.queue, 1024);
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (uartHandle->Instance == USART2)
  {
    /* USER CODE BEGIN USART2_MspInit 0 */

    /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART2 GPIO Configuration    
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_TX Init */
    hdma_usart2_tx.Instance = DMA1_Stream6;
    hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_usart2_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    hdma_usart2_tx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_usart2_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmatx, hdma_usart2_tx);

    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Stream5;
    hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    /*
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_usart2_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_3QUARTERSFULL;
    */
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma_usart2_rx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_usart2_rx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, hdma_usart2_rx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 4, 2);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    /* USER CODE BEGIN USART2_MspInit 1 */

    /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{
  if (uartHandle->Instance == USART2)
  {
    /* USER CODE BEGIN USART2_MspDeInit 0 */

    /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration    
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX 
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5 | GPIO_PIN_6);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
    /* USER CODE BEGIN USART2_MspDeInit 1 */

    /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void USART_20Hz_Routine()
{
  uint32_t uiNbDataToTransmit;

  if (!USART2_TX_Stream.isEmpty())
  {

    if (USART2_TX_Stream.get_head() >= USART2_TX_Stream.get_tail())
      uiNbDataToTransmit = USART2_TX_Stream.capacity() - USART2_TX_Stream.get_head();
    else
      uiNbDataToTransmit = USART2_TX_Stream.size();

    if (HAL_UART_Transmit_DMA(&huart2, &USART2_TX_Stream[0], uiNbDataToTransmit) == HAL_OK)
      USART2_TX_Stream.pop_front(uiNbDataToTransmit);
  }

  int tail = USART_TXRX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx), head = USART2_RX_Stream.get_head();
  // Only handle situations that the tail pointer is not reaching the end of the queue;
  if (tail != USART_TXRX_BUFFER_SIZE)
  {
    USART2_RX_Stream.set_tail(tail);

    if (head < tail)
      USART2_RX_Stream.set_length(tail - head);
    else if (head > tail)
      USART2_RX_Stream.set_length(tail + USART_TXRX_BUFFER_SIZE - head);
    else
    { // if (Head == Tail), then check length to determine whether the queue is full or empty.
      if (USART2_RX_Stream.size() > 0)
        USART2_RX_Stream.set_length(USART_TXRX_BUFFER_SIZE);
    }
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    USART2_RX_Stream.set_tail(0);
    USART2_RX_Stream.set_length(USART_TXRX_BUFFER_SIZE - USART2_RX_Stream.get_head());

    HAL_UART_Receive_DMA(huart, USART2_RX_Stream.queue, 1024);
  }
}

void USART_Putchars(UART_HandleTypeDef *huart, const char *pucArray, int size)
{
  if (huart->Instance == USART2)
  {
    for (int i = 0; i < size; i++)
      USART2_TX_Stream.brute_push_back(pucArray[i]);
  }
}

void USART_Puts(UART_HandleTypeDef *huart, const char *pucStr)
{
  int i = 0;
  if (huart->Instance == USART2)
  {
    while (pucStr[i] != '\0')
    {
      USART2_TX_Stream.brute_push_back(pucStr[i++]);
    }
  }
}

void USART_Putc(UART_HandleTypeDef *huart, const char cChar)
{
  if (huart->Instance == USART2)
    USART2_TX_Stream.brute_push_back(cChar);
}

int USART_Printf(UART_HandleTypeDef *huart, const char *pcString, ...)
{
#define PRINTF_BUFFER_SIZE 128
  int sTotal = 0;
  char pucBuffer[PRINTF_BUFFER_SIZE];
  //Acquire a pointer to the argument list.
  va_list ap;

  //Initialize the list pointer.
  va_start(ap, pcString);
  //Formating the output string.
  sTotal = vsnprintf((char *)pucBuffer, PRINTF_BUFFER_SIZE, pcString, ap);
  // The return value sTotal indicate the number of characters that would have been written if PRINTF_BUFFER_SIZE had been sufficiently large,
  // not counting the terminating null character. If the resulting string would be longer than PRINTF_BUFFER_SIZE-1 characters,
  // the remaining characters are discarded and not stored in pucBuffer, but counted in sTotal.
  if (sTotal > 0)
  {
    // Write this portion of the string. Since sTotal may not indicate the actuall characters count in pucBuffer,
    // and pucBuffer is guaranteed to end with '\0', so we use puts() to transmit the message.
    USART_Puts(huart, pucBuffer);
  }
  va_end(ap);

  return sTotal;
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
