# About SD Card v2 for µCNC

SD card allows to add an SD card read/write support to µCNC.
The SD card can be navigated using grbl commands similar to shell commands
For best performance the hardware SPI should be used.
This version uses FatFs. PetitFs might be implemented in the future

## Changelog

### 2025-05-04

- enable macro to control SPI DMA usage (#95)
- added SD card automount on startup is available (#95)

### 2025-03-24

- integration of custom SD card interface driver implementations (STM32 SDIO)

### 2024-10-09

- prevent settings reloading on card mount/unmount
- allow settings reload on cnc reset

### 2024-10-09

- modifications to make module compatible with new settings safety features

### 2024-10-03

- fixed file open/create if file does not exist (#77)

### 2024-07-04

- modified softspi interface to match improved softspi core module (#56)

### 2023-04-27

- fixed module version checking

### 2023-04-11

- initial implementation (#53)
