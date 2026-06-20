/**
  ******************************************************************************
  * @file           : stm32f1xx_hal_msp.c
  * @brief          : MCU Support Package (MSP) Hardware Initialization Mapping
  ******************************************************************************
  * @attention
  *
  * This file contains the low-level physical hardware initialization functions
  * invoked automatically by the HAL framework layer (`HAL_Init()`). It isolates
  * peripheral clock routing, GPIO alternate-function pinning, and NVIC interrupt
  * gating away from core application routing.
  *
  * ============================================================================
  * HARDWARE INITIALIZATION MAPPING
  * ============================================================================
  * 1. Timer 2 (Signal Generator Base):
  * - Low-level Function: `HAL_TIM_Base_MspInit()`
  * - Core Duties:
  * - Enables the peripheral bus clock gating for TIM2 (`__HAL_RCC_TIM2_CLK_ENABLE`).
  * - Maps Pin PA7 as a general-purpose push-pull output for physical signal probing.
  * - Configures the NVIC channel for `TIM2_IRQn` with appropriate sub-priorities.
  *
  * 2. Timer 3 (Input Capture Instrumentation Engine):
  * - Low-level Function: `HAL_TIM_IC_MspInit()`
  * - Core Duties:
  * - Enables peripheral bus clock gating for TIM3 (`__HAL_RCC_TIM3_CLK_ENABLE`).
  * - Configures Pin PA10 (TIM3 Channel 2) into Alternate Function Input mode
  * (Floating/Pull-up) to interface with the captured external signal.
  * - Configures the NVIC channel for `TIM3_IRQn` to trigger edge-capture flags.
  *
  * 3. USART1 (Serial Telemetry Link):
  * - Low-level Function: `HAL_UART_MspInit()`
  * - Core Duties:
  * - Enables the clock gate for USART1 and routes TX/RX lines to their
  * respective alternate function configurations (e.g., PA2/PA3 or remapped).
  *
  ******************************************************************************
  */

#include "stm32f1xx.h"
#include "msp.h"

extern TIM_HandleTypeDef htimer2;

// low level processor specific initialization
void HAL_MspInit(void)
{
 // low level processor specific inits

	// 1. Set up the priority grouping of the arm cortex mx processor
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	// 2. Enable the required system exceptions of the arm cortex mx processor
	SCB->SHCSR |= 0x7 << 16; //usg fault, memory fault and bus fault system exceptions

	// 3. Configure the priority for the system exceptions
	 HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
	 HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
	 HAL_NVIC_SetPriority(UsageFault_IRQn, 0,0);

}

// low level inits. of timer 2

void HAL_TIM_Base_MspInit (TIM_HandleTypeDef *htim) {

	// 1. Enable the clock for the TIMER 2 peripheral,

	__HAL_RCC_TIM2_CLK_ENABLE();

	// 2. Enable the IRQ and set up the priority (NVIC settings)

	 NVIC_EnableIRQ(TIM2_IRQn);
	 HAL_NVIC_SetPriority(TIM2_IRQn, 10, 0);

}

// low level inits. of timer3

void HAL_TIM_IC_MspInit (TIM_HandleTypeDef *htim) {

	 GPIO_InitTypeDef tim3ch2_gpio;

	// 1. Enable the clock for the TIMER 2 peripheral,

	__HAL_RCC_TIM3_CLK_ENABLE();

	// 2. Configure gpio pin PA7 to behave as timer3 channel 2

	tim3ch2_gpio.Pin = GPIO_PIN_7 ;
	tim3ch2_gpio.Mode = GPIO_MODE_AF_PP ;
	tim3ch2_gpio.Pull = GPIO_PULLUP;
	tim3ch2_gpio.Speed = GPIO_SPEED_FREQ_LOW;
	AFIO_REMAP_DISABLE(AFIO_MAPR_TIM3_REMAP);
	HAL_GPIO_Init(GPIOA,&tim3ch2_gpio);

	// 3. Enable the IRQ and set up the priority (NVIC settings)

	 NVIC_EnableIRQ(TIM3_IRQn);
	 HAL_NVIC_SetPriority(TIM3_IRQn, 15, 0);

}



void HAL_GPIOA_MspInit(void) {

	GPIO_InitTypeDef external_sig;

	// Enable the clock for GPIOA

	__HAL_RCC_GPIOA_CLK_ENABLE();

	// 2. Do the pin muxing configuration : configure PA10 as output

	external_sig.Pin = GPIO_PIN_10;

	external_sig.Pull = GPIO_NOPULL;

	external_sig.Mode = GPIO_MODE_OUTPUT_PP ;

	//3. GPIOA inits.

	HAL_GPIO_Init(GPIOA,&external_sig);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart) {

	 GPIO_InitTypeDef gpio_uart;


	// low level inits. of the USART2 peripheral

	// 1. Enable the clock for the USART2 peripheral

	__HAL_RCC_USART2_CLK_ENABLE();

	// 2. Do the pin muxing configuration
	//AFIO_REMAP_DISABLE(AFIO_MAPR_USART2_REMAP);
	// UART_TX
	gpio_uart.Pin = GPIO_PIN_2 ;
	gpio_uart.Mode = GPIO_MODE_AF_PP ;
	gpio_uart.Pull = GPIO_PULLUP;
	gpio_uart.Speed = GPIO_SPEED_FREQ_LOW;
	AFIO_REMAP_DISABLE(AFIO_MAPR_USART2_REMAP);
	HAL_GPIO_Init(GPIOA,&gpio_uart);
	// UART_RX
	gpio_uart.Pin = GPIO_PIN_3;
	HAL_GPIO_Init(GPIOA,&gpio_uart);
	// 3. Enable the IRQ and set up the priority (NVIC settings)

	  NVIC_EnableIRQ(USART2_IRQn);
	  HAL_NVIC_SetPriority(USART2_IRQn, 15, 0);
}

