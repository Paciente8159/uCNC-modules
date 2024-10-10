# About I2C EEPROM for µCNC

SD card allows to add an SD card read/write support to µCNC.
The SD card can be navigated using grbl commands similar to shell commands
For best performance the hardware SPI should be used.
This version uses FatFs. PetitFs might be implemented in the future

## Changelog

### 2024-10-09

- modifications to make module compatible with new settings safety features

### 2024-09-15

- initial implementation (#72)
