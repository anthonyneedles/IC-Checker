/******************************************************************************
* 	LCD.c
*
* 	This source file handles all LCD functionality and requisite I2C function
* 	calls to send commands/data to and from HD44780 LCD driver for a 1602 LCD.
* 	Expected I2C SCL frequency of 10kHz. LCD Driver written to in 4-bit mode,
* 	so only DB[7:4] are used, which correspond to bits [7:4] resent over I2C data
* 	bus, with bits [3:0
*
* 	Dependent on MSI bus clock of 2.097 MHz
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
#include "stm32l053xx.h"
#include "LCD.h"
#include "I2C.h"

/******************************************************************************
* Private Definitions
******************************************************************************/
#define RESET_NIB_1 0x3
#define RESET_NIB_2 0x3
#define RESET_NIB_3 0x3
#define RESET_NIB_4 0x2
// Nibbles needed to be written at every initialization to reset controller

#define RESET_DELAY_US_1 15000
#define RESET_DELAY_US_2 4100
#define RESET_DELAY_US_3 100

#define WRITE_NIB_ENABLE 0xC
#define WRITE_NIB_DISABLE 0x8
// Nibbles with K-EN set (bit 3), RW and RS cleared (bits 1 and 0
// respectively) and alternated CS/E bit (bit 2)

#define LCD_FUNC_4BIT_2LINE_5X8 0x28
// Byte code for setting 4-bit functionality, 2 line display, and 5x8 dot
// character display

#define LCD_DISP_ON_CURSOR_OFF_NOBLINK 0x0C
// Byte code for turning display and cursor on, with the cursor blinking

#define LCD_DISP_ON_CURSOR_ON_BLINK 0x0F
// Byte code for turning display and cursor on, with the cursor blinking

#define LCD_ENTRY_MODE_INCR 0x06
// Byte code for moving cursor to the right after writing a character

#define LCD_CLEAR_DISP 0x01
// Byte code for clearing entire display

#define LCD_RETURN_HOME 0x02
// Byte code for returning cursor to top left position (1,1)

#define LCD_LINE1_ADDRESS 0x00
// Address of top left position (1,1)

#define LCD_LINE2_ADDRESS 0x40
// Address of bottom left position (1,2)

#define INSTRUCTION_REG 0x00
// Cleared RS bit (bit 1) indicating incoming instruction

#define DATA_REG 0x01
// Set RS bit (bit 1) indicating incoming data

#define TIMER_PRESCALER 4095
#define CLKCYCLES_ONE_SECOND 2097000/TIMER_PRESCALER
// Prescaler needed as time-out time scale is in seconds and unscaled counter
// will result in early roll over

#define TWO_BYTES 2U

/********************************************************************
* Private Function Prototypes
********************************************************************/
static void lcdWriteNibble(uint8_t, uint8_t);
static void lcdWriteIR(uint8_t);
static void lcdWriteDR(uint8_t);
static void lcdDisplayChar(uint8_t);
static void lcdDelay10us(void);

/******************************************************************************
* Private Constants
******************************************************************************/
static const uint8_t ReadyPrompt[] = "Ready...";

/********************************************************************
* lcdWriteNibble - Handles writing 4-bit values to LCD driver
*
* Description:  In 4-bit mode, only four instruction/data bits (nibble)
* 				can be written to the LCD at a time. These four bits
* 				occupy four bits in each I2C transmission, with the other
* 				four bits being K-EN (cathode enable), E (data read/write
* 				enable), R/W (read/write select), RS (register select).
* 				These four bits change based on desired functionality
* 				(instruction write or data write, changed with bit field).
* 				Data read/write is initiated when the HD44780 LCD driver
* 				detects a negative edge of E. Also, a pulse of E with a
* 				length of at least 100ns is required. This function
* 				sends both E-high and E-low bytes over I2C to create
* 				a negative edge of E. At an I2C rate of 10kHz the time
* 				between these two bits is measured to be ~1ms, so the
* 				hold time is not violated.
*
* Return value:	None
*
* Arguments:    uint8_t nibble - Given four data/instruction bits
* 				(lower four bits of byte) to be written to driver
*
* 				uint8_t reg_type_field -  Bit field that will set
* 				(data register selected) or clear (instruction register
* 				selected) the RS bit in the 8-bit transmission.
********************************************************************/
static void lcdWriteNibble(uint8_t nibble, uint8_t reg_type_field)
{
	uint8_t lcd_i2c_data[2];
	lcd_i2c_data[0] = ((nibble << 4) | reg_type_field | WRITE_NIB_ENABLE);
	lcd_i2c_data[1] = ((nibble << 4) | reg_type_field | WRITE_NIB_DISABLE);
	I2CMasterTx(LCD_I2C_ADDRESS, TWO_BYTES, lcd_i2c_data);
}

/********************************************************************
* lcdWriteIR - Handles calling nibble write function for instruction
* 			   writes
*
* Description:  Since only four data/instruction bits can be written
* 				at a time, the given eight bit code must be broken
*				down into two nibbles to be sent separately, MSB first.
*				The instruction register bit field is sent as well to
*				ensure that RS is set to 0 (instruction register selected).
*
* Return value:	None
*
* Arguments:    uint8_t write_byte - 8-bit code desired to be sent
* 				as an instruction write, corresponding to DB[7:0]
* 				in HD44780 Hitachi datasheet.
********************************************************************/
static void lcdWriteIR(uint8_t write_byte)
{
	lcdWriteNibble((write_byte >> 4), INSTRUCTION_REG);
	lcdWriteNibble((write_byte & 0x0F), INSTRUCTION_REG);
}

/********************************************************************
* lcdWriteDR - Handles calling nibble write function for data writes
*
* Description:  Since only four data/instruction bits can be written
* 				at a time, the given eight bit code must be broken
*				down into two nibbles to be sent separately, MSB first.
*				The data register bit field is sent as well to
*				ensure that RS is set to 1 (data register selected).
*
* Return value:	None
*
* Arguments:    uint8_t write_byte - 8-bit code desired to be sent
* 				as a data write, corresponding to DB[7:0] in HD44780
* 				Hitachi datasheet.
********************************************************************/
static void lcdWriteDR(uint8_t write_byte)
{
	lcdWriteNibble((write_byte >> 4), DATA_REG);
	lcdWriteNibble((write_byte & 0x0F), DATA_REG);
}

/********************************************************************
* lcdDisplayChar - Displays character from given ASCII value
*
* Description:  The internal CGROM (character generator ROM) of HD44780
* 				holds many standard ascii characters that will be
* 				generated on the current cursor location upon a data
* 				write of that ASCII value to the LCD driver.
*
* Return value:	None
*
* Arguments:    uint8_t ascii_byte - 8-bit ASCII value for desired
* 				character
********************************************************************/
static void lcdDisplayChar(uint8_t ascii_byte)
{
	lcdWriteDR(ascii_byte);
}

/********************************************************************
* lcdDelay10us() - Function that generates a 10 microsecond delay
*
* Description:  Used to generate minimum delays specified by
* 				initialization routine in HD44780 Hitachi datasheet.
* 				Generates a delay that is slightly over 10us (on average
* 				~11us) which is acceptable due to requirement of only a
* 				minimum delay. Function avoids compiler optimization
* 				by using empty in-line assembly statement every loop.
*
* Return value:	None
*
* Arguments:    None
********************************************************************/
static void lcdDelay10us()
{
	for(uint8_t count = 2; count > 0; count--){
		asm("");
	}
}

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
void LCDInit()
{
		uint16_t delay_count;

		RCC->APB2ENR |= RCC_APB2ENR_TIM21EN ;

		TIM21->DIER |= TIM_DIER_UIE;
		TIM21->PSC = TIMER_PRESCALER;
		TIM21->ARR = (LCD_REPROMPT_DELAY_SECONDS*CLKCYCLES_ONE_SECOND);

		NVIC_EnableIRQ(TIM21_IRQn);

		for(delay_count = RESET_DELAY_US_1/10; delay_count > 0; delay_count--)
		{
			lcdDelay10us();
		}

		lcdWriteNibble(RESET_NIB_1, INSTRUCTION_REG);

		for(delay_count = RESET_DELAY_US_2/10; delay_count > 0; delay_count--)
		{
			lcdDelay10us();
		}

		lcdWriteNibble(RESET_NIB_2, INSTRUCTION_REG);

		for(delay_count = RESET_DELAY_US_3/10; delay_count > 0; delay_count--)
		{
			lcdDelay10us();
		}

		lcdWriteNibble(RESET_NIB_3, INSTRUCTION_REG);

		lcdWriteNibble(RESET_NIB_4, INSTRUCTION_REG);

		lcdWriteIR(LCD_FUNC_4BIT_2LINE_5X8);
		lcdWriteIR(LCD_DISP_ON_CURSOR_OFF_NOBLINK);
		lcdWriteIR(LCD_ENTRY_MODE_INCR);
		lcdWriteIR(LCD_CLEAR_DISP);
		lcdWriteIR(LCD_RETURN_HOME);
		LCDDisplayString(ReadyPrompt);
}

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
void LCDClearDisplay()
{
	lcdWriteIR(LCD_CLEAR_DISP);
}

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
void LCDMoveCursor(uint8_t column, uint8_t row)
{
	switch(row)
	{
		case 1:
			lcdWriteIR(((LCD_LINE1_ADDRESS + column - 1) | 0x80));
			break;

		case 2:
			lcdWriteIR(((LCD_LINE2_ADDRESS + column - 1) | 0x80));
			break;

		default:
			break;
	}
}

/********************************************************************
* LCDDisplayString() - Displays given string
*
* Description:  This function will display the "string"  pointed to
* 				by iterating the character array and using the display
* 				character function until the null terminator is
* 				detected, signaling the end of the "string". Since
* 				enter incrementing is enabled the LCD driver
* 				automatically moves to the right after each byte write.
* 				Reprompt timer is enabled upon finishing display
* 				writes.
*
* Return value:	const uint8_t string - Pointer to start of array of
* 				ASCII characters with null ('\0') termination
*
* Arguments:    None
********************************************************************/
void LCDDisplayString(const uint8_t *string)
{
	uint8_t *string_ptr = (uint8_t *)string;

	while(*string_ptr != '\0')
	{
		lcdDisplayChar(*string_ptr);
		string_ptr++;
	}
	TIM21->CR1 |= TIM_CR1_CEN;
}

/********************************************************************
* TIM21_IRQHandler - Handles interrupt events for TIM21
*
* Description:  When TIM21 reaches end of counting (after delay of
* 				LCD_REPROMPT_DELAY_SECONDS)interrupt flag is cleared,
* 				then LCD clears current display to display the "ready
* 				prompt". The counter is then disabled until display
* 				is changed again (LCDDisplayString() is called).
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void TIM21_IRQHandler(void)
{
	TIM21->SR &= ~TIM_SR_UIF;
	LCDClearDisplay();
	LCDMoveCursor(1U,1U);
	LCDDisplayString(ReadyPrompt);
	TIM21->CR1 &= ~TIM_CR1_CEN;
}
