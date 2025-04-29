#include "xc.h"
#include "timer.h"
#include "uart.h"
#include "spi.h"
#include "parser.h"
#include "buffer.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAG_CS LATDbits.LATD6
#define NUM_READINGS 6
#define MOVING_AVERAGE_SIZE 5

int tmr_counter = 0;
char *patterns[] = {};
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
int16_t calculate_moving_average(int16_t new_value, int16_t buffer[MOVING_AVERAGE_SIZE], uint8_t *idx);
int16_t merge_significant_bits(uint8_t low, uint8_t high, int axis);
void simulate_algorithm(void);
void update_led(void);

int main(void) {
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;

    // Init LED1
    int ret;
    TRISAbits.TRISA0 = 0;
    
    // Init LED2
    TRISGbits.TRISG9 = 0;
    
    buffer_init(&main_buffer_1, patterns, 0);
    buffer_init(&transmit_buffer1, patterns, 0);
    buffer_init(&transmit_buffer2, patterns, 0);
    
    // Init parser
    parser_state pstate;
	pstate.state = STATE_DOLLAR;
	pstate.index_type = 0; 
	pstate.index_payload = 0;
    
    UART_Init(UART_1);
    
    spi_init();
    tmr_wait_ms(TIMER1, 5);

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
    
    send_uart_char(UART_1, chip_id / 16 + '0');
    send_uart_char(UART_1, chip_id % 16 + '0');
    send_uart_char(UART_1, '\n');

    tmr_setup_period(TIMER2, 10);
    tmr_turn(TIMER2, 1); 

    ps.state = STATE_DOLLAR;

    static int mag_send_timer = 0;
    static int yaw_send_timer = 0;
    static int led_timer = 0;
    while (1) {

        simulate_algorithm();
        
        // Read Magnetometer from SPI
        MAG_CS = 0;
        spi_read_multiple(readings, 0x42);
        MAG_CS = 1;
        int16_t x_data = merge_significant_bits(readings[0], readings[1], 1);
        int16_t average_x = calculate_moving_average(x_data, moving_average_buffer_x, &buffer_x_index);
        int16_t y_data = merge_significant_bits(readings[2], readings[3], 2);
        int16_t average_y = calculate_moving_average(y_data, moving_average_buffer_y, &buffer_y_index);
        int16_t z_data = merge_significant_bits(readings[4], readings[5], 3);
        int16_t average_z = calculate_moving_average(z_data, moving_average_buffer_z, &buffer_z_index);

        mag_send_timer += 10;
        yaw_send_timer += 10;
        led_timer += 10;

        if (mag_rate_hz != 0) {
            if (mag_send_timer >= (1000 / mag_rate_hz)) {
                mag_send_timer = 0;
                sprintf(buff, "$MAG,%d,%d,%d*\n", average_x, average_y, average_z);
                send_uart_string(UART_1, buff);
            }
        }

        if (yaw_send_timer >= 200) {
            yaw_send_timer = 0;
            int heading_deg = atan2(average_y, average_x) * (180.0 / M_PI);
            sprintf(buff, "$YAW,%d*\n", heading_deg);
            send_uart_string(UART_1, buff);
        }
        
        if (led_timer >= 500){
            led_timer = 0;
            update_led();
        }

        IEC0bits.U1RXIE = 0;
        process_uart();
        IEC0bits.U1RXIE = 1;
        
        if (transmit_buffer1.count > 0){
            IEC0bits.U1TXIE = 1;
        }
        
        ret = tmr_wait_period_3(TIMER2);
        if (ret){
            LATAbits.LATA0 = 1;
        } else{
            LATAbits.LATA0 = 0;
        }
        
    }   
}

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
        data = data / 2;
    }
    return data;
}

void simulate_algorithm(void) {
    tmr_wait_ms(TIMER1, 7);
}

void update_led(void) {
    LATGbits.LATG9 ^= 1;
}

void process_uart(void) {
    char c;
    while (buffer_read(&main_buffer_1, &c)) {
        if (parse_byte(&ps, c) == NEW_MESSAGE) {
            sprintf(buff, "$MSG,%s,%s*\n", ps.msg_type, ps.msg_payload);
            send_uart_string(UART_1, buff);
            if (strcmp(ps.msg_type, "RATE") == 0) {
                int new_rate = extract_integer(ps.msg_payload);
                if (new_rate == 0 || new_rate == 1 || new_rate == 2 || new_rate == 4 || new_rate == 5 || new_rate == 10) {
                    sprintf(buff, "$NEW_RATE,%d*\n", new_rate);
                    send_uart_string(UART_1, buff);
                    mag_rate_hz = new_rate;
                } else {
                    send_uart_string(UART_1, "$ERR,1*\n");
                }
            }
        }
    }
}

