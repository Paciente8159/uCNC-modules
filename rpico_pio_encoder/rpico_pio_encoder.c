/*
	Name: rpico_pio_encoder.c
	Description: RP2040/RP2350 PIO hardware-assisted encoder backend for uCNC.
	Author: Stanislavz(staskaaa-netizen) - https://github.com/staskaaa-netizen
*/

#include "../../cnc.h"
#include "../encoder.h"

#ifndef ENC0_VIRTUAL_INDEX_ONLY
#define ENC0_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC1_VIRTUAL_INDEX_ONLY
#define ENC1_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC2_VIRTUAL_INDEX_ONLY
#define ENC2_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC3_VIRTUAL_INDEX_ONLY
#define ENC3_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC4_VIRTUAL_INDEX_ONLY
#define ENC4_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC5_VIRTUAL_INDEX_ONLY
#define ENC5_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC6_VIRTUAL_INDEX_ONLY
#define ENC6_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC7_VIRTUAL_INDEX_ONLY
#define ENC7_VIRTUAL_INDEX_ONLY 0
#endif

#if (MCU == MCU_RP2040 || MCU == MCU_RP2350)

#ifndef PIO_ENC
#define PIO_ENC -1
#endif

#if (PIO_ENC == ENC0)
#if (ENC0_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 0 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC0_IS_INCREMENTAL
#error "Encoder 0 requires to be set as incremental"
#endif
#ifndef ENC0_NO_WRAP_CORRECTION
#error "Encoder 0 requires disable wrap correction"
#endif
#define rpico_pio_read enc_custom_read_enc0
#define PIO_VIRTUAL_INDEX_ONLY ENC0_VIRTUAL_INDEX_ONLY
#elif (PIO_ENC == ENC1)
#if (ENC1_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 1 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC1_IS_INCREMENTAL
#error "Encoder 1 requires to be set as incremental"
#endif
#ifndef ENC1_NO_WRAP_CORRECTION
#error "Encoder 1 requires disable wrap correction"
#endif
#define rpico_pio_read enc_custom_read_enc1
#define PIO_VIRTUAL_INDEX_ONLY ENC1_VIRTUAL_INDEX_ONLY
#elif (PIO_ENC == ENC2)
#if (ENC2_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 2 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC2_IS_INCREMENTAL
#error "Encoder 2 requires to be set as incremental"
#endif
#ifndef ENC2_NO_WRAP_CORRECTION
#error "Encoder 2 requires disable wrap correction"
#endif
#define rpico_pio_read enc_custom_read_enc2
#define PIO_VIRTUAL_INDEX_ONLY ENC2_VIRTUAL_INDEX_ONLY
#elif (PIO_ENC == ENC3)
#if (ENC3_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 3 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC3_IS_INCREMENTAL
#error "Encoder 3 requires to be set as incremental"
#endif
#ifndef ENC3_NO_WRAP_CORRECTION
#error "Encoder 3 requires disable wrap correction"
#endif
#define rpico_pio_read enc_custom_read_enc3
#define PIO_VIRTUAL_INDEX_ONLY ENC3_VIRTUAL_INDEX_ONLY
#elif (PIO_ENC == ENC4)
#if (ENC4_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 4 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC4_IS_INCREMENTAL
#error "Encoder 4 requires to be set as incremental"
#endif
#ifndef ENC4_NO_WRAP_CORRECTION
#error "Encoder 4 requires disable wrap correction"
#endif
#define rpico_pio_read enc_custom_read_enc4
#define PIO_VIRTUAL_INDEX_ONLY ENC4_VIRTUAL_INDEX_ONLY
#elif (PIO_ENC == ENC5)
#if (ENC5_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 5 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC5_IS_INCREMENTAL
#error "Encoder 5 requires to be set as incremental"
#endif
#ifndef ENC5_NO_WRAP_CORRECTION
#error "Encoder 5 requires disable wrap correction"
#endif
#define rpico_pio_read enc_custom_read_enc5
#define PIO_VIRTUAL_INDEX_ONLY ENC5_VIRTUAL_INDEX_ONLY
#elif (PIO_ENC == ENC6)
#if (ENC6_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 6 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC6_IS_INCREMENTAL
#error "Encoder 6 requires to be set as incremental"
#endif
#ifndef ENC6_NO_WRAP_CORRECTION
#error "Encoder 6 requires disable wrap correction"
#endif
#define rpico_pio_read enc_custom_read_enc6
#define PIO_VIRTUAL_INDEX_ONLY ENC6_VIRTUAL_INDEX_ONLY
#elif (PIO_ENC == ENC7)
#if (ENC7_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 7 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC7_IS_INCREMENTAL
#error "Encoder 7 requires to be set as incremental"
#endif
#ifndef ENC7_NO_WRAP_CORRECTION
#error "Encoder 7 requires disable wrap correction"
#endif
#define rpico_pio_read enc_custom_read_enc7
#define PIO_VIRTUAL_INDEX_ONLY ENC7_VIRTUAL_INDEX_ONLY
#endif

#if (ENCODERS > 0) && (PIO_ENC >= 0)

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"

#if !defined(PIO_PULSE_GPIO) || !ASSERT_PIN_IO(PIO_PULSE_GPIO)
#error "PIO_PULSE_GPIO must be defined for RP PIO encoder"
#endif

#ifndef PIO_ENC_INDEX
#define PIO_ENC_INDEX 0
#endif

#ifndef PIO_ENC_SM
#define PIO_ENC_SM 0
#endif

#ifndef PIO_ENC_PROGRAM_OFFSET
#define PIO_ENC_PROGRAM_OFFSET 0
#endif

#ifndef PIO_ENC_MAX_STEP_RATE
#define PIO_ENC_MAX_STEP_RATE 0
#endif

static bool rp_pio_encoder_ready;
static PIO rp_pio_encoder_pio;

static const uint16_t quadrature_encoder_program_instructions[] = {
	0x000f, 0x000e, 0x0015, 0x000f,
	0x0015, 0x000f, 0x000f, 0x000e,
	0x000e, 0x000f, 0x000f, 0x0015,
	0x000f, 0x0015,
	0x008f,
	0xa0c2,
	0x8000,
	0x60c2,
	0x4002,
	0xa0e6,
	0xa0a6,
	0xa04a,
	0x0097,
	0xa04a,
};

static const struct pio_program quadrature_encoder_program = {
	.instructions = quadrature_encoder_program_instructions,
	.length = 24,
	.origin = PIO_ENC_PROGRAM_OFFSET,
};

static PIO rpico_pio_get_pio(void)
{
#if PIO_ENC_INDEX == 1
	return pio1;
#else
	return pio0;
#endif
}

static void quadrature_encoder_program_init_inline(PIO pio, uint sm, uint pin, int max_step_rate)
{
	pio_sm_config c;

	pio_sm_set_consecutive_pindirs(pio, sm, pin, 2, false);
	pio_gpio_init(pio, pin);
	pio_gpio_init(pio, pin + 1);
	gpio_pull_up(pin);
	gpio_pull_up(pin + 1);

	c = pio_get_default_sm_config();
	sm_config_set_wrap(&c, 15, 23);
	sm_config_set_in_pins(&c, pin);
	sm_config_set_jmp_pin(&c, pin);
	sm_config_set_in_shift(&c, false, false, 32);
	sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_NONE);

	if (max_step_rate == 0)
	{
		sm_config_set_clkdiv(&c, 1.0f);
	}
	else
	{
		sm_config_set_clkdiv(&c, (float)clock_get_hz(clk_sys) / (10.0f * (float)max_step_rate));
	}

	pio_sm_init(pio, sm, PIO_ENC_PROGRAM_OFFSET, &c);
	pio_sm_set_enabled(pio, sm, true);
}

static int32_t quadrature_encoder_get_count_inline(PIO pio, uint sm)
{
	uint32_t ret = 0;
	int n = pio_sm_get_rx_fifo_level(pio, sm) + 1;
	while (n-- > 0)
	{
		ret = pio_sm_get_blocking(pio, sm);
	}
	return (int32_t)ret;
}

static void encoder_rp_pio_init(void)
{
	rp_pio_encoder_pio = rpico_pio_get_pio();
	pio_add_program_at_offset(rp_pio_encoder_pio, &quadrature_encoder_program, PIO_ENC_PROGRAM_OFFSET);
	quadrature_encoder_program_init_inline(rp_pio_encoder_pio, PIO_ENC_SM, __indirect__(PIO_PULSE_GPIO, BIT), PIO_ENC_MAX_STEP_RATE);
}

static int32_t read_encoder_rp_pio(void)
{
	if (!rp_pio_encoder_ready)
	{
		return 0;
	}
	return quadrature_encoder_get_count_inline(rp_pio_encoder_pio, PIO_ENC_SM);
}

#if defined(PIO_INDEX_GPIO) && !ENC0_VIRTUAL_INDEX_ONLY
static void pio_index_gpio_isr(uint gpio, uint32_t events)
{
	(void)gpio;
	(void)events;
	encoder_record_index_reference(ENC0, read_encoder_rp_pio());
}

static void pio_index_gpio_isr_init(void)
{
	gpio_init(PIO_INDEX_GPIO);
	gpio_set_dir(PIO_INDEX_GPIO, GPIO_IN);
	gpio_pull_up(PIO_INDEX_GPIO);
	gpio_set_irq_enabled_with_callback(PIO_INDEX_GPIO, GPIO_IRQ_EDGE_RISE, true, &pio_index_gpio_isr);
}
#endif

int32_t rpico_pio_read(void)
{
	return read_encoder_rp_pio();
}

DECL_MODULE(rpico_pio_encoder)
{
	if (rp_pio_encoder_ready)
	{
		return;
	}

	encoder_rp_pio_init();
#if defined(PIO_INDEX_GPIO) && !PIO_VIRTUAL_INDEX_ONLY
	pio_index_gpio_isr_init();
#endif
	rp_pio_encoder_ready = true;
}

#else

DECL_MODULE(rpico_pio_encoder)
{
}

#endif
#else
#warning "RP Pico PIO driver not available on this MCU"
#endif
