# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About graphic_display for µCNC

This module adds graphic display support for µCNC.

## Adding graphic_display to µCNC

To use the graphic_display parser module follow these steps:

1. Copy the the `graphic_display` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(graphic_display);
```

3. You must choose the communication bus used by the display (SPI/I2C) and type of bus (Hardware or Software emulated) and configure the necessary pins that operate the display and the rotary encoder `cnc_hal_config.h`
These are the default values

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

4. The last step is to enable `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`
