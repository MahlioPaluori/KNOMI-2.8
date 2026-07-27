#ifndef UI_ROOT_LAYOUT_H
#define UI_ROOT_LAYOUT_H

#include <lvgl.h>

void ui_root_layout_apply_all(void);
lv_obj_t *ui_root_layout_get_app_container(lv_obj_t *screen);
lv_obj_t *ui_root_layout_get_bottom_container(lv_obj_t *screen);

#endif
