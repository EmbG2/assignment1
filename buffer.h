/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#ifndef BUFFER_H
#define	BUFFER_H

#include "timer.h"
#include <xc.h> 

#define MAIN_BUFFER_SIZE 16
#define SECONDARY_BUFFER_SIZE 5
#define MAX_PATTERN_COUNT 10

typedef struct
{
    char data[MAIN_BUFFER_SIZE];
    int head;
    int tail;
    int count;
    int pattern_count;
    char **patterns;   // Array of patterns to detect
    int flags[MAX_PATTERN_COUNT];
} CircularBuffer;

void buffer_init(CircularBuffer *buffer, char **patterns, int pattern_count);
int buffer_write(CircularBuffer *buffer, char value);
int buffer_read(CircularBuffer *buffer, char *value);
int buffer_peek(const CircularBuffer *buffer, int index);
void detect_pattern(CircularBuffer *buffer);

extern CircularBuffer main_buffer_1;
extern CircularBuffer main_buffer_2;

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* BUFFER_H */

