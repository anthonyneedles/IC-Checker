/******************************************************************************
* 	LCD.h
*
* 	Header for LCD.c
*
* 	MCU: STM32L053R8
*
* 	01/21/2019:
*	Created and completed initialization and nibble writing to driver via I2C.
*	Display functions clear display, display char, display string, move cursor
*	written but untested.
*
*	01/22/2019:
*	Completed testing of display functions and required reset delays. Updated
*	all comments.
*
* 	Created on: 01/21/2019
* 	Author: Anthony Needles
******************************************************************************/
#ifndef LCD_H_
#define LCD_H_

/******************************************************************************
* Public Definitions
******************************************************************************/
#define LCD_I2C_ADDRESS 0x27
#define LCD_REPROMPT_DELAY_SECONDS 10
// Desired automatic reprompt delay for LCD

/********************************************************************
* LCDInit() - Handles LCD reset and initialization routine
*
* Description:  Since the MCU can be reset without the HD44780 also
* 				resetting the current state of the driver can not
* 				be determined upon program start initialization
* 				by instruction must be performed. This sequence is
* 				specified in the HD44780 Hitachi datasheet. Minimum
* 				delays are required and executed via a software delay
* 				function. After reset procedure, the LCD is configured
* 				for 4-bit operation, 2 line display, 5x8 characters,
* 				display on, cursor off, blinking off, and entry
* 				increments. The display is then cleared and the
* 				cursor is moved to the top left position (1,1).
*
* Return value:	None
*
* Arguments:    None
********************************************************************/
void LCDInit(void);

/********************************************************************
* LCDClearDisplay() - Clears entire LCD display
*
* Description:  Upon calling, this function send the "clear display"
* 				code to the instruction write manager.
*
* Return value:	None
*
* Arguments:    None
********************************************************************/
void LCDClearDisplay(void);

/********************************************************************
* LCDMoveCursor() - Moves cursor to desired coordinates
*
* Description:  This function will send the "move cursor" code to
* 				instruction write manager. This code consists of
* 				a 0 in the bit 7 position and 1 in bit 6 position,
* 				followed by the address of the desired space. The
* 				address of the first (top) row starts at 0x00 and
* 				increases by one every move to the right. The address
* 				of the second (bottom) row starts at 0x40 and increases
* 				every move to the right.
*
* Return value:	uint8_t column - Desired column of cursor move, with
* 				1 corresponding to left most column.
*
* 				uint8_t row - Desired row of cursor move, with 1
* 				corresponding to top row.
*
* Arguments:    None
********************************************************************/
void LCDMoveCursor(uint8_t, uint8_t);

/********************************************************************
* LCDDisplayString() - Displays given string
*
* Description:  This function will display the "string"  pointed to
* 				by iterating the character array and using the display
* 				character function until the null terminator is
* 				detected, signaling the end of the "string". Since
* 				enter incrementing is enabled the LCD driver
* 				automatically moves to the right after each byte write.
*
* Return value:	const uint8_t string - Pointer to start of array of
* 				ASCII characters with null ('\0') termination
*
* Arguments:    None
********************************************************************/
void LCDDisplayString(const uint8_t *);

/********************************************************************
* TIM21_IRQHandler - Handles interrupt events for TIM21
*
* Description:  When TIM21 reaches end of counting (after delay of
* 				LCD_REPROMPT_DELAY_SECONDS)interrupt flag is cleared,
* 				then LCD clears current display to display the "ready
* 				prompt". The counter is then disabled until display
* 				is changed again.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void TIM21_IRQHandler(void);

#endif /* LCD_H_ */
