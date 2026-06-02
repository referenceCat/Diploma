/*
 * encoder_simple_example.c
 * Reads a quadrature rotary encoder directly through GPIO pins and prints
 * the accumulated position counter over UART.
 */

#include "mcu32_memory_map.h"
#include <power_manager.h>
#include <gpio.h>

#include "uart_lib.h"
#include "xprintf.h"

#define ENC_A_PORT GPIO_0
#define ENC_B_PORT GPIO_0

#define ENC_A_PIN 1
#define ENC_B_PIN 3

void xputc(char c)
{
    UART_WriteByte(UART_1, c);
    UART_WaitTransmission(UART_1);
}

static void delay_ms(uint32_t ms)
{
    const uint32_t delay_1ms = 32000000 / 1000 / 15;

    while (ms--) {
        for (volatile uint32_t i = 0; i < delay_1ms; i++);
    }
}

static void encoder_init(void)
{
    ENC_A_PORT->DIRECTION_IN |= 1 << ENC_A_PIN;
    ENC_B_PORT->DIRECTION_IN |= 1 << ENC_B_PIN;
}

static uint8_t encoder_read_state(void)
{
    uint8_t a = (ENC_A_PORT->STATE >> ENC_A_PIN) & 1;
    uint8_t b = (ENC_B_PORT->STATE >> ENC_B_PIN) & 1;

    return (a << 1) | b;
}

void main(void)
{
    int32_t counter = 0;
    uint8_t previous_state;

    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_GPIO_0_M;

    UART_Init(UART_1, (32000000 / 57600),
              UART_CONTROL1_TE_M | UART_CONTROL1_M_8BIT_M, 0, 0);

    encoder_init();
    previous_state = encoder_read_state();

    while (1) {
        uint8_t current_state = encoder_read_state();

        if (current_state != previous_state) {
            if (((previous_state == 0) && (current_state == 2)) ||
                ((previous_state == 2) && (current_state == 3)) ||
                ((previous_state == 3) && (current_state == 1)) ||
                ((previous_state == 1) && (current_state == 0))) {
                counter++;
            } else if (((previous_state == 0) && (current_state == 1)) ||
                       ((previous_state == 1) && (current_state == 3)) ||
                       ((previous_state == 3) && (current_state == 2)) ||
                       ((previous_state == 2) && (current_state == 0))) {
                counter--;
            }

            previous_state = current_state;
            xprintf("encoder: %d\n\r", counter);
        }

        delay_ms(1);
    }
}
