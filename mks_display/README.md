# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About mks_display for µCNC

This module adds support for using a MKS TS35 display with the LVGL library to create graphical user interfaces.
This module is not limited to the MKS TS35 and might work for other displays and drivers with the correct configuration

## Adding mks_display to µCNC

MKS display module already includes the lvgl library so you don't need to include anything external library.
To use the mks_display module follow these steps:

1. Copy the `mks_display` directory and place it inside the `src/modules/` directory of µCNC. You will need to include the `touch_screen` module to allow using the touch sensor of the screen.
2. Add the LVGL library to `platformio.ini`:
```
lib_deps = 
	lvgl/lvgl@^9.1.0
```
3. Also inside the board ini file on `build_flags` add a build flag to set the lv configuration with the path of `lv_conf.h` file like `-DLV_CONF_PATH="/src/modules/mks_display/lv_conf.h"`:
4. You must choose the communication bus used by the display and the pins used by both the display driver and the touch sensor
These are the default values:

```
/**
 * Display settings
 * **/
 // the display SPI CS pin
#ifndef TFT_DISPLAY_SPI_CS
#define TFT_DISPLAY_SPI_CS DOUT6
#endif
 // the display SPI DC pin
#ifndef TFT_DISPLAY_SPI_DC
#define TFT_DISPLAY_SPI_DC DOUT32
#endif
 // the display SPI Backlight pin
#ifndef TFT_DISPLAY_BKL
#define TFT_DISPLAY_BKL DOUT33
#endif
 // the display SPI Reset pin
#ifndef TFT_DISPLAY_RST
#define TFT_DISPLAY_RST DOUT34
#endif
 // the display SPI HW port (SPI or SPI2)
#ifndef TFT_DISPLAY_SPI_PORT
#define TFT_DISPLAY_SPI_PORT mcu_spi_port
#endif
 // the display SPI HW port resource lock (SPI or SPI2)
#ifndef TFT_DISPLAY_SPI_LOCK
#define TFT_DISPLAY_SPI_LOCK LISTENER_HWSPI_LOCK
#endif
 // the display SPI frequency
#ifndef TFT_DISPLAY_SPI_FREQ
#define TFT_DISPLAY_SPI_FREQ 24000000UL
#endif
 // the display resolution
#ifndef TFT_H_RES
#define TFT_H_RES 320
#endif
#ifndef TFT_V_RES
#define TFT_V_RES 480
#endif

/**
 * Touch screen sensor
 * * */
#ifndef TFT_DISPLAY_TOUCH_SPI_FREQ
#define TFT_DISPLAY_TOUCH_SPI_FREQ 1000000UL
#endif
// touch sensor SPI CS pin
#ifndef TFT_DISPLAY_TOUCH_CS
#define TFT_DISPLAY_TOUCH_CS DOUT35
#endif
// touch sensor pressure detection pin (optional)
#ifndef TFT_DISPLAY_TOUCH_IRQ_PRESS
#define TFT_DISPLAY_TOUCH_IRQ_PRESS DIN35
#endif

// set 0 to disable
// set 8 to perform a byte swap
// other values for custom bit swaping
#ifndef TFT_SWAP_BIT
#define TFT_SWAP_BIT 8
#endif
```
3. Add initialization code for modules inside the `load_modules()` function:
```c
static FORCEINLINE void load_modules(void)
{	
	// Load MKS display module
	LOAD_MODULE(mks_display);

	/* Other modules go here... */
}
```
4. The last step is to enable `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`
