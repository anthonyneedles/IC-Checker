/******************************************************************************
* 	IC_Checker v1.5
*
* 	This program file is for testing capabilities of STML053R8T6 Nucleo64
*
* 	MCU: STM32L053R8
*
* 	12/07/2018:
* 	Added functionality for input/output GPIO ports. Starting incorporating LED.c
* 	and LED.h for cleaner LED on/off function calls.
*
* 	12/08/2018:
* 	Added systick timer and debounced button input.
*
* 	12/09/2018:
* 	Completed basic checker module functionality for specific ICs. Attempted
* 	creating generic function for any given IC.
*
* 	12/10/2018
* 	Created generic function for IC checking based on IC parameter structures.
* 	Basic testing flow created. Longest test length was measured to be ~7ms,
* 	so in order run all tests a state machine must handle transitions from
* 	test to test to keep time slice period under 50ms.
*
* 	12/16/2018
* 	Additional code readability added, along with supporting comments. State
* 	machine created for main control.
*
* 	12/24/2018
* 	Updated all comments project wide.
*
* 	Created on: 12/05/2018
* 	Author: Anthony Needles
******************************************************************************/
#include "stm32l053xx.h"
#include "LED.h"
#include "SysTick.h"
#include "Button.h"
#include "Checker.h"

// Super loop period created by SysTickWaitTask (in milliseconds)
#define TIMESLICE_PERIOD_MS 7

// Bit field mask for setting a single bit for a specific test pass, one per IC
#define MASK_74HC00 0x00000001
#define MASK_74HC02	0x00000002
#define MASK_74HC04 0x00000004
#define MASK_74HC08 0x00000008
#define MASK_74HC10 0x00000010
#define MASK_74HC20 0x00000020
#define MASK_74HC27 0x00000040
#define MASK_74HC86 0x00000080


// Main test control state machine state enumerations
typedef enum{IDLE, CHECK_74HC00, CHECK_74HC02, CHECK_74HC04, CHECK_74HC08, CHECK_74HC10,
			 CHECK_74HC20, CHECK_74HC27, CHECK_74HC86, DISPLAY_RESULT} CONTROL_STATE_T;

// 74HCXX Parameters: IC Designator, # of inputs, # of outputs, list of input pins,
// and list of output pins
// Note: Input lists shall have all input pin(s) for a certain gate grouped together,
// and their corresponding output pin shall be placed accordingly in the output list
const IC_PARAMETERS_T IC_74HC00_PARAM = {IC_74HC00, 8, 4, {1, 2, 4, 5, 9, 10, 12, 13},
										{3, 6, 8, 11} };

const IC_PARAMETERS_T IC_74HC02_PARAM = {IC_74HC02, 8, 4, {2, 3, 5, 6, 8, 9, 11, 12},
										{1, 4, 10, 13} };

const IC_PARAMETERS_T IC_74HC04_PARAM = {IC_74HC04, 6, 6, {1, 3, 5, 9, 11, 13},
										{2, 4, 6, 8, 10, 12} };

const IC_PARAMETERS_T IC_74HC08_PARAM = {IC_74HC08, 8, 4, {1, 2, 4, 5, 9, 10, 12, 13},
										{3, 6, 8, 11} };

const IC_PARAMETERS_T IC_74HC10_PARAM = {IC_74HC10, 9, 3, {1, 2, 13, 3, 4, 5, 9, 10, 11},
										{12, 6, 8} };

const IC_PARAMETERS_T IC_74HC20_PARAM = {IC_74HC20, 8, 2, {1, 2, 4, 5, 9, 10, 12, 13},
										{6, 8} };

const IC_PARAMETERS_T IC_74HC27_PARAM = {IC_74HC27, 9, 3, {1, 2, 13, 3, 4, 5, 9, 10, 11},
										{12, 6, 8} };

const IC_PARAMETERS_T IC_74HC86_PARAM = {IC_74HC86, 8, 4, {1, 2, 4, 5, 9, 10, 12, 13},
										{3, 6, 8, 11} };

void ControlTask(void);

int main(void)
{
	LEDInit();
	SysTickInit();
	ButtonInit();
	CheckerInit();

	// Super loop - executes loop every TIMESLICE_PERIOD_MS
	while(1)
	{
		SysTickWaitTask(TIMESLICE_PERIOD_MS);
		ButtonDBReadTask();
		ControlTask();
	}
}

/********************************************************************
* ControlTask - State machine for test transitions
*
* Description:  Main control state machine structure. Enters every
* 				TIMESLICE_PERIOD_MS. Resets to IDLE state, where
* 				button state is polled until an asserted value is read.
* 				Task then cycles through all given testing designators.
* 				One test is performed every TIMESLICE_PERIOD_MS. If a test
* 				is passed
*
* Return value:	none
*
* Arguments:    None
********************************************************************/
void ControlTask(void)
{
	static CONTROL_STATE_T control_state= IDLE;
	static uint32_t result_field = 0x00000000;

	switch(control_state){
		case IDLE:
			if(ButtonGet() == PRESSED)
			{
				control_state = CHECK_74HC00;
			} else
			{
				control_state = IDLE;
			}
			break;

		case CHECK_74HC00:
			if(CheckerTestIC(IC_74HC00_PARAM) == PASSED) result_field |= MASK_74HC00;
			control_state = CHECK_74HC02;
			break;

		case CHECK_74HC02:
			if(CheckerTestIC(IC_74HC02_PARAM) == PASSED) result_field |= MASK_74HC02;
			control_state = CHECK_74HC04;
			break;

		case CHECK_74HC04:
			if(CheckerTestIC(IC_74HC04_PARAM) == PASSED) result_field |= MASK_74HC04;
			control_state = CHECK_74HC08;
			break;

		case CHECK_74HC08:
			if(CheckerTestIC(IC_74HC08_PARAM) == PASSED) result_field |= MASK_74HC08;
			control_state = CHECK_74HC10;
			break;

		case CHECK_74HC10:
			if(CheckerTestIC(IC_74HC10_PARAM) == PASSED) result_field |= MASK_74HC10;
			control_state = CHECK_74HC20;
			break;

		case CHECK_74HC20:
			if(CheckerTestIC(IC_74HC20_PARAM) == PASSED) result_field |= MASK_74HC20;
			control_state = CHECK_74HC27;
			break;

		case CHECK_74HC27:
			if(CheckerTestIC(IC_74HC27_PARAM) == PASSED) result_field |= MASK_74HC27;
			control_state = CHECK_74HC86;
			break;

		case CHECK_74HC86:
			if(CheckerTestIC(IC_74HC86_PARAM) == PASSED) result_field |= MASK_74HC86;
			control_state = DISPLAY_RESULT;
			break;

		case DISPLAY_RESULT:
			if(result_field == 0x0000)
			{
				LEDRedOn();
			} else if((result_field & (result_field - 1)) == 0x0000)
			{ // "result_field & (result_field - 1)" will result in a zero if one
			  // and only one bit is set in result_field, i.e. is power of 2
				LEDGreenOn();
			} else
			{
				LEDOrangeOn();
			}
			result_field = 0x00000000;
			control_state = IDLE;
			break;

		default:
			result_field = 0x00000000;
			break;
	}
}
