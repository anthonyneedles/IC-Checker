/******************************************************************************
* 	I2C.h
*
* 	Header for I2C.c
*
* 	MCU: STM32L053R8
*
* 	01/17/2019:
* 	Created and completed initialization, master transmit, and set own address
* 	functions for use in communicating to HD44780 LCD controller.
*
* 	Created on: 01/17/2019
* 	Author: Anthony Needles
******************************************************************************/
#ifndef I2C_H_
#define I2C_H_

/********************************************************************
* Public Definitions
********************************************************************/
#define TIMING_CONFIG 0x000006C5
//Calculated for 10kHz via STM32CubeMX

/********************************************************************
* I2CInit - Initialization function for I2C module
*
* Description:  Enables clocks for I2C1 pins as well as I2C1 module.
* 				GPIO configured as open drain with no pull up/down
* 				resistors. Expected 4.7k resistors pulling SCL and
* 				SDA high. AF1 for both GPIO select I2C1 SCL and SDA.
* 				Timing configuration constant is generated via
* 				STM32CubeMX for target I2C frequency of 10kHz. Clock
* 				stretching disabled.
*
* Return value:	None
*
* Arguments:    None
********************************************************************/
void I2CInit(void);

/********************************************************************
* I2CMasterTx - Master transmit function for I2C
*
* Description:  Handles master transmission of data to slave. Clears
* 				current slave address and number of bytes to be sent
* 				values from CR2 register, then populates with passed
* 				values. Also, AUTOEND is enabled (stop condition is
* 				automatically generated when number of bytes is
* 				reached. Start condition is initiated. Data is loaded
* 				into transmit data register when register is ready.
* 				Data array is iterated through num_bytes number of times,
* 				passing all data. The stop condition is confirmed, then
* 				stop flag is cleared and CR2 is cleared of set values.
*
* Return value:	None
*
* Arguments:    uint8_t addr - Address of target slave
*
* 				uint8_t num_bytes - Number of bytes of data that is
* 				desired to be sent
*
* 				uint8_t *tx_data - Pointer to 8-bit data array of
* 				desired transmit data
********************************************************************/
void I2CMasterTx(uint8_t, uint8_t, uint8_t *);

/********************************************************************
* I2CSetOwnAddr - Sets 7-bit address of MCU for slave ability (only OA1)
*
* Description:  First, OA1 (own address #1) enable is cleared as it
* 				must be 0 to set an address. Current address field
* 				is then cleared. Passed address is then loaded into
* 				7-bit address field in OA1 register.
*
* Return value:	None
*
* Arguments:    uint8_t addr - Desired 7-bit address
********************************************************************/
void I2CSetOwnAddr(uint8_t);

#endif /* I2C_H_ */
