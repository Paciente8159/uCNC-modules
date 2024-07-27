/*
	Name: startup.h
	Description: Startup screen for TFT panels

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 27/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef TFT_DISPLAY_H
#error "Please do not include this file manually"
#endif

#include "../bitmaps/logo.h"

GFX_DECL_SCREEN(startup)
{
	GFX_SCREEN_HEADER();
	GFX_CLEAR(GFX_BLACK);
	GFX_PALETTE_BITMAP(180, 100, 40, 40, 2, Logo_Colors, Logo_40x40, 3);
}

