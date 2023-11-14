# About I2C LCD for µCNC

I2C LCD allows to add an I2C LCD module to µCNC that display some basic info about the current machine position and limits state.
It requires any 2 µCNC generic digital input pins of the board. It uses software I2C so no dedicated I2C hardware is required.

## Changelog

### 2023-11-14

- basic integration with system menu
- LCD dimension multi adapt

### 2023-09-28

- fixed I2C function calls

### 2023-05-21

- updated to version 1.8 (#29)

### 2023-05-02

- updated to version 1.7 (#17)

### 2022-09-13

- core I2C functions re-write (#9)
- added option to use HW I2C (#9)

### 2022-09-08

- new module include instructions (#7)

### 2022-08-04

- updated functions declarations to match changes in 1.5.0+

### 2022-05-19

- module initialization improvements

### 2022-05-18

- reviewed initialization procedures

### 2022-05-17

- added HD44780 c driver
- updated loaded proceadures for version 1.4.6+

### 2022-05-16

- migration from main repository
