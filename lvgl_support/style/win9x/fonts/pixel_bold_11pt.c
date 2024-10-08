/*
	Name: pixel_bold_11pt.c
	Description: Bitmapped font designed to imitate the Windows 9x font.

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 04/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#include "../config.h"

#ifdef GUI_STYLE_WIN9X

#include "lvgl.h"

static const uint8_t bitmaps[] = {
0x00, 0xff, 0xff, 0x0f, 0xde, 0xf6, 0x66, 0x66, 0xff, 0xff, 0x66, 0x66, 0x66, 0x66, 0xff, 0xff, 
0x66, 0x66, 0x18, 0x7e, 0xff, 0xdb, 0xd8, 0xf8, 0x7c, 0x1e, 0x1b, 0xdb, 0xff, 0x7e, 0x18, 0x43, 
0xe6, 0xa6, 0xec, 0x4c, 0x18, 0x18, 0x32, 0x37, 0x65, 0x67, 0xc2, 0x70, 0xe3, 0x66, 0xcd, 0x8e, 
0x1e, 0x6d, 0xcf, 0x9b, 0xfb, 0xd0, 0xfc, 0x6f, 0x6d, 0xb6, 0xd9, 0xb0, 0xd9, 0xb6, 0xdb, 0x6f, 
0x60, 0xab, 0xaa, 0xea, 0x80, 0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18, 0x6d, 0xe0, 0xff, 
0xff, 0xf0, 0x18, 0xc6, 0x63, 0x19, 0x8c, 0x66, 0x31, 0x80, 0x7d, 0xff, 0x1e, 0x3c, 0x7a, 0xf5, 
0xe3, 0xc7, 0x8f, 0xfb, 0xe0, 0x37, 0xff, 0x33, 0x33, 0x33, 0x33, 0x7d, 0xff, 0x18, 0x30, 0xe3, 
0x8e, 0x38, 0xe1, 0x83, 0xff, 0xf0, 0x7d, 0xff, 0x18, 0x30, 0x67, 0x8f, 0x03, 0x07, 0x8f, 0xfb, 
0xe0, 0x1c, 0x38, 0xf1, 0xe6, 0xcd, 0xb3, 0x7f, 0xfe, 0x18, 0x30, 0x60, 0xff, 0xff, 0x06, 0x0f, 
0xdf, 0xf1, 0x83, 0x07, 0x8f, 0xfb, 0xe0, 0x7d, 0xff, 0x1e, 0x0c, 0x1f, 0xbf, 0xe3, 0xc7, 0x8f, 
0xfb, 0xe0, 0xff, 0xfc, 0x18, 0x30, 0xc1, 0x86, 0x0c, 0x30, 0x60, 0xc1, 0x80, 0x7d, 0xff, 0x1e, 
0x3c, 0x6f, 0xbf, 0xe3, 0xc7, 0x8f, 0xfb, 0xe0, 0x7d, 0xff, 0x1e, 0x3c, 0x7f, 0xdf, 0x83, 0x07, 
0x8f, 0xfb, 0xe0, 0xf0, 0x0f, 0x6c, 0x00, 0x1b, 0xd8, 0x19, 0xdd, 0xce, 0x38, 0xe3, 0xff, 0xfc, 
0x00, 0x0f, 0xff, 0xc0, 0xc7, 0x1c, 0x73, 0xbb, 0x98, 0x7e, 0xff, 0xc3, 0x03, 0x03, 0x07, 0x0e, 
0x1c, 0x18, 0x00, 0x18, 0x18, 0x3f, 0x1f, 0xee, 0x1f, 0x03, 0xce, 0xf6, 0xbd, 0x2f, 0x6b, 0xcf, 
0xf0, 0xee, 0x01, 0xff, 0x3f, 0x80, 0x38, 0x70, 0xe3, 0x66, 0xcd, 0xb1, 0xe3, 0xff, 0xff, 0x1e, 
0x30, 0xfd, 0xff, 0x1e, 0x3c, 0x7f, 0xbf, 0x63, 0xc7, 0x8f, 0xff, 0xe0, 0x7d, 0xff, 0x9e, 0x0c, 
0x18, 0x30, 0x60, 0xc1, 0xcf, 0xfb, 0xe0, 0xf9, 0xfb, 0x3e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7, 0x9f, 
0xf7, 0xc0, 0xff, 0xff, 0x06, 0x0c, 0x1f, 0xbf, 0x60, 0xc1, 0x83, 0xff, 0xf0, 0xff, 0xff, 0x06, 
0x0c, 0x1f, 0x3e, 0x60, 0xc1, 0x83, 0x06, 0x00, 0x7d, 0xff, 0x9e, 0x3c, 0x18, 0x33, 0xe3, 0xc7, 
0xdf, 0xfb, 0xd0, 0xc7, 0x8f, 0x1e, 0x3c, 0x7f, 0xff, 0xe3, 0xc7, 0x8f, 0x1e, 0x30, 0xff, 0x66, 
0x66, 0x66, 0x66, 0xff, 0xff, 0x33, 0x33, 0x33, 0x3b, 0xf6, 0xc7, 0x9f, 0x77, 0xcf, 0x1c, 0x38, 
0x78, 0xf9, 0xbb, 0x3e, 0x30, 0xc1, 0x83, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc1, 0x83, 0xff, 0xf0, 
0xc1, 0xe0, 0xf8, 0xfe, 0xff, 0xfe, 0xef, 0x27, 0x83, 0xc1, 0xe0, 0xf0, 0x78, 0x30, 0xc7, 0x8f, 
0x9f, 0x3f, 0x7e, 0xf7, 0xef, 0xcf, 0x9f, 0x1e, 0x30, 0x7d, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 
0xc7, 0x8f, 0xfb, 0xe0, 0xfd, 0xff, 0x1e, 0x3c, 0x7f, 0xff, 0x60, 0xc1, 0x83, 0x06, 0x00, 0x7d, 
0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xd7, 0x9f, 0xf3, 0xd0, 0xfd, 0xff, 0x1e, 0x3c, 0x7f, 0xff, 
0x66, 0xc7, 0x8f, 0x1e, 0x30, 0x7d, 0xff, 0x1e, 0x0e, 0x0f, 0x0f, 0x07, 0x07, 0x8f, 0xfb, 0xe0, 
0xff, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xc3, 0xc3, 0xc3, 0xc3, 
0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e, 0x3c, 0xc7, 0x8f, 0x1e, 0x3c, 0x6d, 0x9b, 0x36, 0x6c, 
0x70, 0xe1, 0xc0, 0xc0, 0xf0, 0x3c, 0x0f, 0x03, 0xc0, 0xdb, 0x66, 0xd9, 0xb6, 0x6d, 0x8f, 0xc3, 
0x30, 0xcc, 0xc7, 0x8f, 0x1f, 0x76, 0xc7, 0x0e, 0x36, 0xef, 0x8f, 0x1e, 0x30, 0xc3, 0xc3, 0xc3, 
0x66, 0x66, 0x3c, 0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff, 0xfc, 0x18, 0x30, 0xc3, 0x0c, 0x30, 
0xc1, 0x83, 0xff, 0xf0, 0xff, 0xcc, 0xcc, 0xcc, 0xcc, 0xff, 0xc6, 0x30, 0xc6, 0x30, 0xc6, 0x30, 
0xc6, 0x30, 0xff, 0x33, 0x33, 0x33, 0x33, 0xff, 0x10, 0x71, 0xb6, 0x38, 0x20, 0xff, 0xfc, 0xdd, 
0x80, 0x3c, 0xfc, 0x1b, 0xff, 0xf8, 0xf1, 0xff, 0x7a, 0xc1, 0x83, 0x07, 0xcf, 0xd9, 0xf1, 0xe3, 
0xcf, 0xfb, 0xe0, 0x7d, 0xff, 0x9e, 0x0c, 0x1c, 0xff, 0xbe, 0x06, 0x0c, 0x19, 0xf7, 0xfc, 0xf1, 
0xe3, 0xe6, 0xfc, 0xf8, 0x7d, 0xff, 0x1f, 0xfc, 0x18, 0xff, 0xbe, 0x3d, 0xf7, 0x18, 0x63, 0xcf, 
0x18, 0x61, 0x86, 0x00, 0x7f, 0xff, 0x1e, 0x3c, 0x7f, 0xdf, 0x83, 0xc7, 0xfd, 0xf0, 0xc1, 0x83, 
0x06, 0x0f, 0xdf, 0xf1, 0xe3, 0xc7, 0x8f, 0x18, 0x66, 0x06, 0xee, 0x66, 0x73, 0x6c, 0x3f, 0xdb, 
0x6f, 0xe0, 0xc3, 0x0c, 0x33, 0xdf, 0xef, 0x3e, 0xdf, 0x3c, 0xc0, 0xdb, 0x6d, 0xb6, 0xdd, 0x80, 
0xee, 0xff, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xfd, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0x7d, 
0xff, 0x1e, 0x3c, 0x78, 0xff, 0xbe, 0xfd, 0xff, 0x1e, 0x3c, 0x78, 0xff, 0xfe, 0xc1, 0x83, 0x00, 
0x7f, 0xff, 0x1e, 0x3c, 0x78, 0xff, 0xbf, 0x06, 0x0c, 0x18, 0xbd, 0xff, 0x9e, 0x0c, 0x18, 0x30, 
0x60, 0x7d, 0xff, 0x07, 0xe7, 0xe0, 0xff, 0xbe, 0x66, 0xff, 0x66, 0x66, 0x67, 0x30, 0xcf, 0x3c, 
0xf3, 0xcf, 0x7f, 0xdd, 0xc7, 0x8f, 0x1b, 0x66, 0xcd, 0x8e, 0x1c, 0xc3, 0xc3, 0xdb, 0xdb, 0xdb, 
0xdb, 0x7e, 0x66, 0xc7, 0xdd, 0xb1, 0xc3, 0x8d, 0xbb, 0xe3, 0xc7, 0x8f, 0x1e, 0x3c, 0x7f, 0xdf, 
0x83, 0xc7, 0xfd, 0xf0, 0xff, 0xf0, 0xce, 0x73, 0x0f, 0xff, 0x37, 0x66, 0x6c, 0xc6, 0x66, 0x73, 
0xff, 0xff, 0xff, 0xce, 0x66, 0x63, 0x36, 0x66, 0xec, 0x67, 0xf9, 0x80, };
static const lv_font_fmt_txt_glyph_dsc_t glyphs[] = {{ 0, 0, 0, 0, 0 },
{ 0, 80, 0, 0, 0, 12 },
{ 1, 48, 2, 12, 0, 0 },
{ 4, 96, 5, 3, 0, 9 },
{ 6, 144, 8, 12, 0, 0 },
{ 18, 144, 8, 13, 0, -1 },
{ 31, 144, 8, 12, 0, 0 },
{ 43, 128, 7, 12, 0, 0 },
{ 54, 48, 2, 3, 0, 9 },
{ 55, 64, 3, 12, 0, 0 },
{ 60, 64, 3, 12, 0, 0 },
{ 65, 96, 5, 5, 0, 7 },
{ 69, 144, 8, 8, 0, 2 },
{ 77, 64, 3, 4, 0, -2 },
{ 79, 144, 8, 2, 0, 5 },
{ 81, 48, 2, 2, 0, 0 },
{ 82, 96, 5, 12, 0, 0 },
{ 90, 128, 7, 12, 0, 0 },
{ 101, 80, 4, 12, 0, 0 },
{ 107, 128, 7, 12, 0, 0 },
{ 118, 128, 7, 12, 0, 0 },
{ 129, 128, 7, 12, 0, 0 },
{ 140, 128, 7, 12, 0, 0 },
{ 151, 128, 7, 12, 0, 0 },
{ 162, 128, 7, 12, 0, 0 },
{ 173, 128, 7, 12, 0, 0 },
{ 184, 128, 7, 12, 0, 0 },
{ 195, 48, 2, 8, 0, 2 },
{ 197, 64, 3, 10, 0, 0 },
{ 201, 96, 5, 8, 0, 2 },
{ 206, 128, 7, 6, 0, 3 },
{ 212, 96, 5, 8, 0, 2 },
{ 217, 144, 8, 12, 0, 0 },
{ 229, 176, 10, 13, 0, -1 },
{ 246, 128, 7, 12, 0, 0 },
{ 257, 128, 7, 12, 0, 0 },
{ 268, 128, 7, 12, 0, 0 },
{ 279, 128, 7, 12, 0, 0 },
{ 290, 128, 7, 12, 0, 0 },
{ 301, 128, 7, 12, 0, 0 },
{ 312, 128, 7, 12, 0, 0 },
{ 323, 128, 7, 12, 0, 0 },
{ 334, 80, 4, 12, 0, 0 },
{ 340, 80, 4, 12, 0, 0 },
{ 346, 128, 7, 12, 0, 0 },
{ 357, 128, 7, 12, 0, 0 },
{ 368, 160, 9, 12, 0, 0 },
{ 382, 128, 7, 12, 0, 0 },
{ 393, 128, 7, 12, 0, 0 },
{ 404, 128, 7, 12, 0, 0 },
{ 415, 128, 7, 12, 0, 0 },
{ 426, 128, 7, 12, 0, 0 },
{ 437, 128, 7, 12, 0, 0 },
{ 448, 144, 8, 12, 0, 0 },
{ 460, 144, 8, 12, 0, 0 },
{ 472, 128, 7, 12, 0, 0 },
{ 483, 176, 10, 12, 0, 0 },
{ 498, 128, 7, 12, 0, 0 },
{ 509, 144, 8, 12, 0, 0 },
{ 521, 128, 7, 12, 0, 0 },
{ 532, 80, 4, 12, 0, 0 },
{ 538, 96, 5, 12, 0, 0 },
{ 546, 80, 4, 12, 0, 0 },
{ 552, 128, 7, 5, 0, 7 },
{ 557, 112, 7, 2, 0, 0 },
{ 559, 64, 3, 3, 0, 9 },
{ 561, 128, 7, 9, 0, 0 },
{ 569, 128, 7, 11, 0, 0 },
{ 579, 128, 7, 8, 0, 0 },
{ 586, 128, 7, 11, 0, 0 },
{ 596, 128, 7, 8, 0, 0 },
{ 603, 112, 6, 11, 0, 0 },
{ 612, 128, 7, 11, 0, -3 },
{ 622, 128, 7, 11, 0, 0 },
{ 632, 80, 4, 10, 0, 0 },
{ 637, 64, 3, 12, 0, -2 },
{ 642, 112, 6, 11, 0, 0 },
{ 651, 64, 3, 11, 0, 0 },
{ 656, 144, 8, 8, 0, 0 },
{ 664, 128, 7, 8, 0, 0 },
{ 671, 128, 7, 8, 0, 0 },
{ 678, 128, 7, 11, 0, -3 },
{ 688, 128, 7, 11, 0, -3 },
{ 698, 128, 7, 8, 0, 0 },
{ 705, 128, 7, 8, 0, 0 },
{ 712, 80, 4, 11, 0, 0 },
{ 718, 112, 6, 8, 0, 0 },
{ 724, 128, 7, 8, 0, 0 },
{ 731, 144, 8, 8, 0, 0 },
{ 739, 128, 7, 8, 0, 0 },
{ 746, 128, 7, 11, 0, -3 },
{ 756, 112, 6, 8, 0, 0 },
{ 762, 80, 4, 12, 0, 0 },
{ 768, 48, 2, 12, 0, 0 },
{ 771, 80, 4, 12, 0, 0 },
{ 777, 112, 6, 3, 0, 8 },
};
static const lv_font_fmt_txt_cmap_t cmaps[] = { {
.range_start = 32, .range_length = 95, .glyph_id_start = 1,
.unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
} };
static const lv_font_fmt_txt_dsc_t font_dsc = {.glyph_bitmap = bitmaps,
.glyph_dsc = glyphs,
.cmaps = cmaps,
.kern_dsc = NULL,
.kern_scale = 0,
.cmap_num = 1,
.bpp = 1,
.kern_classes = 0,
.bitmap_format = 0,
};
const lv_font_t font_pixel_bold_11pt = {
.get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,
.get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,
.line_height = 15,
.base_line = 3,
.subpx = LV_FONT_SUBPX_NONE,
.underline_position = 0,
.underline_thickness = 0,
.dsc = &font_dsc
};

#endif
