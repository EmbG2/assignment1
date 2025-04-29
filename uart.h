/* 
 * File:   
 * Author: EMBG2
 * Comments:
 * Revision history: 
 */

#ifndef UART_H
#define	UART_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "buffer.h"

#define UART_1 1
#define UART_2 2

#define FCY 72000000UL
#define BAUDRATE 9600
#define BRGVAL ((FCY / BAUDRATE) / 16 - 1)

#define UART_OVERWRITE_ON_FULL 0

void send_uart_char(unsigned char uart, char data);
void send_uart_string(unsigned char uart, const char *buffer);
void UART_Init(unsigned char uart);
void process_uart(void);

// interrupt function declarations
extern void __attribute__((__interrupt__, auto_psv)) _U1RXInterrupt(void);
extern void __attribute__((__interrupt__, auto_psv)) _U2RXInterrupt(void);
extern void __attribute__((__interrupt__, auto_psv)) _U1TXInterrupt(void);
extern void __attribute__((__interrupt__, auto_psv)) _U2TXInterrupt(void);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* UART_H */

