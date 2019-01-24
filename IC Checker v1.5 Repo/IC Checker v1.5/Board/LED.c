/******************************************************************************
* 	LED.c
*
* 	This source file is used to contain initialization and functionality of
* 	dual color LED on IC Checker v1.5 Shield. LEDs driving ports can be
* 	configured, as well as called to turn on/off. Dependent on 2.097 MHz APB2
* 	timer clock to perform time-out turn off LEDs.
*
* 	MCU: STM32L053R8
*
* 	12/07/2018:
*	Added LEDInit, LEDGreenOn, LEDRedOn, LEDOrangeOn, LEDGreenOff,  LEDRedOff,
*	LEDOrangeOff
*
* 	Created on: 12/07/2018
* 	Author: Anthony Needles
******************************************************************************/
#include "stm32l053xx.h"
#include "LED.h"

// Prescaler needed as time-out time scale is in seconds and unscaled counter
// will result in early roll over
#define TIMER_PRESCALER 4095
#define CLKCYCLES_ONE_SECOND 2097000/TIMER_PRESCALER

// Flag set by LEDInit() so that first time IRQ entrance will bypass
static uint8_t ledInitFlag;

/********************************************************************
* LEDInit - Initializes LEDs
*
* Description:  Enables clock for GPIOB, GPIOC, and TIM21. Enables output
* 				mode for PB2 and PC4. Configures TIM21 for LED timeout disable.
* 				Enables TIM21 interrupts.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void LEDInit(void)
{
	RCC->IOPENR |= (RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN);

	RCC->APB2ENR |= RCC_APB2ENR_TIM21EN ;

	GPIOB->MODER &= ~(GPIO_MODER_MODE2_1);
	GPIOC->MODER &= ~(GPIO_MODER_MODE4_1);

	TIM21->DIER |= TIM_DIER_UIE;
	TIM21->PSC = TIMER_PRESCALER;
	TIM21->ARR = (LED_TURNOFF_DELAY_SECONDS*CLKCYCLES_ONE_SECOND);

	NVIC_EnableIRQ(TIM21_IRQn);

	ledInitFlag = 1;
}

/********************************************************************
* LEDGreenOn - Turns LED green
*
* Description:  Enables green driver and disables red driver. Enables
* 				time-out turn off timer.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void LEDGreenOn(void)
{
	GPIOC->ODR |= (GPIO_ODR_OD4);
	GPIOB->ODR &= ~(GPIO_ODR_OD2);
	TIM21->CR1 |= TIM_CR1_CEN;
}

/********************************************************************
* LEDRedOn - Turns LED red
*
* Description:  Enables red driver and disables green driver. Enables
* 				time-out turn off timer.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void LEDRedOn(void)
{
	GPIOB->ODR |= (GPIO_ODR_OD2);
	GPIOC->ODR &= ~(GPIO_ODR_OD4);
	TIM21->CR1 |= TIM_CR1_CEN;
}

/********************************************************************
* LEDOrangeOn - Turns LED orange
*
* Description:  Enables green driver and enables red driver. Enables
* 				time-out turn off timer.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void LEDOrangeOn(void)
{
	GPIOC->ODR |= (GPIO_ODR_OD4);
	GPIOB->ODR |= (GPIO_ODR_OD2);
	TIM21->CR1 |= TIM_CR1_CEN;
}

/********************************************************************
* LEDsOff - Turns LED off
*
* Description:  Disables green driver and disables red driver
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void LEDsOff(void)
{
	GPIOB->ODR &= ~(GPIO_ODR_OD2);
	GPIOC->ODR &= ~(GPIO_ODR_OD4);
}

/********************************************************************
* TIM21_IRQHandler - Handles interrupt events for TIM21
*
* Description:  When TIM21 reaches end of counting (after delay of
* 				LED_TURNOFF_DELAY_SECONDS) interrupt flag is cleared,
* 				then both LEDs are turned off. The counter is then
* 				disabled.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void TIM21_IRQHandler(void)
{
	TIM21->SR &= ~TIM_SR_UIF;
	if(ledInitFlag == 1)
	{
		ledInitFlag = 0;
	} else
	{
		TIM21->CR1 &= ~TIM_CR1_CEN;
		LEDsOff();
	}

}
