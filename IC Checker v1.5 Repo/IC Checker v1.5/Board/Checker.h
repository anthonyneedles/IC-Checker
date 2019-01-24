/******************************************************************************
* 	Checker.h
*
* 	Header for Checker.c
*
* 	MCU: STM32L053R8
*
* 	Created on: 12/09/2018
* 	Author: Anthony Needles
******************************************************************************/
#ifndef CHECKER_H_
#define CHECKER_H_

// Delay determined to by oscilloscope testing for minimum time of assertion
// for accurate output
#define CYCLES_DELAY 10
#define TRUE 1
#define PASSED 1
#define FAILED 0

// IC failure boolean functions. Used to determine if test set of inputs
// and read output matches expected results.
#define IC_74HC00_FAIL (in_A & in_B) != ~out
#define IC_74HC02_FAIL (in_A | in_B) != ~out
#define IC_74HC04_FAIL in_A != !out
#define IC_74HC08_FAIL (in_A & in_B) != out
#define IC_74HC10_FAIL (in_A & in_B & in_C) != ~out
#define IC_74HC20_FAIL (in_A & in_B & in_C & in_D) != ~out
#define IC_74HC27_FAIL (in_A | in_B | in_C) != ~out
#define IC_74HC86_FAIL (in_A ^ in_B) != out

// Unique identifier for each IC enumerations
typedef enum {IC_74HC00,
			  IC_74HC02,
			  IC_74HC04,
			  IC_74HC08,
			  IC_74HC10,
			  IC_74HC20,
			  IC_74HC27,
			  IC_74HC86} IC_DESIGNATOR_T;


// Structure to hold various parameters for a given IC necessary
// for testing
typedef struct {
	IC_DESIGNATOR_T ic_designator;
	uint8_t num_inputs;
	uint8_t num_outputs;
	uint8_t input_pins[9];
	uint8_t output_pins[6];} IC_PARAMETERS_T;

/********************************************************************
* CheckerInit - Initializes required checker peripherals
*
* Description:  Enables clocks for GPIO ports A, B, and C. Enables
* 				TIM22 with count value of desired delays measured
* 				in cycles. This timer will be used for delaying
* 				small amounts to ensure any output gate change has
* 				time to propagate the system. One pulse mode enabled.
*
* Return value:	none
*
* Arguments:    None
********************************************************************/
void CheckerInit(void);

/********************************************************************
* CheckerTestIC - Task for reading and debouncing current button press
*
* Description:  Main test structure. Performs testing by creating all
* 				possible input combinations and reading resulting outputs.
* 				Made generically for any boolean logic 74HCXX IC with
* 				four or less inputs per gate. Gate inputs are labeled
* 				A, B, C, and D. For gates with less than four inputs,
* 				the loops for unused inputs are bypassed (e.g. two
* 				input gates will only use A and B loops). Only required
* 				input pins are set/cleared. If tests fails at any point
* 				failure result is immediately sent.
*
*
* Return value:	Test pass or test failure
*
* Arguments:    IC_PARAMETERS_T IC - Structure holding IC parameters
********************************************************************/
uint8_t CheckerTestIC(IC_PARAMETERS_T);

#endif /* CHECKER_H_ */
