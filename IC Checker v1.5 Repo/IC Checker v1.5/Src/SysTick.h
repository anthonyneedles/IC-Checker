/******************************************************************************
* 	SysTick.h
*
* 	Header for SysTick.c
*
* 	MCU: STM32L053R8
*
* 	12/08/2018:
* 	Added initialization and handler increments.
*
* 	Created on: 12/08/2018
* 	Author: Anthony Needles
******************************************************************************/
#ifndef SYSTICK_H_
#define SYSTICK_H_

/********************************************************************
* SysTickInit - Initializes SysTick system
*
* Description:  Enables SysTick timer via CMSIS SysTick_Config with
* 				with required clock cycles to result in 1ms interrupts.
* 				Error value returned from SysTick_Config also passed back.
*
* Return value: st_error - Error value returned from SysTick_Config
*
* Arguments:    None
********************************************************************/
uint32_t SysTickInit(void);

/********************************************************************
* SysTickWait - Waits until next time slice period
*
* Description:  Upon first time pass saves current millisecond count
* 				value. Every following call will result in program idle
* 				until next time slice period.
*
* Return value: None
*
* Arguments:    ts_period - Desired time slice period
********************************************************************/
void SysTickWaitTask(const uint32_t);

/********************************************************************
* SysTickHandler - Handles interrupts from SysTick timer
*
* Description:  Interrupts will occur at 1 kHz (every millisecond).
* 				Increments millisecond count every entrance.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void SysTick_Handler(void);

#endif /* SYSTICK_H_ */
