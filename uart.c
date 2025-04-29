#include "xc.h"
#include "uart.h"
#include "timer.h"
#include "buffer.h"


void send_uart_char(unsigned char uart, char data) {
    if (uart == UART_1) {
        buffer_write(&transmit_buffer1, data);
    } else if (uart == UART_2) {
        buffer_write(&transmit_buffer2, data);
    }
}

void send_uart_string(unsigned char uart, const char *buffer) {
    IEC0bits.U1TXIE = 0;
    if (uart == UART_1){
        while (*buffer != '\n') {
            send_uart_char(UART_1, *buffer++);
        }
        send_uart_char(UART_1, '\n');
    } else {
        while (*buffer != '\n') {
            send_uart_char(UART_2, *buffer++);
        }
        send_uart_char(UART_2, '\n');
    }
    IEC0bits.U1TXIE = 1;
}

void UART_Init(unsigned char uart) {
    if (uart == UART_1) {
        // Configure UART1
        U1MODEbits.UARTEN = 0;   // Disable UART1
        RPOR0bits.RP64R = 1;     // TX pin mapping
        RPINR18bits.U1RXR = 75;  // RX pin mapping
        U1MODEbits.UARTEN = 0;   // disable UART
        U1MODEbits.STSEL = 0;    // Stop bit
        U1MODEbits.PDSEL = 0;    // Parity
        U1MODEbits.ABAUD = 0;    // Auto-baud disabled
        U1MODEbits.BRGH = 0;     // Low-speed mode
        U1BRG = BRGVAL;          // Baud rate
        IEC0bits.U1RXIE = 1;
        IFS0bits.U1RXIF =0;
        IEC0bits.U1TXIE = 0;
        IFS0bits.U1TXIF =0;
        U1MODEbits.UARTEN = 1;   // Enable UART1
        U1STAbits.UTXEN = 1;     // Enable transmitter
        
    } else if (uart == UART_2) {
        // Configure UART2
        U2MODEbits.UARTEN = 0;   // Disable UART2
        RPOR0bits.RP64R = 3;     // TX pin mapping
        RPINR19bits.U2RXR = 75;  // RX pin mapping
        U2MODEbits.STSEL = 1;    // Stop bit
        U2MODEbits.PDSEL = 0;    // Parity
        U2MODEbits.ABAUD = 0;    // Auto-baud disabled
        U2MODEbits.BRGH = 0;     // Low-speed mode
        U2BRG = BRGVAL;          // Baud rate
        IEC1bits.U2RXIE = 1;
        IFS1bits.U2RXIF =0;
        IEC1bits.U2TXIE = 0;
        IFS1bits.U2TXIF =0;
        U2MODEbits.UARTEN = 1;   // Enable UART2
        U2STAbits.UTXEN = 1;     // Enable transmitter
    }
}

void __attribute__((__interrupt__, auto_psv)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0;
    while (U1STAbits.URXDA) {
#if UART_OVERWRITE_ON_FULL
        while (!buffer_write(&main_buffer_1, U1RXREG)) {
            char tmp;
            buffer_read(&main_buffer_1, &tmp);
        }
#else
        char incoming = U1RXREG;
        buffer_write(&main_buffer_1, incoming);
#endif
    }
    if (U1STAbits.OERR){
        U1STAbits.OERR = 0;
    }
}

void __attribute__((__interrupt__, auto_psv)) _U2RXInterrupt(void) {  
    IFS1bits.U2RXIF = 0;
    while (U2STAbits.URXDA) {
#if UART_OVERWRITE_ON_FULL
        while (!buffer_write(&main_buffer_2, U2RXREG)) {
            char tmp;
            buffer_read(&main_buffer_2, &tmp);
        }
#else
        buffer_write(&main_buffer_2, U2RXREG);
#endif
    }
    if (U2STAbits.OERR){
        U2STAbits.OERR = 0;
    }
}

void __attribute__((__interrupt__, auto_psv)) _U1TXInterrupt(void){
    char data;
    buffer_read(&transmit_buffer1, &data);
    U1TXREG = data;
    IFS0bits.U1TXIF = 0;
    // if no data ready for sending out
    if (transmit_buffer1.count <= 0){
        IEC0bits.U1TXIE = 0;
    }
}

void __attribute__((__interrupt__, auto_psv)) _U2TXInterrupt(void){
    char data;
    buffer_read(&transmit_buffer2, &data);
    U2TXREG = data;
    IFS1bits.U2TXIF = 0;
    if (transmit_buffer2.count <= 0){
        IEC1bits.U2TXIE = 0;
    }
}
