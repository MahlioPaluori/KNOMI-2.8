#ifndef DISPLAY_KNOMI_H
#define DISPLAY_KNOMI_H

#include <Arduino.h>
#include <lvgl.h>

void display_knomi_init(void);
void display_knomi_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void display_knomi_set_brightness(int8_t level);
bool display_knomi_read_touch(uint16_t *x, uint16_t *y);

#endif
