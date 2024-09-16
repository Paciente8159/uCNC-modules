/*
	Name: tmcdriver.h
	Description: TMC driver support µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-03-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef TMC_DRIVER_H
#define TMC_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#define TMC_UART 1
#define TMC_SPI 2
#define TMC_ONEWIRE 3
#define TMC_UART2_HW 4
#define TMC_SPI_HW 5

#ifdef STEPPER0_HAS_TMC
#ifndef STEPPER0_DRIVER_TYPE
#define STEPPER0_DRIVER_TYPE 2208
#endif
// choose the interface type
#ifndef STEPPER0_TMC_INTERFACE
#define STEPPER0_TMC_INTERFACE TMC_UART
#endif
#if (STEPPER0_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#ifndef STEPPER0_UART_TX
#define STEPPER0_UART_TX DOUT20
#endif
#ifndef STEPPER0_UART_RX
#define STEPPER0_UART_RX DIN20
#endif
#ifndef STEPPER0_BAUDRATE
#define STEPPER0_BAUDRATE 38400
#endif
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER0_UART_CS DOUT12
#elif (STEPPER0_TMC_INTERFACE == TMC_SPI)
#ifndef STEPPER0_SPI_SDO
#define STEPPER0_SPI_SDO DOUT29
#endif
#ifndef STEPPER0_SPI_SDI
#define STEPPER0_SPI_SDI DIN29
#endif
#ifndef STEPPER0_SPI_CLK
#define STEPPER0_SPI_CLK DOUT30
#endif
#ifndef STEPPER0_SPI_CS
#define STEPPER0_SPI_CS DOUT12
#endif
#elif (STEPPER0_TMC_INTERFACE == TMC_UART2_HW)
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER0_UART_CS DOUT12
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL) || defined(UART2_DISABLE_BUFFER)
#error "UART2 needs to be detached from main protocol and must have buffer enable"
#endif
#elif (STEPPER0_TMC_INTERFACE == TMC_SPI_HW) || (STEPPER0_TMC_INTERFACE == TMC_SPI2_HW)
#ifndef STEPPER0_SPI_CS
#define STEPPER0_SPI_CS DOUT12
#endif 
#endif
// basic parameters
#ifndef STEPPER0_CURRENT_MA
#define STEPPER0_CURRENT_MA 800
#endif
#ifndef STEPPER0_MICROSTEP
#define STEPPER0_MICROSTEP 16
#endif
#ifndef STEPPER0_RSENSE
#define STEPPER0_RSENSE 0.11f
#endif
#ifndef STEPPER0_HOLD_MULT
#define STEPPER0_HOLD_MULT 0.7f
#endif
#ifndef STEPPER0_STEALTHCHOP_THERSHOLD
#define STEPPER0_STEALTHCHOP_THERSHOLD 0
#endif
#ifndef STEPPER0_ENABLE_INTERPLATION
#define STEPPER0_ENABLE_INTERPLATION true
#endif
#ifndef STEPPER0_STALL_SENSITIVITY
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER0_STALL_SENSITIVITY 10
#endif
#ifndef STEPPER0_UART_ADDRESS
#define STEPPER0_UART_ADDRESS 0
#endif
#endif
#ifdef STEPPER1_HAS_TMC
#ifndef STEPPER1_DRIVER_TYPE
#define STEPPER1_DRIVER_TYPE 2208
#endif
// choose the interface type
#ifndef STEPPER1_TMC_INTERFACE
#define STEPPER1_TMC_INTERFACE TMC_UART
#endif
#if (STEPPER1_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#ifndef STEPPER1_UART_TX
#define STEPPER1_UART_TX DOUT21
#endif
#ifndef STEPPER1_UART_RX
#define STEPPER1_UART_RX DIN21
#endif
#ifndef STEPPER1_BAUDRATE
#define STEPPER1_BAUDRATE 38400
#endif
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER1_UART_CS DOUT13
#elif (STEPPER1_TMC_INTERFACE == TMC_SPI)
#ifndef STEPPER1_SPI_SDO
#define STEPPER1_SPI_SDO DOUT29
#endif
#ifndef STEPPER1_SPI_SDI
#define STEPPER1_SPI_SDI DIN29
#endif
#ifndef STEPPER1_SPI_CLK
#define STEPPER1_SPI_CLK DOUT30
#endif
#ifndef STEPPER1_SPI_CS
#define STEPPER1_SPI_CS DOUT13
#endif
#elif (STEPPER1_TMC_INTERFACE == TMC_UART2_HW)
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER1_UART_CS DOUT13
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL) || defined(UART2_DISABLE_BUFFER)
#error "UART2 needs to be detached from main protocol and must have buffer enable"
#endif
#elif (STEPPER1_TMC_INTERFACE == TMC_SPI_HW) || (STEPPER1_TMC_INTERFACE == TMC_SPI2_HW)
#ifndef STEPPER1_SPI_CS
#define STEPPER1_SPI_CS DOUT13
#endif 
#endif
// basic parameters
#ifndef STEPPER1_CURRENT_MA
#define STEPPER1_CURRENT_MA 800
#endif
#ifndef STEPPER1_MICROSTEP
#define STEPPER1_MICROSTEP 16
#endif
#ifndef STEPPER1_RSENSE
#define STEPPER1_RSENSE 0.11f
#endif
#ifndef STEPPER1_HOLD_MULT
#define STEPPER1_HOLD_MULT 0.7f
#endif
#ifndef STEPPER1_STEALTHCHOP_THERSHOLD
#define STEPPER1_STEALTHCHOP_THERSHOLD 0
#endif
#ifndef STEPPER1_ENABLE_INTERPLATION
#define STEPPER1_ENABLE_INTERPLATION true
#endif
#ifndef STEPPER1_STALL_SENSITIVITY
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER1_STALL_SENSITIVITY 10
#endif
#ifndef STEPPER1_UART_ADDRESS
#define STEPPER1_UART_ADDRESS 0
#endif
#endif
#ifdef STEPPER2_HAS_TMC
#ifndef STEPPER2_DRIVER_TYPE
#define STEPPER2_DRIVER_TYPE 2208
#endif
// choose the interface type
#ifndef STEPPER2_TMC_INTERFACE
#define STEPPER2_TMC_INTERFACE TMC_UART
#endif
#if (STEPPER2_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#ifndef STEPPER2_UART_TX
#define STEPPER2_UART_TX DOUT22
#endif
#ifndef STEPPER2_UART_RX
#define STEPPER2_UART_RX DIN22
#endif
#ifndef STEPPER2_BAUDRATE
#define STEPPER2_BAUDRATE 38400
#endif
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER2_UART_CS DOUT14
#elif (STEPPER2_TMC_INTERFACE == TMC_SPI)
#ifndef STEPPER2_SPI_SDO
#define STEPPER2_SPI_SDO DOUT29
#endif
#ifndef STEPPER2_SPI_SDI
#define STEPPER2_SPI_SDI DIN29
#endif
#ifndef STEPPER2_SPI_CLK
#define STEPPER2_SPI_CLK DOUT30
#endif
#ifndef STEPPER2_SPI_CS
#define STEPPER2_SPI_CS DOUT14
#endif
#elif (STEPPER2_TMC_INTERFACE == TMC_UART2_HW)
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER2_UART_CS DOUT14
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL) || defined(UART2_DISABLE_BUFFER)
#error "UART2 needs to be detached from main protocol and must have buffer enable"
#endif
#elif (STEPPER2_TMC_INTERFACE == TMC_SPI_HW) || (STEPPER2_TMC_INTERFACE == TMC_SPI2_HW)
#ifndef STEPPER2_SPI_CS
#define STEPPER2_SPI_CS DOUT14
#endif 
#endif
// basic parameters
#ifndef STEPPER2_CURRENT_MA
#define STEPPER2_CURRENT_MA 800
#endif
#ifndef STEPPER2_MICROSTEP
#define STEPPER2_MICROSTEP 16
#endif
#ifndef STEPPER2_RSENSE
#define STEPPER2_RSENSE 0.11f
#endif
#ifndef STEPPER2_HOLD_MULT
#define STEPPER2_HOLD_MULT 0.7f
#endif
#ifndef STEPPER2_STEALTHCHOP_THERSHOLD
#define STEPPER2_STEALTHCHOP_THERSHOLD 0
#endif
#ifndef STEPPER2_ENABLE_INTERPLATION
#define STEPPER2_ENABLE_INTERPLATION true
#endif
#ifndef STEPPER2_STALL_SENSITIVITY
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER2_STALL_SENSITIVITY 10
#endif
#ifndef STEPPER2_UART_ADDRESS
#define STEPPER2_UART_ADDRESS 0
#endif
#endif
#ifdef STEPPER3_HAS_TMC
#ifndef STEPPER3_DRIVER_TYPE
#define STEPPER3_DRIVER_TYPE 2208
#endif
// choose the interface type
#ifndef STEPPER3_TMC_INTERFACE
#define STEPPER3_TMC_INTERFACE TMC_UART
#endif
#if (STEPPER3_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#ifndef STEPPER3_UART_TX
#define STEPPER3_UART_TX DOUT23
#endif
#ifndef STEPPER3_UART_RX
#define STEPPER3_UART_RX DIN23
#endif
#ifndef STEPPER3_BAUDRATE
#define STEPPER3_BAUDRATE 38400
#endif
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER3_UART_CS DOUT15
#elif (STEPPER3_TMC_INTERFACE == TMC_SPI)
#ifndef STEPPER3_SPI_SDO
#define STEPPER3_SPI_SDO DOUT29
#endif
#ifndef STEPPER3_SPI_SDI
#define STEPPER3_SPI_SDI DIN29
#endif
#ifndef STEPPER3_SPI_CLK
#define STEPPER3_SPI_CLK DOUT30
#endif
#ifndef STEPPER3_SPI_CS
#define STEPPER3_SPI_CS DOUT15
#endif
#elif (STEPPER3_TMC_INTERFACE == TMC_UART2_HW)
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER3_UART_CS DOUT15
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL) || defined(UART2_DISABLE_BUFFER)
#error "UART2 needs to be detached from main protocol and must have buffer enable"
#endif
#elif (STEPPER3_TMC_INTERFACE == TMC_SPI_HW) || (STEPPER3_TMC_INTERFACE == TMC_SPI2_HW)
#ifndef STEPPER3_SPI_CS
#define STEPPER3_SPI_CS DOUT15
#endif 
#endif
// basic parameters
#ifndef STEPPER3_CURRENT_MA
#define STEPPER3_CURRENT_MA 800
#endif
#ifndef STEPPER3_MICROSTEP
#define STEPPER3_MICROSTEP 16
#endif
#ifndef STEPPER3_RSENSE
#define STEPPER3_RSENSE 0.11f
#endif
#ifndef STEPPER3_HOLD_MULT
#define STEPPER3_HOLD_MULT 0.7f
#endif
#ifndef STEPPER3_STEALTHCHOP_THERSHOLD
#define STEPPER3_STEALTHCHOP_THERSHOLD 0
#endif
#ifndef STEPPER3_ENABLE_INTERPLATION
#define STEPPER3_ENABLE_INTERPLATION true
#endif
#ifndef STEPPER3_STALL_SENSITIVITY
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER3_STALL_SENSITIVITY 10
#endif
#ifndef STEPPER3_UART_ADDRESS
#define STEPPER3_UART_ADDRESS 0
#endif
#endif
#ifdef STEPPER4_HAS_TMC
#ifndef STEPPER4_DRIVER_TYPE
#define STEPPER4_DRIVER_TYPE 2208
#endif
// choose the interface type
#ifndef STEPPER4_TMC_INTERFACE
#define STEPPER4_TMC_INTERFACE TMC_UART
#endif
#if (STEPPER4_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#ifndef STEPPER4_UART_TX
#define STEPPER4_UART_TX DOUT24
#endif
#ifndef STEPPER4_UART_RX
#define STEPPER4_UART_RX DIN24
#endif
#ifndef STEPPER4_BAUDRATE
#define STEPPER4_BAUDRATE 38400
#endif
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER4_UART_CS DOUT16
#elif (STEPPER4_TMC_INTERFACE == TMC_SPI)
#ifndef STEPPER4_SPI_SDO
#define STEPPER4_SPI_SDO DOUT29
#endif
#ifndef STEPPER4_SPI_SDI
#define STEPPER4_SPI_SDI DIN29
#endif
#ifndef STEPPER4_SPI_CLK
#define STEPPER4_SPI_CLK DOUT30
#endif
#ifndef STEPPER4_SPI_CS
#define STEPPER4_SPI_CS DOUT16
#endif
#elif (STEPPER4_TMC_INTERFACE == TMC_UART2_HW)
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER4_UART_CS DOUT16
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL) || defined(UART2_DISABLE_BUFFER)
#error "UART2 needs to be detached from main protocol and must have buffer enable"
#endif
#elif (STEPPER4_TMC_INTERFACE == TMC_SPI_HW) || (STEPPER4_TMC_INTERFACE == TMC_SPI2_HW)
#ifndef STEPPER4_SPI_CS
#define STEPPER4_SPI_CS DOUT16
#endif 
#endif
// basic parameters
#ifndef STEPPER4_CURRENT_MA
#define STEPPER4_CURRENT_MA 800
#endif
#ifndef STEPPER4_MICROSTEP
#define STEPPER4_MICROSTEP 16
#endif
#ifndef STEPPER4_RSENSE
#define STEPPER4_RSENSE 0.11f
#endif
#ifndef STEPPER4_HOLD_MULT
#define STEPPER4_HOLD_MULT 0.7f
#endif
#ifndef STEPPER4_STEALTHCHOP_THERSHOLD
#define STEPPER4_STEALTHCHOP_THERSHOLD 0
#endif
#ifndef STEPPER4_ENABLE_INTERPLATION
#define STEPPER4_ENABLE_INTERPLATION true
#endif
#ifndef STEPPER4_STALL_SENSITIVITY
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER4_STALL_SENSITIVITY 10
#endif
#ifndef STEPPER4_UART_ADDRESS
#define STEPPER4_UART_ADDRESS 0
#endif
#endif
#ifdef STEPPER5_HAS_TMC
#ifndef STEPPER5_DRIVER_TYPE
#define STEPPER5_DRIVER_TYPE 2208
#endif
// choose the interface type
#ifndef STEPPER5_TMC_INTERFACE
#define STEPPER5_TMC_INTERFACE TMC_UART
#endif
#if (STEPPER5_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#ifndef STEPPER5_UART_TX
#define STEPPER5_UART_TX DOUT25
#endif
#ifndef STEPPER5_UART_RX
#define STEPPER5_UART_RX DIN25
#endif
#ifndef STEPPER5_BAUDRATE
#define STEPPER5_BAUDRATE 38400
#endif
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER5_UART_CS DOUT17
#elif (STEPPER5_TMC_INTERFACE == TMC_SPI)
#ifndef STEPPER5_SPI_SDO
#define STEPPER5_SPI_SDO DOUT29
#endif
#ifndef STEPPER5_SPI_SDI
#define STEPPER5_SPI_SDI DIN29
#endif
#ifndef STEPPER5_SPI_CLK
#define STEPPER5_SPI_CLK DOUT30
#endif
#ifndef STEPPER5_SPI_CS
#define STEPPER5_SPI_CS DOUT17
#endif
#elif (STEPPER5_TMC_INTERFACE == TMC_UART2_HW)
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER5_UART_CS DOUT17
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL) || defined(UART2_DISABLE_BUFFER)
#error "UART2 needs to be detached from main protocol and must have buffer enable"
#endif
#elif (STEPPER5_TMC_INTERFACE == TMC_SPI_HW) || (STEPPER5_TMC_INTERFACE == TMC_SPI2_HW)
#ifndef STEPPER5_SPI_CS
#define STEPPER5_SPI_CS DOUT17
#endif 
#endif
// basic parameters
#ifndef STEPPER5_CURRENT_MA
#define STEPPER5_CURRENT_MA 800
#endif
#ifndef STEPPER5_MICROSTEP
#define STEPPER5_MICROSTEP 16
#endif
#ifndef STEPPER5_RSENSE
#define STEPPER5_RSENSE 0.11f
#endif
#ifndef STEPPER5_HOLD_MULT
#define STEPPER5_HOLD_MULT 0.7f
#endif
#ifndef STEPPER5_STEALTHCHOP_THERSHOLD
#define STEPPER5_STEALTHCHOP_THERSHOLD 0
#endif
#ifndef STEPPER5_ENABLE_INTERPLATION
#define STEPPER5_ENABLE_INTERPLATION true
#endif
#ifndef STEPPER5_STALL_SENSITIVITY
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER5_STALL_SENSITIVITY 10
#endif
#ifndef STEPPER5_UART_ADDRESS
#define STEPPER5_UART_ADDRESS 0
#endif
#endif
#ifdef STEPPER6_HAS_TMC
#ifndef STEPPER6_DRIVER_TYPE
#define STEPPER6_DRIVER_TYPE 2208
#endif
// choose the interface type
#ifndef STEPPER6_TMC_INTERFACE
#define STEPPER6_TMC_INTERFACE TMC_UART
#endif
#if (STEPPER6_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#ifndef STEPPER6_UART_TX
#define STEPPER6_UART_TX DOUT26
#endif
#ifndef STEPPER6_UART_RX
#define STEPPER6_UART_RX DIN26
#endif
#ifndef STEPPER6_BAUDRATE
#define STEPPER6_BAUDRATE 38400
#endif
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER6_UART_CS DOUT18
#elif (STEPPER6_TMC_INTERFACE == TMC_SPI)
#ifndef STEPPER6_SPI_SDO
#define STEPPER6_SPI_SDO DOUT29
#endif
#ifndef STEPPER6_SPI_SDI
#define STEPPER6_SPI_SDI DIN29
#endif
#ifndef STEPPER6_SPI_CLK
#define STEPPER6_SPI_CLK DOUT30
#endif
#ifndef STEPPER6_SPI_CS
#define STEPPER6_SPI_CS DOUT18
#endif
#elif (STEPPER6_TMC_INTERFACE == TMC_UART2_HW)
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER6_UART_CS DOUT18
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL) || defined(UART2_DISABLE_BUFFER)
#error "UART2 needs to be detached from main protocol and must have buffer enable"
#endif
#elif (STEPPER6_TMC_INTERFACE == TMC_SPI_HW) || (STEPPER6_TMC_INTERFACE == TMC_SPI2_HW)
#ifndef STEPPER6_SPI_CS
#define STEPPER6_SPI_CS DOUT18
#endif 
#endif
// basic parameters
#ifndef STEPPER6_CURRENT_MA
#define STEPPER6_CURRENT_MA 800
#endif
#ifndef STEPPER6_MICROSTEP
#define STEPPER6_MICROSTEP 16
#endif
#ifndef STEPPER6_RSENSE
#define STEPPER6_RSENSE 0.11f
#endif
#ifndef STEPPER6_HOLD_MULT
#define STEPPER6_HOLD_MULT 0.7f
#endif
#ifndef STEPPER6_STEALTHCHOP_THERSHOLD
#define STEPPER6_STEALTHCHOP_THERSHOLD 0
#endif
#ifndef STEPPER6_ENABLE_INTERPLATION
#define STEPPER6_ENABLE_INTERPLATION true
#endif
#ifndef STEPPER6_STALL_SENSITIVITY
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER6_STALL_SENSITIVITY 10
#endif
#ifndef STEPPER6_UART_ADDRESS
#define STEPPER6_UART_ADDRESS 0
#endif
#endif
#ifdef STEPPER7_HAS_TMC
#ifndef STEPPER7_DRIVER_TYPE
#define STEPPER7_DRIVER_TYPE 2208
#endif
// choose the interface type
#ifndef STEPPER7_TMC_INTERFACE
#define STEPPER7_TMC_INTERFACE TMC_UART
#endif
#if (STEPPER7_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#ifndef STEPPER7_UART_TX
#define STEPPER7_UART_TX DOUT27
#endif
#ifndef STEPPER7_UART_RX
#define STEPPER7_UART_RX DIN27
#endif
#ifndef STEPPER7_BAUDRATE
#define STEPPER7_BAUDRATE 38400
#endif
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER7_UART_CS DOUT19
#elif (STEPPER7_TMC_INTERFACE == TMC_SPI)
#ifndef STEPPER7_SPI_SDO
#define STEPPER7_SPI_SDO DOUT29
#endif
#ifndef STEPPER7_SPI_SDI
#define STEPPER7_SPI_SDI DIN29
#endif
#ifndef STEPPER7_SPI_CLK
#define STEPPER7_SPI_CLK DOUT30
#endif
#ifndef STEPPER7_SPI_CS
#define STEPPER7_SPI_CS DOUT19
#endif
#elif (STEPPER7_TMC_INTERFACE == TMC_UART2_HW)
// uncomment to enable UART CS (for example using 74HC4066)
// #define STEPPER7_UART_CS DOUT19
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL) || defined(UART2_DISABLE_BUFFER)
#error "UART2 needs to be detached from main protocol and must have buffer enable"
#endif
#elif (STEPPER7_TMC_INTERFACE == TMC_SPI_HW) || (STEPPER7_TMC_INTERFACE == TMC_SPI2_HW)
#ifndef STEPPER7_SPI_CS
#define STEPPER7_SPI_CS DOUT19
#endif 
#endif
// basic parameters
#ifndef STEPPER7_CURRENT_MA
#define STEPPER7_CURRENT_MA 800
#endif
#ifndef STEPPER7_MICROSTEP
#define STEPPER7_MICROSTEP 16
#endif
#ifndef STEPPER7_RSENSE
#define STEPPER7_RSENSE 0.11f
#endif
#ifndef STEPPER7_HOLD_MULT
#define STEPPER7_HOLD_MULT 0.7f
#endif
#ifndef STEPPER7_STEALTHCHOP_THERSHOLD
#define STEPPER7_STEALTHCHOP_THERSHOLD 0
#endif
#ifndef STEPPER7_ENABLE_INTERPLATION
#define STEPPER7_ENABLE_INTERPLATION true
#endif
#ifndef STEPPER7_STALL_SENSITIVITY
// this value must be set between 0 and 255 for TMC2209
// if driver does not support stallGuard this will be ignored
#define STEPPER7_STALL_SENSITIVITY 10
#endif
#ifndef STEPPER7_UART_ADDRESS
#define STEPPER7_UART_ADDRESS 0
#endif
#endif

#if defined(STEPPER0_HAS_TMC) || defined(STEPPER1_HAS_TMC) || defined(STEPPER2_HAS_TMC) || defined(STEPPER3_HAS_TMC) || defined(STEPPER4_HAS_TMC) || defined(STEPPER5_HAS_TMC) || defined(STEPPER6_HAS_TMC) || defined(STEPPER7_HAS_TMC)
#define ENABLE_TMC_DRIVER_MODULE
#endif

#ifdef STEPPER0_HAS_TMC
#if (STEPPER0_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (!ASSERT_PIN(STEPPER0_UART_TX) || !ASSERT_PIN(STEPPER0_UART_RX))
#undef STEPPER0_HAS_TMC
#error "Stepper 0 undefined UART pins"
#endif
#elif (STEPPER0_TMC_INTERFACE == TMC_SPI)
#if (!ASSERT_PIN(STEPPER0_SPI_SDO) || !ASSERT_PIN(STEPPER0_SPI_SDI) || !ASSERT_PIN(STEPPER0_SPI_CLK) || !ASSERT_PIN(STEPPER0_SPI_CS))
#undef STEPPER0_HAS_TMC
#error "Stepper 0 undefined SPI pins"
#endif
#endif
#if ASSERT_PIN(STEPPER0_UART_CS)
#define stepper0_select() io_set_output(STEPPER0_UART_CS)
#define stepper0_deselect() io_clear_output(STEPPER0_UART_CS)
#else
#define stepper0_select()
#define stepper0_deselect()
#endif
#endif
#ifdef STEPPER1_HAS_TMC
#if (STEPPER1_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (!ASSERT_PIN(STEPPER1_UART_TX) || !ASSERT_PIN(STEPPER1_UART_RX))
#undef STEPPER1_HAS_TMC
#error "Stepper 1 undefined UART pins"
#endif
#elif (STEPPER1_TMC_INTERFACE == TMC_SPI)
#if (!ASSERT_PIN(STEPPER1_SPI_SDO) || !ASSERT_PIN(STEPPER1_SPI_SDI) || !ASSERT_PIN(STEPPER1_SPI_CLK) || !ASSERT_PIN(STEPPER1_SPI_CS))
#undef STEPPER1_HAS_TMC
#error "Stepper 1 undefined SPI pins"
#endif
#endif
#if ASSERT_PIN(STEPPER1_UART_CS)
#define stepper1_select() io_set_output(STEPPER1_UART_CS)
#define stepper1_deselect() io_clear_output(STEPPER1_UART_CS)
#else
#define stepper1_select()
#define stepper1_deselect()
#endif
#endif
#ifdef STEPPER2_HAS_TMC
#if (STEPPER2_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (!ASSERT_PIN(STEPPER2_UART_TX) || !ASSERT_PIN(STEPPER2_UART_RX))
#undef STEPPER2_HAS_TMC
#error "Stepper 2 undefined UART pins"
#endif
#elif (STEPPER2_TMC_INTERFACE == TMC_SPI)
#if (!ASSERT_PIN(STEPPER2_SPI_SDO) || !ASSERT_PIN(STEPPER2_SPI_SDI) || !ASSERT_PIN(STEPPER2_SPI_CLK) || !ASSERT_PIN(STEPPER2_SPI_CS))
#undef STEPPER2_HAS_TMC
#error "Stepper 2 undefined SPI pins"
#endif
#endif
#if ASSERT_PIN(STEPPER2_UART_CS)
#define stepper2_select() io_set_output(STEPPER2_UART_CS)
#define stepper2_deselect() io_clear_output(STEPPER2_UART_CS)
#else
#define stepper2_select()
#define stepper2_deselect()
#endif
#endif
#ifdef STEPPER3_HAS_TMC
#if (STEPPER3_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (!ASSERT_PIN(STEPPER3_UART_TX) || !ASSERT_PIN(STEPPER3_UART_RX))
#undef STEPPER3_HAS_TMC
#error "Stepper 3 undefined UART pins"
#endif
#elif (STEPPER3_TMC_INTERFACE == TMC_SPI)
#if (!ASSERT_PIN(STEPPER3_SPI_SDO) || !ASSERT_PIN(STEPPER3_SPI_SDI) || !ASSERT_PIN(STEPPER3_SPI_CLK) || !ASSERT_PIN(STEPPER3_SPI_CS))
#undef STEPPER3_HAS_TMC
#error "Stepper 3 undefined SPI pins"
#endif
#endif
#if ASSERT_PIN(STEPPER3_UART_CS)
#define stepper3_select() io_set_output(STEPPER3_UART_CS)
#define stepper3_deselect() io_clear_output(STEPPER3_UART_CS)
#else
#define stepper3_select()
#define stepper3_deselect()
#endif
#endif
#ifdef STEPPER4_HAS_TMC
#if (STEPPER4_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (!ASSERT_PIN(STEPPER4_UART_TX) || !ASSERT_PIN(STEPPER4_UART_RX))
#undef STEPPER4_HAS_TMC
#error "Stepper 4 undefined UART pins"
#endif
#elif (STEPPER4_TMC_INTERFACE == TMC_SPI)
#if (!ASSERT_PIN(STEPPER4_SPI_SDO) || !ASSERT_PIN(STEPPER4_SPI_SDI) || !ASSERT_PIN(STEPPER4_SPI_CLK) || !ASSERT_PIN(STEPPER4_SPI_CS))
#undef STEPPER4_HAS_TMC
#error "Stepper 4 undefined SPI pins"
#endif
#endif
#if ASSERT_PIN(STEPPER4_UART_CS)
#define stepper4_select() io_set_output(STEPPER4_UART_CS)
#define stepper4_deselect() io_clear_output(STEPPER4_UART_CS)
#else
#define stepper4_select()
#define stepper4_deselect()
#endif
#endif
#ifdef STEPPER5_HAS_TMC
#if (STEPPER5_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (!ASSERT_PIN(STEPPER5_UART_TX) || !ASSERT_PIN(STEPPER5_UART_RX))
#undef STEPPER5_HAS_TMC
#error "Stepper 5 undefined UART pins"
#endif
#elif (STEPPER5_TMC_INTERFACE == TMC_SPI)
#if (!ASSERT_PIN(STEPPER5_SPI_SDO) || !ASSERT_PIN(STEPPER5_SPI_SDI) || !ASSERT_PIN(STEPPER5_SPI_CLK) || !ASSERT_PIN(STEPPER5_SPI_CS))
#undef STEPPER5_HAS_TMC
#error "Stepper 5 undefined SPI pins"
#endif
#endif
#if ASSERT_PIN(STEPPER5_UART_CS)
#define stepper5_select() io_set_output(STEPPER5_UART_CS)
#define stepper5_deselect() io_clear_output(STEPPER5_UART_CS)
#else
#define stepper5_select()
#define stepper5_deselect()
#endif
#endif
#ifdef STEPPER6_HAS_TMC
#if (STEPPER6_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (!ASSERT_PIN(STEPPER6_UART_TX) || !ASSERT_PIN(STEPPER6_UART_RX))
#undef STEPPER6_HAS_TMC
#error "Stepper 6 undefined UART pins"
#endif
#elif (STEPPER6_TMC_INTERFACE == TMC_SPI)
#if (!ASSERT_PIN(STEPPER6_SPI_SDO) || !ASSERT_PIN(STEPPER6_SPI_SDI) || !ASSERT_PIN(STEPPER6_SPI_CLK) || !ASSERT_PIN(STEPPER6_SPI_CS))
#undef STEPPER6_HAS_TMC
#error "Stepper 6 undefined SPI pins"
#endif
#endif
#if ASSERT_PIN(STEPPER6_UART_CS)
#define stepper6_select() io_set_output(STEPPER6_UART_CS)
#define stepper6_deselect() io_clear_output(STEPPER6_UART_CS)
#else
#define stepper6_select()
#define stepper6_deselect()
#endif
#endif
#ifdef STEPPER7_HAS_TMC
#if (STEPPER7_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (!ASSERT_PIN(STEPPER7_UART_TX) || !ASSERT_PIN(STEPPER7_UART_RX))
#undef STEPPER7_HAS_TMC
#error "Stepper 7 undefined UART pins"
#endif
#elif (STEPPER7_TMC_INTERFACE == TMC_SPI)
#if (!ASSERT_PIN(STEPPER7_SPI_SDO) || !ASSERT_PIN(STEPPER7_SPI_SDI) || !ASSERT_PIN(STEPPER7_SPI_CLK) || !ASSERT_PIN(STEPPER7_SPI_CS))
#undef STEPPER7_HAS_TMC
#error "Stepper 7 undefined SPI pins"
#endif
#endif
#if ASSERT_PIN(STEPPER7_UART_CS)
#define stepper7_select() io_set_output(STEPPER7_UART_CS)
#define stepper7_deselect() io_clear_output(STEPPER7_UART_CS)
#else
#define stepper7_select()
#define stepper7_deselect()
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif