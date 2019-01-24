/******************************************************************************
* 	Checker.c
*
* 	This source file is used to contain all functionality and initialization for
* 	performing the failure checking of 74HCXX digital logic integrated circuits.
* 	Developed to work with IC Checker v1.5 Shield. Dependent on 2.097 MHz APB2
* 	timer clock to perform delayed output readings.
*
* 	MCU: STM32L053R8
*
* 	12/09/2018:
* 	Completed initialization function and non-generic test looping.
*
* 	12/10/2018:
* 	Completed bit set/clear functions, as well as read output function.
*
* 	12/12/2018:
* 	Completed set/clear handler function, as well as test pass fail function
*
* 	12/13/2018:
* 	Completed test loop generic to IC, only needing IC parameters. Defines
* 	for loop skipping created.
*
* 	Created on: 12/09/2018
* 	Author: Anthony Needles
******************************************************************************/
#include "stm32l053xx.h"
#include "Checker.h"

/********************************************************************
* Private Definitions
********************************************************************/
#define INPUT_A_LOOP_SKIP (loop_skip_field & 0x01)
#define INPUT_B_LOOP_SKIP ((loop_skip_field & 0x02) >> 1)
#define INPUT_C_LOOP_SKIP ((loop_skip_field & 0x04) >> 2)
#define INPUT_D_LOOP_SKIP ((loop_skip_field & 0x08) >> 3)
//	Defines for determining required number of for loops based on the number of
//	inputs per gate. A bit field of all 1s will be left shifted the number of inputs
//	per gate for the given IC, with zeros shifting in at LSB (this is
//	"loop_skip_field", where bits loop_skip_field[3:0] corresponds to whether [D:A]
//	loops should be skipped or not, respectively). If "INPUT_X_LOOP_SKIP"
//	evaluates to 1, then the loop will be skipped, but if evaluated to 0 will not.
//	For example: The resulting loop_skip_field[3:0] for a two input gate will be
//	0b1100, testing will require only the "gate_input_A" loop and the "gate_input_B"
//	loop, so the "gate_input_C" loop and the "gate_input_D" loop will be "bypassed"
//	by only executing once.

/********************************************************************
* Private Function Prototypes
********************************************************************/
static void checkerSetClrInputs(uint8_t*, uint8_t, uint8_t, uint8_t);
static void checkerSetICInput(uint8_t);
static void checkerClrICInput(uint8_t);
static uint8_t checkerReadICOutput(uint8_t);
static uint8_t checkerFailTest(IC_DESIGNATOR_T, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

/********************************************************************
* CheckerInit - Initializes required checker peripherals
*
* Description:  Enables clocks for GPIO ports A, B, and C. Enables
* 				TIM22 with count value of desired delays measured
* 				in cycles. This timer will be used for delaying
* 				small amounts to ensure any output gate change has
* 				time to propagate the system. One pulse mode enabled.
*
* Return value:	None
*
* Arguments:    None
********************************************************************/
void CheckerInit(void)
{
	RCC->IOPENR |= (RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN);
	RCC->APB2ENR |= RCC_APB2ENR_TIM22EN;

	TIM22->CR1 |= TIM_CR1_OPM;
	TIM22->ARR = CYCLES_DELAY;
}

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
uint8_t CheckerTestIC(IC_PARAMETERS_T IC)
{
	uint8_t num_gates = IC.num_outputs;
	uint8_t num_inputs_gate = IC.num_inputs/IC.num_outputs;
	uint8_t loop_skip_field = (0xFF << num_inputs_gate);
	uint8_t gate_start_index = 0;
	uint8_t test_output;
	uint8_t fail_result;
	uint8_t gate_num;

	for(gate_num = 0; gate_num < num_gates; gate_num++)
	{
		for(uint8_t gate_input_A = INPUT_A_LOOP_SKIP; gate_input_A < 2; gate_input_A++)
		{
			for(uint8_t gate_input_B = INPUT_B_LOOP_SKIP; gate_input_B < 2; gate_input_B++)
			{
				for(uint8_t gate_input_C = INPUT_C_LOOP_SKIP; gate_input_C < 2; gate_input_C++)
				{
					for(uint8_t gate_input_D = INPUT_D_LOOP_SKIP; gate_input_D < 2; gate_input_D++)
					{
						if(INPUT_A_LOOP_SKIP != TRUE) checkerSetClrInputs(IC.input_pins, gate_input_A, gate_start_index, 0);
						if(INPUT_B_LOOP_SKIP != TRUE) checkerSetClrInputs(IC.input_pins, gate_input_B, gate_start_index, 1);
						if(INPUT_C_LOOP_SKIP != TRUE) checkerSetClrInputs(IC.input_pins, gate_input_C, gate_start_index, 2);
						if(INPUT_D_LOOP_SKIP != TRUE) checkerSetClrInputs(IC.input_pins, gate_input_D, gate_start_index, 3);

						test_output = checkerReadICOutput(IC.output_pins[gate_num]);

						fail_result = checkerFailTest(IC.ic_designator, test_output, gate_input_A, gate_input_B, gate_input_C, gate_input_D);

						if(fail_result == FAILED) return FAILED;
					}
				}
			}
		}
		gate_start_index += num_inputs_gate;
	}
	return PASSED;
}

/********************************************************************
* checkerSetClrInputs - Handles directing sets and clears of input pins
*
* Description:  Handed input pin number array for given IC, whether to
* 				set or clear, the index the current gate input group is
* 				located in the pin number array, and which number input
* 				of the gate is specified. This function derives
* 				which specific IC pin is desired to set/clear based on
* 				passed specifications, and calls set/clear functions.
*
* Return value:	None
*
* Arguments:    uint8_t *input_pins - Input IC pin number array (in
* 				order) for tested IC.
*
* 				uint8_t input_value - Desired value to give to input
* 				pin (1 for set, 0 for clear).
*
* 				uint8_t gate_start - Index of pin number array that
* 				current input group starts.
*
* 				uint8_t input_offset - Offset of specific pin in input
* 				group that is desired to be set/cleared.
********************************************************************/
void checkerSetClrInputs(uint8_t *input_pins, uint8_t input_value, uint8_t gate_start, uint8_t input_offset)
{
	if(input_value == 0)
	{
		checkerClrICInput(input_pins[gate_start + input_offset]);
	} else if (input_value == 1)
	{
		checkerSetICInput(input_pins[gate_start + input_offset]);
	} else {}
}

/********************************************************************
* checkerSetICInput - Sets specific pins
*
* Description:  Based on which pin number given from specific IC's
* 				input pin array, corresponding GPIO port is set to
* 				output mode (relative to MCU, but these pins will be
* 				inputs relative to the tested IC). The GPIO is then
* 				set to high logic.
*
* Return value:	None
*
* Arguments:    uint8_t ic_pin - IC pin desired to be set
********************************************************************/
void checkerSetICInput(uint8_t ic_pin)
{
	switch(ic_pin){
		case 1:
			GPIOA->MODER &= ~GPIO_MODER_MODE10_Msk;
			GPIOA->MODER |= GPIO_MODER_MODE10_0;
			GPIOA->ODR |= GPIO_ODR_OD10;
		break;

		case 2:
			GPIOB->MODER &= ~GPIO_MODER_MODE3_Msk;
			GPIOB->MODER |= GPIO_MODER_MODE3_0;
			GPIOB->ODR |= GPIO_ODR_OD3;
		break;

		case 3:
			GPIOB->MODER &= ~GPIO_MODER_MODE5_Msk;
			GPIOB->MODER |= GPIO_MODER_MODE5_0;
			GPIOB->ODR |= GPIO_ODR_OD5;
		break;

		case 4:
			GPIOB->MODER &= ~GPIO_MODER_MODE4_Msk;
			GPIOB->MODER |= GPIO_MODER_MODE4_0;
			GPIOB->ODR |= GPIO_ODR_OD4;
		break;

		case 5:
			GPIOB->MODER &= ~GPIO_MODER_MODE10_Msk;
			GPIOB->MODER |= GPIO_MODER_MODE10_0;
			GPIOB->ODR |= GPIO_ODR_OD10;
		break;

		case 6:
			GPIOA->MODER &= ~GPIO_MODER_MODE8_Msk;
			GPIOA->MODER |= GPIO_MODER_MODE8_0;
			GPIOA->ODR |= GPIO_ODR_OD8;
		break;

		case 8:
			GPIOA->MODER &= ~GPIO_MODER_MODE9_Msk;
			GPIOA->MODER |= GPIO_MODER_MODE9_0;
			GPIOA->ODR |= GPIO_ODR_OD9;
		break;

		case 9:
			GPIOC->MODER &= ~GPIO_MODER_MODE7_Msk;
			GPIOC->MODER |= GPIO_MODER_MODE7_0;
			GPIOC->ODR |= GPIO_ODR_OD7;
		break;

		case 10:
			GPIOB->MODER &= ~GPIO_MODER_MODE6_Msk;
			GPIOB->MODER |= GPIO_MODER_MODE6_0;
			GPIOB->ODR |= GPIO_ODR_OD6;
		break;

		case 11:
			GPIOA->MODER &= ~GPIO_MODER_MODE7_Msk;
			GPIOA->MODER |= GPIO_MODER_MODE7_0;
			GPIOA->ODR |= GPIO_ODR_OD7;
		break;

		case 12:
			GPIOA->MODER &= ~GPIO_MODER_MODE6_Msk;
			GPIOA->MODER |= GPIO_MODER_MODE6_0;
			GPIOA->ODR |= GPIO_ODR_OD6;
		break;

		case 13:
			GPIOA->MODER &= ~GPIO_MODER_MODE5_Msk;
			GPIOA->MODER |= GPIO_MODER_MODE5_0;
			GPIOA->ODR |= GPIO_ODR_OD5;
		break;

		default:
		break;
	}
}

/********************************************************************
* checkerClrICInput - Clears specific pins
*
* Description:  Based on which pin number given from specific IC's
* 				input pin array, corresponding GPIO port is set to
* 				output mode (relative to MCU, but these pins will be
* 				inputs relative to the tested IC). The GPIO is then
* 				set to low logic.
*
* Return value:	None
*
* Arguments:    uint8_t ic_pin - IC pin desired to be cleared
********************************************************************/
void checkerClrICInput(uint8_t ic_pin)
{
	switch(ic_pin){
		case 1:
			GPIOA->MODER &= ~GPIO_MODER_MODE10_Msk;
			GPIOA->MODER |= GPIO_MODER_MODE10_0;
			GPIOA->ODR &= ~(GPIO_ODR_OD10);
			break;

		case 2:
			GPIOB->MODER &= ~GPIO_MODER_MODE3_Msk;
			GPIOB->MODER |= GPIO_MODER_MODE3_0;
			GPIOB->ODR &= ~(GPIO_ODR_OD3);
			break;

		case 3:
			GPIOB->MODER &= ~GPIO_MODER_MODE5_Msk;
			GPIOB->MODER |= GPIO_MODER_MODE5_0;
			GPIOB->ODR &= ~(GPIO_ODR_OD5);
			break;

		case 4:
			GPIOB->MODER &= ~GPIO_MODER_MODE4_Msk;
			GPIOB->MODER |= GPIO_MODER_MODE4_0;
			GPIOB->ODR &= ~(GPIO_ODR_OD4);
			break;

		case 5:
			GPIOB->MODER &= ~GPIO_MODER_MODE10_Msk;
			GPIOB->MODER |= GPIO_MODER_MODE10_0;
			GPIOB->ODR &= ~(GPIO_ODR_OD10);
			break;

		case 6:
			GPIOA->MODER &= ~GPIO_MODER_MODE8_Msk;
			GPIOA->MODER |= GPIO_MODER_MODE8_0;
			GPIOA->ODR &= ~(GPIO_ODR_OD8);
			break;

		case 8:
			GPIOA->MODER &= ~GPIO_MODER_MODE9_Msk;
			GPIOA->MODER |= GPIO_MODER_MODE9_0;
			GPIOA->ODR &= ~(GPIO_ODR_OD9);
			break;

		case 9:
			GPIOC->MODER &= ~GPIO_MODER_MODE7_Msk;
			GPIOC->MODER |= GPIO_MODER_MODE7_0;
			GPIOC->ODR &= ~(GPIO_ODR_OD7);
			break;

		case 10:
			GPIOB->MODER &= ~GPIO_MODER_MODE6_Msk;
			GPIOB->MODER |= GPIO_MODER_MODE6_0;
			GPIOB->ODR &= ~(GPIO_ODR_OD6);
			break;

		case 11:
			GPIOA->MODER &= ~GPIO_MODER_MODE7_Msk;
			GPIOA->MODER |= GPIO_MODER_MODE7_0;
			GPIOA->ODR &= ~(GPIO_ODR_OD7);
			break;

		case 12:
			GPIOA->MODER &= ~GPIO_MODER_MODE6_Msk;
			GPIOA->MODER |= GPIO_MODER_MODE6_0;
			GPIOA->ODR &= ~(GPIO_ODR_OD6);
			break;

		case 13:
			GPIOA->MODER &= ~GPIO_MODER_MODE5_Msk;
			GPIOA->MODER |= GPIO_MODER_MODE5_0;
			GPIOA->ODR &= ~(GPIO_ODR_OD5);
			break;

		default:
			break;
	}
}

/********************************************************************
* checkerReadICOutput - Reads specific pins
*
* Description:  Based on which pin number given from specific IC's
* 				output pin array, corresponding GPIO port is set to
* 				input mode (relative to MCU, but these pins will be
* 				outputs relative to the tested IC). TIM22 is enabled
* 				and has update interrupt flag polled in order to
* 				generate a delay of only a few clock cycles. This allows
* 				any gate output changes time to propagate so that
* 				readings are correct. The resulting read logic level
* 				is then returned.
*
* Return value:	Logic value read from GPIO input
*
* Arguments:    uint8_t ic_pin - IC pin desired to have logic level read
********************************************************************/
uint8_t checkerReadICOutput(uint8_t ic_pin)
{
	switch(ic_pin){
		case 1:
			GPIOA->MODER &= ~(GPIO_MODER_MODE10);
			TIM22->CR1 |= TIM_CR1_CEN;
			while((TIM22->SR & TIM_SR_UIF_Msk) == 0){}
			return ((GPIOA->IDR & GPIO_IDR_ID10_Msk) >> 10);
			break;

		case 2:
			GPIOB->MODER &= ~(GPIO_MODER_MODE3);
			TIM22->CR1 |= TIM_CR1_CEN;
			while((TIM22->SR & TIM_SR_UIF_Msk) == 0){}
			return ((GPIOB->IDR & GPIO_IDR_ID3_Msk) >> 3);
			break;

		case 3:
			GPIOB->MODER &= ~(GPIO_MODER_MODE5);
			TIM22->CR1 |= TIM_CR1_CEN;
			while((TIM22->SR & TIM_SR_UIF_Msk) == 0){}
			return ((GPIOB->IDR & GPIO_IDR_ID5_Msk) >> 5);
			break;

		case 4:
			GPIOB->MODER &= ~(GPIO_MODER_MODE4);
			TIM22->CR1 |= TIM_CR1_CEN;
			while((TIM22->SR & TIM_SR_UIF_Msk) == 0){}
			return ((GPIOB->IDR & GPIO_IDR_ID4_Msk) >> 4);
			break;

		case 5:
			GPIOB->MODER &= ~(GPIO_MODER_MODE10);
			TIM22->CR1 |= TIM_CR1_CEN;
			while((TIM22->SR & TIM_SR_UIF_Msk) == 0){}
			return ((GPIOB->IDR & GPIO_IDR_ID10_Msk) >> 10);
			break;

		case 6:
			GPIOA->MODER &= ~(GPIO_MODER_MODE8);
			TIM22->CR1 |= TIM_CR1_CEN;
			while((TIM22->SR & TIM_SR_UIF_Msk) == 0){}
			return ((GPIOA->IDR & GPIO_IDR_ID8_Msk) >> 8);
			break;

		case 8:
			GPIOA->MODER &= ~(GPIO_MODER_MODE9);
			TIM22->CR1 |= TIM_CR1_CEN;
			while((TIM22->SR & TIM_SR_UIF_Msk) == 0){}
			return ((GPIOA->IDR & GPIO_IDR_ID9_Msk) >> 9);
			break;

		case 9:
			GPIOC->MODER &= ~(GPIO_MODER_MODE7);
			TIM22->CR1 |= TIM_CR1_CEN;
			while((TIM22->SR & TIM_SR_UIF_Msk) == 0){}
			return ((GPIOC->IDR & GPIO_IDR_ID7_Msk) >> 7);
			break;

		case 10:
			GPIOB->MODER &= ~(GPIO_MODER_MODE6);
			TIM22->CR1 |= TIM_CR1_CEN;
			while((TIM22->SR & TIM_SR_UIF_Msk) == 0){}
			return ((GPIOB->IDR & GPIO_IDR_ID6_Msk) >> 6);
			break;

		case 11:
			GPIOA->MODER &= ~(GPIO_MODER_MODE7);
			TIM22->CR1 |= TIM_CR1_CEN;
			while((TIM22->SR & TIM_SR_UIF_Msk) == 0){}
			return ((GPIOA->IDR & GPIO_IDR_ID7_Msk) >> 7);
			break;

		case 12:
			GPIOA->MODER &= ~(GPIO_MODER_MODE6);
			TIM22->CR1 |= TIM_CR1_CEN;
			while((TIM22->SR & TIM_SR_UIF_Msk) == 0){}
			return ((GPIOA->IDR & GPIO_IDR_ID6_Msk) >> 6);
			break;

		case 13:
			GPIOA->MODER &= ~(GPIO_MODER_MODE5);
			TIM22->CR1 |= TIM_CR1_CEN;
			while((TIM22->SR & TIM_SR_UIF_Msk) == 0){}
			return ((GPIOA->IDR & GPIO_IDR_ID5_Msk) >> 5);
			break;

		default:
			return 0xFF;
			break;
	}
}

/********************************************************************
* checkerFailTest - Compares current inputs and output from tested gate
* 					to expected results
*
* Description:  Uses defined IC boolean functions to determine if given
* 				inputs and output match result as expected. IC boolean
* 				functions defined in Checker.h.
*
* Return value:	Pass or failure for specific IC tested
*
* Arguments:    IC_DESIGNATOR_T ic_id - Enumeration of IC's designation
* 				being tested
*
* 				uint8_t out - Read output for given inputs
*
* 				uint8_t in_A - Value given to gate_input_A
*
* 				uint8_t in_B - Value given to gate_input_B
*
* 				uint8_t in_C - Value given to gate_input_C
*
* 				uint8_t in_D - Value given to gate_input_D
*
********************************************************************/
uint8_t checkerFailTest(IC_DESIGNATOR_T ic_id, uint8_t out, uint8_t in_A, uint8_t in_B, uint8_t in_C, uint8_t in_D)
{
	switch(ic_id)
	{
		case IC_74HC00:
			if(IC_74HC00_FAIL) return FAILED;
			break;

		case IC_74HC02:
			if(IC_74HC02_FAIL) return FAILED;
			break;

		case IC_74HC04:
			if(IC_74HC04_FAIL) return FAILED;
			break;

		case IC_74HC08:
			if(IC_74HC08_FAIL) return FAILED;
			break;

		case IC_74HC10:
			if(IC_74HC10_FAIL) return FAILED;
			break;

		case IC_74HC20:
			if(IC_74HC20_FAIL) return FAILED;
			break;

		case IC_74HC27:
			if(IC_74HC27_FAIL) return FAILED;
			break;

		case IC_74HC86:
			if(IC_74HC86_FAIL) return FAILED;
			break;

		default:
			return FAILED;
			break;
	}
	return PASSED;
}
