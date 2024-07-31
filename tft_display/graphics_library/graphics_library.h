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

#include "font.h"

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
  void *sc_arg;
} screen_context_t;

typedef struct screen {
	void (*render_cb)(screen_context_t*);
	void (*dynamic_render_cb)(void);
	uint16_t s_width, s_height;
} screen_t;

extern void gfx_render_screen(screen_t *screen, void *argument);
extern void gfx_render_area(screen_t *screen, void *argument, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
extern bool gfx_is_primary(screen_t *screen);

#define GFX_BLACK			GFX_COLOR(  0,   0,	  0)
#define GFX_BLUE			GFX_COLOR(  0,   0, 255)
#define GFX_GREEN			GFX_COLOR(  0, 255,	  0)
#define GFX_CYAN			GFX_COLOR(  0, 255, 255)
#define GFX_RED				GFX_COLOR(255,   0,	  0)
#define GFX_MAGENTA		GFX_COLOR(255,   0, 255)
#define GFX_YELLOW		GFX_COLOR(255, 255,	  0)
#define GFX_WHITE			GFX_COLOR(255, 255, 255)

#define GFX_DARK_GREEN			GFX_COLOR(  0, 128,	  0)

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

#define GFX_RENDER_SCREEN_1(name) gfx_render_screen(&screen_##name, 0)
#define GFX_RENDER_SCREEN_2(name, arg) gfx_render_screen(&screen_##name, arg)

#define GFX_DECL_SCREEN_4(name, width, height, dyn_cb) \
	void screen_##name##_render(screen_context_t* ctx); \
	screen_t screen_##name = { \
		.render_cb = &screen_##name##_render, \
		.dynamic_render_cb = dyn_cb, \
		.s_width = width, \
		.s_height = height \
	}; \
	void screen_##name##_render(screen_context_t* ctx)

#define GFX_DECL_SCREEN_3(name, width, height) GFX_DECL_SCREEN_4(name, width, height, 0)
#define GFX_DECL_SCREEN_2(name, dyn_cb) GFX_DECL_SCREEN_4(name, GFX_DISPLAY_WIDTH, GFX_DISPLAY_HEIGHT, dyn_cb)
#define GFX_DECL_SCREEN_1(name) GFX_DECL_SCREEN_4(name, GFX_DISPLAY_WIDTH, GFX_DISPLAY_HEIGHT, 0)

/**
 * Declare a screen
 * @param name Name of the screen
 * @param width Width of the screen
 * @param height Height of the screen
 * @param dyn_cb Function called when a rerender happens, can be used to specify areas to render
 * GFX_DECL_SCREEN(name, [width, height], [dyn_cb]) {
 *   GFX_SCREEN_HEADER();
 *   // Screen elements...
 * }
 */
#define GFX_DECL_SCREEN(...) __GFX_VAFUNC(GFX_DECL_SCREEN, __VA_ARGS__)

#define GFX_INCLUDE_SCREEN(name) extern screen_t screen_##name

/**
 * Render a screen
 * @param name Name of the screen
 * @param arg Argument to pass to the screen
 * GFX_RENDER_SCREEN(name, [arg])
 */
#define GFX_RENDER_SCREEN(...) __GFX_VAFUNC(GFX_RENDER_SCREEN, __VA_ARGS__)

#define GFX_RENDER_AREA_6(name, x, y, width, height, arg) gfx_render_area(&screen_##name, arg, x, y, width, height)
#define GFX_RENDER_AREA_5(name, x, y, width, height) gfx_render_area(&screen_##name, 0, x, y, width, height)

/**
 * Render an area of a screen
 * @param name Name of the screen to render
 * @param x Starting x coordinate
 * @param y Starting y coordinate
 * @param width Width of the area
 * @param height Height of the area
 * @param arg Argument to pass to the screen
 * GFX_RENDER_AREA(name, x, y, width, height, [arg])
 */
#define GFX_RENDER_AREA(...) __GFX_VAFUNC(GFX_RENDER_AREA, __VA_ARGS__)

/**
 * Used to access the screen argument passed when rendering
 */
#define GFX_SCREEN_ARG(type) ((type*) ctx->sc_arg)

/**
 * Declares necessary variables for other graphics functions.
 * Always place at the top of the screen declaration.
 */
#define GFX_SCREEN_HEADER() \
	const struct BitmapFont *__gfx_current_font = 0; \
	uint8_t __gfx_font_size = 1; \
	gfx_pixel_t __gfx_last_background = 0; \
	uint16_t __gfx_rel_x = 0, __gfx_rel_y = 0

#define GFX_PUSH_STATE() { \
	const struct BitmapFont *__push__gfx_current_font = __gfx_current_font; \
	uint8_t __push__gfx_font_size = __gfx_font_size; \
	gfx_pixel_t __push__gfx_last_background = __gfx_last_background; \
	uint16_t __push__gfx_rel_x = __gfx_rel_x, __push__gfx_rel_y = __gfx_rel_y; \
	const struct BitmapFont *__gfx_current_font = __push__gfx_current_font; \
	uint8_t __gfx_font_size = __push__gfx_font_size; \
	gfx_pixel_t __gfx_last_background = __push__gfx_last_background; \
	uint16_t __gfx_rel_x = __push__gfx_rel_x, __gfx_rel_y = __push__gfx_rel_y

#define GFX_POP_STATE() }

#define GFX_BUFFER(x, y) ctx->sc_buffer[(x) + (y) * ctx->sc_width]

#define GFX_COLOR_3(r, g, b) GFX_COLOR_INTERNAL(r, g, b)
#define GFX_COLOR_1(rgb_hex) GFX_COLOR_INTERNAL(((rgb_hex) >> 16) & 0xFF, ((rgb_hex) >> 8) & 0xFF, (rgb_hex) & 0xFF)

/**
 * Used to convert standard 8 bit RGB values into proper gfx_pixel_t values.
 * You can either use 3 parameters to specify separate red, green and blue
 * values or use one number to specify all three channels at once in 
 * this format: 0xRRGGBB.
 */
#define GFX_COLOR(...) __GFX_VAFUNC(GFX_COLOR, __VA_ARGS__)

/**
 * Check if the given screen is currently the primary screen
 */
#define GFX_IS_PRIMARY(name) gfx_is_primary(&screen_##name)

extern void gfx_clear(screen_context_t *ctx, gfx_pixel_t color);
extern void gfx_rect(screen_context_t *ctx, uint16_t x, uint16_t y, uint16_t width, uint16_t height, gfx_pixel_t color);
extern void gfx_frame(screen_context_t *ctx, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t thickness, gfx_pixel_t bg_color, gfx_pixel_t fg_color);
extern void gfx_text(screen_context_t *ctx, uint16_t x, uint16_t y, gfx_pixel_t bg_color, gfx_pixel_t fg_color, const struct BitmapFont *font, uint8_t scale, const char *text);
extern void gfx_bitmap(screen_context_t *ctx, uint16_t x, uint16_t y, uint16_t width, uint16_t height, gfx_pixel_t bg_color, gfx_pixel_t fg_color, const void *bitmap, uint8_t scale);
extern void gfx_palette_bitmap(screen_context_t *ctx, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t bpp, const gfx_pixel_t *color_map, const void *bitmap, uint8_t scale);
extern void gfx_invalidate();

extern int16_t gfx_text_center_offset(uint16_t container_width, const struct BitmapFont *font, uint8_t scale, const char *text);
extern void gfx_text_size(uint16_t *width, uint16_t *height, const struct BitmapFont *font, uint8_t scale, const char *text);

/**
 * Clears the whole screen with a single color
 * \param color Clear color
 * GFX_CLEAR(color)
 */
#define GFX_CLEAR(color) \
	if(ctx->sc_first_draw) { gfx_clear(ctx, color); } \
	__gfx_last_background = color; \
	GFX_AFTER_ELEMENT_HOOK()

#define GFX_SET_BACKGROUND(color) __gfx_last_background = color;

#define _GFX_SET_ORIGIN(x, y) __gfx_rel_x = x; __gfx_rel_y = y;
#define GFX_SET_ORIGIN(...) _GFX_SET_ORIGIN(__VA_ARGS__)

#define GFX_BACKGROUND __gfx_last_background

#define _GFX_RECT_static(x, y, width, height, color) if(ctx->sc_first_draw) gfx_rect(ctx, x, y, width, height, color);
#define _GFX_RECT_dynamic(x, y, width, height, color) gfx_rect(ctx, x, y, width, height, color);

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
#define GFX_RECT_dynamic(...) __GFX_VAFUNC(GFX_RECT, dynamic, __VA_ARGS__); GFX_AFTER_ELEMENT_HOOK()

#define _GFX_CONTAINER(x, y, width, height, color) __GFX_VAFUNC(GFX_RECT, static, x, y, width, height, color); \
	__gfx_rel_x = x; __gfx_rel_y = y; \
	__gfx_last_background = color; \
	GFX_AFTER_ELEMENT_HOOK()

/**
 * Draws a rectangle while also setting variables in screen header
 * for relative positioning and background coloring.
 * \param x Starting x coordinate of the container
 * \param y Starting y coordinate of the container
 * \param width Width of the container
 * \param height Height of the container
 * \param color Fill color of the container
 * GFX_CONTAINER[_dynamic](x, y, width, height, color)
 */
#define GFX_CONTAINER(...) _GFX_CONTAINER(__VA_ARGS__)

#define GFX_CONTAINER_dynamic(x, y, width, height, color) __GFX_VAFUNC(GFX_RECT, dynamic, x, y, width, height, color); \
	__gfx_rel_x = x; __gfx_rel_y = y; \
	__gfx_last_background = color; \
	GFX_AFTER_ELEMENT_HOOK()

#define _GFX_FRAME_static(x, y, w, h, t, bg, fg) \
	if(ctx->sc_first_draw) { gfx_frame(ctx, x, y, w, h, t, bg, fg); } \
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
#define GFX_SET_FONT_SIZE(size) __gfx_font_size = size

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

#define GFX_CENTER_TEXT_OFFSET_2(width, text) gfx_text_center_offset(width, __gfx_current_font, __gfx_font_size, text)
#define GFX_CENTER_TEXT_OFFSET_4(width, font, size, text) gfx_text_center_offset(width, font, size, text)

/**
 * Calculates x coordinate offset required for the text to be
 * centered in a container of a given width
 * \param width Width of the container
 * \param font Font pointer
 * \param scale Text scale
 * \param text Text
 * GFX_CENTER_TEXT_OFFSET(width, [font, scale], text)
 */
#define GFX_CENTER_TEXT_OFFSET(...) __GFX_VAFUNC(GFX_CENTER_TEXT_OFFSET, __VA_ARGS__)

#define _GFX_BITMAP_static(x, y, w, h, bg, fg, bitmap, scale) if(ctx->sc_first_draw) gfx_bitmap(ctx, x, y, w, h, bg, fg, bitmap, scale)
#define _GFX_BITMAP_dynamic(x, y, w, h, bg, fg, bitmap, scale) gfx_bitmap(ctx, x, y, w, h, bg, fg, bitmap, scale)

#define GFX_BITMAP_9(dyn, ...) __GFX_merge_2(_GFX_BITMAP_, dyn)(__VA_ARGS__)
#define GFX_BITMAP_8(dyn, ...) __GFX_merge_2(_GFX_BITMAP_, dyn)(__VA_ARGS__, 1)

/**
 * Draws a 2 color bitmap
 * \param x Starting x coordinate
 * \param y Starting y coordinate
 * \param width Bitmap width
 * \param height Bitmap height
 * \param bg Background color of the bitmap
 * \param fg Foreground color of the bitmap
 * \param bitmap Pointer to the bitmap
 * \param scale Scale multiplier
 * GFX_BITMAP[_dynamic](x, y, width, height, bg, fg, bitmap, [scale])
 */
#define GFX_BITMAP(...) __GFX_VAFUNC(GFX_BITMAP, static, __VA_ARGS__); GFX_AFTER_ELEMENT_HOOK()
#define GFX_BITMAP_dynamic(...) __GFX_VAFUNC(GFX_BITMAP, dynamic, __VA_ARGS__); GFX_AFTER_ELEMENT_HOOK()

#define _GFX_PALETTE_BITMAP_static(...) if(ctx->sc_first_draw) gfx_palette_bitmap(ctx, __VA_ARGS__)
#define _GFX_PALETTE_BITMAP_dynamic(...) gfx_palette_bitmap(ctx, __VA_ARGS__)

#define GFX_PALETTE_BITMAP_9(dyn, ...) __GFX_merge_2(_GFX_PALETTE_BITMAP_, dyn)(__VA_ARGS__)
#define GFX_PALETTE_BITMAP_8(dyn, ...) __GFX_merge_2(_GFX_PALETTE_BITMAP_, dyn)(__VA_ARGS__, 1)

/**
 * Draws a multicolor bitmap using a provided color palette
 * \param x Starting x coordinate
 * \param y Starting y coordinate
 * \param width Bitmap width
 * \param height Bitmap height
 * \param bpp Bits per pixels
 * \param color_map Color map
 * \param bitmap Pointer to bitmap
 * \param scale Scale multiplier
 * GFX_PALETTE_BITMAP[_dynamic](x, y, width, height, bpp, color_map, bitmap, [scale])
 */
#define GFX_PALETTE_BITMAP(...) __GFX_VAFUNC(GFX_PALETTE_BITMAP, static, __VA_ARGS__); GFX_AFTER_ELEMENT_HOOK()
#define GFX_PALETTE_BITMAP_dynamic(...) __GFX_VAFUNC(GFX_PALETTE_BITMAP, dynamic, __VA_ARGS__); GFX_AFTER_ELEMENT_HOOK()


#define GFX_DECL_ELEMENT(name, ...) void gfx_element_##name(screen_context_t *ctx, \
	uint16_t __gfx_rel_x, uint16_t __gfx_rel_y, const struct BitmapFont *__gfx_current_font, \
	uint8_t __gfx_font_size, gfx_pixel_t __gfx_last_background, __VA_ARGS__)

#define GFX_DRAW_ELEMENT(name, ...) if(ctx->sc_first_draw) gfx_element_##name(ctx, __gfx_rel_x, __gfx_rel_y, __gfx_current_font, __gfx_font_size, __gfx_last_background, __VA_ARGS__); GFX_AFTER_ELEMENT_HOOK()
#define GFX_DRAW_ELEMENT_dynamic(name, ...) gfx_element_##name(ctx, __gfx_rel_x, __gfx_rel_y, __gfx_current_font, __gfx_font_size, __gfx_last_background, __VA_ARGS__); GFX_AFTER_ELEMENT_HOOK()

#ifdef __cplusplus
}
#endif

#endif

