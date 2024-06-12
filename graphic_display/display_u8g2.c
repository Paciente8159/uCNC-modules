#include "../../cnc.h"
#include "display_u8g2.h"

#if (GRAPHIC_DISPLAY_INTERFACE & (GRAPHIC_DISPLAY_SW_SPI | GRAPHIC_DISPLAY_HW_SPI))

#include "../softspi.h"
static softspi_port_t *graphic_port;

#if (GRAPHIC_DISPLAY_INTERFACE == GRAPHIC_DISPLAY_SW_SPI)
#ifndef GRAPHIC_DISPLAY_SPI_CLOCK
#define GRAPHIC_DISPLAY_SPI_CLOCK DOUT4
#endif
#ifndef GRAPHIC_DISPLAY_SPI_DATA
#define GRAPHIC_DISPLAY_SPI_DATA DOUT5
#endif
SOFTSPI(graphic_spi, 1000000UL, 0, GRAPHIC_DISPLAY_SPI_DATA, GRAPHIC_DISPLAY_SPI_DATA, GRAPHIC_DISPLAY_SPI_CLOCK)
#endif

#ifndef GRAPHIC_DISPLAY_SPI_CS
#define GRAPHIC_DISPLAY_SPI_CS DOUT6
#endif
uint8_t u8x8_byte_ucnc_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	cnc_dotasks();

	uint8_t *data;
	switch (msg)
	{
	case U8X8_MSG_BYTE_SEND:
		data = (uint8_t *)arg_ptr;
		while (arg_int > 0)
		{
			softspi_xmit(graphic_port, (uint8_t)*data);
			data++;
			arg_int--;
			cnc_dotasks();
		}
		break;
	case U8X8_MSG_BYTE_INIT:
		mcu_set_output(GRAPHIC_DISPLAY_SPI_CS);
		break;
	case U8X8_MSG_BYTE_SET_DC:
		u8x8_gpio_SetDC(u8x8, arg_int);
		break;
	case U8X8_MSG_BYTE_START_TRANSFER:
		softspi_config(graphic_port, u8x8->display_info->spi_mode, u8x8->display_info->sck_clock_hz);
		/* SPI mode has to be mapped to the mode of the current controller, at least Uno, Due, 101 have different SPI_MODEx values */
		u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
		u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->post_chip_enable_wait_ns, NULL);
		break;
	case U8X8_MSG_BYTE_END_TRANSFER:
		u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL);
		u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
		break;
	default:
		return 0;
	}
	return 1;
}
#endif

#if (GRAPHIC_DISPLAY_INTERFACE & (GRAPHIC_DISPLAY_SW_I2C | GRAPHIC_DISPLAY_HW_I2C))
#include "../softi2c.h"
static softi2c_port_t *graphic_port;

#if (GRAPHIC_DISPLAY_INTERFACE == GRAPHIC_DISPLAY_SW_I2C)
#ifndef GRAPHIC_DISPLAY_I2C_CLOCK
#define GRAPHIC_DISPLAY_I2C_CLOCK DIN30
#endif
#ifndef GRAPHIC_DISPLAY_I2C_DATA
#define GRAPHIC_DISPLAY_I2C_DATA DIN31
#endif
SOFTI2C(graphic_i2c, 100000UL, GRAPHIC_DISPLAY_I2C_CLOCK, GRAPHIC_DISPLAY_I2C_DATA)
#endif
uint8_t u8x8_byte_ucnc_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	static uint8_t i2c_buffer[32]; /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
	static uint8_t i2c_buffer_offset = 0;

	switch (msg)
	{
	case U8X8_MSG_BYTE_SEND:
		memcpy(&i2c_buffer[i2c_buffer_offset], arg_ptr, arg_int);
		i2c_buffer_offset += arg_int;
		break;
	case U8X8_MSG_BYTE_INIT:
		/* add your custom code to init i2c subsystem */
		break;
	case U8X8_MSG_BYTE_SET_DC:
		/* ignored for i2c */
		break;
	case U8X8_MSG_BYTE_START_TRANSFER:
		i2c_buffer_offset = 0;
		break;
	case U8X8_MSG_BYTE_END_TRANSFER:
#if (UCNC_MODULE_VERSION < 10808)
		softi2c_send(graphic_port, u8x8_GetI2CAddress(u8x8) >> 1, i2c_buffer, i2c_buffer_offset, true);
#else
		softi2c_send(graphic_port, u8x8_GetI2CAddress(u8x8) >> 1, i2c_buffer, i2c_buffer_offset, true, 20);
#endif
		i2c_buffer_offset = 0;
		break;
	default:
		return 0;
	}

	return 1;
}
#endif

uint8_t u8x8_gpio_and_delay_ucnc(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	cnc_dotasks();

	switch (msg)
	{
	case U8X8_MSG_GPIO_AND_DELAY_INIT: // called once during init phase of u8g2/u8x8
		break;													 // can be used to setup pins
	case U8X8_MSG_DELAY_NANO:					 // delay arg_int * 1 nano second
		while (arg_int--)
			;
		break;
	case U8X8_MSG_DELAY_100NANO: // delay arg_int * 100 nano seconds
		while (arg_int--)
			mcu_delay_100ns();
		break;
	case U8X8_MSG_DELAY_10MICRO: // delay arg_int * 10 micro seconds
		while (arg_int--)
			mcu_delay_us(10);
		break;
	case U8X8_MSG_DELAY_MILLI: // delay arg_int * 1 milli second
		cnc_delay_ms(arg_int);
		break;
	case U8X8_MSG_DELAY_I2C: // arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
		if (arg_int == 1)
		{
			mcu_delay_us(5);
		}
		else
		{
			mcu_delay_us(1);
		}
		break;							 // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
	case U8X8_MSG_GPIO_D0: // D0 or SPI clock pin: Output level in arg_int
#if GRAPHIC_DISPLAY_SPI_CLOCK != UNDEF_PIN
		io_set_pinvalue(GRAPHIC_DISPLAY_SPI_CLOCK, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D1: // D1 or SPI data pin: Output level in arg_int
#if GRAPHIC_DISPLAY_SPI_DATA != UNDEF_PIN
		io_set_pinvalue(GRAPHIC_DISPLAY_SPI_DATA, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D2: // D2 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D2_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_D2_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D3: // D3 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D3_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_D3_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D4: // D4 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D4_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_D4_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D5: // D5 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D5_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_D5_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D6: // D6 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D6_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_D6_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D7: // D7 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D7_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_D7_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_E: // E/WR pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_E_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_E_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_CS: // CS (chip select) pin: Output level in arg_int
#if GRAPHIC_DISPLAY_SPI_CS != UNDEF_PIN
		io_set_pinvalue(GRAPHIC_DISPLAY_SPI_CS, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_DC: // DC (data/cmd, A0, register select) pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_DC_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_DC_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_RESET: // Reset pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_RESET_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_RESET_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_CS1: // CS1 (chip select) pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_CS1_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_CS1_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_CS2: // CS2 (chip select) pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_CS2_PIN
		io_set_pinvalue(U8X8_MSG_GPIO_CS2_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_I2C_CLOCK: // arg_int=0: Output low at I2C clock pin
#if GRAPHIC_DISPLAY_I2C_CLOCK != UNDEF_PIN
		if (arg_int)
		{
			mcu_config_input(GRAPHIC_DISPLAY_I2C_CLOCK);
			mcu_config_pullup(GRAPHIC_DISPLAY_I2C_CLOCK);
			u8x8_SetGPIOResult(u8x8, mcu_get_input(GRAPHIC_DISPLAY_I2C_CLOCK));
		}
		else
		{
			mcu_config_output(GRAPHIC_DISPLAY_I2C_CLOCK);
			mcu_clear_output(GRAPHIC_DISPLAY_I2C_CLOCK);
			u8x8_SetGPIOResult(u8x8, 0);
		}
#endif
		break;										 // arg_int=1: Input dir with pullup high for I2C clock pin
	case U8X8_MSG_GPIO_I2C_DATA: // arg_int=0: Output low at I2C data pin
#if GRAPHIC_DISPLAY_I2C_DATA != UNDEF_PIN
		if (arg_int)
		{
			mcu_config_input(GRAPHIC_DISPLAY_I2C_DATA);
			mcu_config_pullup(GRAPHIC_DISPLAY_I2C_DATA);
			u8x8_SetGPIOResult(u8x8, mcu_get_input(GRAPHIC_DISPLAY_I2C_DATA));
		}
		else
		{
			mcu_config_output(GRAPHIC_DISPLAY_I2C_DATA);
			mcu_clear_output(GRAPHIC_DISPLAY_I2C_DATA);
			u8x8_SetGPIOResult(u8x8, 0);
		}
#endif
		break; // arg_int=1: Input dir with pullup high for I2C data pin
	case U8X8_MSG_GPIO_MENU_SELECT:
#ifdef U8X8_MSG_GPIO_MENU_SELECT_PIN
		u8x8_SetGPIOResult(u8x8, io_get_pinvalue(U8X8_MSG_GPIO_MENU_SELECT_PIN));
#else
		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
#endif
		break;
	case U8X8_MSG_GPIO_MENU_NEXT:
#ifdef U8X8_MSG_GPIO_MENU_NEXT_PIN
		u8x8_SetGPIOResult(u8x8, io_get_pinvalue(U8X8_MSG_GPIO_MENU_NEXT_PIN));
#else
		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
#endif
		break;
	case U8X8_MSG_GPIO_MENU_PREV:
#ifdef U8X8_MSG_GPIO_MENU_PREV_PIN
		u8x8_SetGPIOResult(u8x8, io_get_pinvalue(U8X8_MSG_GPIO_MENU_PREV_PIN));
#else
		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
#endif
		break;
	case U8X8_MSG_GPIO_MENU_HOME:
#ifdef U8X8_MSG_GPIO_MENU_HOME_PIN
		u8x8_SetGPIOResult(u8x8, io_get_pinvalue(U8X8_MSG_GPIO_MENU_HOME_PIN));
#else
		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
#endif
		break;
	default:
		u8x8_SetGPIOResult(u8x8, 1); // default return value
		break;
	}
	return 1;
}
