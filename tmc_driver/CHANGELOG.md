# About TMC driver for µCNC

TMC driver adds support to several Trinamic programmable stepper drivers µCNC.
They can be interfaced either via UART or SPI (all software emulated)

## Changelog

### 2024-02-16

- added support for HW UART2 and SPI ports (#50)

### 2024-02-16

- configurable UART baudrate for each driver (#49)
- fixed timeout calculations (#49)

### 2024-02-16

- added UART chip select optional pin (#47)

### 2024-01-27

- removed from core code to modules

### 2022-03-20

- initial implementation
