/*
 * keyboard_bitbang_example.c
 * Reads the button keyboard through cascaded 74HC165 shift registers using
 * manually toggled GPIO lines instead of the SPI peripheral.
 */

#include "mcu32_memory_map.h"
// Example based on the working project code, but this file was not tested separately.
#include <power_manager.h>
#include "pad_config.h"
#include <gpio.h>

#include "uart_lib.h"
#include "xprintf.h"

#define HC165_DATA_PORT GPIO_1
#define HC165_CLK_PORT  GPIO_1
#define HC165_LOAD_PORT GPIO_1

#define HC165_DATA_PIN 0
#define HC165_CLK_PIN  2
#define HC165_LOAD_PIN 6

static const uint8_t button_map[13] = {
    10, // 0
    7,  // 1
    6,  // 2
    5,  // 3
    3,  // 4
    2,  // 5
    1,  // 6
    15, // 7
    0,  // 8
    14, // 9
    11, // A
    9,  // B
    8   // encoder button
};

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

static void delay_us(uint32_t us)
{
    while (us--) {
        for (volatile uint32_t i = 0; i < 20; i++);
    }
}

static void pin_high(GPIO_TypeDef *port, uint32_t pin)
{
    port->OUTPUT |= 1 << pin;
}

static void pin_low(GPIO_TypeDef *port, uint32_t pin)
{
    port->OUTPUT &= ~(1 << pin);
}

static uint8_t pin_read(GPIO_TypeDef *port, uint32_t pin)
{
    return (port->STATE >> pin) & 1;
}

static void keyboard_init(void)
{
    HC165_DATA_PORT->DIRECTION_IN |= 1 << HC165_DATA_PIN;
    HC165_CLK_PORT->DIRECTION_OUT |= 1 << HC165_CLK_PIN;
    HC165_LOAD_PORT->DIRECTION_OUT |= 1 << HC165_LOAD_PIN;

    pin_low(HC165_CLK_PORT, HC165_CLK_PIN);
    pin_high(HC165_LOAD_PORT, HC165_LOAD_PIN);
}

static uint16_t keyboard_read_raw(void)
{
    uint16_t data = 0;

    pin_low(HC165_LOAD_PORT, HC165_LOAD_PIN);
    delay_us(1);
    pin_high(HC165_LOAD_PORT, HC165_LOAD_PIN);
    delay_us(1);

    for (uint8_t i = 0; i < 16; i++) {
        data |= pin_read(HC165_DATA_PORT, HC165_DATA_PIN) << i;

        pin_high(HC165_CLK_PORT, HC165_CLK_PIN);
        delay_us(1);
        pin_low(HC165_CLK_PORT, HC165_CLK_PIN);
        delay_us(1);
    }

    return data;
}

static uint16_t keyboard_read_buttons(void)
{
    uint16_t raw = keyboard_read_raw();
    uint16_t buttons = 0;

    for (uint8_t i = 0; i < 13; i++) {
        buttons |= ((raw >> button_map[i]) & 1) << i;
    }

    return buttons;
}

void main(void)
{
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_GPIO_0_M
                      | PM_CLOCK_APB_P_GPIO_1_M
                      | PM_CLOCK_APB_P_GPIO_2_M;
    PM->CLK_APB_M_SET = PM_CLOCK_APB_M_PAD_CONFIG_M;

    UART_Init(UART_1, (32000000 / 57600),
              UART_CONTROL1_TE_M | UART_CONTROL1_M_8BIT_M, 0, 0);

    keyboard_init();

    while (1) {
        uint16_t buttons = keyboard_read_buttons();

        xprintf("keyboard: 0:%d 1:%d 2:%d 3:%d 4:%d 5:%d 6:%d 7:%d 8:%d 9:%d A:%d B:%d ENC:%d\n\r",
                (buttons >> 0) & 1,
                (buttons >> 1) & 1,
                (buttons >> 2) & 1,
                (buttons >> 3) & 1,
                (buttons >> 4) & 1,
                (buttons >> 5) & 1,
                (buttons >> 6) & 1,
                (buttons >> 7) & 1,
                (buttons >> 8) & 1,
                (buttons >> 9) & 1,
                (buttons >> 10) & 1,
                (buttons >> 11) & 1,
                (buttons >> 12) & 1);

        delay_ms(100);
    }
}
