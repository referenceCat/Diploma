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

#define sW1_PORT GPIO_1
#define sW2_PORT GPIO_1
#define sW3_PORT GPIO_1
#define sW4_PORT GPIO_1
#define sW5_PORT GPIO_2
#define sW6_PORT GPIO_2
#define sW7_PORT GPIO_1
#define sW8_PORT GPIO_1

#define sW1_PIN 2
#define sW2_PIN 0
#define sW3_PIN 1
#define sW4_PIN 4
#define sW5_PIN 6
#define sW6_PIN 7
#define sW7_PIN 3
#define sW8_PIN 6

#define MIK32V2

#define BTN0_BIT

// must be called at 1 kHz
void update();

// must be called once
void init();

uint32_t getLeversBitmask();

uint32_t getButtonsBitmask();

uint16_t getPotValueMv(uint8_t pot);

enum ButtonIndexes {
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

const uint32_t buttonsMap[] = {
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

uint8_t getButtonState(enum ButtonIndexes button);

static uint32_t updateCounter		= 0;
static uint32_t buttonsStateBitmask	= 0; // b0, ... b9, bA, bB, bEnc
static uint32_t leversStateBitmask	= 0;
static uint32_t potValueLeft		= 0;
static uint32_t potValueRight		= 0;

static ADC_HandleTypeDef hadc;
static SPI_HandleTypeDef hspi0;

// https://habr.com/ru/articles/836796/
static void pollPotentiometers() {
	/* Получение значения с разных каналов */
	        ADC_SEL_CHANNEL(hadc.Instance, 0);
	        HAL_ADC_SINGLE(hadc.Instance); // Первое измерение для переключение на канал 0.
	        HAL_ADC_WaitValid(&hadc);

	        HAL_ADC_SINGLE_AND_SET_CH(hadc.Instance, 2); // P1.7 ch1
	        potValueLeft = HAL_ADC_WaitAndGetValue(&hadc); /* Ожидание и чтение актуальных данных (режим одиночного преобразования) */

	        HAL_ADC_SINGLE_AND_SET_CH(hadc.Instance, 1); // P0.2 ch2
	        potValueRight = HAL_ADC_WaitAndGetValue(&hadc); /* Ожидание и чтение актуальных данных (режим одиночного преобразования) */
}

uint8_t getButtonState(enum ButtonIndexes button) {
	return (buttonsStateBitmask & (1 << button)) ? 1 : 0;
}


static void pollLevers() {
	leversStateBitmask = 0;
	leversStateBitmask |= (sW1_PORT->STATE & (1 << sW1_PIN)) << 0;
	leversStateBitmask |= (sW2_PORT->STATE & (1 << sW2_PIN)) << 1;
	leversStateBitmask |= (sW3_PORT->STATE & (1 << sW3_PIN)) << 2;
	leversStateBitmask |= (sW4_PORT->STATE & (1 << sW4_PIN)) << 3;
	leversStateBitmask |= (sW5_PORT->STATE & (1 << sW5_PIN)) << 4;
	leversStateBitmask |= (sW6_PORT->STATE & (1 << sW6_PIN)) << 5;
	leversStateBitmask |= (sW7_PORT->STATE & (1 << sW7_PIN)) << 6;
	leversStateBitmask |= (sW8_PORT->STATE & (1 << sW8_PIN)) << 7;
}

// https://habr.com/ru/articles/832228/
static void pollButtons() {
	uint16_t data = 0;

    /* Начало передачи в ручном режиме управления CS */
    if (hspi0.Init.ManualCS == SPI_MANUALCS_ON)
    {
        __HAL_SPI_ENABLE(&hspi0);
        HAL_SPI_CS_Enable(&hspi0, SPI_CS_0);
    }

    /* Передача и прием данных */
    HAL_StatusTypeDef SPI_Status = HAL_SPI_Exchange(&hspi0, (uint8_t*)&data, (uint8_t*)&data, 2, SPI_TIMEOUT_DEFAULT); // sending data doesnt do anything
    if (SPI_Status != HAL_OK) {
    	// ignore error
        HAL_SPI_ClearError(&hspi0);
    }

    /* Конец передачи в ручном режиме управления CS */
    if (hspi0.Init.ManualCS == SPI_MANUALCS_ON)
    {
        HAL_SPI_CS_Disable(&hspi0);
        __HAL_SPI_DISABLE(&hspi0);
    }

    buttonsStateBitmask = 0;
    buttonsStateBitmask |= (((data >> buttonsMap[B_0]) & 1) << B_0);
    buttonsStateBitmask |= (((data >> buttonsMap[B_1]) & 1) << B_1);
    buttonsStateBitmask |= (((data >> buttonsMap[B_2]) & 1) << B_2);
    buttonsStateBitmask |= (((data >> buttonsMap[B_3]) & 1) << B_3);
    buttonsStateBitmask |= (((data >> buttonsMap[B_4]) & 1) << B_4);
    buttonsStateBitmask |= (((data >> buttonsMap[B_5]) & 1) << B_5);
    buttonsStateBitmask |= (((data >> buttonsMap[B_6]) & 1) << B_6);
    buttonsStateBitmask |= (((data >> buttonsMap[B_7]) & 1) << B_7);
    buttonsStateBitmask |= (((data >> buttonsMap[B_8]) & 1) << B_8);
    buttonsStateBitmask |= (((data >> buttonsMap[B_9]) & 1) << B_9);
    buttonsStateBitmask |= (((data >> buttonsMap[B_A]) & 1) << B_A);
    buttonsStateBitmask |= (((data >> buttonsMap[B_B]) & 1) << B_B);
    buttonsStateBitmask |= (((data >> buttonsMap[B_ENC]) & 1) << B_ENC);
}

void update() {
	updateCounter++;

	// 100 Hz
	if (updateCounter % 10 == 0) {
		pollButtons();
	}

	// 33 Hz
	if (updateCounter % 30 == 0) {
		pollLevers();
	}

	if (updateCounter % 10 == 0) {
		pollPotentiometers();
	}
}

void init() {
	// TODO pins should not be hardcoded but passed as config struct
	// TODO lever pin are swapped on schematics (1 to 8 instead of 8 to 1)
	sW1_PORT->DIRECTION_IN |= 1 << sW1_PIN;
	sW2_PORT->DIRECTION_IN |= 1 << sW2_PIN;
	sW3_PORT->DIRECTION_IN |= 1 << sW3_PIN;
	sW4_PORT->DIRECTION_IN |= 1 << sW4_PIN;
	sW5_PORT->DIRECTION_IN |= 1 << sW5_PIN;
	sW6_PORT->DIRECTION_IN |= 1 << sW6_PIN;
	sW7_PORT->DIRECTION_IN |= 1 << sW7_PIN;
	sW8_PORT->DIRECTION_IN |= 1 << sW8_PIN;

	PAD_CONFIG->PORT_1_PUPD |= 1 << (sW1_PIN * 2 + 1);
	PAD_CONFIG->PORT_1_PUPD |= 1 << (sW2_PIN * 2 + 1);
	PAD_CONFIG->PORT_1_PUPD |= 1 << (sW3_PIN * 2 + 1);
	PAD_CONFIG->PORT_1_PUPD |= 1 << (sW4_PIN * 2 + 1);
	PAD_CONFIG->PORT_2_PUPD |= 1 << (sW5_PIN * 2 + 1);
	PAD_CONFIG->PORT_2_PUPD |= 1 << (sW6_PIN * 2 + 1);
	PAD_CONFIG->PORT_1_PUPD |= 1 << (sW7_PIN * 2 + 1);
	PAD_CONFIG->PORT_1_PUPD |= 1 << (sW8_PIN * 2 + 1);

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
}

uint32_t getLeversBitmask() {
	return leversStateBitmask;
}

enum Pot {
	Left = 0,
	Right = 1
};

uint16_t getPotValueMv(uint8_t pot) {
	return pot ? (potValueRight * 1300 / 4096) : (potValueLeft * 1300 / 4096);
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
