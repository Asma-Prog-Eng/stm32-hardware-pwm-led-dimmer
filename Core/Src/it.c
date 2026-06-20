/**
  ******************************************************************************
  * @file           : stm32f1xx_it.c
  * @brief          : Interrupt Service Routines & High-Speed Vector Handlers
  ******************************************************************************
  * @attention
  *
  * This module encapsulates the time-critical Interrupt Service Routines (ISRs)
  * executed by the ARM Cortex-M3 core upon hardware peripheral flag events.
  *
  * ============================================================================
  * CORE INTERRUPT SERVICE ROUTINES
  * ============================================================================
  * 1. `TIM2_IRQHandler(void)` - Timebase Generation Context
  * - Trigger Interval: Every 40 microseconds (driven by the 26 MHz timer clock).
  * - Execution Logic: Manually handles periodic update events, clearing the
  * hardware update interrupt flag (UIF), and toggling Pin PA9 to generate
  * the 12.5 kHz test square wave observed on the logic analyzer.
  *
  * 2. `TIM3_IRQHandler(void)` - Edge Instrumentation Context
  * - Trigger Interval: Driven by incoming rising edges on the physical PA10 pin.
  * - Execution Logic: Invokes `HAL_TIM_IRQHandler()`, which evaluates the active
  * capture source channel and routes execution immediately into the weak-linked
  * `HAL_TIM_IC_CaptureCallback` override block.
  *
  * ============================================================================
  * REAL-TIME CONSTRAINT MITIGATION
  * ============================================================================
  * Both handlers are critical to keeping the system from throwing an infinite loop
  * trap. High-frequency execution demands that any heavy data formatting (`sprintf`)
  * or slow blocking peripheral calls (`HAL_UART_Transmit`) are strictly banned
  * from executing inside this file's context.
  *
  * Instead, these routines operate purely as fast snapshot registers and flag
  * indicators, leaving data processing to the main loop background layer.
  *
  ******************************************************************************
  */
#include "stm32f1xx_hal.h"

extern TIM_HandleTypeDef htimer2, htimer3;

// override the default handler, = the infinite loop
// When the SysTick interrupt is triggered, control is transferred to this handler,
//and the system gets stuck in this loop. Consequently, other operations,
// like printing messages, cannot proceed beyond a certain point.

void SysTick_Handler(void) {

	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}


// 2. Run the TIM2 HAL handler

void TIM2_IRQHandler(void) {

	HAL_TIM_IRQHandler(&htimer2);

}

// 2. Run the TIM3 HAL handler

void TIM3_IRQHandler(void) {


	HAL_TIM_IRQHandler(&htimer3);
}
