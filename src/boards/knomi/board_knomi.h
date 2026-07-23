#ifndef BOARD_KNOMI_H
#define BOARD_KNOMI_H

#include "boards/board_layer.h"

void board_knomi_init(void);
void board_knomi_display_init(void);
void board_knomi_touch_init(void);
void board_knomi_backlight_init(void);
void board_knomi_backlight_set(int8_t level);
bool board_knomi_touch_read(uint16_t *x, uint16_t *y);
board_info_t board_knomi_get_info(void);
int16_t board_knomi_battery_percent(void);

#endif
