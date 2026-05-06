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

void main()
{
	PM->CLK_APB_P_SET =   PM_CLOCK_APB_P_GPIO_0_M
						| PM_CLOCK_APB_P_GPIO_1_M
						| PM_CLOCK_APB_P_GPIO_2_M
						| PM_CLOCK_APB_P_GPIO_IRQ_M;
	PM->CLK_APB_M_SET =   PM_CLOCK_APB_M_PAD_CONFIG_M
						| PM_CLOCK_APB_M_WU_M
						| PM_CLOCK_APB_M_PM_M
						| PM_CLOCK_APB_M_EPIC_M;
	PM->CLK_AHB_SET |= PM_CLOCK_AHB_SPIFI_M;


	GPIO_0->DIRECTION_OUT = 1 << 10;
	GPIO_0->DIRECTION_OUT |= 1 << 9;

	init();

	const uint32_t delay_1ms = 32000000 / 1000 / 15;

	while (1)
	{
		for (int i = 0; i < delay_1ms; i++);
		update();
		GPIO_0->OUTPUT = 0;
		if (getPotValueMv(Right) < 500) {
			GPIO_0->OUTPUT |= 1 << 10;
		}

		// if (getPotValueMv(Right) > 2048) {
		//	GPIO_0->OUTPUT |= 1 << 9;
		// }
	}
}
