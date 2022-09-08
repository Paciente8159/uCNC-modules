/*
	Name: ic74hc595.c
	Description: This module adds the ability to control the IC74HC595 shift register controller (used for example in the MKS-DLC32 board) to µCNC.
				 Up to 56 output pins can be assigned.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01/09/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"
#include "ic74hc595.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_PARSER_MODULES

#ifndef UCNC_MODULE_VERSION_1_5_0_PLUS
#error "This module is not compatible with the current version of µCNC"
#endif

#ifndef IC74HC595_COUNT
#define IC74HC595_COUNT 0
#endif

#ifndef IC74HC595_DATA
#define IC74HC595_DATA DOUT4
#endif

#ifndef IC74HC595_CLK
#define IC74HC595_CLK DOUT5
#endif

#ifndef IC74HC595_LATCH
#define IC74HC595_LATCH DOUT6
#endif

#ifndef IC74HC595_DELAY_CYCLES
#define IC74HC595_DELAY_CYCLES 0
#endif

static uint8_t io_pins[IC74HC595_COUNT];

static void shift_io_pins(void)
{
	for (uint8_t i = IC74HC595_COUNT - 1; i != 0;)
	{
		for (uint8_t j = 0x80; j != 0; j >>= 1)
		{
			if (io_pins[i] & j)
			{
				mcu_set_output(IC74HC595_DATA);
			}
			else
			{
				mcu_clear_output(IC74HC595_DATA);
			}
			ic74hc595_delay();
			mcu_set_output(IC74HC595_CLK);
			ic74hc595_delay();
			mcu_clear_output(IC74HC595_CLK);
		}
	}
	ic74hc595_delay();
	mcu_set_output(IC74HC595_LATCH);
	ic74hc595_delay();
	mcu_clear_output(IC74HC595_LATCH);
}

#ifdef IC74HC595_HAS_STEPS
OVERRIDE_EVENT_HANDLER(set_steps)
{
	uint8_t bits = (uint8_t)*args;
#if !(STEP0_IO_OFFSET < 0)
	if (bits & (1 << 0))
	{
		io_pins[STEP0_IO_BYTEOFFSET] |= STEP0_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP0_IO_BYTEOFFSET] &= ~STEP0_IO_BITOFFSET;
	}
#endif
#if !(STEP1_IO_OFFSET < 0)
	if (bits & (1 << 1))
	{
		io_pins[STEP1_IO_BYTEOFFSET] |= STEP1_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP1_IO_BYTEOFFSET] &= ~STEP1_IO_BITOFFSET;
	}
#endif
#if !(STEP2_IO_OFFSET < 0)
	if (bits & (1 << 2))
	{
		io_pins[STEP2_IO_BYTEOFFSET] |= STEP2_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP2_IO_BYTEOFFSET] &= ~STEP2_IO_BITOFFSET;
	}
#endif
#if !(STEP3_IO_OFFSET < 0)
	if (bits & (1 << 3))
	{
		io_pins[STEP3_IO_BYTEOFFSET] |= STEP3_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP3_IO_BYTEOFFSET] &= ~STEP3_IO_BITOFFSET;
	}
#endif
#if !(STEP4_IO_OFFSET < 0)
	if (bits & (1 << 4))
	{
		io_pins[STEP4_IO_BYTEOFFSET] |= STEP4_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP4_IO_BYTEOFFSET] &= ~STEP4_IO_BITOFFSET;
	}
#endif
#if !(STEP5_IO_OFFSET < 0)
	if (bits & (1 << 5))
	{
		io_pins[STEP5_IO_BYTEOFFSET] |= STEP5_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP5_IO_BYTEOFFSET] &= ~STEP5_IO_BITOFFSET;
	}
#endif
#if !(STEP6_IO_OFFSET < 0)
	if (bits & (1 << 6))
	{
		io_pins[STEP6_IO_BYTEOFFSET] |= STEP6_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP6_IO_BYTEOFFSET] &= ~STEP6_IO_BITOFFSET;
	}
#endif
#if !(STEP7_IO_OFFSET < 0)
	if (bits & (1 << 7))
	{
		io_pins[STEP7_IO_BYTEOFFSET] |= STEP7_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP7_IO_BYTEOFFSET] &= ~STEP7_IO_BITOFFSET;
	}
#endif

	shift_io_pins();
}

OVERRIDE_EVENT_HANDLER(toggle_steps)
{
	uint8_t bits = (uint8_t)*args;
#if !(STEP0_IO_OFFSET < 0)
	if (bits & (1 << 0))
	{
		io_pins[STEP0_IO_BYTEOFFSET] ^= STEP0_IO_BITOFFSET;
	}
#endif
#if !(STEP1_IO_OFFSET < 0)
	if (bits & (1 << 1))
	{
		io_pins[STEP1_IO_BYTEOFFSET] ^= STEP1_IO_BITOFFSET;
	}
#endif
#if !(STEP2_IO_OFFSET < 0)
	if (bits & (1 << 2))
	{
		io_pins[STEP2_IO_BYTEOFFSET] ^= STEP2_IO_BITOFFSET;
	}
#endif
#if !(STEP3_IO_OFFSET < 0)
	if (bits & (1 << 3))
	{
		io_pins[STEP3_IO_BYTEOFFSET] ^= STEP3_IO_BITOFFSET;
	}
#endif
#if !(STEP4_IO_OFFSET < 0)
	if (bits & (1 << 4))
	{
		io_pins[STEP4_IO_BYTEOFFSET] ^= STEP4_IO_BITOFFSET;
	}
#endif
#if !(STEP5_IO_OFFSET < 0)
	if (bits & (1 << 5))
	{
		io_pins[STEP5_IO_BYTEOFFSET] ^= STEP5_IO_BITOFFSET;
	}
#endif
#if !(STEP6_IO_OFFSET < 0)
	if (bits & (1 << 6))
	{
		io_pins[STEP6_IO_BYTEOFFSET] ^= STEP6_IO_BITOFFSET;
	}
#endif
#if !(STEP7_IO_OFFSET < 0)
	if (bits & (1 << 7))
	{
		io_pins[STEP7_IO_BYTEOFFSET] ^= STEP7_IO_BITOFFSET;
	}
#endif

shift_io_pins();
}
#endif

#ifdef IC74HC595_HAS_DIRS
OVERRIDE_EVENT_HANDLER(set_dirs)
{
	uint8_t bits = (uint8_t)*args;
#if !(DIR0_IO_OFFSET < 0)
	if (bits & (1 << 0))
	{
		io_pins[DIR0_IO_BYTEOFFSET] |= DIR0_IO_BITOFFSET;
	}
	else
	{
		io_pins[DIR0_IO_BYTEOFFSET] &= ~DIR0_IO_BITOFFSET;
	}
#endif
#if !(DIR1_IO_OFFSET < 0)
	if (bits & (1 << 1))
	{
		io_pins[DIR1_IO_BYTEOFFSET] |= DIR1_IO_BITOFFSET;
	}
	else
	{
		io_pins[DIR1_IO_BYTEOFFSET] &= ~DIR1_IO_BITOFFSET;
	}
#endif
#if !(DIR2_IO_OFFSET < 0)
	if (bits & (1 << 2))
	{
		io_pins[DIR2_IO_BYTEOFFSET] |= DIR2_IO_BITOFFSET;
	}
	else
	{
		io_pins[DIR2_IO_BYTEOFFSET] &= ~DIR2_IO_BITOFFSET;
	}
#endif
#if !(DIR3_IO_OFFSET < 0)
	if (bits & (1 << 3))
	{
		io_pins[DIR3_IO_BYTEOFFSET] |= DIR3_IO_BITOFFSET;
	}
	else
	{
		io_pins[DIR3_IO_BYTEOFFSET] &= ~DIR3_IO_BITOFFSET;
	}
#endif
#if !(DIR4_IO_OFFSET < 0)
	if (bits & (1 << 4))
	{
		io_pins[DIR4_IO_BYTEOFFSET] |= DIR4_IO_BITOFFSET;
	}
	else
	{
		io_pins[DIR4_IO_BYTEOFFSET] &= ~DIR4_IO_BITOFFSET;
	}
#endif
#if !(DIR5_IO_OFFSET < 0)
	if (bits & (1 << 5))
	{
		io_pins[DIR5_IO_BYTEOFFSET] |= DIR5_IO_BITOFFSET;
	}
	else
	{
		io_pins[DIR5_IO_BYTEOFFSET] &= ~DIR5_IO_BITOFFSET;
	}
#endif
#if !(DIR6_IO_OFFSET < 0)
	if (bits & (1 << 6))
	{
		io_pins[DIR6_IO_BYTEOFFSET] |= DIR6_IO_BITOFFSET;
	}
	else
	{
		io_pins[DIR6_IO_BYTEOFFSET] &= ~DIR6_IO_BITOFFSET;
	}
#endif
#if !(DIR7_IO_OFFSET < 0)
	if (bits & (1 << 7))
	{
		io_pins[DIR7_IO_BYTEOFFSET] |= DIR7_IO_BITOFFSET;
	}
	else
	{
		io_pins[DIR7_IO_BYTEOFFSET] &= ~DIR7_IO_BITOFFSET;
	}
#endif

shift_io_pins();
}
#endif

#ifdef IC74HC595_HAS_STEPS_EN
OVERRIDE_EVENT_HANDLER(enable_steppers)
{
	uint8_t bits = (uint8_t)*args;
#if !(STEP0_EN_IO_OFFSET < 0)
	if (bits & (1 << 0))
	{
		io_pins[STEP0_EN_IO_BYTEOFFSET] |= STEP0_EN_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP0_EN_IO_BYTEOFFSET] &= ~STEP0_EN_IO_BITOFFSET;
	}
#endif
#if !(STEP1_EN_IO_OFFSET < 0)
	if (bits & (1 << 1))
	{
		io_pins[STEP1_EN_IO_BYTEOFFSET] |= STEP1_EN_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP1_EN_IO_BYTEOFFSET] &= ~STEP1_EN_IO_BITOFFSET;
	}
#endif
#if !(STEP2_EN_IO_OFFSET < 0)
	if (bits & (1 << 2))
	{
		io_pins[STEP2_EN_IO_BYTEOFFSET] |= STEP2_EN_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP2_EN_IO_BYTEOFFSET] &= ~STEP2_EN_IO_BITOFFSET;
	}
#endif
#if !(STEP3_EN_IO_OFFSET < 0)
	if (bits & (1 << 3))
	{
		io_pins[STEP3_EN_IO_BYTEOFFSET] |= STEP3_EN_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP3_EN_IO_BYTEOFFSET] &= ~STEP3_EN_IO_BITOFFSET;
	}
#endif
#if !(STEP4_EN_IO_OFFSET < 0)
	if (bits & (1 << 4))
	{
		io_pins[STEP4_EN_IO_BYTEOFFSET] |= STEP4_EN_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP4_EN_IO_BYTEOFFSET] &= ~STEP4_EN_IO_BITOFFSET;
	}
#endif
#if !(STEP5_EN_IO_OFFSET < 0)
	if (bits & (1 << 5))
	{
		io_pins[STEP5_EN_IO_BYTEOFFSET] |= STEP5_EN_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP5_EN_IO_BYTEOFFSET] &= ~STEP5_EN_IO_BITOFFSET;
	}
#endif
#if !(STEP6_EN_IO_OFFSET < 0)
	if (bits & (1 << 6))
	{
		io_pins[STEP6_EN_IO_BYTEOFFSET] |= STEP6_EN_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP6_EN_IO_BYTEOFFSET] &= ~STEP6_EN_IO_BITOFFSET;
	}
#endif
#if !(STEP7_EN_IO_OFFSET < 0)
	if (bits & (1 << 7))
	{
		io_pins[STEP7_EN_IO_BYTEOFFSET] |= STEP7_EN_IO_BITOFFSET;
	}
	else
	{
		io_pins[STEP7_EN_IO_BYTEOFFSET] &= ~STEP7_EN_IO_BITOFFSET;
	}
#endif

shift_io_pins();
}
#endif

#ifdef IC74HC595_HAS_DOUTS
OVERRIDE_EVENT_HANDLER(set_output)
{
	set_output_args_t *output = (uint8_t)*args;

	switch (output->pin)
	{
#if !(DOUT0_IO_OFFSET < 0)
	case DOUT0:
		if (output->state)
		{
			io_pins[DOUT0_IO_BYTEOFFSET] |= DOUT0_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT0_IO_BYTEOFFSET] &= ~DOUT0_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT1_IO_OFFSET < 0)
	case DOUT1:
		if (output->state)
		{
			io_pins[DOUT1_IO_BYTEOFFSET] |= DOUT1_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT1_IO_BYTEOFFSET] &= ~DOUT1_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT2_IO_OFFSET < 0)
	case DOUT2:
		if (output->state)
		{
			io_pins[DOUT2_IO_BYTEOFFSET] |= DOUT2_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT2_IO_BYTEOFFSET] &= ~DOUT2_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT3_IO_OFFSET < 0)
	case DOUT3:
		if (output->state)
		{
			io_pins[DOUT3_IO_BYTEOFFSET] |= DOUT3_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT3_IO_BYTEOFFSET] &= ~DOUT3_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT4_IO_OFFSET < 0)
	case DOUT4:
		if (output->state)
		{
			io_pins[DOUT4_IO_BYTEOFFSET] |= DOUT4_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT4_IO_BYTEOFFSET] &= ~DOUT4_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT5_IO_OFFSET < 0)
	case DOUT5:
		if (output->state)
		{
			io_pins[DOUT5_IO_BYTEOFFSET] |= DOUT5_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT5_IO_BYTEOFFSET] &= ~DOUT5_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT6_IO_OFFSET < 0)
	case DOUT6:
		if (output->state)
		{
			io_pins[DOUT6_IO_BYTEOFFSET] |= DOUT6_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT6_IO_BYTEOFFSET] &= ~DOUT6_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT7_IO_OFFSET < 0)
	case DOUT7:
		if (output->state)
		{
			io_pins[DOUT7_IO_BYTEOFFSET] |= DOUT7_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT7_IO_BYTEOFFSET] &= ~DOUT7_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT8_IO_OFFSET < 0)
	case DOUT8:
		if (output->state)
		{
			io_pins[DOUT8_IO_BYTEOFFSET] |= DOUT8_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT8_IO_BYTEOFFSET] &= ~DOUT8_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT9_IO_OFFSET < 0)
	case DOUT9:
		if (output->state)
		{
			io_pins[DOUT9_IO_BYTEOFFSET] |= DOUT9_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT9_IO_BYTEOFFSET] &= ~DOUT9_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT10_IO_OFFSET < 0)
	case DOUT10:
		if (output->state)
		{
			io_pins[DOUT10_IO_BYTEOFFSET] |= DOUT10_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT10_IO_BYTEOFFSET] &= ~DOUT10_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT11_IO_OFFSET < 0)
	case DOUT11:
		if (output->state)
		{
			io_pins[DOUT11_IO_BYTEOFFSET] |= DOUT11_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT11_IO_BYTEOFFSET] &= ~DOUT11_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT12_IO_OFFSET < 0)
	case DOUT12:
		if (output->state)
		{
			io_pins[DOUT12_IO_BYTEOFFSET] |= DOUT12_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT12_IO_BYTEOFFSET] &= ~DOUT12_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT13_IO_OFFSET < 0)
	case DOUT13:
		if (output->state)
		{
			io_pins[DOUT13_IO_BYTEOFFSET] |= DOUT13_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT13_IO_BYTEOFFSET] &= ~DOUT13_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT14_IO_OFFSET < 0)
	case DOUT14:
		if (output->state)
		{
			io_pins[DOUT14_IO_BYTEOFFSET] |= DOUT14_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT14_IO_BYTEOFFSET] &= ~DOUT14_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT15_IO_OFFSET < 0)
	case DOUT15:
		if (output->state)
		{
			io_pins[DOUT15_IO_BYTEOFFSET] |= DOUT15_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT15_IO_BYTEOFFSET] &= ~DOUT15_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT16_IO_OFFSET < 0)
	case DOUT16:
		if (output->state)
		{
			io_pins[DOUT16_IO_BYTEOFFSET] |= DOUT16_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT16_IO_BYTEOFFSET] &= ~DOUT16_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT17_IO_OFFSET < 0)
	case DOUT17:
		if (output->state)
		{
			io_pins[DOUT17_IO_BYTEOFFSET] |= DOUT17_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT17_IO_BYTEOFFSET] &= ~DOUT17_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT18_IO_OFFSET < 0)
	case DOUT18:
		if (output->state)
		{
			io_pins[DOUT18_IO_BYTEOFFSET] |= DOUT18_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT18_IO_BYTEOFFSET] &= ~DOUT18_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT19_IO_OFFSET < 0)
	case DOUT19:
		if (output->state)
		{
			io_pins[DOUT19_IO_BYTEOFFSET] |= DOUT19_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT19_IO_BYTEOFFSET] &= ~DOUT19_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT20_IO_OFFSET < 0)
	case DOUT20:
		if (output->state)
		{
			io_pins[DOUT20_IO_BYTEOFFSET] |= DOUT20_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT20_IO_BYTEOFFSET] &= ~DOUT20_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT21_IO_OFFSET < 0)
	case DOUT21:
		if (output->state)
		{
			io_pins[DOUT21_IO_BYTEOFFSET] |= DOUT21_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT21_IO_BYTEOFFSET] &= ~DOUT21_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT22_IO_OFFSET < 0)
	case DOUT22:
		if (output->state)
		{
			io_pins[DOUT22_IO_BYTEOFFSET] |= DOUT22_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT22_IO_BYTEOFFSET] &= ~DOUT22_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT23_IO_OFFSET < 0)
	case DOUT23:
		if (output->state)
		{
			io_pins[DOUT23_IO_BYTEOFFSET] |= DOUT23_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT23_IO_BYTEOFFSET] &= ~DOUT23_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT24_IO_OFFSET < 0)
	case DOUT24:
		if (output->state)
		{
			io_pins[DOUT24_IO_BYTEOFFSET] |= DOUT24_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT24_IO_BYTEOFFSET] &= ~DOUT24_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT25_IO_OFFSET < 0)
	case DOUT25:
		if (output->state)
		{
			io_pins[DOUT25_IO_BYTEOFFSET] |= DOUT25_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT25_IO_BYTEOFFSET] &= ~DOUT25_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT26_IO_OFFSET < 0)
	case DOUT26:
		if (output->state)
		{
			io_pins[DOUT26_IO_BYTEOFFSET] |= DOUT26_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT26_IO_BYTEOFFSET] &= ~DOUT26_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT27_IO_OFFSET < 0)
	case DOUT27:
		if (output->state)
		{
			io_pins[DOUT27_IO_BYTEOFFSET] |= DOUT27_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT27_IO_BYTEOFFSET] &= ~DOUT27_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT28_IO_OFFSET < 0)
	case DOUT28:
		if (output->state)
		{
			io_pins[DOUT28_IO_BYTEOFFSET] |= DOUT28_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT28_IO_BYTEOFFSET] &= ~DOUT28_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT29_IO_OFFSET < 0)
	case DOUT29:
		if (output->state)
		{
			io_pins[DOUT29_IO_BYTEOFFSET] |= DOUT29_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT29_IO_BYTEOFFSET] &= ~DOUT29_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT30_IO_OFFSET < 0)
	case DOUT30:
		if (output->state)
		{
			io_pins[DOUT30_IO_BYTEOFFSET] |= DOUT30_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT30_IO_BYTEOFFSET] &= ~DOUT30_IO_BITOFFSET;
		}
		break;
#endif
#if !(DOUT31_IO_OFFSET < 0)
	case DOUT31:
		if (output->state)
		{
			io_pins[DOUT31_IO_BYTEOFFSET] |= DOUT31_IO_BITOFFSET;
		}
		else
		{
			io_pins[DOUT31_IO_BYTEOFFSET] &= ~DOUT31_IO_BITOFFSET;
		}
		break;
#endif
	}

	shift_io_pins();
}
#endif

#endif