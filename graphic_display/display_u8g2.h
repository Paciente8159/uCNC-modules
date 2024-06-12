#ifndef DISPLAY_U8G2_H
#define DISPLAY_U8G2_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <clib/u8g2.h>
#include <clib/u8x8.h>

	extern u8g2_t graphiclcd_u8g2;
#define U8G2 ((u8g2_t *)&graphiclcd_u8g2)

#define LCDWIDTH u8g2_GetDisplayWidth(U8G2)
#define LCDHEIGHT u8g2_GetDisplayHeight(U8G2)
#define FONTHEIGHT (u8g2_GetAscent(U8G2) - u8g2_GetDescent(U8G2))
#define ALIGN_CENTER(t) ((LCDWIDTH - u8g2_GetUTF8Width(U8G2, t)) / 2)
#define ALIGN_RIGHT(t) (LCDWIDTH - u8g2_GetUTF8Width(U8G2, t))
#define ALIGN_LEFT 0
#define JUSTIFY_CENTER ((LCDHEIGHT + FONTHEIGHT) / 2)
#define JUSTIFY_BOTTOM (LCDHEIGHT + u8g2_GetDescent(U8G2))
#define JUSTIFY_TOP u8g2_GetAscent(U8G2)
#define TEXT_WIDTH(t) u8g2_GetUTF8Width(U8G2, t)

#if (GRAPHIC_DISPLAY_INTERFACE & (GRAPHIC_DISPLAY_SW_SPI | GRAPHIC_DISPLAY_HW_SPI))
	uint8_t u8x8_byte_ucnc_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
#elif (GRAPHIC_DISPLAY_INTERFACE & (GRAPHIC_DISPLAY_SW_I2C | GRAPHIC_DISPLAY_HW_I2C))
uint8_t u8x8_byte_ucnc_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
#endif

	uint8_t u8x8_gpio_and_delay_ucnc(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

#ifndef u8g2_driver_init
#define u8g2_driver_init() u8g2_Setup_st7920_s_128x64_f(U8G2, U8G2_R0, u8x8_byte_ucnc_hw_spi, u8x8_gpio_and_delay_ucnc)
#endif

#ifndef display_init
#define display_init()          \
	{                             \
		u8g2_driver_init();         \
		u8g2_InitDisplay(U8G2);     \
		u8g2_ClearDisplay(U8G2);    \
		u8g2_SetPowerSave(U8G2, 0); \
		u8g2_FirstPage(U8G2);       \
	}
#endif

#ifndef display_clear
#define display_clear() u8g2_ClearBuffer(U8G2)
#endif

#ifndef display_flush
#define display_flush() u8g2_SendBuffer(U8G2)
#endif

#ifndef display_print
#define display_print(X, Y, STR) u8g2_DrawStr(U8G2, (X), (Y), (STR));
#endif

#ifndef display_line
#define display_line(X1, Y1, X2, Y2) u8g2_DrawLine(U8G2, (X1), (Y1), (X2), (Y2));
#endif

#ifndef display_setcolor
#define display_setcolor(COLOR) u8g2_SetDrawColor(U8G2, (COLOR));
#endif

#ifndef display_box
#define display_box(X, Y, W, H) u8g2_DrawBox(U8G2, (X), (Y), (W), (H))
#endif

#ifndef display_box_text
#define display_box_text(X, Y, STR, PAD_V, PAD_H, INV) u8g2_DrawButtonUTF8(U8G2, (X), (Y), ((INV)?U8G2_BTN_INV:U8G2_BTN_BW1), (PAD_V), (PAD_H))
#endif

#ifndef display_triangle
#define display_triangle(X1, Y1, X2, Y2,X3, Y3) u8g2_DrawTriangle(U8G2, (X1), (Y1), (X2), (Y2), (X3), (Y3))
#endif

#ifndef display_startup_draw
#define display_startup_draw()                                                                                    \
	{                                                                                                               \
		u8g2_ClearBuffer(U8G2);                                                                                       \
		char buff[SYSTEM_MENU_MAX_STR_LEN];                                                                           \
		rom_strcpy(buff, __romstr__("µCNC"));                                                                         \
		u8g2_ClearBuffer(U8G2);                                                                                       \
		u8g2_SetFont(U8G2, u8g2_font_9x15_t_symbols);                                                                 \
		u8g2_DrawUTF8X2(U8G2, (LCDWIDTH / 2 - u8g2_GetUTF8Width(U8G2, buff)), JUSTIFY_CENTER - FONTHEIGHT / 2, buff); \
		rom_strcpy(buff, __romstr__(("v" CNC_VERSION)));                                                              \
		u8g2_SetFont(U8G2, u8g2_font_6x12_tr);                                                                        \
		u8g2_DrawStr(U8G2, ALIGN_CENTER(buff), JUSTIFY_CENTER + FONTHEIGHT, buff);                                    \
		u8g2_SendBuffer(U8G2);                                                                                        \
	}
#endif

#ifdef __cplusplus
}
#endif

#endif