/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Dual-Timer Hardware Frequency Measurement
  ******************************************************************************
  * @attention
  *
  * This application implements a complete closed-loop hardware instrumentation
  * system using two independent timers on an STM32F103 microcontroller. One
  * timer acts as a precise signal generator, while a second timer operates as a
  * measurement instrument using Input Capture Mode.
  *
  * ============================================================================
  * CLOCK TREE CONFIGURATION
  * ============================================================================
  * - System Clock (SYSCLK)      = 52 MHz (Via PLL, sourcing HSI divided by 2)
  * - AHB Bus Prescaler          = 1 (52 MHz)
  * - APB1 Peripheral Prescaler  = 4 (13 MHz Clock / 26 MHz Timer Base Clock)
  * - APB2 Peripheral Prescaler  = 4 (13 MHz Clock)
  *
  * *Note on Timer Clocking:* Because the APB1 prescaler is not 1, the hardware
  * internal timer multiplier (x2) automatically activates. Therefore, both
  * TIM2 and TIM3 operate on a dedicated **26 MHz internal clock base**.
  *
  * ============================================================================
  * PERIPHERAL ARCHITECTURE & HARDWARE INTERACTION
  * ============================================================================
  * 1. Signal Generator (Timer 2):
  * - Set up to trigger an internal Update Event (UEV) interrupt every 40 µs.
  * - The ISR toggles Pin PA9, outputting a precise 12.5 kHz square wave
  * with a total cycle period of 80 µs.
  *
  * 2. Input Capture Engine (Timer 3):
  * - Channel 2 (Pin PA10) is physically bridged to Pin PA7.
  * - Configured to catch consecutive rising edges on input capture prescaler 1
  * (`TIM_ICPSC_DIV1`), providing an internal tick resolution of 38.46 ns.
  * - Leverages low-level preprocessor macro `__HAL_TIM_GET_COMPARE` to pull
  * capture variables with zero function-call overhead.
  *
  * 3. Telemetry Stream (USART1):
  * - Configured at 115200 baud to stream calculated data back to a computer.
  *
  * ============================================================================
  * MATHEMATICAL FORMULAS & WORKFLOW CONSTRAINTS
  * ============================================================================
  * - Tick Resolution:
  * 1 / 26,000,000 Hz = 38.4615 nanoseconds per timer count.
  *
  * - Target Delta-Tick Capture:
  * 80 µs (signal period) / 38.4615 ns = 2,080 counts.
  *
  * - Counter Rollover Protection:
  * tick_difference = (0xFFFF - capture_val1) + capture_val2 + 1;
  *
  * - Core Frequency Equation:
  * user_sig_freq = 26000000.0f / (float)tick_difference;
  *
  ******************************************************************************
  */

#include "stm32f1xx_hal.h"
#include "main_app.h"
#include "msp.h"
#include "it.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

// RCC OSC struct. declaration
RCC_OscInitTypeDef osc_init;

// CLK Config struct. declaration
RCC_ClkInitTypeDef clk_init;


TIM_HandleTypeDef htimer2, htimer3;

// uart struct. declaration
UART_HandleTypeDef huart2;

uint8_t is_capture_done = FALSE ;

double timer3_cnt_freq = 0;

double  timer3_cnt_res = 0;

double  user_sig_time_period = 0;

//int test = 0;

double user_sig_freq = 0;

uint32_t capture_difference = 0  ;

uint32_t input_capture[2] = {0} ;

uint8_t count = 1;

// data buffer
char msg[100];

int main(void){

	char *user_data = "\n\n*************** The application is running ***************\r\n\n";

	char usr_msg[100];

	// HAL library inits.
	HAL_Init();

	// SYSCLK configuration
	SYSCLK_Config(SYS_CLOCK_FREQ_52MHZ);

	// configure PA10
	HAL_GPIOA_MspInit();

	// uart inits.
	UART2_Init();

	// display clk frequencies
	Print_Freq();

	// send user data to the serial terminal
	HAL_UART_Transmit(&huart2, (uint8_t*) user_data, (uint16_t) strlen(user_data), HAL_MAX_DELAY ) ;

	// Timer 2/3 inits.

	TIMER2_Init() ;

	TIMER3_Init();

	// start timer 2 in interrupt mode
	if ( HAL_TIM_Base_Start_IT(&htimer2) != HAL_OK) {

		Error_handler();
	};

	// start timer 3 in input capture -interrupt mode

	if ( HAL_TIM_IC_Start_IT(&htimer3, TIM_CHANNEL_2)!= HAL_OK) {

			Error_handler();
	};

	while(1) {


			if (is_capture_done) {

				// Calculate elapsed time ticks and handle counter rollover

				if ( input_capture[1] >  input_capture[0]) {

					capture_difference = input_capture[1] - input_capture[0];
				}

				else {

					capture_difference = (0xFFFF - input_capture[0]) + input_capture[1]+1;
				}

				// Compute the real-world frequency based on the 26 MHz timer base clock

				user_sig_freq = (HAL_RCC_GetPCLK1Freq() * 2 ) /  (float) capture_difference;

				// Print the accurate snapshot value safely
				sprintf(usr_msg, "Frequency of the signal applied = %f\r\n", user_sig_freq);
				HAL_UART_Transmit(&huart2, (uint8_t*) usr_msg, (uint16_t) strlen(usr_msg), HAL_MAX_DELAY ) ;

				// Clear flag
				is_capture_done = FALSE;

			}

		}

	while(1);

    return 0 ;

}


void SYSCLK_Config(uint8_t clock_freq) {

	// 1. Enable HSI SYSCLK and configure it as source clock

		memset(&osc_init, 0, sizeof(osc_init));

		osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI;

		osc_init.HSIState = RCC_HSI_ON  ;

		osc_init.HSICalibrationValue = 16;

		osc_init.PLL.PLLSource =  RCC_PLLSOURCE_HSI_DIV2 ;


		osc_init.PLL.PLLState = RCC_PLL_ON;


		// 2 . Configure AHB , APB1 AND APB2 Prescalers

		memset(&clk_init, 0, sizeof(clk_init));

		clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK \
							| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 ;


		clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK ;

		switch(clock_freq) {

			case(SYS_CLOCK_FREQ_36MHZ):

			{
				osc_init.PLL.PLLMUL =  RCC_PLL_MUL9;

				clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1 ;

				clk_init.APB1CLKDivider = RCC_HCLK_DIV4;

				clk_init.APB2CLKDivider = RCC_HCLK_DIV4 ;

				break;
			}



			case(SYS_CLOCK_FREQ_40MHZ):

		   {
				osc_init.PLL.PLLMUL =  RCC_PLL_MUL10;

				clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1 ;

				clk_init.APB1CLKDivider = RCC_HCLK_DIV2 ;

				clk_init.APB2CLKDivider = RCC_HCLK_DIV4 ;

				break;

		   }

			case(SYS_CLOCK_FREQ_4MHZ):

			{
					osc_init.PLL.PLLMUL =  RCC_PLL_MUL2;

					clk_init.AHBCLKDivider = RCC_SYSCLK_DIV2 ;

					clk_init.APB1CLKDivider = RCC_HCLK_DIV4 ;

					clk_init.APB2CLKDivider = RCC_HCLK_DIV4 ;

				break;

			}


			case(SYS_CLOCK_FREQ_52MHZ):

	        {
				osc_init.PLL.PLLMUL =  RCC_PLL_MUL13;

				clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1 ;

				clk_init.APB1CLKDivider = RCC_HCLK_DIV4 ;

				clk_init.APB2CLKDivider = RCC_HCLK_DIV4 ;

				break;
	        }

			case(SYS_CLOCK_FREQ_60MHZ):

	        {

				osc_init.PLL.PLLMUL =  RCC_PLL_MUL15;

				clk_init.AHBCLKDivider = RCC_HCLK_DIV2 ;

				clk_init.APB1CLKDivider = RCC_HCLK_DIV2 ;

				clk_init.APB2CLKDivider = RCC_HCLK_DIV2 ;

				break;

	        }

			default:
				return;

		}


		if (HAL_RCC_OscConfig(&osc_init) != HAL_OK) {

			 // there is a problem
					 Error_handler();

		}

		HAL_RCC_ClockConfig(&clk_init, FLASH_ACR_LATENCY_0);

		// Sysclk configuration

		HAL_SYSTICK_Config( HAL_RCC_GetSysClockFreq()/1000);

		HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

}

// Timer 2 parameter inits.
// to toggle a led every 40 micro seconds

void TIMER2_Init(void) {

	htimer2.Instance = TIM2;
	htimer2.Init.Prescaler = 9;
	htimer2.Init.Period = 104 -1;
	if (HAL_TIM_Base_Init(&htimer2) != HAL_OK )
	{
		Error_handler();
	}

}

void TIMER3_Init(void) {

	TIM_IC_InitTypeDef timerI3C_Config;

	// initialize peripheral base address
	htimer3.Instance = TIM3;
	// initialize the time base unit
	htimer3.Init.Prescaler = TIM_ICPSC_DIV1;
	htimer3.Init.Period = 0xFFFF;
	htimer3.Init.CounterMode =  TIM_COUNTERMODE_UP;
	if (HAL_TIM_IC_Init(&htimer3) != HAL_OK )
	{
		Error_handler();
	}

	// Configure the input channel of the timer
	timerI3C_Config.ICSelection = TIM_ICSELECTION_DIRECTTI ;
	timerI3C_Config.ICPolarity = TIM_ICPOLARITY_RISING;
	timerI3C_Config.ICFilter = 0;
	timerI3C_Config.ICPrescaler = TIM_ICPSC_DIV1;

	if ( HAL_TIM_IC_ConfigChannel(&htimer3, &timerI3C_Config, TIM_CHANNEL_2) != HAL_OK)
	{
		Error_handler();
	}

}


void Print_Freq(void) {

	    // display SYSCLK Feq.
		memset(msg, 0, sizeof(msg));
		sprintf(msg, "SYSCLK : %lu \r\n", HAL_RCC_GetSysClockFreq());
		HAL_UART_Transmit(&huart2, (uint8_t*) msg,  strlen(msg), HAL_MAX_DELAY);

		 // display PCLK1 Feq.
		memset(msg, 0, sizeof(msg));
		sprintf(msg, "PCLK1  : %lu \r\n", HAL_RCC_GetPCLK1Freq());
		HAL_UART_Transmit(&huart2, (uint8_t*) msg,  strlen(msg), HAL_MAX_DELAY);

}

void UART2_Init(void)
{

	 huart2.Instance =  USART2;
	 huart2.Init.BaudRate = 115200;
	 huart2.Init.WordLength = UART_WORDLENGTH_8B ;
	 huart2.Init.StopBits = UART_STOPBITS_1;
	 huart2.Init.Parity = UART_PARITY_NONE ;
	 huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	 huart2.Init.Mode = UART_MODE_TX_RX  ;
	 HAL_UART_Init(&huart2);
	 if( HAL_UART_Init(&huart2)!= HAL_OK){

		 // there is a problem
		 Error_handler();

	 }
	 HAL_UART_MspInit(&huart2);
}

/** Capture the first and second rising edge of the external signal
   generated by Timer 2 (PA9) and read via Timer 3 Channel 2 (PA10)
**/

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

	#if 1
	if(htim->Instance == TIM3 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {

		if(!is_capture_done) {


			if (count == 1 ) {

				// Read and store the timestamp of the 1st rising edge
				input_capture[0] = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_2);
				count++;
			}

			else if (count == 2)

			{
				// Read and store the timestamp of the 2nd rising edge
				input_capture [1]= __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_2);
				count = 1;
				is_capture_done = TRUE;
			}
		}

	}

	#endif

}


void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef *htimer){

		// User code can be executed
	 if (htimer->Instance == TIM2) {
		    	// Toggle the application LED on Channel 1 (PA10)
		    	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_10);
		    }



}



void Error_handler(void){

	// Infinite loop if error occurs, blinking a led can be used too here instead

	while(1);

}


