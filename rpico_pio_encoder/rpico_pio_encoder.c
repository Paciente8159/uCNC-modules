/*
	Name: rpico_pio_encoder.c
	Description: RP2040/RP2350 PIO hardware-assisted encoder backend for uCNC.
	Author: Stanislavz(staskaaa-netizen) - https://github.com/staskaaa-netizen

	Each RPICO_PIO_ENC0..RPICO_PIO_ENC7 instance can be assigned to any uCNC
	encoder. The selected encoder must be ENC_TYPE_CUSTOM; its PULSE and DIR
	pins are resolved automatically. Equal PULSE/DIR pins select the single-wire
	undirectional counter, while distinct pins select quadrature decoding.
*/

#include "../../cnc.h"
#include "../encoder.h"

#include <stdbool.h>
#include <stdint.h>

#if (MCU == MCU_RP2040 || MCU == MCU_RP2350)

#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"

#define rpico_pio_pin_helper_ex(x, y) ENC##x##_##y
#define rpico_pio_pin_helper(x, y) rpico_pio_pin_helper_ex(x, y)
#define rpico_pio_type_helper_ex(x) ENC##x##_TYPE
#define rpico_pio_type_helper(x) rpico_pio_type_helper_ex(x)
#define rpico_pio_read_helper_ex(x) enc_custom_read_enc##x
#define rpico_pio_read_helper(x) rpico_pio_read_helper_ex(x)

/* Backward-compatible name for the former single-instance configuration. */
#if defined(PIO_ENC) && !defined(RPICO_PIO_ENC0) && (PIO_ENC >= 0)
#define RPICO_PIO_ENC0 PIO_ENC
#endif

#define RPICO_PIO_QUADRATURE_PROGRAM_OFFSET 0
#define RPICO_PIO_SINGLE_PROGRAM_OFFSET 24

typedef struct
{
	PIO pio;
	uint8_t sm;
	bool single_wire;
	bool ready;
} rpico_pio_encoder_t;

static bool rpico_quadrature_loaded[3];
static bool rpico_single_loaded[3];

/* Raspberry Pi's canonical full-resolution quadrature encoder program. It is
 * fixed at offset zero because its first 16 instructions are a jump table. */
static const uint16_t quadrature_encoder_program_instructions[] = {
	0x000f, 0x000e, 0x0015, 0x000f,
	0x0015, 0x000f, 0x000f, 0x000e,
	0x000e, 0x000f, 0x000f, 0x0015,
	0x000f, 0x0015, 0x008f, 0xa0c2,
	0x8000, 0x60c2, 0x4002, 0xa0e6,
	0xa0a6, 0xa04a, 0x0097, 0xa04a,
};

static const struct pio_program quadrature_encoder_program = {
	.instructions = quadrature_encoder_program_instructions,
	.length = 24,
	.origin = RPICO_PIO_QUADRATURE_PROGRAM_OFFSET,
};

/* X is decremented on each falling edge because PIO has a native decrement
 * operation. The read helper negates it to expose an increasing pulse count. */
static const uint16_t single_encoder_program_instructions[] = {
	0xa0c1, /* mov isr, x   */
	0x8000, /* push noblock */
	0x00c6, /* jmp pin, 6   */
	0x0045, /* jmp y--, 5   */
	0x0000, /* jmp 0        */
	0x0020, /* jmp x--, 0   */
	0xe041, /* set y, 1     */
	0x0000, /* jmp 0        */
};

static const struct pio_program single_encoder_program = {
	.instructions = single_encoder_program_instructions,
	.length = 8,
	.origin = RPICO_PIO_SINGLE_PROGRAM_OFFSET,
};

static PIO rpico_pio_get_pio(uint8_t index)
{
#if (MCU == MCU_RP2350)
	if (index == 2)
	{
		return pio2;
	}
#endif
	return (index == 1) ? pio1 : pio0;
}

static bool rpico_pio_load_program(PIO pio, bool single_wire)
{
	uint8_t index = (uint8_t)pio_get_index(pio);
	bool *loaded = single_wire ? &rpico_single_loaded[index] : &rpico_quadrature_loaded[index];
	const struct pio_program *program = single_wire ? &single_encoder_program : &quadrature_encoder_program;
	uint offset = single_wire ? RPICO_PIO_SINGLE_PROGRAM_OFFSET : RPICO_PIO_QUADRATURE_PROGRAM_OFFSET;

	if (*loaded)
	{
		return true;
	}
	if (!pio_can_add_program_at_offset(pio, program, offset))
	{
		return false;
	}
	pio_add_program_at_offset(pio, program, offset);
	*loaded = true;
	return true;
}

static bool rpico_pio_encoder_configure(rpico_pio_encoder_t *encoder, uint8_t pio_index,
	uint8_t sm, uint pulse_pin, uint dir_pin, uint32_t max_step_rate)
{
	bool single_wire = (pulse_pin == dir_pin);
	PIO pio;
	pio_sm_config config;

	if (sm >= 4 || pio_index > 2)
	{
		return false;
	}
#if (MCU == MCU_RP2040)
	if (pio_index > 1)
	{
		return false;
	}
#endif
	/* The canonical quadrature program samples two consecutive input pins. */
	if (!single_wire && dir_pin != (pulse_pin + 1))
	{
		return false;
	}

	pio = rpico_pio_get_pio(pio_index);
	if (!rpico_pio_load_program(pio, single_wire))
	{
		return false;
	}

	pio_sm_set_consecutive_pindirs(pio, sm, pulse_pin, single_wire ? 1 : 2, false);
	pio_gpio_init(pio, pulse_pin);
	if (!single_wire)
	{
		pio_gpio_init(pio, dir_pin);
	}

	config = pio_get_default_sm_config();
	if (single_wire)
	{
		sm_config_set_wrap(&config, RPICO_PIO_SINGLE_PROGRAM_OFFSET,
			RPICO_PIO_SINGLE_PROGRAM_OFFSET + single_encoder_program.length - 1);
		sm_config_set_jmp_pin(&config, pulse_pin);
	}
	else
	{
		sm_config_set_wrap(&config, 15, 23);
		sm_config_set_in_pins(&config, pulse_pin);
		sm_config_set_jmp_pin(&config, pulse_pin);
		sm_config_set_in_shift(&config, false, false, 32);
	}
	sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_NONE);
	if (max_step_rate == 0)
	{
		sm_config_set_clkdiv(&config, 1.0f);
	}
	else
	{
		sm_config_set_clkdiv(&config, (float)clock_get_hz(clk_sys) / (10.0f * (float)max_step_rate));
	}

	pio_sm_init(pio, sm, single_wire ? RPICO_PIO_SINGLE_PROGRAM_OFFSET : 0, &config);
	pio_sm_clear_fifos(pio, sm);
	pio_sm_set_enabled(pio, sm, true);
	encoder->pio = pio;
	encoder->sm = sm;
	encoder->single_wire = single_wire;
	encoder->ready = true;
	return true;
}

static int32_t rpico_pio_encoder_read(rpico_pio_encoder_t *encoder)
{
	uint32_t value = 0;
	int available;

	if (!encoder->ready)
	{
		return 0;
	}
	available = pio_sm_get_rx_fifo_level(encoder->pio, encoder->sm) + 1;
	while (available-- > 0)
	{
		value = pio_sm_get_blocking(encoder->pio, encoder->sm);
	}
	return encoder->single_wire ? (int32_t)(0u - value) : (int32_t)value;
}

/* Instances 0..3 default to PIO0 SM0..3; 4..7 default to PIO1 SM0..3. */
#ifndef RPICO_PIO0_INDEX
#ifdef PIO_ENC_INDEX
#define RPICO_PIO0_INDEX PIO_ENC_INDEX
#else
#define RPICO_PIO0_INDEX 0
#endif
#endif
#ifndef RPICO_PIO0_SM
#ifdef PIO_ENC_SM
#define RPICO_PIO0_SM PIO_ENC_SM
#else
#define RPICO_PIO0_SM 0
#endif
#endif
#ifndef RPICO_PIO0_MAX_STEP_RATE
#ifdef PIO_ENC_MAX_STEP_RATE
#define RPICO_PIO0_MAX_STEP_RATE PIO_ENC_MAX_STEP_RATE
#else
#define RPICO_PIO0_MAX_STEP_RATE 0
#endif
#endif

#ifndef RPICO_PIO1_INDEX
#define RPICO_PIO1_INDEX 0
#endif
#ifndef RPICO_PIO1_SM
#define RPICO_PIO1_SM 1
#endif
#ifndef RPICO_PIO1_MAX_STEP_RATE
#define RPICO_PIO1_MAX_STEP_RATE 0
#endif
#ifndef RPICO_PIO2_INDEX
#define RPICO_PIO2_INDEX 0
#endif
#ifndef RPICO_PIO2_SM
#define RPICO_PIO2_SM 2
#endif
#ifndef RPICO_PIO2_MAX_STEP_RATE
#define RPICO_PIO2_MAX_STEP_RATE 0
#endif
#ifndef RPICO_PIO3_INDEX
#define RPICO_PIO3_INDEX 0
#endif
#ifndef RPICO_PIO3_SM
#define RPICO_PIO3_SM 3
#endif
#ifndef RPICO_PIO3_MAX_STEP_RATE
#define RPICO_PIO3_MAX_STEP_RATE 0
#endif
#ifndef RPICO_PIO4_INDEX
#define RPICO_PIO4_INDEX 1
#endif
#ifndef RPICO_PIO4_SM
#define RPICO_PIO4_SM 0
#endif
#ifndef RPICO_PIO4_MAX_STEP_RATE
#define RPICO_PIO4_MAX_STEP_RATE 0
#endif
#ifndef RPICO_PIO5_INDEX
#define RPICO_PIO5_INDEX 1
#endif
#ifndef RPICO_PIO5_SM
#define RPICO_PIO5_SM 1
#endif
#ifndef RPICO_PIO5_MAX_STEP_RATE
#define RPICO_PIO5_MAX_STEP_RATE 0
#endif
#ifndef RPICO_PIO6_INDEX
#define RPICO_PIO6_INDEX 1
#endif
#ifndef RPICO_PIO6_SM
#define RPICO_PIO6_SM 2
#endif
#ifndef RPICO_PIO6_MAX_STEP_RATE
#define RPICO_PIO6_MAX_STEP_RATE 0
#endif
#ifndef RPICO_PIO7_INDEX
#define RPICO_PIO7_INDEX 1
#endif
#ifndef RPICO_PIO7_SM
#define RPICO_PIO7_SM 3
#endif
#ifndef RPICO_PIO7_MAX_STEP_RATE
#define RPICO_PIO7_MAX_STEP_RATE 0
#endif

#define RPICO_DECLARE_INSTANCE(n) \
	static rpico_pio_encoder_t rpico_pio_encoder##n

#ifdef RPICO_PIO_ENC0
#define RPICO_PIO0_PULSE rpico_pio_pin_helper(RPICO_PIO_ENC0, PULSE)
#define RPICO_PIO0_DIR rpico_pio_pin_helper(RPICO_PIO_ENC0, DIR)
#define RPICO_PIO0_READ rpico_pio_read_helper(RPICO_PIO_ENC0)
#if (rpico_pio_type_helper(RPICO_PIO_ENC0) != ENC_TYPE_CUSTOM)
#error "RPICO_PIO_ENC0 must select an ENC_TYPE_CUSTOM encoder"
#endif
RPICO_DECLARE_INSTANCE(0);
int32_t RPICO_PIO0_READ(void) { return rpico_pio_encoder_read(&rpico_pio_encoder0); }
#endif
#ifdef RPICO_PIO_ENC1
#define RPICO_PIO1_PULSE rpico_pio_pin_helper(RPICO_PIO_ENC1, PULSE)
#define RPICO_PIO1_DIR rpico_pio_pin_helper(RPICO_PIO_ENC1, DIR)
#define RPICO_PIO1_READ rpico_pio_read_helper(RPICO_PIO_ENC1)
#if (rpico_pio_type_helper(RPICO_PIO_ENC1) != ENC_TYPE_CUSTOM)
#error "RPICO_PIO_ENC1 must select an ENC_TYPE_CUSTOM encoder"
#endif
RPICO_DECLARE_INSTANCE(1);
int32_t RPICO_PIO1_READ(void) { return rpico_pio_encoder_read(&rpico_pio_encoder1); }
#endif
#ifdef RPICO_PIO_ENC2
#define RPICO_PIO2_PULSE rpico_pio_pin_helper(RPICO_PIO_ENC2, PULSE)
#define RPICO_PIO2_DIR rpico_pio_pin_helper(RPICO_PIO_ENC2, DIR)
#define RPICO_PIO2_READ rpico_pio_read_helper(RPICO_PIO_ENC2)
#if (rpico_pio_type_helper(RPICO_PIO_ENC2) != ENC_TYPE_CUSTOM)
#error "RPICO_PIO_ENC2 must select an ENC_TYPE_CUSTOM encoder"
#endif
RPICO_DECLARE_INSTANCE(2);
int32_t RPICO_PIO2_READ(void) { return rpico_pio_encoder_read(&rpico_pio_encoder2); }
#endif
#ifdef RPICO_PIO_ENC3
#define RPICO_PIO3_PULSE rpico_pio_pin_helper(RPICO_PIO_ENC3, PULSE)
#define RPICO_PIO3_DIR rpico_pio_pin_helper(RPICO_PIO_ENC3, DIR)
#define RPICO_PIO3_READ rpico_pio_read_helper(RPICO_PIO_ENC3)
#if (rpico_pio_type_helper(RPICO_PIO_ENC3) != ENC_TYPE_CUSTOM)
#error "RPICO_PIO_ENC3 must select an ENC_TYPE_CUSTOM encoder"
#endif
RPICO_DECLARE_INSTANCE(3);
int32_t RPICO_PIO3_READ(void) { return rpico_pio_encoder_read(&rpico_pio_encoder3); }
#endif
#ifdef RPICO_PIO_ENC4
#define RPICO_PIO4_PULSE rpico_pio_pin_helper(RPICO_PIO_ENC4, PULSE)
#define RPICO_PIO4_DIR rpico_pio_pin_helper(RPICO_PIO_ENC4, DIR)
#define RPICO_PIO4_READ rpico_pio_read_helper(RPICO_PIO_ENC4)
#if (rpico_pio_type_helper(RPICO_PIO_ENC4) != ENC_TYPE_CUSTOM)
#error "RPICO_PIO_ENC4 must select an ENC_TYPE_CUSTOM encoder"
#endif
RPICO_DECLARE_INSTANCE(4);
int32_t RPICO_PIO4_READ(void) { return rpico_pio_encoder_read(&rpico_pio_encoder4); }
#endif
#ifdef RPICO_PIO_ENC5
#define RPICO_PIO5_PULSE rpico_pio_pin_helper(RPICO_PIO_ENC5, PULSE)
#define RPICO_PIO5_DIR rpico_pio_pin_helper(RPICO_PIO_ENC5, DIR)
#define RPICO_PIO5_READ rpico_pio_read_helper(RPICO_PIO_ENC5)
#if (rpico_pio_type_helper(RPICO_PIO_ENC5) != ENC_TYPE_CUSTOM)
#error "RPICO_PIO_ENC5 must select an ENC_TYPE_CUSTOM encoder"
#endif
RPICO_DECLARE_INSTANCE(5);
int32_t RPICO_PIO5_READ(void) { return rpico_pio_encoder_read(&rpico_pio_encoder5); }
#endif
#ifdef RPICO_PIO_ENC6
#define RPICO_PIO6_PULSE rpico_pio_pin_helper(RPICO_PIO_ENC6, PULSE)
#define RPICO_PIO6_DIR rpico_pio_pin_helper(RPICO_PIO_ENC6, DIR)
#define RPICO_PIO6_READ rpico_pio_read_helper(RPICO_PIO_ENC6)
#if (rpico_pio_type_helper(RPICO_PIO_ENC6) != ENC_TYPE_CUSTOM)
#error "RPICO_PIO_ENC6 must select an ENC_TYPE_CUSTOM encoder"
#endif
RPICO_DECLARE_INSTANCE(6);
int32_t RPICO_PIO6_READ(void) { return rpico_pio_encoder_read(&rpico_pio_encoder6); }
#endif
#ifdef RPICO_PIO_ENC7
#define RPICO_PIO7_PULSE rpico_pio_pin_helper(RPICO_PIO_ENC7, PULSE)
#define RPICO_PIO7_DIR rpico_pio_pin_helper(RPICO_PIO_ENC7, DIR)
#define RPICO_PIO7_READ rpico_pio_read_helper(RPICO_PIO_ENC7)
#if (rpico_pio_type_helper(RPICO_PIO_ENC7) != ENC_TYPE_CUSTOM)
#error "RPICO_PIO_ENC7 must select an ENC_TYPE_CUSTOM encoder"
#endif
RPICO_DECLARE_INSTANCE(7);
int32_t RPICO_PIO7_READ(void) { return rpico_pio_encoder_read(&rpico_pio_encoder7); }
#endif

#define RPICO_CONFIGURE_INSTANCE(n) \
	do { \
		if (!rpico_pio_encoder##n.ready) \
			rpico_pio_encoder_configure(&rpico_pio_encoder##n, RPICO_PIO##n##_INDEX, RPICO_PIO##n##_SM, \
				__indirect__(RPICO_PIO##n##_PULSE, BIT), __indirect__(RPICO_PIO##n##_DIR, BIT), \
				RPICO_PIO##n##_MAX_STEP_RATE); \
	} while (0)

DECL_MODULE(rpico_pio_encoder)
{
#ifdef RPICO_PIO_ENC0
	RPICO_CONFIGURE_INSTANCE(0);
#endif
#ifdef RPICO_PIO_ENC1
	RPICO_CONFIGURE_INSTANCE(1);
#endif
#ifdef RPICO_PIO_ENC2
	RPICO_CONFIGURE_INSTANCE(2);
#endif
#ifdef RPICO_PIO_ENC3
	RPICO_CONFIGURE_INSTANCE(3);
#endif
#ifdef RPICO_PIO_ENC4
	RPICO_CONFIGURE_INSTANCE(4);
#endif
#ifdef RPICO_PIO_ENC5
	RPICO_CONFIGURE_INSTANCE(5);
#endif
#ifdef RPICO_PIO_ENC6
	RPICO_CONFIGURE_INSTANCE(6);
#endif
#ifdef RPICO_PIO_ENC7
	RPICO_CONFIGURE_INSTANCE(7);
#endif
}

#else

#warning "RP Pico PIO encoder driver not available on this MCU"
DECL_MODULE(rpico_pio_encoder) {}

#endif
