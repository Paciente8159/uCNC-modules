/*
	Name: tft_display.h
	Description: TFT display panel support for µCNC

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 20/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../module.h"

#define CMD(cmd) tft_command(cmd)
#define DAT(dat) tft_data(dat)
#define DAT_BULK(data, len) tft_bulk_data(data, len)

#define TFT_ALWAYS_16BIT
#include "graphics_library/graphics_library.h"

DECL_MODULE(tft_display);

#ifdef __cplusplus
}
#endif

#endif

