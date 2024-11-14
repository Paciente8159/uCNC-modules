# About TMC driver for µCNC

TMC driver adds support to several Trinamic programmable stepper drivers µCNC.
They can be interfaced either via UART or SPI (all software emulated)

## Changelog

### 2024-11-15

- fix one wire transmission (replaced input weak pullup by output driven in output communication) (#86)

### 2024-11-15

- replace all TMC registers struct enums and bit fields by bit shifting operations since GCC does not garantee bit field order during optimization (#85)
- added defaults to write only shadow registers (#85)

### 2024-11-13

- fix TMC commands status print (#84)
- fix ONEWIRE default configurations (#84)
- added verbose to debug communications (#84)
- migrated to new GRBL protocol (#84)

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
