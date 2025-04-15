/**
 * @file uart.c
 * @brief UART driver implementation for dsPIC33EP512MU810.
 *
 * This module provides functions for configuring, receiving, and transmitting data
 * over UART1 and UART2. It includes interrupt service routines for handling received data
 * and buffering mechanisms to store and process UART communication efficiently.
 *
 * @author EmbeddedG2
 * @date March 25, 2025
 */

#include "xc.h"
#include "uart.h"

// UART1 Buffers and Indices
static int UART1_producer_index = 0;
static char UART1_receive_buffer[BUFFER_SIZE];
static int UART1_consumer_index = 0;

// UART2 Buffers and Indices
static int UART2_producer_index = 0;
static char UART2_receive_buffer[BUFFER_SIZE];
static int UART2_consumer_index = 0;

/**
 * @brief Configures the UART module with the specified parameters.
 *
 * This function initializes and configures the selected UART module
 * (UART1 or UART2) with the given stop bit and parity check settings.
 * It also enables UART reception interrupts and activates transmission.
 *
 * @param[in] URT The UART module to configure (UART1 or UART2).
 * @param[in] stop_bit The stop bit setting (0 = 1 stop bit, 1 = 2 stop bits).
 * @param[in] parity_check The parity check setting:
 *             - 00: 8-bit data, no parity
 *             - 01: 8-bit data, even parity
 *             - 10: 8-bit data, odd parity
 *             - 11: 9-bit data, no parity
 *
 * @note The function disables the UART before configuration and re-enables it after setup.
 * @note It sets the baud rate to 9600 using the predefined `BRGVAL`.
 * @note The function enables UART receive interrupts to handle incoming data.
 *
 * @warning Ensure that `BRGVAL` is correctly defined based on the system clock.
 *
 * @example
 * @code
 * // Configure UART1 with 1 stop bit and no parity
 * uart_config(URT1, 0, 0);
 * @endcode
 */
void uart_config(int URT, int stop_bit, int parity_check){    
    switch (URT){
        case URT1:
            // Disable the UART 1 before configuration
            U1MODEbits.UARTEN   = 0;
            // UART configuration
            U1MODEbits.STSEL    = stop_bit;     // Stop bit
            U1MODEbits.PDSEL    = parity_check; // Parity check
            U1MODEbits.ABAUD    = 0;            // Disable Auto-Baud
            U1MODEbits.BRGH     = 0;            // Set speed mode
            U1BRG               = BRGVAL;       // Set baud rate to 9600
            // Interrupts for receiving
            IFS0bits.U1RXIF = 0;
            IEC0bits.U1RXIE = 1;
            // Activating transmission
            U1MODEbits.UARTEN   = 1;            // Enable UART 1
            U1STAbits.UTXEN     = 1;            // Enable UART 1 transmission
            break;
        case URT2:
            // Disable the UART 1 before configuration
            U2MODEbits.UARTEN   = 0;
            // UART configuration
            U2MODEbits.STSEL    = stop_bit;     // Stop bit
            U2MODEbits.PDSEL    = parity_check; // Parity check
            U2MODEbits.ABAUD    = 0;            // Disable Auto-Baud
            U2MODEbits.BRGH     = 0;            // Set speed mode
            U2BRG               = BRGVAL;       // BAUD Rate Setting for 9600
            // Interrupts for receiving
            IFS1bits.U2RXIF = 0;
            IEC1bits.U2RXIE = 1;
            // Activating transmission
            U2MODEbits.UARTEN   = 1;            // Enable UART 2
            U2STAbits.UTXEN     = 1;            // Enable UART 2 transmission
            break;
        default:
            return;
    }
}

/**
 * @brief UART1 Receive Interrupt Service Routine (ISR).
 *
 * This interrupt is triggered when data is received in the UART1 module.
 * It reads the received data from the UART1 receive register (U1RXREG) and
 * stores it in the receive buffer. The producer index is updated to track
 * the next available position in the buffer while preventing buffer overflow.
 *
 * @note The function clears the UART1 receive interrupt flag (U1RXIF) at the start.
 * @note The buffer management ensures that new data does not overwrite unread data.
 *
 * @warning The producer index update previously contained a post-increment issue.
 *          The corrected logic `UART1_producer_index = (UART1_producer_index + 1) % BUFFER_SIZE;`
 *          is used to prevent unintended behavior.
 *
 * @example
 * @code
 * // The ISR is automatically triggered when UART1 receives data.
 * // The received data is stored in UART1_receive_buffer for later processing.
 * @endcode
 */
void __attribute__((__interrupt__, auto_psv)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0;
    while (U1STAbits.URXDA && (UART1_producer_index + 2) % BUFFER_SIZE != UART1_consumer_index) {
        UART1_receive_buffer[UART1_producer_index] = U1RXREG;
        UART1_producer_index = (UART1_producer_index + 1) % BUFFER_SIZE;
    }
    
    if (U1STAbits.OERR){
        U1STAbits.OERR = 0;
    }
}

/**
 * @brief UART2 Receive Interrupt Service Routine (ISR).
 *
 * This interrupt is triggered when data is received in the UART2 module.
 * It reads the received data from the UART2 receive register (U2RXREG) and
 * stores it in the receive buffer. The producer index is updated to track
 * the next available position in the buffer.
 *
 * @note The function clears the UART2 receive interrupt flag (U2RXIF) at the start.
 * @note The function prevents buffer overflow by ensuring that the producer index
 *       does not overwrite unread data in the buffer.
 *
 * @warning The producer index update logic contains a post-increment issue (`(UART2_producer_index + 1) % BUFFER_SIZE`),
 *          which should be corrected to avoid unintended behavior. Use:
 *          `UART2_producer_index = (UART2_producer_index + 1) % BUFFER_SIZE;`
 *
 * @example
 * @code
 * // The ISR is automatically triggered when UART2 receives data.
 * // The received data is stored in UART2_receive_buffer for later processing.
 * @endcode
 */
void __attribute__((__interrupt__, auto_psv)) _U2RXInterrupt(void) {  
    IFS1bits.U2RXIF = 0;
    while (U2STAbits.URXDA && (UART2_producer_index + 2)  % BUFFER_SIZE != UART2_consumer_index) {
        
        UART2_receive_buffer[UART2_producer_index] = U2RXREG;
        UART2_producer_index = (UART2_producer_index + 1) % BUFFER_SIZE;
    }

    if (U2STAbits.OERR){
        U2STAbits.OERR = 0;
    }
}

/**
 * @brief Receives data from the specified UART module.
 *
 * This function reads all available data from the UART receive buffer and stores it
 * into the provided buffer. The function stops reading when the receive buffer is empty.
 *
 * @param URT The UART module to use (UART1 or UART2).
 * @param UART_receive Pointer to the buffer where received data will be stored.
 * @return 0 if data was successfully read, -1 if an invalid UART module was specified.
 *
 * @note This function does not add a null terminator (`\0`) at the end of the received data.
 *       The caller is responsible for handling the received data accordingly.
 * @warning Ensure that the `UART_receive` buffer is large enough to store the received data.
 *          Otherwise, buffer overflow may occur.
 * 
 * @example
 * @code
 * char received_data[BUFFER_SIZE];
 * if (uart_receive(URT1, received_data) == 0) {
 *     // Process received data
 * } else {
 *     // Handle error
 * }
 * @endcode
 */
 int uart_receive(int URT, char* UART_receive){
    int count = 0;
    int max_count = 100;
    switch (URT) {
        case URT1:
            while (UART1_consumer_index != UART1_producer_index && count < max_count) {
                UART_receive[count++] = UART1_receive_buffer[UART1_consumer_index];
                UART1_consumer_index = (UART1_consumer_index + 1) % BUFFER_SIZE;
            }
            UART_receive[count] = 0;
            break;
            
        case URT2:
            while (UART2_consumer_index != UART2_producer_index && count < max_count) {
                UART_receive[count++] = UART2_receive_buffer[UART2_consumer_index];
                UART2_consumer_index = (UART2_consumer_index + 1) % BUFFER_SIZE;
            }
            UART_receive[count] = 0;
            break;
            
        default:
            return -1;
    }
    return count;
}

/**
 * @brief Transmits a buffer of characters over the specified UART module.
 *
 * This function sends data via UART1 or UART2 by writing to the transmit register.
 * It ensures that data is transmitted only when the transmit buffer is not full.
 *
 * @param URT The UART module to use (UART1 or UART2).
 * @param UART_transmit_buffer Pointer to the buffer containing data to be transmitted.
 * @param UART_transmit_buffer_size The number of characters to transmit.
 * @return 0 if the transmission is successful, -1 if an invalid UART module is specified.
 *
 * @note This function blocks execution while waiting for the transmit buffer to have space.
 * @warning If the UART transmit buffer is full, this function will wait indefinitely.
 *          Consider using a timeout mechanism or an interrupt-based approach for non-blocking behavior.
 * 
 * @example
 * @code
 * char message[] = "Hello, UART!";
 * int result = uart_transmit(URT1, message, sizeof(message) - 1);
 * if (result == 0) {
 *     // Transmission successful
 * } else {
 *     // Handle error
 * }
 * @endcode
 */
int uart_transmit(int URT, char* UART_transmit_buffer, int UART_transmit_buffer_size){
    int ret = 0;
    int count = 0;
    switch (URT) {
        case URT1:
            do {
                while (U1STAbits.UTXBF);
                U1TXREG = UART_transmit_buffer[count++];
            } while (count < UART_transmit_buffer_size);
            break;
            
        case URT2:
            do {
                while (U2STAbits.UTXBF);
                U2TXREG = UART_transmit_buffer[count++];
            } while (count < UART_transmit_buffer_size);
            break;
            
        default:
            ret = -1;
            break;
    }
    
    return ret;
}

