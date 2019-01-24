/******************************************************************************
* 	I2C.c
*
* 	This source file handles all I2C communications. Configured for 10kHz
* 	SCL frequency via "TIMING_CONFIG" constant calculated via STM32CubeMX.
*	4.7k resistors are expected close to master device to pull SDA and SCL
*	busses high.
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
#include "stm32l053xx.h"
#include "I2C.h"

/********************************************************************
* Private Defines
********************************************************************/
#define GPIO_AFRL_AFSEL6_AF1 (0x1 << GPIO_AFRL_AFSEL6_Pos)
#define GPIO_AFRL_AFSEL7_AF1 (0x1 << GPIO_AFRL_AFSEL7_Pos)
#define TX_REG_EMPTY_FLAG ((I2C1->ISR & I2C_ISR_TXE_Msk) >> I2C_ISR_TXE_Pos)
#define STOP_COND_GEN_FLAG ((I2C1->ISR & I2C_ISR_STOPF_Msk) >> I2C_ISR_STOPF_Pos)
#define SET 1U

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
void I2CInit()
{
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	RCC->IOPENR |= RCC_IOPENR_IOPBEN;

	GPIOB->MODER &= ~(GPIO_MODER_MODE6_Msk | GPIO_MODER_MODE7_Msk);
	GPIOB->MODER |= (GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);
	GPIOB->OTYPER |= (GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7);
	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6_Msk | GPIO_AFRL_AFSEL7_Msk);
	GPIOB->AFR[0] |= (GPIO_AFRL_AFSEL6_AF1 | GPIO_AFRL_AFSEL7_AF1);

	I2C1->TIMINGR = TIMING_CONFIG;
	I2C1->CR1 |= (I2C_CR1_NOSTRETCH | I2C_CR1_PE);
}

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
void I2CMasterTx(uint8_t addr, uint8_t num_bytes, uint8_t *tx_data)
{
	uint8_t *tx_data_ptr = tx_data;

	I2C1->CR2 &= ~(I2C_CR2_SADD_Msk | I2C_CR2_NBYTES_Msk);
	I2C1->CR2 |= (((uint32_t)addr << 1U) | I2C_CR2_AUTOEND | ((uint32_t)num_bytes << I2C_CR2_NBYTES_Pos) | I2C_CR2_START);

	while(num_bytes > 0)
	{
		while(TX_REG_EMPTY_FLAG != SET){}

		I2C1->TXDR = *tx_data_ptr;
		tx_data_ptr++;
		num_bytes--;
	}

	while(STOP_COND_GEN_FLAG != SET){}
	I2C1->ICR |= (I2C_ICR_STOPCF);

	I2C1->CR2 &= ~(I2C_CR2_SADD_Msk | I2C_CR2_AUTOEND_Msk | I2C_CR2_NBYTES_Msk);
}

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
void I2CSetOwnAddr(uint8_t addr)
{
	I2C1->OAR1 &= ~(I2C_OAR1_OA1EN_Msk);
	I2C1->OAR1 &= ~(I2C_OAR1_OA1_Msk);
	I2C1->OAR1 |= (((uint32_t)addr << 1U) | I2C_OAR1_OA1EN);
}
