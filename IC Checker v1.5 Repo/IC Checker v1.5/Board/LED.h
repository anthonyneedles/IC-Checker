/******************************************************************************
* 	LED.h
*
* 	Header for LED.c
*
* 	MCU: STM32L053R8
*
* 	Created on: 12/07/2018
* 	Author: Anthony Needles
******************************************************************************/
#ifndef LED_H_
#define LED_H_

// Desired automatic time-out turn off delay for the LED
#define LED_TURNOFF_DELAY_SECONDS 10

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
void LEDInit(void);

/********************************************************************
* LEDGreenOn - Turns LED green
*
* Description:  Enables green driver and disables red driver
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void LEDGreenOn(void);

/********************************************************************
* LEDRedOn - Turns LED red
*
* Description:  Enables red driver and disables green driver
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void LEDRedOn(void);

/********************************************************************
* LEDOrangeOn - Turns LED orange
*
* Description:  Enables green driver and enables red driver
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void LEDOrangeOn(void);

/********************************************************************
* LEDsOff - Turns LED off
*
* Description:  Disables green driver and disables red driver
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void LEDsOff(void);

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
void PIT0_IRQHandler(void);

#endif /* LED_H_ */
