/*
 * main_app.h
 *
 *  Created on: May 3, 2026
 *      Author: asmae
 */

#ifndef INC_MAIN_APP_H_
#define INC_MAIN_APP_H_

#define TRUE 1

#define FALSE 0

#define SYS_CLOCK_FREQ_36MHZ 36

#define SYS_CLOCK_FREQ_40MHZ 40

#define SYS_CLOCK_FREQ_52MHZ 52

#define SYS_CLOCK_FREQ_60MHZ 60

#define SYS_CLOCK_FREQ_4MHZ 4

void Error_handler(void);
void SYSCLK_Config(uint8_t clock_freq);
void TIMER2_Init(void) ;
void TIMER3_Init(void);
void Print_Freq(void);
void UART2_Init(void);
#endif /* INC_MAIN_APP_H_ */
