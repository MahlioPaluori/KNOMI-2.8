#include "display_knomi.h"
#include "pinout.h"
#include "boards/board_layer.h"

#if !defined(WAVESHARE28C) && !defined(ESP32_S3_TOUCH_LCD_2)

#include <TFT_eSPI.h>

namespace {

TFT_eSPI tft_gc9a01 = TFT_eSPI();

} // namespace

void display_knomi_init(void) {
    board_display_init();

#ifdef CST816S_SUPPORT
    board_touch_init();
#endif

    tft_gc9a01.begin();
    tft_gc9a01.invertDisplay(1);
    tft_gc9a01.fillScreen(TFT_BLACK);
    board_backlight_init();
    delay(50);
    board_backlight_set(16);
}

void display_knomi_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft_gc9a01.startWrite();
    tft_gc9a01.setAddrWindow(area->x1, area->y1, w, h);
    tft_gc9a01.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft_gc9a01.endWrite();

    lv_disp_flush_ready(disp);
}

void display_knomi_set_brightness(int8_t level) {
    board_backlight_set(level);
}

bool display_knomi_read_touch(uint16_t *x, uint16_t *y) {
    return board_touch_read(x, y);
}

void tft_fps_test(void) {
    uint8_t test_sec = 3;
    uint32_t ms = millis() + test_sec * 1000;
    uint32_t frames = 0;
    const uint32_t test_colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE};
    while(ms > millis()) {
        tft_gc9a01.fillScreen(test_colors[frames % (sizeof(test_colors) / sizeof(test_colors[0]))]);
        frames++;
    }
    Serial.println("\r\n******** lcd fps test *****\r\n");
    Serial.print("fps=");
    Serial.println(frames / test_sec, DEC);
    Serial.print("test_sec=");
    Serial.println(test_sec, DEC);
    Serial.println("\r\n***************************\r\n");
}

#endif // !WAVESHARE28C && !ESP32_S3_TOUCH_LCD_2
