// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.2
// LVGL version: 9.1.0
// Project name: uCNC-GFX

#ifndef _UI_THEMES_H
#define _UI_THEMES_H

#ifdef __cplusplus
extern "C" {
#endif

#define UI_THEME_COLOR_BUTTON 0
#define UI_THEME_COLOR_BUTTONPRESSED 1
#define UI_THEME_COLOR_ALARM 2
#define UI_THEME_COLOR_HOLD 3
#define UI_THEME_COLOR_ALARMPRESSED 4
#define UI_THEME_COLOR_CHECKED 5
#define UI_THEME_COLOR_CONTAINER 6

#define UI_THEME_DEFAULT 0

extern const ui_theme_variable_t _ui_theme_color_button[1];
extern const ui_theme_variable_t _ui_theme_alpha_button[1];

extern const ui_theme_variable_t _ui_theme_color_buttonpressed[1];
extern const ui_theme_variable_t _ui_theme_alpha_buttonpressed[1];

extern const ui_theme_variable_t _ui_theme_color_alarm[1];
extern const ui_theme_variable_t _ui_theme_alpha_alarm[1];

extern const ui_theme_variable_t _ui_theme_color_hold[1];
extern const ui_theme_variable_t _ui_theme_alpha_hold[1];

extern const ui_theme_variable_t _ui_theme_color_alarmpressed[1];
extern const ui_theme_variable_t _ui_theme_alpha_alarmpressed[1];

extern const ui_theme_variable_t _ui_theme_color_checked[1];
extern const ui_theme_variable_t _ui_theme_alpha_checked[1];

extern const ui_theme_variable_t _ui_theme_color_container[1];
extern const ui_theme_variable_t _ui_theme_alpha_container[1];

extern const uint32_t * ui_theme_colors[1];
extern const uint8_t * ui_theme_alphas[1];
extern uint8_t ui_theme_idx;

void ui_theme_set(uint8_t theme_idx);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
