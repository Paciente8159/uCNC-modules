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

