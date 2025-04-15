#include "parser.h"
#include "uart.h"

void parse_uart_commands(char* uart_receive_buffer, int length, CommandState* commands) {
    if (uart_receive_buffer[0] == '\0') return;

    int cmd_i = 0;
    while (commands[cmd_i].name != 0) {
        commands[cmd_i].stop = 0;
        cmd_i++;
    }

    uart_transmit(URT1, uart_receive_buffer, length);

    int buf_i = 0;
    while (uart_receive_buffer[buf_i] != '\0') {
        cmd_i = 0;
        while (commands[cmd_i].name != 0) {
            if (commands[cmd_i].stop) {
                cmd_i++;
                continue;
            }

            int match = 1;
            int j = 0;

            while (commands[cmd_i].name[commands[cmd_i].save_index + j] != '\0') {
                char received = uart_receive_buffer[buf_i + j];
                char expected = commands[cmd_i].name[commands[cmd_i].save_index + j];

                if (received == '\0') {
                    commands[cmd_i].save_index += j;
                    commands[cmd_i].stop = 1;
                    match = 0;
                    break;
                }

                if (received != expected) {
                    match = 0;
                    break;
                }

                j++;
            }

            if (!commands[cmd_i].stop && commands[cmd_i].save_index != 0 && !match) {
                commands[cmd_i].save_index = 0;
            }

            if (match && !commands[cmd_i].stop) {
                commands[cmd_i].activations++;
            }

            cmd_i++;
        }

        buf_i++;
    }
}