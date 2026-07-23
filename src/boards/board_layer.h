#ifndef BOARD_LAYER_H
#define BOARD_LAYER_H

#include <Arduino.h>

enum class LayoutType {
    Square240,
    Portrait240x320,
};

struct board_info_t {
    const char *name;
    uint16_t width;
    uint16_t height;
    bool has_touch;
    bool has_battery;
    LayoutType layout;
};

void board_init(void);
void board_display_init(void);
void board_touch_init(void);
void board_backlight_init(void);
void board_backlight_set(int8_t level);
bool board_touch_read(uint16_t *x, uint16_t *y);
board_info_t board_get_info(void);

// Battery interface stub for boards that do not expose battery telemetry yet.
int16_t board_battery_percent(void);

#endif
