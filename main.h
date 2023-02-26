#ifndef __MAIN_H
#define __MAIN_H

#define UART_ID uart0
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

typedef enum
{
    LED_NONE,
    LED_RED,
    LED_GREEN,
    LED_BLUE, 
}LED_Color_te;

#endif /* __MAIN_H */