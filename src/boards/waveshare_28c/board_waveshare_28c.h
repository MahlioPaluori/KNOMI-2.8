#ifndef BOARD_WAVESHARE_28C_H
#define BOARD_WAVESHARE_28C_H

#include "boards/board_layer.h"

void board_waveshare_28c_init(void);
void board_waveshare_28c_display_init(void);
void board_waveshare_28c_touch_init(void);
void board_waveshare_28c_backlight_init(void);
void board_waveshare_28c_backlight_set(int8_t level);
bool board_waveshare_28c_touch_read(uint16_t *x, uint16_t *y);
board_info_t board_waveshare_28c_get_info(void);
int16_t board_waveshare_28c_battery_percent(void);

#endif
