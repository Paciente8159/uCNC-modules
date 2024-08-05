/*
	Name: driver.h
	Description: Defines which driver should be used for the TFT module and configures it

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 03/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef TFT_DRIVER_H
#define TFT_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

// Used for RPi type screens
#define TFT_ALWAYS_16BIT

// Makes sure the driver pulses the chip select line before
// every transfer, this can be used to synchronize the
// receiver in case of lost clocks.
#define TFT_SYNC_CS

#include "driver/ILI9486.h"

#ifdef __cplusplus
}
#endif

#endif

