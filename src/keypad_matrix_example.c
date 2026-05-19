#include "mcu32_memory_map.h"
// Example based on the working project code, but this file was not tested separately.
#include <power_manager.h>
#include <gpio.h>

#include "uart_lib.h"
#include "xprintf.h"

/*
 * Matrix keypad example for the pins used in the diploma text.
 *
 * Inputs are pulled up to 3.3 V:
 *   P1.2, P1.0, P1.1, P1.4
 *
 * Outputs are high by default and are driven low one by one:
 *   P2.6, P2.7, P1.3, P1.6
 *
 * With the listed pins this is a 4x4 matrix.
 */

#define MATRIX_ROWS 4
#define MATRIX_COLS 4

static GPIO_TypeDef *const row_ports[MATRIX_ROWS] = {
    GPIO_1, GPIO_1, GPIO_1, GPIO_1
};

static const uint8_t row_pins[MATRIX_ROWS] = {
    2, 0, 1, 4
};

static GPIO_TypeDef *const col_ports[MATRIX_COLS] = {
    GPIO_2, GPIO_2, GPIO_1, GPIO_1
};

static const uint8_t col_pins[MATRIX_COLS] = {
    6, 7, 3, 6
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

static void pin_high(GPIO_TypeDef *port, uint8_t pin)
{
    port->OUTPUT |= 1 << pin;
}

static void pin_low(GPIO_TypeDef *port, uint8_t pin)
{
    port->OUTPUT &= ~(1 << pin);
}

static uint8_t pin_read(GPIO_TypeDef *port, uint8_t pin)
{
    return (port->STATE >> pin) & 1;
}

static void keypad_init(void)
{
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        row_ports[row]->DIRECTION_IN |= 1 << row_pins[row];
    }

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        col_ports[col]->DIRECTION_OUT |= 1 << col_pins[col];
        pin_high(col_ports[col], col_pins[col]);
    }
}

static uint16_t keypad_scan(void)
{
    uint16_t keys = 0;

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        pin_low(col_ports[col], col_pins[col]);
        delay_us(5);

        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            /*
             * Rows are pulled up, so pressed key reads as 0
             * when the selected column is driven low.
             */
            if (!pin_read(row_ports[row], row_pins[row])) {
                keys |= 1 << (col * MATRIX_ROWS + row);
            }
        }

        pin_high(col_ports[col], col_pins[col]);
    }

    return keys;
}

void main(void)
{
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_GPIO_0_M
                      | PM_CLOCK_APB_P_GPIO_1_M
                      | PM_CLOCK_APB_P_GPIO_2_M;
    PM->CLK_APB_M_SET = PM_CLOCK_APB_M_PAD_CONFIG_M;

    UART_Init(UART_1, (32000000 / 57600),
              UART_CONTROL1_TE_M | UART_CONTROL1_M_8BIT_M, 0, 0);

    keypad_init();

    while (1) {
        uint16_t keys = keypad_scan();

        xprintf("matrix: %04x\n\r", keys);
        delay_ms(100);
    }
}
