#ifndef __MAIN_H
#define __MAIN_H

#define UART_ID uart0
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

#define LED_COLOR_CNT       3
#define RED_INDEX           0
#define GREEN_INDEX         1
#define BLUE_INDEX          2

typedef enum
{
    LED_NONE,
    LED_WHITE = 0x01,
    LED_ALL = 0x01,
    LED_RED,
    LED_GREEN,
    LED_BLUE, 
    LED_YELLOW,
    LED_PINK,
    LED_TURQUOISE,
    
}LED_Color_te;

#endif /* __MAIN_H */