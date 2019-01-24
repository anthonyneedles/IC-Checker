/******************************************************************************
* 	IC_Checker v2.0
*
* 	This program file is for testing capabilities of STML053R8T6 Nucleo64
*
* 	MCU: STM32L053R8
*
* 	01/15/2019:
* 	Copied code base from IC_Checker v2.0 and removed unneeded LED module.
*
* 	01/17/2019:
* 	Created and completed basic 10kHz I2C module for master broadcast communication
* 	to PCF8574 I2C port expander necessary for LCD functionality. Added I2C
* 	initialization function to init list and added header include.
*
* 	01/21/2019:
* 	Created LCD module for communicating with HD44780 LCD driver
* 	for LCD human interface. Added LCD initialization function to init list and
* 	added header include. NOTE: LCD init must be performed AFTER I2C init. Updated
* 	all comments.
*
* 	Created on: 01/15/2019
* 	Author: Anthony Needles
******************************************************************************/
#include "stm32l053xx.h"
#include "SysTick.h"
#include "Button.h"
#include "Checker.h"
#include "I2C.h"
#include "LCD.h"

/******************************************************************************
* Public Definitions
******************************************************************************/
#define TIMESLICE_PERIOD_MS 7
// Super loop period created by SysTickWaitTask (in milliseconds)

#define MASK_FAILURE 0x00000000
#define MASK_74HC00 0x00000001
#define MASK_74HC02	0x00000002
#define MASK_74HC04 0x00000004
#define MASK_74HC08 0x00000008
#define MASK_74HC10 0x00000010
#define MASK_74HC20 0x00000020
#define MASK_74HC27 0x00000040
#define MASK_74HC86 0x00000080
// Bit field mask for setting a single bit for a specific test pass, one per IC

typedef enum{IDLE, CHECK_74HC00, CHECK_74HC02, CHECK_74HC04, CHECK_74HC08, CHECK_74HC10,
			 CHECK_74HC20, CHECK_74HC27, CHECK_74HC86, DISPLAY_RESULT} CONTROL_STATE_T;
// Main test control state machine state enumerations

/******************************************************************************
* Public Constants
******************************************************************************/
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
// 74HCXX Parameters: IC Designator, # of inputs, # of outputs, list of input pins,
// and list of output pins
// Note: Input lists shall have all input pin(s) for a certain gate grouped together,
// and their corresponding output pin shall be placed accordingly in the output list

/******************************************************************************
* Public Constants
******************************************************************************/
const uint8_t TestingPrompt[] = "Testing...";
const uint8_t FailPrompt[] = "Test Fail";
const uint8_t Pass74HC00Prompt[] = "74HC00 Passed";
const uint8_t Pass74HC02Prompt[] = "74HC02 Passed";
const uint8_t Pass74HC04Prompt[] = "74HC04 Passed";
const uint8_t Pass74HC08Prompt[] = "74HC08 Passed";
const uint8_t Pass74HC10Prompt[] = "74HC10 Passed";
const uint8_t Pass74HC20Prompt[] = "74HC20 Passed";
const uint8_t Pass74HC27Prompt[] = "74HC27 Passed";
const uint8_t Pass74HC86Prompt[] = "74HC86 Passed";
const uint8_t MultiPrompt[] = "Multipass Error";
// String to be displayed at the end of each test as well as when in the middle
// of testing.

/******************************************************************************
* Public Function Prototypes
******************************************************************************/
void ControlTask(void);
void DisplayResult(uint32_t);

int main(void)
{
	SysTickInit();
	ButtonInit();
	CheckerInit();
	I2CInit();
	LCDInit();
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
				LCDClearDisplay();
				LCDMoveCursor(1U,1U);
				LCDDisplayString(TestingPrompt);
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
			DisplayResult(result_field);
			result_field = 0x00000000;
			control_state = IDLE;
			break;

		default:
			result_field = 0x00000000;
			break;
	}
}

/********************************************************************
* Display Result - Function for managing user prompts
*
* Description:  Upon finishing testing, the result is passed to this
* 				function to determine which prompt to display to the
* 				user. The display will clear and the cursor will return
* 				to the home position (1,1). The prompt will display for
* 				LCD_REPROMPT_DELAY_SECONDS located in LCD.h.
*
* Return value:	none
*
* Arguments:    uint32_t result_field - Bit field with each bit
* 				representing pass/fail (1/0) for each test.
********************************************************************/
void DisplayResult(uint32_t result_field)
{
	switch(result_field)
	{
		case MASK_FAILURE:
			LCDClearDisplay();
			LCDMoveCursor(1U,1U);
			LCDDisplayString(FailPrompt);
			break;

		case MASK_74HC00:
			LCDClearDisplay();
			LCDMoveCursor(1U,1U);
			LCDDisplayString(Pass74HC00Prompt);
			break;

		case MASK_74HC02:
			LCDClearDisplay();
			LCDMoveCursor(1U,1U);
			LCDDisplayString(Pass74HC02Prompt);
			break;

		case MASK_74HC04:
			LCDClearDisplay();
			LCDMoveCursor(1U,1U);
			LCDDisplayString(Pass74HC04Prompt);;
			break;

		case MASK_74HC08:
			LCDClearDisplay();
			LCDMoveCursor(1U,1U);
			LCDDisplayString(Pass74HC08Prompt);
			break;

		case MASK_74HC10:
			LCDClearDisplay();
			LCDMoveCursor(1U,1U);
			LCDDisplayString(Pass74HC10Prompt);
			break;

		case MASK_74HC20:
			LCDClearDisplay();
			LCDMoveCursor(1U,1U);
			LCDDisplayString(Pass74HC20Prompt);
			break;

		case MASK_74HC27:
			LCDClearDisplay();
			LCDMoveCursor(1U,1U);
			LCDDisplayString(Pass74HC27Prompt);
			break;

		case MASK_74HC86:
			LCDClearDisplay();
			LCDMoveCursor(1U,1U);
			LCDDisplayString(Pass74HC86Prompt);
			break;

		default:
			LCDClearDisplay();
			LCDMoveCursor(1U,1U);
			LCDDisplayString(MultiPrompt);
			break;
	}
}
