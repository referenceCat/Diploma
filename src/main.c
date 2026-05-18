#include "riscv_csr_encoding.h"
#include "scr1_csr_encoding.h"
#include "mcu32_memory_map.h"
#include <power_manager.h>
#include "pad_config.h"
#include <gpio_irq.h>
#include <epic.h>
#include <csr.h>
#include <gpio.h>
#include "lab_input.h"
#include "mik32_hal_spi.h"
#include "mik32_hal_pcc.h"
#include "mik32_hal_gpio.h"
#include "mik32_hal_scr1_timer.h"

#include "uart_lib.h"
#include "xprintf.h"

void xputc(char c) // this function is weak-defined in uart_lib.c and uses uart_0
{
	UART_WriteByte(UART_1, c);
	UART_WaitTransmission(UART_1);
}


void main()
{

	const uint32_t delay_1ms = 32000000 / 1000 / 15;
	uint32_t counter = 0;

	PM->CLK_APB_P_SET =   PM_CLOCK_APB_P_GPIO_0_M
						| PM_CLOCK_APB_P_GPIO_1_M
						| PM_CLOCK_APB_P_GPIO_2_M
						| PM_CLOCK_APB_P_GPIO_IRQ_M;
	PM->CLK_APB_M_SET =   PM_CLOCK_APB_M_PAD_CONFIG_M
						| PM_CLOCK_APB_M_WU_M
						| PM_CLOCK_APB_M_PM_M
						| PM_CLOCK_APB_M_EPIC_M;
	PM->CLK_AHB_SET |= PM_CLOCK_AHB_SPIFI_M;

	UART_Init(UART_1, (32000000 / 57600), UART_CONTROL1_TE_M | UART_CONTROL1_M_8BIT_M, 0, 0);

	GPIO_0->DIRECTION_OUT = (1 << 10) | (1 << 9);
	GPIO_0->OUTPUT = 0;
	li_init();

	while (1)
	{
		for (volatile int i = 0; i < delay_1ms / 2; i++);
		counter++;
		li_update();

		// blink red LED
		if (counter % 400 == 0) {
			GPIO_0->OUTPUT ^= 1 << 10;
		}

		uint16_t left_pot_mv = li_get_pot_value_mv(Left);
		uint16_t right_pot_mv = li_get_pot_value_mv(Right);
		uint16_t levers_bitmask = li_get_levers_bitmask();
		uint16_t keyboard_bitmask = li_get_keyboard_bitmask();

		char levers_str[9] = {0};
		for (int i = 0; i < 8; i++) {
			levers_str[i] = ((levers_bitmask >> i) & 1) ? '1' : '0';
		}

		char keyboard_str[17] = {0};
		for (int i = 0; i < 16; i++) {
			keyboard_str[i] = ((keyboard_bitmask >> i) & 1) ? '1' :'0';
		}

		int32_t enc_counter = li_get_enc_counter();

		// too slow, causes encoder to miss steps
		if (counter % 200 == 0) {
			xprintf("tick: %d;\tleft  pot: %d (mv);\tright  pot: %d (mv);\tlevers: %s;\tkeyboard: %s;\tenc: %d \n\r", counter, left_pot_mv, right_pot_mv, levers_str, keyboard_str, enc_counter);
		}
	}
}
