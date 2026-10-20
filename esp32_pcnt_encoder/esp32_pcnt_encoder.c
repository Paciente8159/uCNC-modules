/*
    Name: esp32_pcnt_encoder.c
    Description: ESP32 PCNT hardware counter backend for uCNC encoders.
    Author: Stanislavz(staskaaa-netizen) - https://github.com/staskaaa-netizen

    Each ESP32 PCNT instance can independently be assigned to any uCNC encoder:

        #define ESP32_PCNT_ENC0 ENC0
        #define ESP32_PCNT_ENC1 ENC3
        #define ESP32_PCNT_ENC2 ENC7

    The corresponding pulse/dir pins are automatically resolved as:

        ESP32_PCNT_ENC0 -> ENC0_PULSE / ENC0_DIR
        ESP32_PCNT_ENC1 -> ENC3_PULSE / ENC3_DIR
        ESP32_PCNT_ENC2 -> ENC7_PULSE / ENC7_DIR

    Only ENC_TYPE_CUSTOM encoders are supported by this backend.
    Index handling remains owned by encoder.c / io_control.c.
*/

#include "../../cnc.h"
#include "../encoder.h"

#include <stdint.h>
#include <stdbool.h>

#if (MCU == MCU_ESP32 || MCU == MCU_ESP32S3 || MCU == MCU_ESP32C3)

#include "driver/pcnt.h"

/* -------------------------------------------------------------------------- */
/*  Macro helpers                                                            */
/* -------------------------------------------------------------------------- */

#define esp_pcnt_pin_helper_ex(x, y) ENC##x##_##y
#define esp_pcnt_pin_helper(x, y)    esp_pcnt_pin_helper_ex(x, y)

#define esp_pcnt_type_helper_ex(x)   ENC##x##_TYPE
#define esp_pcnt_type_helper(x)      esp_pcnt_type_helper_ex(x)

#define esp_pcnt_read_helper_ex(x)   enc_custom_read_enc##x
#define esp_pcnt_read_helper(x)      esp_pcnt_read_helper_ex(x)

/* -------------------------------------------------------------------------- */
/*  PCNT encoder assignments                                                 */
/* -------------------------------------------------------------------------- */

/*
 * ESP32_PCNT_ENC0..ENC7 are independent.
 *
 * Example:
 *
 *     #define ESP32_PCNT_ENC0 ENC0
 *     #define ESP32_PCNT_ENC1 ENC4
 *
 * The pulse and direction pins are automatically resolved from the selected
 * uCNC encoder.
 */

#ifdef ESP32_PCNT_ENC0

#define ESP32_PCNT0_PULSE \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC0, PULSE)

#define ESP32_PCNT0_DIR \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC0, DIR)

#define ESP32_PCNT0_READ \
    esp_pcnt_read_helper(ESP32_PCNT_ENC0)

#if (esp_pcnt_type_helper(ESP32_PCNT_ENC0) != ENC_TYPE_CUSTOM)
#error "ESP32_PCNT_ENC0 must select an ENC_TYPE_CUSTOM encoder"
#endif

#if !ASSERT_PIN(ESP32_PCNT0_PULSE)
#error "The selected ESP32_PCNT_ENC0 pulse pin is not defined"
#endif

#if !ASSERT_PIN(ESP32_PCNT0_DIR)
#error "The selected ESP32_PCNT_ENC0 dir pin is not defined"
#endif

extern int32_t ESP32_PCNT0_READ(void);

#endif /* ESP32_PCNT_ENC0 */


#ifdef ESP32_PCNT_ENC1

#define ESP32_PCNT1_PULSE \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC1, PULSE)

#define ESP32_PCNT1_DIR \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC1, DIR)

#define ESP32_PCNT1_READ \
    esp_pcnt_read_helper(ESP32_PCNT_ENC1)

#if (esp_pcnt_type_helper(ESP32_PCNT_ENC1) != ENC_TYPE_CUSTOM)
#error "ESP32_PCNT_ENC1 must select an ENC_TYPE_CUSTOM encoder"
#endif

#if !ASSERT_PIN(ESP32_PCNT1_PULSE)
#error "The selected ESP32_PCNT_ENC1 pulse pin is not defined"
#endif

#if !ASSERT_PIN(ESP32_PCNT1_DIR)
#error "The selected ESP32_PCNT_ENC1 dir pin is not defined"
#endif

extern int32_t ESP32_PCNT1_READ(void);

#endif /* ESP32_PCNT_ENC1 */


#ifdef ESP32_PCNT_ENC2

#define ESP32_PCNT2_PULSE \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC2, PULSE)

#define ESP32_PCNT2_DIR \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC2, DIR)

#define ESP32_PCNT2_READ \
    esp_pcnt_read_helper(ESP32_PCNT_ENC2)

#if (esp_pcnt_type_helper(ESP32_PCNT_ENC2) != ENC_TYPE_CUSTOM)
#error "ESP32_PCNT_ENC2 must select an ENC_TYPE_CUSTOM encoder"
#endif

#if !ASSERT_PIN(ESP32_PCNT2_PULSE)
#error "The selected ESP32_PCNT_ENC2 pulse pin is not defined"
#endif

#if !ASSERT_PIN(ESP32_PCNT2_DIR)
#error "The selected ESP32_PCNT_ENC2 dir pin is not defined"
#endif

extern int32_t ESP32_PCNT2_READ(void);

#endif /* ESP32_PCNT_ENC2 */


#ifdef ESP32_PCNT_ENC3

#define ESP32_PCNT3_PULSE \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC3, PULSE)

#define ESP32_PCNT3_DIR \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC3, DIR)

#define ESP32_PCNT3_READ \
    esp_pcnt_read_helper(ESP32_PCNT_ENC3)

#if (esp_pcnt_type_helper(ESP32_PCNT_ENC3) != ENC_TYPE_CUSTOM)
#error "ESP32_PCNT_ENC3 must select an ENC_TYPE_CUSTOM encoder"
#endif

#if !ASSERT_PIN(ESP32_PCNT3_PULSE)
#error "The selected ESP32_PCNT_ENC3 pulse pin is not defined"
#endif

#if !ASSERT_PIN(ESP32_PCNT3_DIR)
#error "The selected ESP32_PCNT_ENC3 dir pin is not defined"
#endif

extern int32_t ESP32_PCNT3_READ(void);

#endif /* ESP32_PCNT_ENC3 */


#ifdef ESP32_PCNT_ENC4

#define ESP32_PCNT4_PULSE \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC4, PULSE)

#define ESP32_PCNT4_DIR \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC4, DIR)

#define ESP32_PCNT4_READ \
    esp_pcnt_read_helper(ESP32_PCNT_ENC4)

#if (esp_pcnt_type_helper(ESP32_PCNT_ENC4) != ENC_TYPE_CUSTOM)
#error "ESP32_PCNT_ENC4 must select an ENC_TYPE_CUSTOM encoder"
#endif

#if !ASSERT_PIN(ESP32_PCNT4_PULSE)
#error "The selected ESP32_PCNT_ENC4 pulse pin is not defined"
#endif

#if !ASSERT_PIN(ESP32_PCNT4_DIR)
#error "The selected ESP32_PCNT_ENC4 dir pin is not defined"
#endif

extern int32_t ESP32_PCNT4_READ(void);

#endif /* ESP32_PCNT_ENC4 */


#ifdef ESP32_PCNT_ENC5

#define ESP32_PCNT5_PULSE \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC5, PULSE)

#define ESP32_PCNT5_DIR \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC5, DIR)

#define ESP32_PCNT5_READ \
    esp_pcnt_read_helper(ESP32_PCNT_ENC5)

#if (esp_pcnt_type_helper(ESP32_PCNT_ENC5) != ENC_TYPE_CUSTOM)
#error "ESP32_PCNT_ENC5 must select an ENC_TYPE_CUSTOM encoder"
#endif

#if !ASSERT_PIN(ESP32_PCNT5_PULSE)
#error "The selected ESP32_PCNT_ENC5 pulse pin is not defined"
#endif

#if !ASSERT_PIN(ESP32_PCNT5_DIR)
#error "The selected ESP32_PCNT_ENC5 dir pin is not defined"
#endif

extern int32_t ESP32_PCNT5_READ(void);

#endif /* ESP32_PCNT_ENC5 */


#ifdef ESP32_PCNT_ENC6

#define ESP32_PCNT6_PULSE \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC6, PULSE)

#define ESP32_PCNT6_DIR \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC6, DIR)

#define ESP32_PCNT6_READ \
    esp_pcnt_read_helper(ESP32_PCNT_ENC6)

#if (esp_pcnt_type_helper(ESP32_PCNT_ENC6) != ENC_TYPE_CUSTOM)
#error "ESP32_PCNT_ENC6 must select an ENC_TYPE_CUSTOM encoder"
#endif

#if !ASSERT_PIN(ESP32_PCNT6_PULSE)
#error "The selected ESP32_PCNT_ENC6 pulse pin is not defined"
#endif

#if !ASSERT_PIN(ESP32_PCNT6_DIR)
#error "The selected ESP32_PCNT_ENC6 dir pin is not defined"
#endif

extern int32_t ESP32_PCNT6_READ(void);

#endif /* ESP32_PCNT_ENC6 */


#ifdef ESP32_PCNT_ENC7

#define ESP32_PCNT7_PULSE \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC7, PULSE)

#define ESP32_PCNT7_DIR \
    esp_pcnt_pin_helper(ESP32_PCNT_ENC7, DIR)

#define ESP32_PCNT7_READ \
    esp_pcnt_read_helper(ESP32_PCNT_ENC7)

#if (esp_pcnt_type_helper(ESP32_PCNT_ENC7) != ENC_TYPE_CUSTOM)
#error "ESP32_PCNT_ENC7 must select an ENC_TYPE_CUSTOM encoder"
#endif

#if !ASSERT_PIN(ESP32_PCNT7_PULSE)
#error "The selected ESP32_PCNT_ENC7 pulse pin is not defined"
#endif

#if !ASSERT_PIN(ESP32_PCNT7_DIR)
#error "The selected ESP32_PCNT_ENC7 dir pin is not defined"
#endif

extern int32_t ESP32_PCNT7_READ(void);

#endif /* ESP32_PCNT_ENC7 */


/* -------------------------------------------------------------------------- */
/*  PCNT unit assignment                                                     */
/* -------------------------------------------------------------------------- */

#ifndef ESP32_PCNT0_UNIT
#define ESP32_PCNT0_UNIT PCNT_UNIT_0
#endif

#ifndef ESP32_PCNT1_UNIT
#define ESP32_PCNT1_UNIT PCNT_UNIT_1
#endif

#ifndef ESP32_PCNT2_UNIT
#define ESP32_PCNT2_UNIT PCNT_UNIT_2
#endif

#ifndef ESP32_PCNT3_UNIT
#define ESP32_PCNT3_UNIT PCNT_UNIT_3
#endif

#ifndef ESP32_PCNT4_UNIT
#define ESP32_PCNT4_UNIT PCNT_UNIT_4
#endif

#ifndef ESP32_PCNT5_UNIT
#define ESP32_PCNT5_UNIT PCNT_UNIT_5
#endif

#ifndef ESP32_PCNT6_UNIT
#define ESP32_PCNT6_UNIT PCNT_UNIT_6
#endif

#ifndef ESP32_PCNT7_UNIT
#define ESP32_PCNT7_UNIT PCNT_UNIT_7
#endif


/* -------------------------------------------------------------------------- */
/*  PCNT channel assignment                                                  */
/* -------------------------------------------------------------------------- */

#ifndef ESP32_PCNT_CHANNEL_A
#define ESP32_PCNT_CHANNEL_A PCNT_CHANNEL_0
#endif

#ifndef ESP32_PCNT_CHANNEL_B
#define ESP32_PCNT_CHANNEL_B PCNT_CHANNEL_1
#endif


/* -------------------------------------------------------------------------- */
/*  PCNT configuration                                                       */
/* -------------------------------------------------------------------------- */

#ifndef ESP32_PCNT_RECENTER_THRESHOLD
#define ESP32_PCNT_RECENTER_THRESHOLD 20000
#endif

#if (ESP32_PCNT_RECENTER_THRESHOLD > 32767)
#error "ESP32_PCNT_RECENTER_THRESHOLD must be <= 32767"
#endif

#ifndef ESP32_PCNT_FILTER
#define ESP32_PCNT_FILTER 0
#endif

/* When pulse and direction resolve to the same GPIO, use PCNT as a simple
 * one-edge pulse counter instead of configuring the two quadrature channels.
 * The falling edge is counted by default; either mode may be overridden by
 * the board configuration if the opposite edge is required. */
#ifndef ESP32_PCNT_SINGLE_POS_MODE
#define ESP32_PCNT_SINGLE_POS_MODE PCNT_COUNT_DIS
#endif

#ifndef ESP32_PCNT_SINGLE_NEG_MODE
#define ESP32_PCNT_SINGLE_NEG_MODE PCNT_COUNT_INC
#endif


/* -------------------------------------------------------------------------- */
/*  Per-PCNT state                                                           */
/* -------------------------------------------------------------------------- */

#ifdef ESP32_PCNT_ENC0
static int32_t esp32_pcnt0_offset;
static bool    esp32_pcnt0_ready;
#endif

#ifdef ESP32_PCNT_ENC1
static int32_t esp32_pcnt1_offset;
static bool    esp32_pcnt1_ready;
#endif

#ifdef ESP32_PCNT_ENC2
static int32_t esp32_pcnt2_offset;
static bool    esp32_pcnt2_ready;
#endif

#ifdef ESP32_PCNT_ENC3
static int32_t esp32_pcnt3_offset;
static bool    esp32_pcnt3_ready;
#endif

#ifdef ESP32_PCNT_ENC4
static int32_t esp32_pcnt4_offset;
static bool    esp32_pcnt4_ready;
#endif

#ifdef ESP32_PCNT_ENC5
static int32_t esp32_pcnt5_offset;
static bool    esp32_pcnt5_ready;
#endif

#ifdef ESP32_PCNT_ENC6
static int32_t esp32_pcnt6_offset;
static bool    esp32_pcnt6_ready;
#endif

#ifdef ESP32_PCNT_ENC7
static int32_t esp32_pcnt7_offset;
static bool    esp32_pcnt7_ready;
#endif

/* -------------------------------------------------------------------------- */
/*  PCNT unit configuration helper                                            */
/* -------------------------------------------------------------------------- */

#define ESP32_PCNT_CONFIGURE(unit_, pulse_, dir_, encoder_)                  \
    do                                                                       \
    {                                                                        \
        if ((pulse_) == (dir_))                                              \
        {                                                                    \
            pcnt_config_t single = {                                         \
                .pulse_gpio_num = (pulse_),                                  \
                .ctrl_gpio_num  = PCNT_PIN_NOT_USED,                         \
                .lctrl_mode     = PCNT_MODE_KEEP,                            \
                .hctrl_mode     = PCNT_MODE_KEEP,                            \
                .pos_mode       = ESP32_PCNT_SINGLE_POS_MODE,                \
                .neg_mode       = ESP32_PCNT_SINGLE_NEG_MODE,                \
                .counter_h_lim  = 32767,                                     \
                .counter_l_lim  = -32768,                                    \
                .unit           = (unit_),                                   \
                .channel        = ESP32_PCNT_CHANNEL_A,                      \
            };                                                               \
            pcnt_unit_config(&single);                                       \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            pcnt_config_t ch_a = {                                           \
                .pulse_gpio_num = (pulse_),                                  \
                .ctrl_gpio_num  = (dir_),                                    \
                .lctrl_mode     = PCNT_MODE_REVERSE,                         \
                .hctrl_mode     = PCNT_MODE_KEEP,                            \
                .pos_mode       = PCNT_COUNT_INC,                            \
                .neg_mode       = PCNT_COUNT_DEC,                            \
                .counter_h_lim  = 32767,                                     \
                .counter_l_lim  = -32768,                                    \
                .unit           = (unit_),                                   \
                .channel        = ESP32_PCNT_CHANNEL_A,                      \
            };                                                               \
                                                                               \
            pcnt_config_t ch_b = {                                           \
                .pulse_gpio_num = (dir_),                                    \
                .ctrl_gpio_num  = (pulse_),                                  \
                .lctrl_mode     = PCNT_MODE_KEEP,                            \
                .hctrl_mode     = PCNT_MODE_REVERSE,                         \
                .pos_mode       = PCNT_COUNT_INC,                            \
                .neg_mode       = PCNT_COUNT_DEC,                            \
                .counter_h_lim  = 32767,                                     \
                .counter_l_lim  = -32768,                                    \
                .unit           = (unit_),                                   \
                .channel        = ESP32_PCNT_CHANNEL_B,                      \
            };                                                               \
                                                                               \
            pcnt_unit_config(&ch_a);                                         \
            pcnt_unit_config(&ch_b);                                         \
        }                                                                    \
                                                                               \
        /* PCNT input filtering is intentionally owned here. */             \
        /* GPIO mode/pulls remain owned by the uCNC IO subsystem. */         \
        /* No GPIO interrupt or index handling is installed here. */        \
        if (ESP32_PCNT_FILTER)                                               \
        {                                                                    \
            pcnt_set_filter_value((unit_), ESP32_PCNT_FILTER);               \
            pcnt_filter_enable((unit_));                                     \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            pcnt_filter_disable((unit_));                                    \
        }                                                                    \
                                                                               \
        pcnt_counter_pause((unit_));                                        \
        pcnt_counter_clear((unit_));                                        \
        pcnt_counter_resume((unit_));                                       \
                                                                               \
        (encoder_) = 0;                                                      \
    } while (0)


/* -------------------------------------------------------------------------- */
/*  PCNT initialization                                                       */
/* -------------------------------------------------------------------------- */

static void esp32_pcnt_encoder_config(void)
{
#ifdef ESP32_PCNT_ENC0
    ESP32_PCNT_CONFIGURE(
        ESP32_PCNT0_UNIT,
        __indirect__(ESP32_PCNT0_PULSE, BIT),
        __indirect__(ESP32_PCNT0_DIR, BIT),
        esp32_pcnt0_offset);

    esp32_pcnt0_ready = true;
#endif

#ifdef ESP32_PCNT_ENC1
    ESP32_PCNT_CONFIGURE(
        ESP32_PCNT1_UNIT,
        __indirect__(ESP32_PCNT1_PULSE, BIT),
        __indirect__(ESP32_PCNT1_DIR, BIT),
        esp32_pcnt1_offset);

    esp32_pcnt1_ready = true;
#endif

#ifdef ESP32_PCNT_ENC2
    ESP32_PCNT_CONFIGURE(
        ESP32_PCNT2_UNIT,
        __indirect__(ESP32_PCNT2_PULSE, BIT),
        __indirect__(ESP32_PCNT2_DIR, BIT),
        esp32_pcnt2_offset);

    esp32_pcnt2_ready = true;
#endif

#ifdef ESP32_PCNT_ENC3
    ESP32_PCNT_CONFIGURE(
        ESP32_PCNT3_UNIT,
        __indirect__(ESP32_PCNT3_PULSE, BIT),
        __indirect__(ESP32_PCNT3_DIR, BIT),
        esp32_pcnt3_offset);

    esp32_pcnt3_ready = true;
#endif

#ifdef ESP32_PCNT_ENC4
    ESP32_PCNT_CONFIGURE(
        ESP32_PCNT4_UNIT,
        __indirect__(ESP32_PCNT4_PULSE, BIT),
        __indirect__(ESP32_PCNT4_DIR, BIT),
        esp32_pcnt4_offset);

    esp32_pcnt4_ready = true;
#endif

#ifdef ESP32_PCNT_ENC5
    ESP32_PCNT_CONFIGURE(
        ESP32_PCNT5_UNIT,
        __indirect__(ESP32_PCNT5_PULSE, BIT),
        __indirect__(ESP32_PCNT5_DIR, BIT),
        esp32_pcnt5_offset);

    esp32_pcnt5_ready = true;
#endif

#ifdef ESP32_PCNT_ENC6
    ESP32_PCNT_CONFIGURE(
        ESP32_PCNT6_UNIT,
        __indirect__(ESP32_PCNT6_PULSE, BIT),
        __indirect__(ESP32_PCNT6_DIR, BIT),
        esp32_pcnt6_offset);

    esp32_pcnt6_ready = true;
#endif

#ifdef ESP32_PCNT_ENC7
    ESP32_PCNT_CONFIGURE(
        ESP32_PCNT7_UNIT,
        __indirect__(ESP32_PCNT7_PULSE, BIT),
        __indirect__(ESP32_PCNT7_DIR, BIT),
        esp32_pcnt7_offset);

    esp32_pcnt7_ready = true;
#endif
}


/* -------------------------------------------------------------------------- */
/*  PCNT read helper                                                          */
/* -------------------------------------------------------------------------- */

#ifdef ESP32_PCNT_ENC0

int32_t enc_custom_read_enc0(void)
{
    int16_t raw;
    int32_t position;

    if (!esp32_pcnt0_ready)
    {
        return 0;
    }

    pcnt_get_counter_value(ESP32_PCNT0_UNIT, &raw);

    position = esp32_pcnt0_offset + (int32_t)raw;

    if ((raw >= ESP32_PCNT_RECENTER_THRESHOLD) ||
        (raw <= -ESP32_PCNT_RECENTER_THRESHOLD))
    {
        pcnt_counter_pause(ESP32_PCNT0_UNIT);
        pcnt_counter_clear(ESP32_PCNT0_UNIT);
        pcnt_counter_resume(ESP32_PCNT0_UNIT);

        esp32_pcnt0_offset = position;
    }

    return position;
}

#endif


#ifdef ESP32_PCNT_ENC1

int32_t enc_custom_read_enc1(void)
{
    int16_t raw;
    int32_t position;

    if (!esp32_pcnt1_ready)
    {
        return 0;
    }

    pcnt_get_counter_value(ESP32_PCNT1_UNIT, &raw);

    position = esp32_pcnt1_offset + (int32_t)raw;

    if ((raw >= ESP32_PCNT_RECENTER_THRESHOLD) ||
        (raw <= -ESP32_PCNT_RECENTER_THRESHOLD))
    {
        pcnt_counter_pause(ESP32_PCNT1_UNIT);
        pcnt_counter_clear(ESP32_PCNT1_UNIT);
        pcnt_counter_resume(ESP32_PCNT1_UNIT);

        esp32_pcnt1_offset = position;
    }

    return position;
}

#endif


#ifdef ESP32_PCNT_ENC2

int32_t enc_custom_read_enc2(void)
{
    int16_t raw;
    int32_t position;

    if (!esp32_pcnt2_ready)
    {
        return 0;
    }

    pcnt_get_counter_value(ESP32_PCNT2_UNIT, &raw);

    position = esp32_pcnt2_offset + (int32_t)raw;

    if ((raw >= ESP32_PCNT_RECENTER_THRESHOLD) ||
        (raw <= -ESP32_PCNT_RECENTER_THRESHOLD))
    {
        pcnt_counter_pause(ESP32_PCNT2_UNIT);
        pcnt_counter_clear(ESP32_PCNT2_UNIT);
        pcnt_counter_resume(ESP32_PCNT2_UNIT);

        esp32_pcnt2_offset = position;
    }

    return position;
}

#endif


#ifdef ESP32_PCNT_ENC3

int32_t enc_custom_read_enc3(void)
{
    int16_t raw;
    int32_t position;

    if (!esp32_pcnt3_ready)
    {
        return 0;
    }

    pcnt_get_counter_value(ESP32_PCNT3_UNIT, &raw);

    position = esp32_pcnt3_offset + (int32_t)raw;

    if ((raw >= ESP32_PCNT_RECENTER_THRESHOLD) ||
        (raw <= -ESP32_PCNT_RECENTER_THRESHOLD))
    {
        pcnt_counter_pause(ESP32_PCNT3_UNIT);
        pcnt_counter_clear(ESP32_PCNT3_UNIT);
        pcnt_counter_resume(ESP32_PCNT3_UNIT);

        esp32_pcnt3_offset = position;
    }

    return position;
}

#endif


#ifdef ESP32_PCNT_ENC4

int32_t enc_custom_read_enc4(void)
{
    int16_t raw;
    int32_t position;

    if (!esp32_pcnt4_ready)
    {
        return 0;
    }

    pcnt_get_counter_value(ESP32_PCNT4_UNIT, &raw);

    position = esp32_pcnt4_offset + (int32_t)raw;

    if ((raw >= ESP32_PCNT_RECENTER_THRESHOLD) ||
        (raw <= -ESP32_PCNT_RECENTER_THRESHOLD))
    {
        pcnt_counter_pause(ESP32_PCNT4_UNIT);
        pcnt_counter_clear(ESP32_PCNT4_UNIT);
        pcnt_counter_resume(ESP32_PCNT4_UNIT);

        esp32_pcnt4_offset = position;
    }

    return position;
}

#endif


#ifdef ESP32_PCNT_ENC5

int32_t enc_custom_read_enc5(void)
{
    int16_t raw;
    int32_t position;

    if (!esp32_pcnt5_ready)
    {
        return 0;
    }

    pcnt_get_counter_value(ESP32_PCNT5_UNIT, &raw);

    position = esp32_pcnt5_offset + (int32_t)raw;

    if ((raw >= ESP32_PCNT_RECENTER_THRESHOLD) ||
        (raw <= -ESP32_PCNT_RECENTER_THRESHOLD))
    {
        pcnt_counter_pause(ESP32_PCNT5_UNIT);
        pcnt_counter_clear(ESP32_PCNT5_UNIT);
        pcnt_counter_resume(ESP32_PCNT5_UNIT);

        esp32_pcnt5_offset = position;
    }

    return position;
}

#endif


#ifdef ESP32_PCNT_ENC6

int32_t enc_custom_read_enc6(void)
{
    int16_t raw;
    int32_t position;

    if (!esp32_pcnt6_ready)
    {
        return 0;
    }

    pcnt_get_counter_value(ESP32_PCNT6_UNIT, &raw);

    position = esp32_pcnt6_offset + (int32_t)raw;

    if ((raw >= ESP32_PCNT_RECENTER_THRESHOLD) ||
        (raw <= -ESP32_PCNT_RECENTER_THRESHOLD))
    {
        pcnt_counter_pause(ESP32_PCNT6_UNIT);
        pcnt_counter_clear(ESP32_PCNT6_UNIT);
        pcnt_counter_resume(ESP32_PCNT6_UNIT);

        esp32_pcnt6_offset = position;
    }

    return position;
}

#endif


#ifdef ESP32_PCNT_ENC7

int32_t enc_custom_read_enc7(void)
{
    int16_t raw;
    int32_t position;

    if (!esp32_pcnt7_ready)
    {
        return 0;
    }

    pcnt_get_counter_value(ESP32_PCNT7_UNIT, &raw);

    position = esp32_pcnt7_offset + (int32_t)raw;

    if ((raw >= ESP32_PCNT_RECENTER_THRESHOLD) ||
        (raw <= -ESP32_PCNT_RECENTER_THRESHOLD))
    {
        pcnt_counter_pause(ESP32_PCNT7_UNIT);
        pcnt_counter_clear(ESP32_PCNT7_UNIT);
        pcnt_counter_resume(ESP32_PCNT7_UNIT);

        esp32_pcnt7_offset = position;
    }

    return position;
}

#endif


/* -------------------------------------------------------------------------- */
/*  Module initialization                                                     */
/* -------------------------------------------------------------------------- */

DECL_MODULE(esp32_pcnt_encoder)
{
#if defined(ESP32_PCNT_ENC0) || \
    defined(ESP32_PCNT_ENC1) || \
    defined(ESP32_PCNT_ENC2) || \
    defined(ESP32_PCNT_ENC3) || \
    defined(ESP32_PCNT_ENC4) || \
    defined(ESP32_PCNT_ENC5) || \
    defined(ESP32_PCNT_ENC6) || \
    defined(ESP32_PCNT_ENC7)

    esp32_pcnt_encoder_config();

#endif
}

#else

#warning "ESP32 PCNT driver not available on this MCU"

DECL_MODULE(esp32_pcnt_encoder)
{
}

#endif
