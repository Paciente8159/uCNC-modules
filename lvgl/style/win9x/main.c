/*
	Name: main.c
	Description: Main file of the Win9x GUI style

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 29/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/
#include "../support.h"

#ifdef TFT_STYLE_WIN9X

// Declare fonts for later use in this style
#include "fonts/freemonobold12pt7b.h"
#include "fonts/freesans9pt7b.h"
#include "fonts/symbols_8x8.h"

GFX_INCLUDE_SCREEN(startup);
GFX_INCLUDE_SCREEN(idle);
GFX_INCLUDE_SCREEN(popup);

void style_startup()
{
	GFX_RENDER_SCREEN(startup);
}

void style_idle()
{
	GFX_RENDER_SCREEN(idle);
}

void style_popup(const char* text)
{
	// GFX_RENDER_SCREEN(popup);
}

void style_alarm()
{
	// This screen takes care of alarms
	GFX_RENDER_SCREEN(idle);
}

#endif

