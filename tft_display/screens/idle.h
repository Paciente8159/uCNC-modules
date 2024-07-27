/*
	Name: idle.h
	Description: Idle screen for TFT panels

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

GFX_DECL_SCREEN(main)
{
	GFX_SCREEN_HEADER();
	GFX_SET_FONT(FONT_MONO, 1);

	GFX_CLEAR(BASE_BACKGROUND);

	GFX_RECT(0, 0, GFX_DISPLAY_WIDTH, 22, TOP_BAR);
	GFX_TEXT(GFX_DISPLAY_WIDTH - 19, 3, GFX_WHITE, FONT_SYMBOL, 2, "\x01");
	GFX_TEXT(2, 0, GFX_WHITE, FONT_SANS, 1, "G54 Abs [mm] T1");

	BOX_OUTSET(0, 30, 50, 50);
	GFX_BITMAP(GFX_REL(9, 11), 15, 14, BOX_BACKGROUND, CHARCOAL, ZeroPosBitmap_15x14, 2);

	BOX_OUTSET(0, 115, 50, 50);
	GFX_BITMAP(GFX_REL(5, 5), 20, 20, BOX_BACKGROUND, CHARCOAL, MoveBitmap_20x20, 2);

	BOX_OUTSET(0, 200, 50, 50);

	/* Coordinates */
	BOX_INSET(70, 30, 168, 111);

	GFX_TEXT(GFX_REL(10, 10), GFX_RED, "X");
	GFX_TEXT(GFX_REL(10, 45), GFX_DARK_GREEN, "Y");
	GFX_TEXT(GFX_REL(10, 80), GFX_BLUE, "Z");
	
	GFX_TEXT(GFX_REL(75, 10), GFX_BLACK, "000.00");
	GFX_TEXT(GFX_REL(75, 45), GFX_BLACK, "000.00");
	GFX_TEXT(GFX_REL(75, 80), GFX_BLACK, "000.00");

	/* Feed and Spindle */
	BOX_INSET(70, 150, 168, 75);

	GFX_TEXT(GFX_REL(10, 10), GFX_BLACK, "F");
	GFX_TEXT(GFX_REL(75 + 28, 10), GFX_BLACK, "0000");

	GFX_TEXT(GFX_REL(10, 45), GFX_BLACK, "S");
	GFX_TEXT(GFX_REL(75 + 14, 45), GFX_BLACK, "00000");

	/* Status */
	BOX_INSET(70, 235, 168, 37);
	GFX_TEXT(GFX_REL(10, 8), GFX_BLACK, "Alarm");
}
