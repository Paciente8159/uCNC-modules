# About SD Card for µCNC

SD card allows to add an SD card read/write support to µCNC.
The SD card can be navigated using grbl commands similar to shell commands
For best performance the hardware SPI should be used.

## Changelog

### 2023-05-09

- integrated sd with system menus (#24)
- removed LFN support to reduce memory requirements (#24)

### 2023-05-02

- updated to version 1.7 (#17)

### 2022-10-10

- modified sd card handlers to allow multiple settings event listeners (#13)

### 2022-09-20

- implemented initial support for settings saving (#10)
- fixed SD writing and SD-SPI card recognition (#10)

### 2022-09-08

- initial implementation (#8)
