/*
 * user_input.h
 *
 *  Created on: 30 april 2026 г.
 *      Author: Buyanov Mikhail
 */

#ifndef USER_INPUT_H_
#define USER_INPUT_H_

#include <stdint.h>
#include "mik32_hal_adc.h"
#include "mik32_hal_gpio.h"
#include "mik32_hal_spi.h"

#define LI_sW1_PORT GPIO_1
#define LI_sW2_PORT GPIO_1
#define LI_sW3_PORT GPIO_1
#define LI_sW4_PORT GPIO_1
#define LI_sW5_PORT GPIO_2
#define LI_sW6_PORT GPIO_2
#define LI_sW7_PORT GPIO_1
#define LI_sW8_PORT GPIO_1

#define LI_sW1_PIN 2
#define LI_sW2_PIN 0
#define LI_sW3_PIN 1
#define LI_sW4_PIN 4
#define LI_sW5_PIN 6
#define LI_sW6_PIN 7
#define LI_sW7_PIN 3
#define LI_sW8_PIN 6

#define LI_ENC0_PORT GPIO_0
#define LI_ENC1_PORT GPIO_0

#define LI_ENC0_PIN 1
#define LI_ENC1_PIN 3

#define MIK32V2

#define LI_BTN0_BIT

// must be called at 1 kHz
void li_update();

// must be called once
void li_init();

enum LI_ButtonIndexes {
	B_0,
	B_1,
	B_2,
	B_3,
	B_4,
	B_5,
	B_6,
	B_7,
	B_8,
	B_9,
	B_A,
	B_B,
	B_ENC
};

static const uint32_t li_buttons_map[] = {
		9, // B_0
		6, // B_1
		5,
		4,
		2,
		1,
		0,
		8 + 6,
		8 + 7,
		8 + 5,
		8 + 0, // B_A
		8 + 2, // B_B
		8 + 4  // B_ENC TODO value is likely wrong
};

uint8_t li_get_button_state(enum LI_ButtonIndexes button);

static uint32_t li_update_counter			= 0;
static uint32_t li_buttons_state_bitmask	= 0; // b0, ... b9, bA, bB, bEnc
static uint32_t li_levers_state_bitmask		= 0;
static uint32_t li_pot_value_left			= 0;
static uint32_t li_pot_value_right			= 0;

static int32_t li_enc_counter				= 0;
static ADC_HandleTypeDef hadc;
static SPI_HandleTypeDef hspi0;

static void li_poll_enc() {
	static uint8_t p0 = 0, p1 = 0; // encoder last state
	uint8_t e0 = (LI_ENC0_PORT->STATE >> LI_ENC0_PIN) & 1;
	uint8_t e1 = (LI_ENC1_PORT->STATE >> LI_ENC1_PIN) & 1;

	if (p0 ^ p1 ^ e0 ^ e1) {
		(p1 ^ e0) ? li_enc_counter++ : li_enc_counter--;
		p0 = e0;
		p1 = e1;
	}

}

// https://habr.com/ru/articles/836796/
static void li_poll_potentiometers() {
	/* Получение значения с разных каналов */
	        ADC_SEL_CHANNEL(hadc.Instance, 1); // set ch1 (P1.7)
	        HAL_ADC_SINGLE(hadc.Instance); // Первое измерение для переключение на канал 0.
	        HAL_ADC_WaitValid(&hadc);

	        HAL_ADC_SINGLE_AND_SET_CH(hadc.Instance, 2); // read ch1 (P1.7), then set channel to 2
	        li_pot_value_left = HAL_ADC_WaitAndGetValue(&hadc);

	        HAL_ADC_SINGLE(hadc.Instance); // read ch2 (P0.2)
	        li_pot_value_right = HAL_ADC_WaitAndGetValue(&hadc);
}

uint8_t li_get_button_state(enum LI_ButtonIndexes button) {
	return (li_buttons_state_bitmask & (1 << button)) ? 1 : 0;
}


static void li_poll_levers() {
	li_levers_state_bitmask = 0;
	li_levers_state_bitmask |= ((LI_sW1_PORT->STATE >> LI_sW1_PIN) & 1) << 0;
	li_levers_state_bitmask |= ((LI_sW2_PORT->STATE >> LI_sW2_PIN) & 1) << 1;
	li_levers_state_bitmask |= ((LI_sW3_PORT->STATE >> LI_sW3_PIN) & 1) << 2;
	li_levers_state_bitmask |= ((LI_sW4_PORT->STATE >> LI_sW4_PIN) & 1) << 3;
	li_levers_state_bitmask |= ((LI_sW5_PORT->STATE >> LI_sW5_PIN) & 1) << 4;
	li_levers_state_bitmask |= ((LI_sW6_PORT->STATE >> LI_sW6_PIN) & 1) << 5;
	li_levers_state_bitmask |= ((LI_sW7_PORT->STATE >> LI_sW7_PIN) & 1) << 6;
	li_levers_state_bitmask |= ((LI_sW8_PORT->STATE >> LI_sW8_PIN) & 1) << 7;
}

//// https://habr.com/ru/articles/832228/
//static void pollButtons() {
//	uint16_t data = 0;
//
//    /* Начало передачи в ручном режиме управления CS */
//    if (hspi0.Init.ManualCS == SPI_MANUALCS_ON)
//    {
//        __HAL_SPI_ENABLE(&hspi0);
//        HAL_SPI_CS_Enable(&hspi0, SPI_CS_0);
//    }
//
//    /* Передача и прием данных */
//    HAL_StatusTypeDef SPI_Status = HAL_SPI_Exchange(&hspi0, (uint8_t*)&data, (uint8_t*)&data, 2, SPI_TIMEOUT_DEFAULT); // sending data doesnt do anything
//    if (SPI_Status != HAL_OK) {
//    	// ignore error
//        HAL_SPI_ClearError(&hspi0);
//    }
//
//    /* Конец передачи в ручном режиме управления CS */
//    if (hspi0.Init.ManualCS == SPI_MANUALCS_ON)
//    {
//        HAL_SPI_CS_Disable(&hspi0);
//        __HAL_SPI_DISABLE(&hspi0);
//    }
//
//    buttonsStateBitmask = 0;
//    buttonsStateBitmask |= (((data >> buttonsMap[B_0]) & 1) << B_0);
//    buttonsStateBitmask |= (((data >> buttonsMap[B_1]) & 1) << B_1);
//    buttonsStateBitmask |= (((data >> buttonsMap[B_2]) & 1) << B_2);
//    buttonsStateBitmask |= (((data >> buttonsMap[B_3]) & 1) << B_3);
//    buttonsStateBitmask |= (((data >> buttonsMap[B_4]) & 1) << B_4);
//    buttonsStateBitmask |= (((data >> buttonsMap[B_5]) & 1) << B_5);
//    buttonsStateBitmask |= (((data >> buttonsMap[B_6]) & 1) << B_6);
//    buttonsStateBitmask |= (((data >> buttonsMap[B_7]) & 1) << B_7);
//    buttonsStateBitmask |= (((data >> buttonsMap[B_8]) & 1) << B_8);
//    buttonsStateBitmask |= (((data >> buttonsMap[B_9]) & 1) << B_9);
//    buttonsStateBitmask |= (((data >> buttonsMap[B_A]) & 1) << B_A);
//    buttonsStateBitmask |= (((data >> buttonsMap[B_B]) & 1) << B_B);
//    buttonsStateBitmask |= (((data >> buttonsMap[B_ENC]) & 1) << B_ENC);
//}

void li_update() {
	li_update_counter++;

	// 1 KHz
	li_poll_enc();

	// 100 Hz
	if (li_update_counter % 10 == 0) {
		// pollButtons();
	}

	// 33 Hz
	if (li_update_counter % 30 == 0) {
		li_poll_levers();
	}

	if (li_update_counter % 10 == 0) {
		li_poll_potentiometers();
	}
}

void li_init() {
	// TODO pins should not be hardcoded but passed as config struct
	// TODO lever pin are swapped on schematics (1 to 8 instead of 8 to 1)
	LI_sW1_PORT->DIRECTION_IN |= 1 << LI_sW1_PIN;
	LI_sW2_PORT->DIRECTION_IN |= 1 << LI_sW2_PIN;
	LI_sW3_PORT->DIRECTION_IN |= 1 << LI_sW3_PIN;
	LI_sW4_PORT->DIRECTION_IN |= 1 << LI_sW4_PIN;
	LI_sW5_PORT->DIRECTION_IN |= 1 << LI_sW5_PIN;
	LI_sW6_PORT->DIRECTION_IN |= 1 << LI_sW6_PIN;
	LI_sW7_PORT->DIRECTION_IN |= 1 << LI_sW7_PIN;
	LI_sW8_PORT->DIRECTION_IN |= 1 << LI_sW8_PIN;

	PAD_CONFIG->PORT_1_PUPD |= 1 << (LI_sW1_PIN * 2 + 1);
	PAD_CONFIG->PORT_1_PUPD |= 1 << (LI_sW2_PIN * 2 + 1);
	PAD_CONFIG->PORT_1_PUPD |= 1 << (LI_sW3_PIN * 2 + 1);
	PAD_CONFIG->PORT_1_PUPD |= 1 << (LI_sW4_PIN * 2 + 1);
	PAD_CONFIG->PORT_2_PUPD |= 1 << (LI_sW5_PIN * 2 + 1);
	PAD_CONFIG->PORT_2_PUPD |= 1 << (LI_sW6_PIN * 2 + 1);
	PAD_CONFIG->PORT_1_PUPD |= 1 << (LI_sW7_PIN * 2 + 1);
	PAD_CONFIG->PORT_1_PUPD |= 1 << (LI_sW8_PIN * 2 + 1);

	hadc.Instance = ANALOG_REG;
	hadc.Init.Sel = ADC_CHANNEL0;
	hadc.Init.EXTRef = ADC_EXTREF_OFF;    /* Выбор источника опорного напряжения: «1» - внешний; «0» - встроенный */
	hadc.Init.EXTClb = ADC_EXTCLB_ADCREF; /* Выбор источника внешнего опорного напряжения: «1» - внешний вывод; «0» - настраиваемый ОИН */
	HAL_ADC_Init(&hadc);

	hspi0.Instance = SPI_0;

	/* Режим SPI */
	hspi0.Init.SPI_Mode = HAL_SPI_MODE_MASTER;

	/* Настройки */
	hspi0.Init.CLKPhase = SPI_PHASE_ON;
	hspi0.Init.CLKPolarity = SPI_POLARITY_HIGH;
	hspi0.Init.ThresholdTX = 4;

	/* Настройки для ведущего */
	hspi0.Init.BaudRateDiv = SPI_BAUDRATE_DIV64;
	hspi0.Init.Decoder = SPI_DECODER_NONE;
	hspi0.Init.ManualCS = SPI_MANUALCS_OFF;
	hspi0.Init.ChipSelect = SPI_CS_0;

	LI_ENC0_PORT->DIRECTION_IN |= 1 << LI_ENC0_PIN;
	LI_ENC1_PORT->DIRECTION_IN |= 1 << LI_ENC1_PIN;
}

uint32_t li_get_levers_bitmask() {
	return li_levers_state_bitmask;
}

int32_t li_get_enc_counter() {
	return li_enc_counter;
}

enum LI_Pot {
	Left = 0,
	Right = 1
};

uint16_t li_get_pot_value_mv(uint8_t pot) {
	return pot ? (li_pot_value_right * 1300 / 4096) : (li_pot_value_left * 1300 / 4096);
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_PCC_ANALOG_REGS_CLK_ENABLE();

    if ((hadc->Init.EXTClb == ADC_EXTCLB_ADCREF) && (hadc->Init.EXTRef == ADC_EXTREF_ON))
    {
#ifdef MIK32V0
        GPIO_InitStruct.Pin = GPIO_PIN_10;
#endif

#ifdef MIK32V2
        GPIO_InitStruct.Pin = GPIO_PIN_11;
#endif
    }

    GPIO_InitStruct.Mode = HAL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = HAL_GPIO_PULL_NONE;
    HAL_GPIO_Init(GPIO_1, &GPIO_InitStruct);

    /* Настройка выводов АЦП */
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIO_1, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_7 | GPIO_PIN_9;
    HAL_GPIO_Init(GPIO_0, &GPIO_InitStruct);
}

#endif /* USER_INPUT_H_ */
