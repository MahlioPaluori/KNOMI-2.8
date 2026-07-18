#ifndef DISPLAY_HAL_H
#define DISPLAY_HAL_H

#include <Arduino.h>
#include <lvgl.h>

void display_init(void);
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void display_set_brightness(int8_t level);
bool display_read_touch(uint16_t *x, uint16_t *y);

#endif
