/******************************************************************************
* 	SysTick.c
*
* 	This source file handles all SysTick initialization and functionality for
* 	time slice preemptive multitasking. Dependent on 2.097 MHz Cortex system
* 	timer and
*
* 	MCU: STM32L053R8
*
* 	12/08/2018:
* 	Added initialization and handler increments. Added wait task.
*
* 	Created on: 12/08/2018
* 	Author: Anthony Needles
******************************************************************************/
#include "stm32l053xx.h"
#include "SysTick.h"

// Cycles per millisecond dependent on use of 2.097 MHz low-power MSI
#define CLKCYCLES_ONE_MS 2097

// 1ms global counting variable
static volatile uint32_t systickCurrentCount;

// Global variable to hold count at last time slice period to determine current elapsed time
static uint32_t systickLastCount;

// Flag that allows ensuring initialization of SysTick
static uint8_t systickInitFlag;

/********************************************************************
* SysTickInit - Initializes SysTick system
*
* Description:  Enables SysTick timer via CMSIS SysTick_Config with
* 				with required clock cycles to result in 1ms interrupts.
* 				Error value returned from SysTick_Config also passed back.
*
* Return value: Error value returned from SysTick_Config
*
* Arguments:    None
********************************************************************/
uint32_t SysTickInit(void)
{
	uint32_t st_error;
	systickCurrentCount = 0;
	systickInitFlag = 1;
	st_error = SysTick_Config(CLKCYCLES_ONE_MS);
	return st_error;
}

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
void SysTickWaitTask(const uint32_t ts_period)
{
    if(systickInitFlag == 0){
        while((systickCurrentCount - systickLastCount) < ts_period){}
    }else{
    	systickInitFlag = 0;
    }
    systickLastCount = systickCurrentCount;
}

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
void SysTick_Handler(void)
{
	systickCurrentCount++;
}
