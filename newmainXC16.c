/*
 * File:   newmainXC16.c
 * Author: shady
 *
 * Created on April 21, 2025, 11:54 AM
 */

#include "xc.h"
#include "timer.h"
#include "uart.h"
#include "spi.h"
#include "parser.h"
#include "buffer.h"
#include <stdio.h>
#include <math.h>

#define FCY 72000000UL
#define BAUDRATE 9600
#define BRGVAL ((FCY / BAUDRATE) / 16 - 1)

#define MAG_CS LATDbits.LATD6
#define NUM_READINGS 6
#define MOVING_AVERAGE_SIZE 10

char buff[35];

int16_t moving_average_buffer_x[MOVING_AVERAGE_SIZE];
int16_t moving_average_buffer_y[MOVING_AVERAGE_SIZE];
int16_t moving_average_buffer_z[MOVING_AVERAGE_SIZE];
uint8_t buffer_x_index = 0;
uint8_t buffer_y_index = 0;
uint8_t buffer_z_index = 0;

uint8_t readings[NUM_READINGS];

parser_state ps;
volatile int mag_rate_hz = 5; // default 5 Hz

int16_t calculate_moving_average(int16_t new_value, int16_t buffer[MOVING_AVERAGE_SIZE], uint8_t *idx) {
    buffer[*idx] = new_value;
    *idx = (*idx + 1) % MOVING_AVERAGE_SIZE;
    int16_t sum = 0;
    for (uint8_t i = 0; i < MOVING_AVERAGE_SIZE; i++) {
        sum += buffer[i];
    }
    return (int16_t)(sum / MOVING_AVERAGE_SIZE);
}

int16_t merge_significant_bits(uint8_t low, uint8_t high, int axis) {
    int16_t data;
    if (axis == 1 || axis == 2) {
        uint8_t masked_low = low & 0xF8;
        data = (int16_t)((high << 8) | masked_low);
        data = data / 8;
    } else {
        uint8_t masked_low = low & 0xFE;
        data = (int16_t)((high << 8) | masked_low);
        data = data / 8;
    }
    return data;
}

void simulate_algorithm(void) {
    tmr_wait_ms(TIMER1, 7);
}

void update_led(void) {
    LATDbits.LATD2 ^= 1;
}

void process_uart(void) {
    char c;
    while (buffer_read(&main_buffer_1, &c)) {
        if (parse_byte(&ps, c) == NEW_MESSAGE) {
            if (strcmp(ps.msg_type, "RATE") == 0) {
                int new_rate = extract_integer(ps.msg_payload);
                if (new_rate == 0 || new_rate == 1 || new_rate == 2 || new_rate == 4 || new_rate == 5 || new_rate == 10) {
                    mag_rate_hz = new_rate;
                } else {
                    send_uart_string("$ERR,1*\n");
                }
            }
        }
    }
}

int main(void) {
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;


    UART_Init(UART_1);
    send_uart_char(UART_1, 'S');
    send_uart_char(UART_1, '\n');

    spi_init();
    tmr_wait_ms(TIMER1, 100);

    MAG_CS = 0;
    spi_write(0x4B, 0x01);
    MAG_CS = 1;
    tmr_wait_ms(TIMER1, 5);

    MAG_CS = 0;
    spi_write(0x4C, 0x30);
    MAG_CS = 1;
    tmr_wait_ms(TIMER1, 5);

    MAG_CS = 0;
    uint8_t chip_id = spi_read(0x40);
    MAG_CS = 1;
    tmr_wait_ms(TIMER1, 10);

    U1TXREG = chip_id / 16 + '0';
    while (U1STAbits.UTXBF);
    U1TXREG = chip_id % 16 + '0';
    while (U1STAbits.UTXBF);
    U1TXREG = 'E';
    while (U1STAbits.UTXBF);
    send_uart_char(UART_1, '\n');

    tmr_setup_period(TIMER1, 10);
    tmr_turn(TIMER1, 1);
    tmr_setup_period(TIMER3, 500);
    tmr_turn(TIMER3, 1);

    ps.state = STATE_DOLLAR;

    TRISDbits.TRISD2 = 0;

    while (1) {
        static int mag_send_timer = 0;
        static int yaw_send_timer = 0;

        simulate_algorithm();

        if (IFS0bits.T3IF) {
            IFS0bits.T3IF = 0;
            update_led();
        }

        // Magnetometer read (still every 10 ms for moving average)
        MAG_CS = 0;
        spi_read_multiple(readings, 0x42);
        MAG_CS = 1;

        int16_t x_data = merge_significant_bits(readings[0], readings[1], 1);
        int16_t average_x = calculate_moving_average(x_data, moving_average_buffer_x, &buffer_x_index);

        int16_t y_data = merge_significant_bits(readings[2], readings[3], 2);
        int16_t average_y = calculate_moving_average(y_data, moving_average_buffer_y, &buffer_y_index);

        int16_t z_data = merge_significant_bits(readings[4], readings[5], 3);
        int16_t average_z = calculate_moving_average(z_data, moving_average_buffer_z, &buffer_z_index);

        mag_send_timer += 10; // Increase by 10 ms every loop
        yaw_send_timer += 10;

        // Send $MAG at mag_rate_hz
        if (mag_rate_hz != 0 && mag_send_timer >= (1000 / mag_rate_hz)) {
            mag_send_timer = 0;
            sprintf(buff, "$MAG,%d,%d,%d*\n", average_x, average_y, average_z);
            send_uart_string(buff);
        }

        // Send $YAW every 200 ms (5 Hz)
        if (yaw_send_timer >= 200) {
            yaw_send_timer = 0;
            int heading_deg = atan2(average_y, average_x) * (180.0 / M_PI);
            sprintf(buff, "$YAW,%d*\n", heading_deg);
            send_uart_string(buff);
        }

        process_uart();
        tmr_wait_period(TIMER1);
    }
}
