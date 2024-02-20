#include "main.h"
#include "string.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/time.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pin_def.h"
#include "ascii_lut.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

#define GET_MS us_to_ms(get_absolute_time()._private_us_since_boot)

static void led_set_array(uint8_t arr, LED_Color_te color);
static uint8_t reverse(uint8_t b);
static int leftShift(char *words, int len);

static uint8_t uart_rx_buffer[50] = "MEKAN";
static uint8_t screenStr[50];
static LED_Color_te colorLED = LED_RED;
static uint32_t last_received_ms;
static uint16_t waitTimeUs = 275;

static uint64_t flowing_time;
static bool flowing_enable = false;

// RX interrupt handler
void on_uart_rx()
{
    while (uart_is_readable(UART_ID))
    {
        static int chars_rxed = 0;

        if ((1000 < GET_MS - last_received_ms) && (last_received_ms != 0))
        {
            memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));
            chars_rxed = 0;
        }

        uart_rx_buffer[chars_rxed++] = uart_getc(UART_ID);

        last_received_ms = GET_MS;
    }
}

int main()
{
    uart_init(UART_ID, BAUD_RATE);
    uart_set_translate_crlf(UART_ID, false);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    //  Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);
    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    irq_set_priority(UART_IRQ, 0x10);
    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    gpio_init(HAL_INPUT_PIN);
    gpio_set_dir(HAL_INPUT_PIN, GPIO_IN);
    gpio_pull_up(HAL_INPUT_PIN);

    gpio_init(LED_1_R);
    gpio_init(LED_1_G);
    gpio_init(LED_1_B);

    gpio_init(LED_2_R);
    gpio_init(LED_2_G);
    gpio_init(LED_2_B);

    gpio_init(LED_3_R);
    gpio_init(LED_3_G);
    gpio_init(LED_3_B);

    gpio_init(LED_4_R);
    gpio_init(LED_4_G);
    gpio_init(LED_4_B);

    gpio_init(LED_5_R);
    gpio_init(LED_5_G);
    gpio_init(LED_5_B);

    gpio_init(LED_6_R);
    gpio_init(LED_6_G);
    gpio_init(LED_6_B);

    gpio_init(LED_7_R);
    gpio_init(LED_7_G);
    gpio_init(LED_7_B);

    gpio_init(LED_8_R);
    gpio_init(LED_8_G);
    gpio_init(LED_8_B);

    gpio_set_dir(LED_1_R, GPIO_OUT);
    gpio_set_dir(LED_1_G, GPIO_OUT);
    gpio_set_dir(LED_1_B, GPIO_OUT);

    gpio_set_dir(LED_2_R, GPIO_OUT);
    gpio_set_dir(LED_2_G, GPIO_OUT);
    gpio_set_dir(LED_2_B, GPIO_OUT);

    gpio_set_dir(LED_3_R, GPIO_OUT);
    gpio_set_dir(LED_3_G, GPIO_OUT);
    gpio_set_dir(LED_3_B, GPIO_OUT);

    gpio_set_dir(LED_4_R, GPIO_OUT);
    gpio_set_dir(LED_4_G, GPIO_OUT);
    gpio_set_dir(LED_4_B, GPIO_OUT);

    gpio_set_dir(LED_5_R, GPIO_OUT);
    gpio_set_dir(LED_5_G, GPIO_OUT);
    gpio_set_dir(LED_5_B, GPIO_OUT);

    gpio_set_dir(LED_6_R, GPIO_OUT);
    gpio_set_dir(LED_6_G, GPIO_OUT);
    gpio_set_dir(LED_6_B, GPIO_OUT);

    gpio_set_dir(LED_7_R, GPIO_OUT);
    gpio_set_dir(LED_7_G, GPIO_OUT);
    gpio_set_dir(LED_7_B, GPIO_OUT);

    gpio_set_dir(LED_8_R, GPIO_OUT);
    gpio_set_dir(LED_8_G, GPIO_OUT);
    gpio_set_dir(LED_8_B, GPIO_OUT);

    sleep_ms(1000);

    strcpy(uart_rx_buffer, "MEKAN");

    while (true)
    {
        if ((100 < GET_MS - last_received_ms))
        {
            if (strstr(uart_rx_buffer, "LED="))
            {
                colorLED = (LED_Color_te)atoi(&uart_rx_buffer[4]);
                memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));
            }
            else if (strstr(uart_rx_buffer, "KAYMA=") != NULL)
            {
                flowing_enable = (bool)atoi(&uart_rx_buffer[6]);
                memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));
            }
            else if (strstr(uart_rx_buffer, "HARF GENISLIK=") != NULL)
            {
                waitTimeUs = atoi(&uart_rx_buffer[14]);
                memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));
            }

            if (!gpio_get(HAL_INPUT_PIN))
            {
                if (uart_rx_buffer[0] != 0)
                {
                    memset(screenStr, 0, sizeof(screenStr));
                    memcpy(screenStr, uart_rx_buffer, sizeof(uart_rx_buffer));
                    memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));
                }

                for (uint8_t char_index = 0; char_index < strlen(screenStr); char_index++)
                {
                    for (uint32_t char_len = 0; char_len < CHR_LEN; char_len++)
                    {
                        led_set_array(reverse(ascii_lut[(screenStr[char_index])][char_len]), colorLED);
                        sleep_us(waitTimeUs);
                    }
                }
            }
            else
            {
                led_set_array(0x00, LED_ALL);
            }

            if (flowing_enable == true && GET_MS - flowing_time >= 1000)
            {
                flowing_time = GET_MS;
                if (screenStr != NULL)
                {
                    leftShift(screenStr, strlen(screenStr));
                }
            }
        }
    }
}

void led_set_array(uint8_t arr, LED_Color_te color)
{
    uint led1[LED_COLOR_CNT] = {0};
    uint led2[LED_COLOR_CNT] = {0};
    uint led3[LED_COLOR_CNT] = {0};
    uint led4[LED_COLOR_CNT] = {0};
    uint led5[LED_COLOR_CNT] = {0};
    uint led6[LED_COLOR_CNT] = {0};
    uint led7[LED_COLOR_CNT] = {0};
    uint led8[LED_COLOR_CNT] = {0};

    switch (color)
    {
    case LED_RED:
        led1[RED_INDEX] = LED_1_R;
        led2[RED_INDEX] = LED_2_R;
        led3[RED_INDEX] = LED_3_R;
        led4[RED_INDEX] = LED_4_R;
        led5[RED_INDEX] = LED_5_R;
        led6[RED_INDEX] = LED_6_R;
        led7[RED_INDEX] = LED_7_R;
        led8[RED_INDEX] = LED_8_R;
        break;
    case LED_GREEN:
        led1[GREEN_INDEX] = LED_1_G;
        led2[GREEN_INDEX] = LED_2_G;
        led3[GREEN_INDEX] = LED_3_G;
        led4[GREEN_INDEX] = LED_4_G;
        led5[GREEN_INDEX] = LED_5_G;
        led6[GREEN_INDEX] = LED_6_G;
        led7[GREEN_INDEX] = LED_7_G;
        led8[GREEN_INDEX] = LED_8_G;
        break;
    case LED_BLUE:
        led1[BLUE_INDEX] = LED_1_B;
        led2[BLUE_INDEX] = LED_2_B;
        led3[BLUE_INDEX] = LED_3_B;
        led4[BLUE_INDEX] = LED_4_B;
        led5[BLUE_INDEX] = LED_5_B;
        led6[BLUE_INDEX] = LED_6_B;
        led7[BLUE_INDEX] = LED_7_B;
        led8[BLUE_INDEX] = LED_8_B;
        break;

    case LED_YELLOW:
        led1[RED_INDEX] = LED_1_R;
        led2[RED_INDEX] = LED_2_R;
        led3[RED_INDEX] = LED_3_R;
        led4[RED_INDEX] = LED_4_R;
        led5[RED_INDEX] = LED_5_R;
        led6[RED_INDEX] = LED_6_R;
        led7[RED_INDEX] = LED_7_R;
        led8[RED_INDEX] = LED_8_R;
        led1[GREEN_INDEX] = LED_1_G;
        led2[GREEN_INDEX] = LED_2_G;
        led3[GREEN_INDEX] = LED_3_G;
        led4[GREEN_INDEX] = LED_4_G;
        led5[GREEN_INDEX] = LED_5_G;
        led6[GREEN_INDEX] = LED_6_G;
        led7[GREEN_INDEX] = LED_7_G;
        led8[GREEN_INDEX] = LED_8_G;
        break;

    case LED_PINK:
        led1[RED_INDEX] = LED_1_R;
        led2[RED_INDEX] = LED_2_R;
        led3[RED_INDEX] = LED_3_R;
        led4[RED_INDEX] = LED_4_R;
        led5[RED_INDEX] = LED_5_R;
        led6[RED_INDEX] = LED_6_R;
        led7[RED_INDEX] = LED_7_R;
        led8[RED_INDEX] = LED_8_R;
        led1[BLUE_INDEX] = LED_1_B;
        led2[BLUE_INDEX] = LED_2_B;
        led3[BLUE_INDEX] = LED_3_B;
        led4[BLUE_INDEX] = LED_4_B;
        led5[BLUE_INDEX] = LED_5_B;
        led6[BLUE_INDEX] = LED_6_B;
        led7[BLUE_INDEX] = LED_7_B;
        led8[BLUE_INDEX] = LED_8_B;
        break;

    case LED_TURQUOISE:
        led1[BLUE_INDEX] = LED_1_B;
        led2[BLUE_INDEX] = LED_2_B;
        led3[BLUE_INDEX] = LED_3_B;
        led4[BLUE_INDEX] = LED_4_B;
        led5[BLUE_INDEX] = LED_5_B;
        led6[BLUE_INDEX] = LED_6_B;
        led7[BLUE_INDEX] = LED_7_B;
        led8[BLUE_INDEX] = LED_8_B;
        led1[GREEN_INDEX] = LED_1_G;
        led2[GREEN_INDEX] = LED_2_G;
        led3[GREEN_INDEX] = LED_3_G;
        led4[GREEN_INDEX] = LED_4_G;
        led5[GREEN_INDEX] = LED_5_G;
        led6[GREEN_INDEX] = LED_6_G;
        led7[GREEN_INDEX] = LED_7_G;
        led8[GREEN_INDEX] = LED_8_G;
        break;

    case LED_WHITE:
        led1[RED_INDEX] = LED_1_R;
        led2[RED_INDEX] = LED_2_R;
        led3[RED_INDEX] = LED_3_R;
        led4[RED_INDEX] = LED_4_R;
        led5[RED_INDEX] = LED_5_R;
        led6[RED_INDEX] = LED_6_R;
        led7[RED_INDEX] = LED_7_R;
        led8[RED_INDEX] = LED_8_R;
        led1[GREEN_INDEX] = LED_1_G;
        led2[GREEN_INDEX] = LED_2_G;
        led3[GREEN_INDEX] = LED_3_G;
        led4[GREEN_INDEX] = LED_4_G;
        led5[GREEN_INDEX] = LED_5_G;
        led6[GREEN_INDEX] = LED_6_G;
        led7[GREEN_INDEX] = LED_7_G;
        led8[GREEN_INDEX] = LED_8_G;
        led1[BLUE_INDEX] = LED_1_B;
        led2[BLUE_INDEX] = LED_2_B;
        led3[BLUE_INDEX] = LED_3_B;
        led4[BLUE_INDEX] = LED_4_B;
        led5[BLUE_INDEX] = LED_5_B;
        led6[BLUE_INDEX] = LED_6_B;
        led7[BLUE_INDEX] = LED_7_B;
        led8[BLUE_INDEX] = LED_8_B;

        break;
    }

    if (arr & 0x01)
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led1[i], (led1[i] != 0 ? 1 : 0));
        }
    }
    else
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led1[i], 0);
        }
    }

    if (arr & 0x02)
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led2[i], (led1[i] != 0 ? 1 : 0));
        }
    }
    else
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led2[i], 0);
        }
    }

    if (arr & 0x04)
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led3[i], (led1[i] != 0 ? 1 : 0));
        }
    }
    else
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led3[i], 0);
        }
    }

    if (arr & 0x08)
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led4[i], (led1[i] != 0 ? 1 : 0));
        }
    }
    else
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led4[i], 0);
        }
    }

    if (arr & 0x10)
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led5[i], (led1[i] != 0 ? 1 : 0));
        }
    }
    else
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led5[i], 0);
        }
    }

    if (arr & 0x20)
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led6[i], (led1[i] != 0 ? 1 : 0));
        }
    }
    else
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led6[i], 0);
        }
    }

    if (arr & 0x40)
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led7[i], (led1[i] != 0 ? 1 : 0));
        }
    }
    else
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led7[i], 0);
        }
    }

    if (arr & 0x80)
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led8[i], (led1[i] != 0 ? 1 : 0));
        }
    }
    else
    {
        for (uint8_t i = 0; i < LED_COLOR_CNT; i++)
        {
            gpio_put(led8[i], 0);
        }
    }
}

uint8_t reverse(uint8_t b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

static int leftShift(char *const words, int len)
{
    if (len == 0)
        return -1;

    int i;
    for (i = 1; i < len; i++)
    {
        words[i - 1] = words[i];
    }
    words[len - 1] = 0;
    len--;
    return len;
}