// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.2
// LVGL version: 9.1.0
// Project name: uCNC-GFX

#include "ui.h"

void ui_edit_screen_init(void)
{
    ui_edit = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_edit, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_flex_flow(ui_edit, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_edit, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_border_color(ui_edit, lv_color_hex(0x303030), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_edit, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_edit, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_edit, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_edit_panel_header = lv_obj_create(ui_edit);
    lv_obj_set_width(ui_edit_panel_header, lv_pct(100));
    lv_obj_set_height(ui_edit_panel_header, LV_SIZE_CONTENT);    /// 50
    lv_obj_set_align(ui_edit_panel_header, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_edit_panel_header, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_edit_panel_header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_edit_panel_header, lv_color_hex(0x303030), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_edit_panel_header, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_edit_panel_header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_edit_panel_header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_edit_panel_header, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_edit_panel_header, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_edit_label_headerlabel = lv_label_create(ui_edit_panel_header);
    lv_obj_set_width(ui_edit_label_headerlabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_edit_label_headerlabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_edit_label_headerlabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_edit_label_headerlabel, "text");

    ui_edit_button_btnclose = lv_button_create(ui_edit_panel_header);
    lv_obj_set_height(ui_edit_button_btnclose, 50);
    lv_obj_set_width(ui_edit_button_btnclose, LV_SIZE_CONTENT);   /// 100
    lv_obj_set_align(ui_edit_button_btnclose, LV_ALIGN_RIGHT_MID);
    lv_obj_add_flag(ui_edit_button_btnclose, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_remove_flag(ui_edit_button_btnclose, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    ui_object_set_themeable_style_property(ui_edit_button_btnclose, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
                                           _ui_theme_color_alarm);
    ui_object_set_themeable_style_property(ui_edit_button_btnclose, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_OPA,
                                           _ui_theme_alpha_alarm);
    ui_object_set_themeable_style_property(ui_edit_button_btnclose, LV_PART_MAIN | LV_STATE_PRESSED, LV_STYLE_BG_COLOR,
                                           _ui_theme_color_alarmpressed);
    ui_object_set_themeable_style_property(ui_edit_button_btnclose, LV_PART_MAIN | LV_STATE_PRESSED, LV_STYLE_BG_OPA,
                                           _ui_theme_alpha_alarmpressed);
    lv_obj_set_style_border_color(ui_edit_button_btnclose, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(ui_edit_button_btnclose, 255, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(ui_edit_button_btnclose, 2, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_side(ui_edit_button_btnclose, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_FOCUSED);

    ui_edit_image_image2 = lv_image_create(ui_edit_button_btnclose);
    lv_image_set_src(ui_edit_image_image2, &ui_img_cross_png);
    lv_obj_set_width(ui_edit_image_image2, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_edit_image_image2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_edit_image_image2, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_edit_image_image2, LV_OBJ_FLAG_CLICKABLE);     /// Flags
    lv_obj_remove_flag(ui_edit_image_image2, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_edit_container_bodyedit = lv_obj_create(ui_edit);
    lv_obj_remove_style_all(ui_edit_container_bodyedit);
    lv_obj_set_width(ui_edit_container_bodyedit, lv_pct(100));
    lv_obj_set_height(ui_edit_container_bodyedit, LV_SIZE_CONTENT);    /// 50
    lv_obj_set_align(ui_edit_container_bodyedit, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_edit_container_bodyedit, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_edit_textarea_vareditinput = lv_textarea_create(ui_edit_container_bodyedit);
    lv_obj_set_width(ui_edit_textarea_vareditinput, lv_pct(75));
    lv_obj_set_height(ui_edit_textarea_vareditinput, LV_SIZE_CONTENT);    /// 70
    lv_obj_set_align(ui_edit_textarea_vareditinput, LV_ALIGN_CENTER);
    if("-+0123456789." == "") lv_textarea_set_accepted_chars(ui_edit_textarea_vareditinput, NULL);
    else lv_textarea_set_accepted_chars(ui_edit_textarea_vareditinput, "-+0123456789.");
    lv_textarea_set_max_length(ui_edit_textarea_vareditinput, 20);
    lv_textarea_set_text(ui_edit_textarea_vareditinput, "0");
    lv_textarea_set_placeholder_text(ui_edit_textarea_vareditinput, "Value...");
    lv_textarea_set_one_line(ui_edit_textarea_vareditinput, true);



    ui_edit_container_footeredit = lv_obj_create(ui_edit);
    lv_obj_remove_style_all(ui_edit_container_footeredit);
    lv_obj_set_width(ui_edit_container_footeredit, lv_pct(100));
    lv_obj_set_flex_grow(ui_edit_container_footeredit, 1);
    lv_obj_set_align(ui_edit_container_footeredit, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_edit_container_footeredit, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_edit_keyboard_keyboardedit = lv_keyboard_create(ui_edit_container_footeredit);
    lv_keyboard_set_mode(ui_edit_keyboard_keyboardedit, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_set_width(ui_edit_keyboard_keyboardedit, lv_pct(100));
    lv_obj_set_height(ui_edit_keyboard_keyboardedit, lv_pct(100));
    lv_obj_set_align(ui_edit_keyboard_keyboardedit, LV_ALIGN_CENTER);

    lv_obj_add_event_cb(ui_edit_button_btnclose, ui_event_edit_button_btnclose, LV_EVENT_ALL, NULL);
    lv_keyboard_set_textarea(ui_edit_keyboard_keyboardedit, ui_edit_textarea_vareditinput);
    lv_obj_add_event_cb(ui_edit_keyboard_keyboardedit, ui_event_edit_keyboard_keyboardedit, LV_EVENT_ALL, NULL);

}