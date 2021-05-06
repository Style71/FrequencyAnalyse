/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
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

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
Queue<unsigned char, USART_TXRX_BUFFER_SIZE> USART1_TX_Stream;
Queue<unsigned char, USART_TXRX_BUFFER_SIZE> USART1_RX_Stream;
Queue<unsigned char, USART_TXRX_BUFFER_SIZE> USART2_TX_Stream;
Queue<unsigned char, USART_TXRX_BUFFER_SIZE> USART2_RX_Stream;
/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_UART_Receive_DMA(&huart1, USART1_RX_Stream.queue, USART_TXRX_BUFFER_SIZE);
}
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
  HAL_UART_Receive_DMA(&huart2, USART2_RX_Stream.queue, USART_TXRX_BUFFER_SIZE);
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (uartHandle->Instance == USART1)
  {
    /* USER CODE BEGIN USART1_MspInit 0 */

    /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART1 GPIO Configuration    
    PB14     ------> USART1_TX
    PB15     ------> USART1_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /* USART1 DMA Init */
    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA2_Stream7;
    hdma_usart1_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_usart1_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    hdma_usart1_tx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_usart1_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmatx, hdma_usart1_tx);

    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA2_Stream2;
    hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_NORMAL;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma_usart1_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_3QUARTERSFULL;
    hdma_usart1_rx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_usart1_rx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, hdma_usart1_rx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 2);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    /* USER CODE BEGIN USART1_MspInit 1 */

    /* USER CODE END USART1_MspInit 1 */
  }
  else if (uartHandle->Instance == USART2)
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

  if (uartHandle->Instance == USART1)
  {
    /* USER CODE BEGIN USART1_MspDeInit 0 */

    /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration    
    PB14     ------> USART1_TX
    PB15     ------> USART1_RX 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14 | GPIO_PIN_15);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_DMA_DeInit(uartHandle->hdmarx);
    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    /* USER CODE BEGIN USART1_MspDeInit 1 */

    /* USER CODE END USART1_MspDeInit 1 */
  }
  else if (uartHandle->Instance == USART2)
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
  uint32_t uiBytesToTransmit;

  if (!USART1_TX_Stream.isEmpty())
  {
    if (USART1_TX_Stream.get_head() >= USART1_TX_Stream.get_tail())
      uiBytesToTransmit = USART1_TX_Stream.capacity() - USART1_TX_Stream.get_head();
    else
      uiBytesToTransmit = USART1_TX_Stream.size();

    if (HAL_UART_Transmit_DMA(&huart1, &USART1_TX_Stream[0], uiBytesToTransmit) == HAL_OK)
      USART1_TX_Stream.pop_front(uiBytesToTransmit);
  }

  if (!USART2_TX_Stream.isEmpty())
  {
    if (USART2_TX_Stream.get_head() >= USART2_TX_Stream.get_tail())
      uiBytesToTransmit = USART2_TX_Stream.capacity() - USART2_TX_Stream.get_head();
    else
      uiBytesToTransmit = USART2_TX_Stream.size();

    if (HAL_UART_Transmit_DMA(&huart2, &USART2_TX_Stream[0], uiBytesToTransmit) == HAL_OK)
      USART2_TX_Stream.pop_front(uiBytesToTransmit);
  }

  int tail = USART_TXRX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx), head = USART1_RX_Stream.get_head();
  // Only handle situations that the tail pointer is not reaching the end of the queue;
  if (tail != USART_TXRX_BUFFER_SIZE)
  {
    USART1_RX_Stream.set_tail(tail);

    if (head < tail)
      USART1_RX_Stream.set_length(tail - head);
    else if (head > tail)
      USART1_RX_Stream.set_length(tail + USART_TXRX_BUFFER_SIZE - head);
    else
    { // if (Head == Tail), then check length to determine whether the queue is full or empty.
      if (USART1_RX_Stream.size() > 0)
        USART1_RX_Stream.set_length(USART_TXRX_BUFFER_SIZE);
    }
  }

  tail = USART_TXRX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx);
  head = USART2_RX_Stream.get_head();
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
  if (huart->Instance == USART1)
  {
    USART1_RX_Stream.set_tail(0);
    USART1_RX_Stream.set_length(USART_TXRX_BUFFER_SIZE - USART1_RX_Stream.get_head());

    HAL_UART_Receive_DMA(huart, USART1_RX_Stream.queue, USART_TXRX_BUFFER_SIZE);
  }

  if (huart->Instance == USART2)
  {
    USART2_RX_Stream.set_tail(0);
    USART2_RX_Stream.set_length(USART_TXRX_BUFFER_SIZE - USART2_RX_Stream.get_head());

    HAL_UART_Receive_DMA(huart, USART2_RX_Stream.queue, USART_TXRX_BUFFER_SIZE);
  }
}

void USART_Putchars(UART_HandleTypeDef *huart, const char *pucArray, int size)
{
  static uint64_t BytesBufferedTX1 = 0, BytesBufferedTX2 = 0;
  if (huart->Instance == USART1)
  {
    for (int i = 0; i < size; i++)
      USART1_TX_Stream.brute_push_back(pucArray[i]);
    BytesBufferedTX1 += size;
  }
  else if (huart->Instance == USART2)
  {
    for (int i = 0; i < size; i++)
      USART2_TX_Stream.brute_push_back(pucArray[i]);
    BytesBufferedTX2 += size;
  }
}

void USART_Puts(UART_HandleTypeDef *huart, const char *pucStr)
{
  int i = 0;
  if (huart->Instance == USART1)
  {
    while (pucStr[i] != '\0')
    {
      USART1_TX_Stream.brute_push_back(pucStr[i++]);
    }
  }
  else if (huart->Instance == USART2)
  {
    while (pucStr[i] != '\0')
    {
      USART2_TX_Stream.brute_push_back(pucStr[i++]);
    }
  }
}

void USART_Putc(UART_HandleTypeDef *huart, const char cChar)
{
  if (huart->Instance == USART1)
    USART1_TX_Stream.brute_push_back(cChar);
  else if (huart->Instance == USART2)
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
  if (sTotal > 0)
  {
    // Write this portion of the string.
    USART_Putchars(huart, pucBuffer, sTotal);
  }
  va_end(ap);

  return sTotal;
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
