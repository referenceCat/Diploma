#include "mcu32_memory_map.h"
#include <power_manager.h>

#include "mik32_hal_adc.h"
#include "mik32_hal_gpio.h"

#include "uart_lib.h"
#include "xprintf.h"

#define POT_LEFT_CHANNEL  1
#define POT_RIGHT_CHANNEL 2

#define ADC_MAX_VALUE 4095
#define ADC_REF_MV    1300

static ADC_HandleTypeDef hadc;

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

static void adc_init(void)
{
    hadc.Instance = ANALOG_REG;
    hadc.Init.Sel = ADC_CHANNEL0;
    hadc.Init.EXTRef = ADC_EXTREF_OFF;
    hadc.Init.EXTClb = ADC_EXTCLB_ADCREF;
    HAL_ADC_Init(&hadc);
}

static uint16_t adc_read_channel(uint8_t channel)
{
    ADC_SEL_CHANNEL(hadc.Instance, channel);

    HAL_ADC_SINGLE(hadc.Instance);
    HAL_ADC_WaitValid(&hadc);

    HAL_ADC_SINGLE(hadc.Instance);
    return HAL_ADC_WaitAndGetValue(&hadc);
}

static uint16_t filter_adc(uint16_t previous, uint16_t current)
{
    return (previous * 7 + current) / 8;
}

static uint16_t adc_to_mv(uint16_t value)
{
    return (uint32_t)value * ADC_REF_MV / (ADC_MAX_VALUE + 1);
}

static uint8_t adc_to_percent(uint16_t value)
{
    return (uint32_t)value * 100 / ADC_MAX_VALUE;
}

void main(void)
{
    uint16_t left_filtered = 0;
    uint16_t right_filtered = 0;

    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_GPIO_0_M
                      | PM_CLOCK_APB_P_GPIO_1_M;

    UART_Init(UART_1, (32000000 / 57600),
              UART_CONTROL1_TE_M | UART_CONTROL1_M_8BIT_M, 0, 0);

    adc_init();

    left_filtered = adc_read_channel(POT_LEFT_CHANNEL);
    right_filtered = adc_read_channel(POT_RIGHT_CHANNEL);

    while (1) {
        uint16_t left_raw = adc_read_channel(POT_LEFT_CHANNEL);
        uint16_t right_raw = adc_read_channel(POT_RIGHT_CHANNEL);

        left_filtered = filter_adc(left_filtered, left_raw);
        right_filtered = filter_adc(right_filtered, right_raw);

        xprintf("left: raw=%d, %d mV, %d%%; right: raw=%d, %d mV, %d%%\n\r",
                left_filtered,
                adc_to_mv(left_filtered),
                adc_to_percent(left_filtered),
                right_filtered,
                adc_to_mv(right_filtered),
                adc_to_percent(right_filtered));

        delay_ms(50);
    }
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_PCC_ANALOG_REGS_CLK_ENABLE();

    GPIO_InitStruct.Mode = HAL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = HAL_GPIO_PULL_NONE;

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIO_1, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIO_0, &GPIO_InitStruct);
}
