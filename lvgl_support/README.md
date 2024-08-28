# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About lvgl for µCNC

This module adds support for using the LVGL toolkit to build graphical user interfaces.

## Adding lvgl to µCNC

To use the lvgl module follow these steps:

1. Copy the `lvgl` directory and place it inside the `src/modules/` directory of µCNC
2. Add initialization code for modules inside the `load_modules()` function:
```c
static FORCEINLINE void load_modules(void)
{
	/* Display module goes here... */

	// Load LVGL module
	LOAD_MODULE(lvgl_support);

	/* Other modules go here... */
}
```
3. Install display and input modules as needed.
4. Make sure to define `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`
5. Add the LVGL library to `platformio.ini`:
```
lib_deps = 
	lvgl/lvgl@9.1.0
```
6. Add a build flag (`-DLV_CONF_PATH`) in `platformio.ini`:
```
[common]
build_flags = 
	${webconfig.build_flags}
  -DLV_CONF_PATH="${platformio.include_dir}/src/modules/lvgl_support/lv_conf.h"
```

