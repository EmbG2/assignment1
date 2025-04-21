/*
 * File:   buffer.c
 * Author: shady
 *
 * Created on April 8, 2025, 8:22 PM
 */


#include "buffer.h"
#include "xc.h"

CircularBuffer main_buffer_1;
CircularBuffer main_buffer_2;

void buffer_init(CircularBuffer *buffer, char **patterns, int pattern_count)
{
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;
    buffer->patterns = patterns;
    buffer->pattern_count = pattern_count;
    for (int i = 0; i < pattern_count; i++) {
        buffer->flags[i] = 0;
    }
}

int buffer_write(CircularBuffer *buffer, char value)
{
    if (buffer->count == MAIN_BUFFER_SIZE)
    {
        return 0;
    }
    buffer->data[buffer->tail] = value;
    buffer->tail = (buffer->tail + 1) % MAIN_BUFFER_SIZE;
    buffer->count++;
    return 1;
}

int buffer_read(CircularBuffer *buffer, char *value)
{
    if (buffer->count == 0)
    {
        return 0;
    }
    *value = buffer->data[buffer->head];
    buffer->head = (buffer->head + 1) % MAIN_BUFFER_SIZE;
    buffer->count--;
    return 1;
}

int buffer_peek(const CircularBuffer *buffer, int index)
{
    if (index >= buffer->count)
    {
        return -1;
    }
    return buffer->data[(buffer->head + index) % MAIN_BUFFER_SIZE];
}

void uart_debug_send(char c) {
    while (U1STAbits.UTXBF); // Wait while TX buffer is full
    U1TXREG = c;
}


void detect_pattern(CircularBuffer *buffer)
{
    char temp[10];
    int match_found = 0;
    while (buffer->count > 2)
    {
        for (int i = 0; i < buffer->pattern_count; i++)
        {
            int pattern_len = 0;
            while (buffer->patterns[i][pattern_len] != '\0')
            {
                pattern_len++;
            }

            int j = 0;
            int valid_pattern = 1;

            for (; j < pattern_len; j++)
            {
                if (buffer_peek(buffer, j) != buffer->patterns[i][j])
                {
                    valid_pattern = 0;
                    break;
                }
            }

            if (valid_pattern)
            {
                for (int k = 0; k < pattern_len; k++)
                {
                    buffer_read(buffer, &temp[k]);
                }
                buffer->flags[i] = 1;
                match_found = 1;
                break;
            }
        }

        if (!match_found)
        {
            buffer_read(buffer, temp);
        }

        match_found = 0;
    }
}
