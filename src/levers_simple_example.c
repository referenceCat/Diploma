/*
 * levers_simple_example.c
 * Reads eight lever switches through GPIO inputs with pull-up configuration
 * and prints their state bitmask over UART.
 */

#include "mcu32_memory_map.h"
#include <power_manager.h>
#include "pad_config.h"
#include <gpio.h>

#include "uart_lib.h"
#include "xprintf.h"

#define SW1_PORT GPIO_1
#define SW2_PORT GPIO_1
#define SW3_PORT GPIO_1
#define SW4_PORT GPIO_1
#define SW5_PORT GPIO_2
#define SW6_PORT GPIO_2
#define SW7_PORT GPIO_1
#define SW8_PORT GPIO_1

#define SW1_PIN 2
#define SW2_PIN 0
#define SW3_PIN 1
#define SW4_PIN 4
#define SW5_PIN 6
#define SW6_PIN 7
#define SW7_PIN 3
#define SW8_PIN 6

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

static void levers_init(void)
{
    SW1_PORT->DIRECTION_IN |= 1 << SW1_PIN;
    SW2_PORT->DIRECTION_IN |= 1 << SW2_PIN;
    SW3_PORT->DIRECTION_IN |= 1 << SW3_PIN;
    SW4_PORT->DIRECTION_IN |= 1 << SW4_PIN;
    SW5_PORT->DIRECTION_IN |= 1 << SW5_PIN;
    SW6_PORT->DIRECTION_IN |= 1 << SW6_PIN;
    SW7_PORT->DIRECTION_IN |= 1 << SW7_PIN;
    SW8_PORT->DIRECTION_IN |= 1 << SW8_PIN;

    PAD_CONFIG->PORT_1_PUPD |= 1 << (SW1_PIN * 2 + 1);
    PAD_CONFIG->PORT_1_PUPD |= 1 << (SW2_PIN * 2 + 1);
    PAD_CONFIG->PORT_1_PUPD |= 1 << (SW3_PIN * 2 + 1);
    PAD_CONFIG->PORT_1_PUPD |= 1 << (SW4_PIN * 2 + 1);
    PAD_CONFIG->PORT_2_PUPD |= 1 << (SW5_PIN * 2 + 1);
    PAD_CONFIG->PORT_2_PUPD |= 1 << (SW6_PIN * 2 + 1);
    PAD_CONFIG->PORT_1_PUPD |= 1 << (SW7_PIN * 2 + 1);
    PAD_CONFIG->PORT_1_PUPD |= 1 << (SW8_PIN * 2 + 1);
}

static uint8_t levers_read(void)
{
    uint8_t state = 0;

    state |= ((SW1_PORT->STATE >> SW1_PIN) & 1) << 0;
    state |= ((SW2_PORT->STATE >> SW2_PIN) & 1) << 1;
    state |= ((SW3_PORT->STATE >> SW3_PIN) & 1) << 2;
    state |= ((SW4_PORT->STATE >> SW4_PIN) & 1) << 3;
    state |= ((SW5_PORT->STATE >> SW5_PIN) & 1) << 4;
    state |= ((SW6_PORT->STATE >> SW6_PIN) & 1) << 5;
    state |= ((SW7_PORT->STATE >> SW7_PIN) & 1) << 6;
    state |= ((SW8_PORT->STATE >> SW8_PIN) & 1) << 7;

    return state;
}

void main(void)
{
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_GPIO_0_M
                      | PM_CLOCK_APB_P_GPIO_1_M
                      | PM_CLOCK_APB_P_GPIO_2_M;
    PM->CLK_APB_M_SET = PM_CLOCK_APB_M_PAD_CONFIG_M;

    UART_Init(UART_1, (32000000 / 57600),
              UART_CONTROL1_TE_M | UART_CONTROL1_M_8BIT_M, 0, 0);

    levers_init();

    while (1) {
        uint8_t levers = levers_read();

        xprintf("levers: %d%d%d%d%d%d%d%d\n\r",
                (levers >> 0) & 1,
                (levers >> 1) & 1,
                (levers >> 2) & 1,
                (levers >> 3) & 1,
                (levers >> 4) & 1,
                (levers >> 5) & 1,
                (levers >> 6) & 1,
                (levers >> 7) & 1);

        delay_ms(100);
    }
}
