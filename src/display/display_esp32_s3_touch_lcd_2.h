#ifndef DISPLAY_ESP32_S3_TOUCH_LCD_2_H
#define DISPLAY_ESP32_S3_TOUCH_LCD_2_H

#include <Arduino.h>
#include <lvgl.h>

void display_esp32_s3_touch_lcd_2_init(void);
void display_esp32_s3_touch_lcd_2_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void display_esp32_s3_touch_lcd_2_set_brightness(int8_t level);
bool display_esp32_s3_touch_lcd_2_read_touch(uint16_t *x, uint16_t *y);

#endif
