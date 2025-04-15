/* Microchip Technology Inc. and its subsidiaries... [license left untouched] */

#ifndef PARSER_H
#define	PARSER_H

#ifdef	__cplusplus
extern "C" {
#endif

// Struct for command parsing state
typedef struct {
    const char* name;
    int save_index;
    int stop;
    int activations;
} CommandState;

// Parses the received UART buffer against known command patterns
void parse_uart_commands(char *msg, int length, CommandState *commands);

#ifdef	__cplusplus
}
#endif

#endif	/* PARSER_H */