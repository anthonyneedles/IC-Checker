/******************************************************************************
* 	Button.h
*
* 	Header for Button.c
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
#ifndef BUTTON_H_
#define BUTTON_H_

#define PRESSED 1
#define NOT_PRESSED 0

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
void ButtonInit(void);

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
* Return value:	none
*
* Arguments:    None
********************************************************************/
void ButtonDBReadTask(void);

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
uint8_t ButtonGet(void);

#endif /* BUTTON_H_ */
