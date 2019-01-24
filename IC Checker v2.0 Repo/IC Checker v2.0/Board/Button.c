/******************************************************************************
* 	Button.c
*
* 	This source file is used to contain initialization and functionality of the
* 	button on IC Checker v1.5 Shield. Software debouncing is completed in a state
* 	machine structured task, with a minimum debouncing period of 2 time slice periods.
* 	Debounced button press state can be fetched by simple function call.
*
* 	MCU: STM32L053R8
*
* 	12/08/2018:
*	Added initialization and ButtonDBReadTask. Added ButtonGet for fetching state.
*
*	12/08/2018:
*	Updated and added comments.
*
* 	Created on: 12/08/2018
* 	Author: Anthony Needles
******************************************************************************/
#include "stm32l053xx.h"
#include "Button.h"

/********************************************************************
* Private Global Variables
********************************************************************/
static uint8_t buttonLatch;
// Latched debounced button press state, set when button denouncer
// state enters BUTTON_VERF, cleared when read by ButtonGet()

/********************************************************************
* Private Definitions
********************************************************************/
typedef enum{BUTTON_UP, BUTTON_EDGE, BUTTON_VERF, BUTTON_HELD} BUTTON_STATE_T;
// Button debouncer state machine state enumerations

#define BUTTON_STATE (GPIOC->IDR & GPIO_IDR_ID0_Msk)
// Button state is determined by digital input state on PC0

#define BUTTON_PRESSED 0x0001
// Expected bit field of BUTTON_STATE when button is pressed

/********************************************************************
* ButtonInit - Initializes button input
*
* Description:  Enables clock for GPIOC. Enables input mode for PC0 with
* 				pull down.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void ButtonInit(void)
{
	RCC->IOPENR |= RCC_IOPENR_GPIOCEN;

	GPIOC->MODER &= ~(GPIO_MODER_MODE0);
	GPIOC->PUPDR |= GPIO_PUPDR_PUPD0_1;

	buttonLatch = NOT_PRESSED;
}

/********************************************************************
* ButtonDBReadTask - Task for reading and debouncing current button press
*
* Description:  Reads input port tied to button. Implements debouncing
* 				state machine that waits for positive edge of button
* 				press, then verifies press for 4*TIMESLICE_PERIOD_MS.
* 				This is due to slice counter allowing entrance to task
* 				every other time slice, and  the required two states of hold
* 				required to reach verification. The latched button press
* 				is then set (this is cleared when ButtonGet is called).
* 				State stays in BUTTON_HELD until negative edge is detected.
*
* Return value:	None
*
* Arguments:    None
********************************************************************/
void ButtonDBReadTask(void)
{
	static uint8_t slice_counter = 1;
	static BUTTON_STATE_T button_state = BUTTON_UP;
	uint32_t button_temp = BUTTON_STATE;

	if(slice_counter > 0)
	{
		slice_counter = 0;
		switch(button_state)
		{
			case BUTTON_UP:
				if(button_temp == BUTTON_PRESSED)
				{
					button_state = BUTTON_EDGE;
				} else
				{
					button_state = BUTTON_UP;
				}
				break;

			case BUTTON_EDGE:
				if(button_temp == BUTTON_PRESSED)
				{
					button_state = BUTTON_VERF;
				} else
				{
					button_state = BUTTON_UP;
				}
				break;

			case BUTTON_VERF:
				buttonLatch = PRESSED;

				button_state = BUTTON_HELD;
				break;

			case BUTTON_HELD:
				if(button_temp == BUTTON_PRESSED)
				{
					button_state = BUTTON_HELD;
				} else
				{
					button_state = BUTTON_UP;
				}
				break;

			default:
				button_state = BUTTON_UP;
				break;

		}
	} else
	{
		slice_counter++;
	}
}

/********************************************************************
* ButtonGet - Fetch function to return debounced button state
*
* Description:  Reads current latched value of button, and resets the
* 				value if read as asserted. Returns the read state.
*
* Return value:	State of debounced, latched button state.
*
* Arguments:    None
********************************************************************/
uint8_t ButtonGet(void)
{
	uint8_t button_temp = buttonLatch;
	if(button_temp == PRESSED) buttonLatch = NOT_PRESSED;
	return button_temp;
}
