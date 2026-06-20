/*
 * msp.h
 *
 *  Created on: May 3, 2026
 *      Author: asmae
 */

#ifndef INC_MSP_H_
#define INC_MSP_H_

void HAL_MspInit(void);
void HAL_TIM_Base_MspInit (TIM_HandleTypeDef *htim);
void HAL_GPIOA_MspInit(void);
void HAL_UART_MspInit(UART_HandleTypeDef *huart) ;
#endif /* INC_MSP_H_ */
