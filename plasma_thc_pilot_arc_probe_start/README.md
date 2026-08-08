# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

# About Plasma THC Pilot Arc Probe and Start for µCNC

This module adds modifies the Plasma THC probe and start action to take advantage of a torch with pilot arc µCNC.

## Adding Plasma THC Pilot Arc Probe and Start to µCNC

To use the Plasma THC Pilot Arc Probe and Start module follow these steps:

1. Copy the the `plasma_thc_pilot_arc_probe_start` directory and place it inside the `src/modules/` directory of µCNC
2. The last step is to enable `PROBE_ENABLE_CUSTOM_CALLBACK` inside `cnc_hal_config.h`.
