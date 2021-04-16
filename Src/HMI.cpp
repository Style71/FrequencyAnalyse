/**
  ******************************************************************************
  * File Name          : HMI.c
  * Description        : This file contains all the functions prototypes for 
  *                      the push buttons.  
	* Author						 : Qi Yang
	* Date							 : Oct, 31, 2020
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "HMI.h"
#include "usart.h"
#include "adc.h"
#include "DSP.h"

extern PrintState stage;
extern int oneshoot_count;

Button Buttons_list[] = {
	BUTTON_INIT(S1),
	BUTTON_INIT(S2),
};

void UpdateButtons(void)
{
	Button *pBTN;
	for (int i = 0; i < (sizeof(Buttons_list) / sizeof(Button)); i++)
	{
		pBTN = &Buttons_list[i];
		if (!HAL_GPIO_ReadPin(pBTN->port, pBTN->pin))
		{
			pBTN->count++;
			if (pBTN->count == 1)
				pBTN->state |= BTN1_CLICK;
			else if ((pBTN->count > 50) && ((pBTN->count % 10) == 0)) //If the button is pressed for more than 0.5 seconds, invoke the OnBTN1Press routine.
				pBTN->state |= BTN1_PRESS;							  //This routine is invoked 10 times per second.
		}
		else
		{
			if (pBTN->count > 0)
			{
				pBTN->state |= BTN1_RELEASE;
				pBTN->count = 0;
			}
		}
	}
}

void ExecuteBtnRoutine(void)
{
	Button *pBTN;
	for (int i = 0; i < (sizeof(Buttons_list) / sizeof(Button)); i++)
	{
		pBTN = &Buttons_list[i];
		//If the flags are on, invoke the corresponding button routines.
		if (pBTN->state & BTN1_CLICK)
		{
			pBTN->ClickFunc();
			//Button Routine finished, clear the update flag.
			pBTN->state &= ~BTN1_CLICK;
		}

		if (pBTN->state & BTN1_PRESS)
		{
			pBTN->PressFunc();
			pBTN->state &= ~BTN1_PRESS;
		}

		if (pBTN->state & BTN1_RELEASE)
		{
			pBTN->ReleaseFunc();
			pBTN->state &= ~BTN1_RELEASE;
		}
	}
}

void OnBTNS1Click(void)
{
	oneshoot_count = 0;
	stage = WaitSample;
	//USART_Printf(&huart1, "OnBTNS1Click() invoked.\n");
}

void OnBTNS1Press(void)
{
	USART_Printf(&huart1, "OnBTNS1Press() invoked.\n");
}

void OnBTNS1Release(void)
{
	USART_Printf(&huart1, "OnBTNS1Release() invoked.\n");
}

void OnBTNS2Click(void)
{
	USART_Printf(&huart2, "OnBTNS2Click() invoked.\n");
}

void OnBTNS2Press(void)
{
	USART_Printf(&huart2, "OnBTNS2Press() invoked.\n");
}

void OnBTNS2Release(void)
{
	USART_Printf(&huart2, "OnBTNS2Release() invoked.\n");
}