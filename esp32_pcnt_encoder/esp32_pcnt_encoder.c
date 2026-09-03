/*
	Name: esp32_pcnt_encoder.c
	Description: ESP32 PCNT hardware counter backend for uCNC encoders.
	Author: Stanislavz(staskaaa-netizen) - https://github.com/staskaaa-netizen
*/

#include "../../cnc.h"
#include "../encoder.h"
#include <stdint.h>

#if (MCU == MCU_ESP32 || MCU == MCU_ESP32S3 || MCU == MCU_ESP32C3)

#ifndef ESP32_PCNT_ENC
#define ESP32_PCNT_ENC -1
#endif

#if (ESP32_PCNT_ENC == ENC0)
#if (ENC0_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 0 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC0_IS_INCREMENTAL
#error "Encoder 0 requires to be set as incremental"
#endif
#ifndef ENC0_NO_WRAP_CORRECTION
#error "Encoder 0 requires disable wrap correction"
#endif
#define esp32_pcnt_read enc_custom_read_enc0
#elif (ESP32_PCNT_ENC == ENC1)
#if (ENC1_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 1 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC1_IS_INCREMENTAL
#error "Encoder 1 requires to be set as incremental"
#endif
#ifndef ENC1_NO_WRAP_CORRECTION
#error "Encoder 1 requires disable wrap correction"
#endif
#define esp32_pcnt_read enc_custom_read_enc1
#elif (ESP32_PCNT_ENC == ENC2)
#if (ENC2_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 2 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC2_IS_INCREMENTAL
#error "Encoder 2 requires to be set as incremental"
#endif
#ifndef ENC2_NO_WRAP_CORRECTION
#error "Encoder 2 requires disable wrap correction"
#endif
#define esp32_pcnt_read enc_custom_read_enc2
#elif (ESP32_PCNT_ENC == ENC3)
#if (ENC3_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 3 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC3_IS_INCREMENTAL
#error "Encoder 3 requires to be set as incremental"
#endif
#ifndef ENC3_NO_WRAP_CORRECTION
#error "Encoder 3 requires disable wrap correction"
#endif
#define esp32_pcnt_read enc_custom_read_enc3
#elif (ESP32_PCNT_ENC == ENC4)
#if (ENC4_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 4 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC4_IS_INCREMENTAL
#error "Encoder 4 requires to be set as incremental"
#endif
#ifndef ENC4_NO_WRAP_CORRECTION
#error "Encoder 4 requires disable wrap correction"
#endif
#define esp32_pcnt_read enc_custom_read_enc4
#elif (ESP32_PCNT_ENC == ENC5)
#if (ENC5_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 5 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC5_IS_INCREMENTAL
#error "Encoder 5 requires to be set as incremental"
#endif
#ifndef ENC5_NO_WRAP_CORRECTION
#error "Encoder 5 requires disable wrap correction"
#endif
#define esp32_pcnt_read enc_custom_read_enc5
#elif (ESP32_PCNT_ENC == ENC6)
#if (ENC6_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 6 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC6_IS_INCREMENTAL
#error "Encoder 6 requires to be set as incremental"
#endif
#ifndef ENC6_NO_WRAP_CORRECTION
#error "Encoder 6 requires disable wrap correction"
#endif
#define esp32_pcnt_read enc_custom_read_enc6
#elif (ESP32_PCNT_ENC == ENC7)
#if (ENC7_TYPE != ENC_TYPE_CUSTOM)
#error "Encoder 7 requires to be of type custom encoder to use ESP32 PCNT"
#endif
#ifndef ENC7_IS_INCREMENTAL
#error "Encoder 7 requires to be set as incremental"
#endif
#ifndef ENC7_NO_WRAP_CORRECTION
#error "Encoder 7 requires disable wrap correction"
#endif
#define esp32_pcnt_read enc_custom_read_enc7
#endif

#if (ENCODERS > 0) && (ESP32_PCNT_ENC >= 0)

#include "driver/gpio.h"
#include "driver/pcnt.h"

#ifndef ESP32_PCNT_UNIT
#define ESP32_PCNT_UNIT PCNT_UNIT_0
#endif

#ifndef ESP32_PCNT_CHANNEL_A
#define ESP32_PCNT_CHANNEL_A PCNT_CHANNEL_0
#endif

#ifndef ESP32_PCNT_CHANNEL_B
#define ESP32_PCNT_CHANNEL_B PCNT_CHANNEL_1
#endif

#ifndef ESP32_PCNT_RECENTER_THRESHOLD
#define ESP32_PCNT_RECENTER_THRESHOLD 20000
#endif

#ifndef ESP32_PCNT_FILTER
#define ESP32_PCNT_FILTER 0
#endif

#if !defined(ESP32_PCNT_PULSE_GPIO) || !ASSERT_PIN_IO(ESP32_PCNT_PULSE_GPIO)
#error "ESP32_PCNT_PULSE_GPIO must be defined for ESP32 PCNT encoder"
#endif

#ifndef !defined(ESP32_PCNT_DIR_GPIO) || !ASSERT_PIN_IO(ESP32_PCNT_DIR_GPIO)
#error "ESP32_PCNT_DIR_GPIO must be defined for ESP32 PCNT encoder"
#endif

static bool esp32_pcnt_encoder_ready;
static int32_t esp32_pcnt_encoder_offset;

static void encoder_esp32_pcnt_init(void)
{
	const int gpio_a = __indirect__(ESP32_PCNT_PULSE_GPIO, BIT);
	const int gpio_b = __indirect__(ESP32_PCNT_DIR_GPIO, BIT);

	pcnt_config_t ch_a = {
		.pulse_gpio_num = gpio_a,
		.ctrl_gpio_num = gpio_b,
		.lctrl_mode = PCNT_MODE_REVERSE,
		.hctrl_mode = PCNT_MODE_KEEP,
		.pos_mode = PCNT_COUNT_INC,
		.neg_mode = PCNT_COUNT_DEC,
		.counter_h_lim = 32767,
		.counter_l_lim = -32768,
		.unit = (pcnt_unit_t)ESP32_PCNT_UNIT,
		.channel = (pcnt_channel_t)ESP32_PCNT_CHANNEL_A,
	};

	pcnt_config_t ch_b = {
		.pulse_gpio_num = gpio_b,
		.ctrl_gpio_num = gpio_a,
		.lctrl_mode = PCNT_MODE_KEEP,
		.hctrl_mode = PCNT_MODE_REVERSE,
		.pos_mode = PCNT_COUNT_INC,
		.neg_mode = PCNT_COUNT_DEC,
		.counter_h_lim = 32767,
		.counter_l_lim = -32768,
		.unit = (pcnt_unit_t)ESP32_PCNT_UNIT,
		.channel = (pcnt_channel_t)ESP32_PCNT_CHANNEL_B,
	};

	pcnt_unit_config(&ch_a);
	pcnt_unit_config(&ch_b);

#if ESP32_PCNT_FILTER
	pcnt_set_filter_value((pcnt_unit_t)ESP32_PCNT_UNIT, ESP32_PCNT_FILTER);
	pcnt_filter_enable((pcnt_unit_t)ESP32_PCNT_UNIT);
#else
	pcnt_filter_disable((pcnt_unit_t)ESP32_PCNT_UNIT);
#endif

	pcnt_counter_pause((pcnt_unit_t)ESP32_PCNT_UNIT);
	pcnt_counter_clear((pcnt_unit_t)ESP32_PCNT_UNIT);
	pcnt_counter_resume((pcnt_unit_t)ESP32_PCNT_UNIT);
}

static int32_t read_encoder_esp32_pcnt(void)
{
	int16_t value = 0;
	int32_t position;

	if (!esp32_pcnt_encoder_ready)
	{
		return 0;
	}

	pcnt_get_counter_value((pcnt_unit_t)ESP32_PCNT_UNIT, &value);
	position = esp32_pcnt_encoder_offset + (int32_t)value;

	if (value >= ESP32_PCNT_RECENTER_THRESHOLD || value <= -ESP32_PCNT_RECENTER_THRESHOLD)
	{
		pcnt_counter_pause((pcnt_unit_t)ESP32_PCNT_UNIT);
		pcnt_counter_clear((pcnt_unit_t)ESP32_PCNT_UNIT);
		pcnt_counter_resume((pcnt_unit_t)ESP32_PCNT_UNIT);
		esp32_pcnt_encoder_offset = position;
	}

	return position;
}

#if defined(ENC0_INDEX_GPIO) && !ENC0_VIRTUAL_INDEX_ONLY
static void IRAM_ATTR enc0_index_gpio_isr(void *arg)
{
	int16_t raw = 0;
	(void)arg;
	pcnt_get_counter_value((pcnt_unit_t)ESP32_PCNT_UNIT, &raw);
	encoder_record_index_reference(ENC0, esp32_pcnt_encoder_offset + (int32_t)raw);
}

static void enc0_index_gpio_isr_init(void)
{
	gpio_set_intr_type((gpio_num_t)ENC0_INDEX_GPIO, GPIO_INTR_POSEDGE);
	gpio_isr_handler_add((gpio_num_t)ENC0_INDEX_GPIO, enc0_index_gpio_isr, NULL);
}
#endif

int32_t esp32_pcnt_read(void)
{
	return read_encoder_esp32_pcnt();
}

DECL_MODULE(esp32_pcnt_encoder)
{
	if (esp32_pcnt_encoder_ready)
	{
		return;
	}

	encoder_esp32_pcnt_init();
#if defined(ENC0_INDEX_GPIO) && !ENC0_VIRTUAL_INDEX_ONLY
	gpio_install_isr_service(0);
	enc0_index_gpio_isr_init();
#endif
	esp32_pcnt_encoder_ready = true;
}

#else

DECL_MODULE(esp32_pcnt_encoder)
{
}

#endif
#else
#warning "ESP32 PCNT driver not available on this MCU"
#endif
