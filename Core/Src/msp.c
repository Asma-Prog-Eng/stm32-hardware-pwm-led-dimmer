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

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef  *htim) {

	 GPIO_InitTypeDef tim2OC_ch_gpios;

	// 1. Enable peripheral bus clocks
	__HAL_RCC_TIM2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();

	//  2. Configure Partial Remap 2: CH1=PA0, CH2=PA1, CH3=PB10, CH4=PB11

	 __HAL_AFIO_REMAP_TIM2_PARTIAL_2();

	// --- Configure Channel 1 and 2 on Port A ---
	tim2OC_ch_gpios.Pin = GPIO_PIN_0 | GPIO_PIN_1 ;
	tim2OC_ch_gpios.Mode = GPIO_MODE_AF_PP ;
	tim2OC_ch_gpios.Pull =  GPIO_PULLUP;
	tim2OC_ch_gpios.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA,&tim2OC_ch_gpios);

	// --- Configure Channel 3 and 4 on Port B ---
	tim2OC_ch_gpios.Pin = GPIO_PIN_10 | GPIO_PIN_11 ;
	tim2OC_ch_gpios.Mode = GPIO_MODE_AF_PP ;
	tim2OC_ch_gpios.Pull =  GPIO_PULLUP;
	tim2OC_ch_gpios.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB,&tim2OC_ch_gpios);

	// 3. Enable the IRQ and set up the priority (NVIC settings)
	 NVIC_EnableIRQ(TIM2_IRQn);
	 HAL_NVIC_SetPriority(TIM2_IRQn, 10, 0);

}


