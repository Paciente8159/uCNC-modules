# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About tft_display for µCNC

This module adds support for using a tft display with the LVGL library to create graphical user interfaces

## Adding tft_display to µCNC

To use the tft_display module follow these steps:

1. Copy the `tft_display` directory and place it inside the `src/modules/` directory of µCNC
2. Follow instructions to install the LVGL module.
3. Add initialization code for modules inside the `load_modules()` function:
```c
static FORCEINLINE void load_modules(void)
{
	// Load LVGL module
	LOAD_MODULE(lvgl_support);
	
	// Load TFT module
	// Must be placed after LVGL
	LOAD_MODULE(tft_display);

	/* Other modules go here... */

	// Finish LVGL init
	// Place at the end of the function
	lvgl_support_end_init();
}
```
4. Select the appropriate driver for your display inside `driver.h` and configure it as needed.
	You can also choose to enable a builtin display driver inside `lv_conf.h`, LVGL natively
	supports ST7735, ST7789, ST7796 and ILI9341 driver chips:
```c
/* Drivers for LCD devices connected via SPI/parallel port */
#define LV_USE_ST7735		0
#define LV_USE_ST7789		0
#define LV_USE_ST7796		0
#define LV_USE_ILI9341		0
```
	You'll also have to add this inside `driver.h`, instead of including a driver:
```c
// Specify display resolution
#define TFT_DISPLAY_WIDTH 320
#define TFT_DISPLAY_HEIGHT 240
// Set display rotation
#define TFT_DISPLAY_ROTATION LV_DISPLAY_ROTATION_0
// Set other optional flags
#define TFT_DISPLAY_FLAGS 0
// Use LVGL driver
#define TFT_LV_DRIVER 1
```

5. Configure communication port for the TFT display. The bare minimum are defining an SPI port (hardware or software) and
and a register select pin. Here are all the configuration options of this module:
```c
// Define the hardware SPI port to use
#define TFT_SPI_HARDWARE_PORT mcu_spi_port

// If these are defined software emulated SPI is used.
// Only clock and data out pins are needed.
#define TFT_SPI_CLK DOUT0
#define TFT_SPI_MOSI DOUT1

// Register select pin for the display.
#define TFT_RS DOUT2

// Chip select pin for the display.
#define TFT_CS DOUT3

// SPI frequency used for communicating with the display.
#define TFT_SPI_FREQ 1000000

// Size of buffer used for rendering.
#define TFT_BUFFER_SIZE 32768

// If this is set, the chip select is pulsed before every SPI transaction.
#define TFT_SYNC_CS 1

// Might be needed for some configurations. Please see tft_display.c for more info
#define TFT_CLK_SETTLE_DELAY() mcu_delay_us(1)
```

6. Make sure to define `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`