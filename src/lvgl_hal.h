#ifndef LVGL_HAL_H
#define LVGL_HAL_H

#include <Arduino.h>
#include "lvgl.h"

void lvgl_hal_init(void);
void tft_set_backlight(int8_t light);

#endif
