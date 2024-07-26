/*
	Name: graphics_librarary.h
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

#ifndef GRAPHICS_LIBRARY_H
#define GRAPHICS_LIBRARY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#include "fonts/font.h"

#include "config.h"

#ifndef __GFX_DRIVER
#error "Graphics library is missing a driver"
#endif

#ifndef GFX_RENDER_BUFFER_SIZE
#define GFX_RENDER_BUFFER_SIZE 256
#endif

#ifndef GFX_AFTER_ELEMENT_HOOK
#define GFX_AFTER_ELEMENT_HOOK()
#endif

typedef struct screen_context {
	bool sc_first_draw;
	uint16_t sc_x;
	uint16_t sc_y;
	uint16_t sc_width;
	uint16_t sc_height;
	gfx_pixel_t *sc_buffer;
	bool sc_dirty;
	uint32_t sc_time;
} screen_context_t;

typedef void screen_t(screen_context_t*);
typedef void partial_render_t(screen_t*);

extern void gfx_render_screen(screen_t *screen);
extern void gfx_partial_render(partial_render_t *partial, screen_t *screen);
extern void gfx_partial_render_area(screen_t *screen, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

#define GFX_BLACK			GFX_COLOR(  0,   0,	  0)
#define GFX_BLUE			GFX_COLOR(  0,   0, 255)
#define GFX_GREEN			GFX_COLOR(  0, 255,	  0)
#define GFX_CYAN			GFX_COLOR(  0, 255, 255)
#define GFX_RED				GFX_COLOR(255,   0,	  0)
#define GFX_MAGENTA		GFX_COLOR(255,   0, 255)
#define GFX_YELLOW		GFX_COLOR(255, 255,	  0)
#define GFX_WHITE			GFX_COLOR(255, 255, 255)

#define GFX_DARK_GREEN			GFX_COLOR(  0, 128,	  0)

#define GFX_DECL_SCREEN(name) void screen_##name##_render(screen_context_t* ctx)
#define GFX_RENDER_SCREEN(name) gfx_render_screen(&screen_##name##_render)

#define GFX_SCREEN_HEADER() \
	const struct BitmapFont *__gfx_current_font = 0; \
	uint8_t __gfx_font_size = 1; \
	gfx_pixel_t __gfx_last_background = 0; \
	uint16_t __gfx_rel_x = 0, __gfx_rel_y = 0;


#define GFX_DECL_PARTIAL_RENDER(name) void partial_render_##name(screen_t *screen, bool full_render)
#define GFX_RENDER_PARTIAL(name, screen) gfx_partial_render(&partial_render_##name, &screen_##screen##_render)

#define GFX_PARTIAL_RENDER_AREA(x, y, width, height) gfx_partial_render_area(screen, x, y, width, height)


// Some macro helper functions
#define __GFX_NARG(...) __GFX_NARG_(__VA_ARGS__, __GFX_RSEQ_N())
#define __GFX_NARG_(...) __GFX_ARG_N(__VA_ARGS__)
#define __GFX_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
#define __GFX_RSEQ_N() 9,8,7,6,5,4,3,2,1,0

#define __GFX_merge_2_(a, b) a##b
#define __GFX_merge_2(a, b) __GFX_merge_2_(a, b)

#define __GFX_VAFUNC(name, ...) __GFX_merge_2(name##_, __GFX_NARG(__VA_ARGS__))(__VA_ARGS__)

#define GFX_MIN_1(a) (a)
#define GFX_MIN_2(a, b) ((a) < (b) ? (a) : (b))
#define GFX_MIN_3(a, b, c) ((c) < (b) && (c) < (a) ? (c) : GFX_MIN_2(a, b))
#define GFX_MIN_4(a, b, c, d) ((d) < (c) && (d) < (b) && (d) < (a) ? (d) : GFX_MIN_3(a, b, c))

#define GFX_MIN(...) __GFX_merge_2(GFX_MIN_, __GFX_NARG(__VA_ARGS__))(__VA_ARGS__)


#define GFX_MAX_1(a) (a)
#define GFX_MAX_2(a, b) ((a) > (b) ? (a) : (b))
#define GFX_MAX_3(a, b, c) ((c) > (b) && (c) > (a) ? (c) : GFX_MAX_2(a, b))
#define GFX_MAX_4(a, b, c, d) ((d) > (c) && (d) > (b) && (d) > (a) ? (d) : GFX_MAX_3(a, b, c))

#define GFX_MAX(...) __GFX_merge_2(GFX_MAX_, __GFX_NARG(__VA_ARGS__))(__VA_ARGS__)


#define GFX_ABS(x, y) x, y
#define GFX_REL(x, y) __gfx_rel_x + (x), __gfx_rel_y + (y)

/***** -------======= Graphics library functions =======------- *****/
/**
 * Most of the drawing functions accept a _dynamic suffix in their name
 * which allows these elements to be redrawn on a screen redraw.
 */

#define GFX_BUFFER(x, y) ctx->sc_buffer[(x) + (y) * ctx->sc_width]

extern void gfx_clear(screen_context_t *ctx, gfx_pixel_t color);
extern void gfx_rect(screen_context_t *ctx, uint16_t x, uint16_t y, uint16_t width, uint16_t height, gfx_pixel_t color);
extern void gfx_frame(screen_context_t *ctx, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t thickness, gfx_pixel_t bg_color, gfx_pixel_t fg_color);
extern void gfx_text(screen_context_t *ctx, uint16_t x, uint16_t y, gfx_pixel_t bg_color, gfx_pixel_t fg_color, const struct BitmapFont *font, uint8_t scale, const char *text);

/**
 * Clears the whole screen with a single color
 * \param color Clear color
 * GFX_CLEAR(color)
 */
#define GFX_CLEAR(color) \
	if(ctx->sc_first_draw) gfx_clear(ctx, color); \
	__gfx_last_background = color; \
	GFX_AFTER_ELEMENT_HOOK()


#define _GFX_RECT_static(x, y, width, height, color) \
	if(ctx->sc_first_draw) gfx_rect(ctx, x, y, width, height, color); \
	__gfx_rel_x = x; __gfx_rel_y = y; \
	__gfx_last_background = color
#define _GFX_RECT_dynamic(x, y, width, height, color) \
	gfx_rect(ctx, x, y, width, height, color); \
	__gfx_rel_x = x; __gfx_rel_y = y; \
	__gfx_last_background = color

#define GFX_RECT_6(dyn, ...) __GFX_merge_2(_GFX_RECT_, dyn)(__VA_ARGS__)

/**
 * Draws a rectangle of the given color and dimensions
 * \param x Starting x coordinate of the rectangle
 * \param y Starting y coordinate of the rectangle
 * \param width Width of the rectangle
 * \param height Height of the rectangle
 * \param color Fill color of the rectangle
 * GFX_RECT[_dynamic](x, y, width, height, color)
 */
#define GFX_RECT(...) __GFX_VAFUNC(GFX_RECT, static, __VA_ARGS__); GFX_AFTER_ELEMENT_HOOK()
#define GFX_RECT_dynamic(x, y, width, height, color) __GFX_VAFUNC(GFX_RECT, dynamic, __VA_ARGS__); GFX_AFTER_ELEMENT_HOOK()


#define _GFX_FRAME_static(x, y, w, h, t, bg, fg) \
	if(ctx->sc_first_draw) gfx_frame(ctx, x, y, w, h, t, bg, fg); \
	__gfx_rel_x = (x) + (t); __gfx_rel_y = (y) + (t); \
	__gfx_last_background = bg
#define _GFX_FRAME_dynamic(x, y, w, h, t, bg, fg) \
	gfx_frame(ctx, x, y, w, h, t, bg, fg); \
	__gfx_rel_x = (x) + (t); __gfx_rel_y = (y) + (t); \
	__gfx_last_background = bg

#define GFX_FRAME_8(dyn, ...) __GFX_merge_2(_GFX_FRAME_, dyn)(__VA_ARGS__)

/**
 * Draws a rectangle with an outer border
 * \param x Starting x coordinate of the rectangle
 * \param y Starting y coordinate of the rectangle
 * \param width Width of the rectangle
 * \param height Height of the rectangle
 * \param thickness Border thickness
 * \param bg Color of the inner fill
 * \param fg Color of the border
 * GFX_FRAME[_dynamic](x, y, width, height, thickness, bg, fg)
 */
#define GFX_FRAME(...) __GFX_VAFUNC(GFX_FRAME, static, __VA_ARGS__); GFX_AFTER_ELEMENT_HOOK()
#define GFX_FRAME_dynamic(...) __GFX_VAFUNC(GFX_FRAME, dynamic, __VA_ARGS__); GFX_AFTER_ELEMENT_HOOK()

#define GFX_SET_FONT(font, size) __gfx_current_font = font; __gfx_font_size = size

#define _GFX_TEXT_static(x, y, bg, fg, font, scale, text) if(ctx->sc_first_draw) gfx_text(ctx, x, y, bg, fg, font, scale, text)
#define _GFX_TEXT_dynamic(x, y, bg, fg, font, scale, text) gfx_text(ctx, x, y, bg, fg, font, scale, text)

#define GFX_TEXT_8(dyn, x, y, bg, fg, font, scale, text) __GFX_merge_2(_GFX_TEXT_, dyn)(x, y, bg, fg, font, scale, text)
#define GFX_TEXT_6(dyn, x, y, bg, fg, text) GFX_TEXT_8(dyn, x, y, bg, fg, __gfx_current_font, __gfx_font_size, text)
#define GFX_TEXT_5(dyn, x, y, fg, text) GFX_TEXT_6(dyn, x, y, __gfx_last_background, fg, text)
#define GFX_TEXT_7(dyn, x, y, fg, font, scale, text) GFX_TEXT_8(dyn, x, y, __gfx_last_background, fg, font, scale, text)

/**
 * Draws a line of text (This function doesn't do multiline text!)
 * \param x Starting x coordinate of the text
 * \param y Starting y coordinate of the text
 * \param bg Background color of the text
 * \param fg Foreground color of the text
 * \param font Font pointer
 * \param scale Text scale
 * \param text A string of text to draw
 * GFX_TEXT[_dynamic](x, y, [bg], fg, [font, size], text)
 */
#define GFX_TEXT(...) __GFX_VAFUNC(GFX_TEXT, static, __VA_ARGS__); GFX_AFTER_ELEMENT_HOOK()
#define GFX_TEXT_dynamic(...) __GFX_VAFUNC(GFX_TEXT, dynamic, __VA_ARGS__); GFX_AFTER_ELEMENT_HOOK()

#ifdef __cplusplus
}
#endif

#endif

