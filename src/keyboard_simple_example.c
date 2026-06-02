/*
 * keyboard_simple_example.c
 * Reads the button keyboard through cascaded 74HC165 shift registers using
 * SPI1 and prints the decoded button bitmask over UART.
 */

#include "mcu32_memory_map.h"
// Example based on the working project code, but this file was not tested separately.
#include <power_manager.h>
#include "mik32_hal_spi.h"

#include "uart_lib.h"
#include "xprintf.h"

static SPI_HandleTypeDef hspi;

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

static void keyboard_init(void)
{
    hspi.Instance = SPI_1;
    hspi.Init.SPI_Mode = HAL_SPI_MODE_MASTER;
    hspi.Init.CLKPhase = SPI_PHASE_ON;
    hspi.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi.Init.ThresholdTX = 4;
    hspi.Init.BaudRateDiv = SPI_BAUDRATE_DIV256;
    hspi.Init.Decoder = SPI_DECODER_NONE;
    hspi.Init.ManualCS = SPI_MANUALCS_ON;
    hspi.Init.ChipSelect = SPI_CS_2;

    if (HAL_SPI_Init(&hspi) != HAL_OK) {
        while (1);
    }
}

static uint16_t keyboard_read_raw(void)
{
    uint16_t data = 0;

    HAL_SPI_Enable(&hspi);
    HAL_SPI_CS_Disable(&hspi);

    if (HAL_SPI_Exchange(&hspi, (uint8_t *)&data, (uint8_t *)&data,
                         2, SPI_TIMEOUT_DEFAULT) != HAL_OK) {
        while (1);
    }

    HAL_SPI_CS_Enable(&hspi, SPI_CS_2);
    HAL_SPI_Disable(&hspi);

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
