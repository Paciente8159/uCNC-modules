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
	// Load TFT module
	// Must be placed before LVGL
	LOAD_MODULE(tft_display);

	// Load LVGL module
	LOAD_MODULE(lvgl_support);

	/* Other modules go here... */
}
```
4. Select the appropriate driver for your display inside `driver.h` and configure it as needed.
	Make sure that you only ever include one driver at a time.
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
	Remember to remove any other included drivers when using LVGL drivers.

5. Configure communication port for the TFT display. The bare minimum are defining an SPI port (hardware or software) and
and a register select pin. Here are all the configuration options of this module:
```c
// Define the hardware SPI port (1 or 2) to use
#define TFT_SPI_HARDWARE_PORT 1

// If these are defined software emulated SPI is used.
// Only clock and data out pins are needed.
#define TFT_SPI_CLK DOUT0
#define TFT_SPI_MOSI DOUT1

// Register select pin for the display.
#define TFT_RS DOUT2

// Chip select pin for the display.
#define TFT_CS DOUT3

// Define a hardware reset pin for the LCD.
#define TFT_RESET UNDEF_PIN

// Define a pin to control the LCD backlight
#define TFT_BACKLIGHT UNDEF_PIN

// SPI frequency used for communicating with the display.
#define TFT_SPI_FREQ 1000000

// Size of buffer used for rendering.
#define TFT_BUFFER_SIZE 32768

// If this is set, the chip select is pulsed before every SPI transaction.
#define TFT_SYNC_CS 1

// Might be needed for some configurations. Please see tft_display.c for more info
#define TFT_CLK_SETTLE_DELAY() mcu_delay_us(1)

//you can also enable touch support by defining the touch CS and touch detect pins
// this requires you to to include the touch_screen module inside the modules directory
#define TFT_TOUCH_CS DOUT35
#define TFT_TOUCH_DETECT DIN35
```

6. Make sure to define `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`
