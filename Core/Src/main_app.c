/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Hardware PWM LED Fading Engine via Clock-Scaled Duty Modulation
  ******************************************************************************
  * @attention
  *
  * This application demonstrates advanced hardware peripheral scaling on an ARM
  * Cortex-M3 (STM32F103RB) to drive a 1.08 kHz PWM LED breathing animation cycle.
  *
  * ============================================================================
  * SYSTEM CLOCK TREE & TIMER TIMEBASE MATH
  * ============================================================================
  * - System Clock (SYSCLK)      = 52 MHz (Via PLL)
  * - Timer 2 Base Clock         = 26 MHz
  * - Timer Prescaler            = 49 (Yields a internal count rate of 520 kHz)
  * - Timer Period (ARR)         = 479 (480-1 counts)
  *
  * *Resulting PWM Frequency:* 520,000 Hz / 480 counts = 1,083.33 Hz (Flicker-Free)
  *
  * ============================================================================
  * ALGORITHMIC CONTROL MECHANISM
  * ============================================================================
  * The main execution thread uses a dual-nested blocking state layout inside the
  * primary `while(1)` super-loop to smoothly step the luminance level:
  *
  * 1. FADE UP LOOP: Increments the `brightness` counter by 50 ticks every 1000ms
  * until it reaches the 479 limit (~10 seconds total ramp-up).
  * 2. FADE DOWN LOOP: Decrements the `brightness` counter by 50 ticks every 1000ms
  * back down to zero (~10 seconds total ramp-down).
  *
  * - **Zero-Interrupt Execution:** Pin switching runs entirely in hardware layout
  * logic (`TIM_OCMODE_PWM1`). The CPU only modifies the active Duty Cycle values
  * by writing directly to the compare register via `__HAL_TIM_SET_COMPARE`.
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


TIM_HandleTypeDef htimer2;



int main(void){

	uint16_t brightness = 0;

	// HAL library inits.
	HAL_Init();

	// SYSCLK configuration
	SYSCLK_Config_HSE();


	// Timer 2 inits.

	TIMER2_Init() ;


	// start timer 2 in PWM mode
	if (HAL_TIM_PWM_Start(&htimer2,TIM_CHANNEL_1) != HAL_OK) {

		Error_handler();
	}

	while(1) {


		while (brightness < htimer2.Init.Period) {

			brightness += 50;
			__HAL_TIM_SET_COMPARE(&htimer2, TIM_CHANNEL_1 ,brightness);
			HAL_Delay(1000);
		}

		while (brightness > 0) {

			brightness -= 50;
			__HAL_TIM_SET_COMPARE(&htimer2, TIM_CHANNEL_1 ,brightness);
			HAL_Delay(1000);
		}
	}


    return 0 ;

}


void SYSCLK_Config_HSE(void) {

	// 1. Enable HSI SYSCLK and configure it as source clock

		memset(&osc_init, 0, sizeof(osc_init));

		osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;

		osc_init.HSEState =  RCC_HSE_BYPASS;

		osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE ;

		osc_init.PLL.PLLState = RCC_PLL_ON;


		// 2 . Configure AHB , APB1 AND APB2 Prescalers

		memset(&clk_init, 0, sizeof(clk_init));

		clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK \
							| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 ;


		clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK ;

		osc_init.PLL.PLLMUL =  RCC_PLL_MUL6;

		clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1 ;

		clk_init.APB1CLKDivider = RCC_HCLK_DIV4 ;

		clk_init.APB2CLKDivider = RCC_HCLK_DIV4 ;


		if (HAL_RCC_OscConfig(&osc_init) != HAL_OK) {

			 // there is a problem
					 Error_handler();

		}

		if(HAL_RCC_ClockConfig(&clk_init, FLASH_ACR_LATENCY_1) != HAL_OK) {

			Error_handler();
		};

		__HAL_RCC_HSI_DISABLE();

		// Sysclk configuration

		HAL_SYSTICK_Config( HAL_RCC_GetSysClockFreq()/1000);

		HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

}

// Timer 2 parameter inits.

void TIMER2_Init(void) {

	TIM_OC_InitTypeDef tim2PWM_init;

	// initialize the output compare time base unit

	htimer2.Instance = TIM2;
	htimer2.Init.Prescaler = 49;
	htimer2.Init.Period = 480-1;
	if (HAL_TIM_PWM_Init(&htimer2)!= HAL_OK )
	{
		Error_handler();
	}

	memset(&tim2PWM_init , 0, sizeof(tim2PWM_init));
	// --- Shared Channel Hardware Configuration Parameters ---

	tim2PWM_init.OCMode = TIM_OCMODE_PWM1 ;
	tim2PWM_init.OCPolarity =  TIM_OCPOLARITY_HIGH;
	tim2PWM_init.Pulse = 0;
	//tim2PWM_init.Pulse = (htimer2.Init.Period * 25)/100 ;

	// configure output compare channel 1

	if(HAL_TIM_PWM_ConfigChannel(&htimer2, &tim2PWM_init,TIM_CHANNEL_1) != HAL_OK) {

		 Error_handler();
	}




}

void Error_handler(void){

	// Infinite loop if error occurs, blinking a led can be used too here instead

	while(1);

}


