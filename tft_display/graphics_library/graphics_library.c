/*
	Name: graphics_librarary.c
	Description: Graphics library for µCNC
		What this library is attempting to accomplish is
		drawing to a TFT display with the least number of
		(relatively slow) SPI transactions. This should
		result in a more reactive GUI

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 20/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef GFX_TESTING
#include "../../../cnc.h"
#endif

#if defined(ENABLE_GFX) | defined(GFX_TESTING)

#include "graphics_library.h"

static uint8_t gfx_render_buffer[GFX_RENDER_BUFFER_SIZE];
static screen_t *gfx_primary_screen = 0;
static bool gfx_rendering = false;

void gfx_render_screen(screen_t* screen)
{
	// Guard against rendering 2 screens at once
	if(!gfx_rendering)
	{
		gfx_rendering = true;
		
		screen_context_t ctx = { 0 };
		ctx.sc_buffer = (gfx_pixel_t*) gfx_render_buffer;
		ctx.sc_first_draw = screen != gfx_primary_screen;
		ctx.sc_x = 0;
		ctx.sc_width = GFX_DISPLAY_WIDTH;
		ctx.sc_time = GFX_CTX_TIME();

		// Calculate max row count we can render with
		// the given buffer size
		const uint16_t max_row_count = (GFX_RENDER_BUFFER_SIZE / (sizeof(gfx_pixel_t) * GFX_DISPLAY_WIDTH));

		uint16_t current_row = 0;
		while(current_row < GFX_DISPLAY_HEIGHT)
		{
			// Get row count to render
			uint16_t render_rows = GFX_DISPLAY_HEIGHT - current_row;
			if(render_rows > max_row_count)
				render_rows = max_row_count;

			ctx.sc_y = current_row;
			ctx.sc_height = render_rows;
			ctx.sc_dirty = false;

			// Render part of screen...
			screen(&ctx);
			// ...and send it to the display
			if(ctx.sc_dirty)
				GFX_BLIT(0, current_row, GFX_DISPLAY_WIDTH, render_rows, ctx.sc_buffer);

			current_row += render_rows;
		}

		gfx_rendering = false;
		gfx_primary_screen = screen;
	}
}

void gfx_partial_render(partial_render_t *partial, screen_t *screen)
{
	partial(screen);
	gfx_primary_screen = screen;
}

void gfx_partial_render_area(screen_t *screen, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	// Guard against rendering 2 screens at once
	if(!gfx_rendering)
	{
		gfx_rendering = true;
		
		screen_context_t ctx = { 0 };
		ctx.sc_buffer = (gfx_pixel_t*) gfx_render_buffer;
		ctx.sc_first_draw = gfx_primary_screen != screen;
		ctx.sc_x = x;
		ctx.sc_width = width;
		ctx.sc_time = GFX_CTX_TIME();

		// Calculate max row count we can render with
		// the given buffer size
		const uint16_t max_row_count = (GFX_RENDER_BUFFER_SIZE / (sizeof(gfx_pixel_t) * width));

		uint16_t current_row = 0;
		while(current_row < height)
		{
			// Get row count to render
			uint16_t render_rows = height - current_row;
			if(render_rows > max_row_count)
				render_rows = max_row_count;

			ctx.sc_y = y + current_row;
			ctx.sc_height = render_rows;
			ctx.sc_dirty = false;

			// Render part of screen...
			screen(&ctx);
			// ...and send it to the display
			if(ctx.sc_dirty)
				GFX_BLIT(x, y + current_row, width, render_rows, ctx.sc_buffer);

			current_row += render_rows;
		}

		gfx_rendering = false;
	}
}

void gfx_clear(screen_context_t *ctx, gfx_pixel_t color)
{
	for(int i = 0; i < ctx->sc_width * ctx->sc_height; ++i)
		ctx->sc_buffer[i] = color;
	ctx->sc_dirty = true;
}

void gfx_rect(screen_context_t *ctx, uint16_t x, uint16_t y, uint16_t width, uint16_t height, gfx_pixel_t color)
{
	int16_t s_x = x - ctx->sc_x, s_y = y - ctx->sc_y;
	int16_t e_x = s_x + width, e_y = s_y + height;
	if(e_y <= 0 || e_x <= 0 || s_y >= ctx->sc_height || s_x >= ctx->sc_width)
	{
		// Out of requested range
		return;
	}
	// Limit coordinates to the requested screen region
	if(s_x < 0) s_x = 0;
	if(s_y < 0) s_y = 0;
	if(e_x > ctx->sc_width) e_x = ctx->sc_width;
	if(e_y > ctx->sc_height) e_y = ctx->sc_height;

	for(int16_t i = s_y; i < e_y; ++i)
	{
		for(int16_t j = s_x; j < e_x; ++j)
		{
			GFX_BUFFER(j, i) = color;
		}
	}

	ctx->sc_dirty = true;
}

void gfx_frame(screen_context_t *ctx, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t thickness, gfx_pixel_t bg_color, gfx_pixel_t fg_color)
{
	const int16_t s_x = x - ctx->sc_x, s_y = y - ctx->sc_y;
	const int16_t e_x = s_x + width, e_y = s_y + height;
	if(e_y < 0 || e_x < 0 || s_y >= ctx->sc_height || s_x >= ctx->sc_width)
	{
		// Out of requested range
		return;
	}
	// Limit coordinates to the requested screen region
	int16_t c_x = s_x, c_y = s_y, m_x = e_x, m_y = e_y;
	if(c_x < 0) c_x = 0;
	if(c_y < 0) c_y = 0;
	if(m_x > ctx->sc_width) m_x = ctx->sc_width;
	if(m_y > ctx->sc_height) m_y = ctx->sc_height;

	for(int16_t j = c_y; j < m_y; ++j)
	{
		for(int16_t i = c_x; i < m_x; ++i)
		{
			// Calculate manhattan distance
			const int16_t d1_x = s_x - i < 0 ? i - s_x : s_x - i;
			const int16_t d1_y = s_y - j < 0 ? j - s_y : s_y - j;
			const int16_t d2_x = i - e_x + 1 < 0 ? e_x - i - 1 : i - e_x + 1;
			const int16_t d2_y = j - e_y + 1 < 0 ? e_y - j - 1 : j - e_y + 1;
			const int16_t d = GFX_MIN(d1_x, d1_y, d2_x, d2_y);

			GFX_BUFFER(i, j) = d < thickness ? fg_color : bg_color;
		}
	}

	ctx->sc_dirty = true;
}

static inline bool gfx_font_bit(const uint8_t* bitmap, uint16_t char_offset, uint8_t char_x, uint8_t char_y, uint8_t char_width)
{
	const uint16_t bit_pos = char_x + char_y * char_width;
	const uint8_t byte = bitmap[char_offset + (bit_pos >> 3)];
	return (byte >> ((~bit_pos) & 0x7)) & 1;
}

void gfx_text(screen_context_t *ctx, uint16_t x, uint16_t y, gfx_pixel_t bg_color, gfx_pixel_t fg_color, const struct BitmapFont *font, uint8_t scale, const char *text)
{
	int16_t s_x = x - ctx->sc_x, s_y = y - ctx->sc_y;
	if(s_y >= ctx->sc_height || s_x >= ctx->sc_width || s_y + font->bf_yAdvance * scale < 0)
	{
		// Out of requested range
		return;
	}

	int8_t column = 0;

	// First we need to find the correct point in text
	while((s_x + column) < 0)
	{
		if(*text < font->bf_firstChar || *text > font->bf_lastChar)
		{
			++text;
			continue;
		}

		uint8_t glyph_idx = *text - font->bf_firstChar;
		const struct BitmapFontGlyph* glyph = font->bf_glyphs + glyph_idx;

		if(s_x + glyph->bfg_xAdvance * scale > 0)
		{
			// Character crosses the boundary
			// and isn't fully drawn
			column = -s_x;
			s_x = 0;
		}
		else
		{
			// Character still not at the boundary
			s_x += glyph->bfg_xAdvance * scale;
			++text;
		}
	}

	const struct BitmapFontGlyph* glyph = 0;

	while((s_x + column) < ctx->sc_width)
	{
		if(!glyph)
		{
			char c = *text++;
			if(c == 0)
				break;
			if(c < font->bf_firstChar || c > font->bf_lastChar)
				continue;
			glyph = font->bf_glyphs + c - font->bf_firstChar;
		}

		const int16_t bitmap_y_start = (font->bf_lineHeight + glyph->bfg_yOffset) * scale;
		const int16_t bitmap_y_end = bitmap_y_start + glyph->bfg_height * scale;
		const int16_t bitmap_x_start = glyph->bfg_xOffset * scale;
		const int16_t bitmap_x_end = bitmap_x_start + glyph->bfg_width * scale;
		for(int16_t y = -GFX_MIN(0, s_y); y < font->bf_yAdvance * scale && y + s_y < ctx->sc_height; ++y)
		{
			if(y >= bitmap_y_start && y < bitmap_y_end &&
				column >= bitmap_x_start && column < bitmap_x_end)
			{
				bool bit = gfx_font_bit(font->bf_bitmap, glyph->bfg_bitmapOffset, (column - bitmap_x_start) / scale, (y - bitmap_y_start) / scale, glyph->bfg_width);
				GFX_BUFFER(s_x + column, s_y + y) = bit ? fg_color : bg_color;
			}
			else
			{
				GFX_BUFFER(s_x + column, s_y + y) = bg_color;
			}
		}

		if(++column >= glyph->bfg_xAdvance * scale)
		{
			s_x += column;
			glyph = 0;
			column = 0;
		}
	}

	ctx->sc_dirty = true;
}

#endif

