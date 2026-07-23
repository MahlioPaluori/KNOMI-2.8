#ifndef BOARD_ESP32_S3_TOUCH_LCD_2_H
#define BOARD_ESP32_S3_TOUCH_LCD_2_H

#include "boards/board_layer.h"

void board_esp32_s3_touch_lcd_2_init(void);
void board_esp32_s3_touch_lcd_2_display_init(void);
void board_esp32_s3_touch_lcd_2_touch_init(void);
void board_esp32_s3_touch_lcd_2_backlight_init(void);
void board_esp32_s3_touch_lcd_2_backlight_set(int8_t level);
bool board_esp32_s3_touch_lcd_2_touch_read(uint16_t *x, uint16_t *y);
board_info_t board_esp32_s3_touch_lcd_2_get_info(void);
int16_t board_esp32_s3_touch_lcd_2_battery_percent(void);

#endif
