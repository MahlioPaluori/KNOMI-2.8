#ifndef DISPLAY_WAVESHARE_H
#define DISPLAY_WAVESHARE_H

#include <Arduino.h>
#include <lvgl.h>

void display_waveshare_init(void);
void display_waveshare_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void display_waveshare_set_brightness(int8_t level);
bool display_waveshare_read_touch(uint16_t *x, uint16_t *y);

#endif
