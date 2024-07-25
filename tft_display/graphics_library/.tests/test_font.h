#include "../fonts/font.h"

const uint8_t TestFontBitmaps[] = {
  /* 0: */ 0x80,
	/* 1: */ 0x98,
};

const struct BitmapFontGlyph TestFontGlyphs[] = {
	{ 0, 1, 1, 2, 0, -1 }, // 0x01
	{ 1, 2, 3, 4, 1, -2 }, // 0x02
};

const struct BitmapFont TestFont = {
  TestFontBitmaps,
  TestFontGlyphs,
  0x01, 0x02, 5, 3
};

