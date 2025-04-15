/** 
 * @file main.c
 * @author gianmarco
 * @date 2025-04-13
 * @brief Main function 
 */

#include "xc.h"
#include "timer.h"
#include "uart.h"

int count = 0;
struct parser_config
{
    char* cmds[8];
    int cmd_count[8];
    int save_indx_letter[8];
    int stop_check[8];
};
struct parser_config parser_det = {
    {
        "$RATE,0",
        "$RATE,1",
        "$RATE,2",
        "$RATE,4",
        "$RATE,5",
        "$RATE,10",
        "$RATE,",
        0
    }, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0}
};


void algorithm();

int main(){
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    TRISGbits.TRISG9 = 0;

    tmr_setup_period(TIMER1, 10);
    
    while(1){
        algorithm();
        //                                                                                                      CODE HERE
        count++;
        if (count >= 50){
            LATGbits.LATG9 ^= 1;
            count = 0;
        }
        //                                                                                                      STOP CODE HERE
        int ret = tmr_wait_period_3(TIMER1);
    }
    return 0;
};

void algorithm(){
    tmr_wait_ms(TIMER2, 7);
}
