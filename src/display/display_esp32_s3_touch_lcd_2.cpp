#include "display_esp32_s3_touch_lcd_2.h"
#include "pinout.h"
#include "boards/board_layer.h"

#ifdef ESP32_S3_TOUCH_LCD_2

#include <TFT_eSPI.h>

namespace {
TFT_eSPI tft = TFT_eSPI();
}

void display_esp32_s3_touch_lcd_2_init(void) {
    board_display_init();

    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    board_touch_init();

    board_backlight_init();
    board_backlight_set(16);
}

void display_esp32_s3_touch_lcd_2_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = static_cast<uint32_t>(area->x2 - area->x1 + 1);
    uint32_t h = static_cast<uint32_t>(area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

void display_esp32_s3_touch_lcd_2_set_brightness(int8_t level) {
    board_backlight_set(level);
}

bool display_esp32_s3_touch_lcd_2_read_touch(uint16_t *x, uint16_t *y) {
    return board_touch_read(x, y);
}

#endif
