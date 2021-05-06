/**
  ******************************************************************************
  * File Name          : HMI.h
  * Description        : This file contains all the functions prototypes for 
  *                      the push buttons.  
	* Author						 : Qi Yang
	* Date							 : Oct, 31, 2020
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _HMI_H
#define _HMI_H

#include <stdint.h>
#include "main.h"

/**
  * @brief  Button state definition.
  */
typedef enum
{
  BTN_IDLE = 0x00U,
  BTN1_CLICK = 0x01U,
  BTN1_PRESS = 0x02U,
  BTN1_RELEASE = 0x04U
} Button_State;

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
  uint8_t state;
  uint8_t count;
  void (*ClickFunc)(void);
  void (*PressFunc)(void);
  void (*ReleaseFunc)(void);
} Button;

#define BUTTON_INIT(NAME)     \
  {                                      \
    .port = NAME##_GPIO_Port,          \
    .pin = NAME##_Pin,                 \
    .state = BTN_IDLE,                   \
    .count = 0,                          \
    .ClickFunc = OnBTN##NAME##Click,     \
    .PressFunc = OnBTN##NAME##Press,     \
    .ReleaseFunc = OnBTN##NAME##Release, \
  }

//*****************************************************************************
// Update the buttons' state, this function should be involked 100 times per second.
//*****************************************************************************
void UpdateButtons(void);

//*****************************************************************************
// Execute button routines, put this function in main loop.
//*****************************************************************************
void ExecuteBtnRoutine(void);

void OnBTNS1Click(void);

void OnBTNS1Press(void);

void OnBTNS1Release(void);

void OnBTNS2Click(void);

void OnBTNS2Press(void);

void OnBTNS2Release(void);

#endif /*__HMI_H */
