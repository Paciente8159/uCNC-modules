// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.2
// LVGL version: 9.1.0
// Project name: uCNC-GFX

#ifndef _UI_COMP_HEADERTEMPLATE_H
#define _UI_COMP_HEADERTEMPLATE_H

#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

// COMPONENT HeaderTemplate
#define UI_COMP_CONTAINER_HEADERTEMPLATE_CONTAINER_HEADERTEMPLATE 0
#define UI_COMP_CONTAINER_HEADERTEMPLATE_BUTTON_BTNHOME 1
#define UI_COMP_CONTAINER_HEADERTEMPLATE_BUTTON_BTNHOME_LABEL_BTNHOMELABEL 2
#define UI_COMP_CONTAINER_HEADERTEMPLATE_CONTAINER_STATUSINFO 3
#define UI_COMP_CONTAINER_HEADERTEMPLATE_CONTAINER_STATUSINFO_LABEL_STATUSLABEL 4
#define UI_COMP_CONTAINER_HEADERTEMPLATE_CONTAINER_STATUSINFO_LABEL_STATUSVALUE 5
#define UI_COMP_CONTAINER_HEADERTEMPLATE_BUTTON_BTNHOLDRES 6
#define UI_COMP_CONTAINER_HEADERTEMPLATE_BUTTON_BTNHOLDRES_LABEL_BTNHOLDRESLABEL 7
#define _UI_COMP_CONTAINER_HEADERTEMPLATE_NUM 8
lv_obj_t * ui_container_headertemplate_create(lv_obj_t * comp_parent);
void ui_event_comp_container_headertemplate_button_btnhome(lv_event_t * e);
void ui_event_comp_container_headertemplate_button_btnholdres(lv_event_t * e);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
