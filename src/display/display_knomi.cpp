#include "display_knomi.h"
#include "pinout.h"

#ifndef WAVESHARE28C

#include <TFT_eSPI.h>
#include <CST816S.h>

extern TwoWire i2c0;

namespace {

TFT_eSPI tft_gc9a01 = TFT_eSPI();

#ifdef CST816S_SUPPORT
CST816S ts_cst816s = CST816S(CST816S_RST_PIN, CST816S_IRQ_PIN, &i2c0);
#endif

int8_t aw9346_from_light = -1;

void tft_backlight_init(void) {
    pinMode(LCD_BL_PIN, OUTPUT);
    digitalWrite(LCD_BL_PIN, LOW);
    delay(3);
    aw9346_from_light = 0;
}

void tft_set_backlight(int8_t aw9346_to_light) {
    if (aw9346_to_light > 16) aw9346_to_light = 16;
    if (aw9346_to_light < 0) aw9346_to_light = 0;
    if (aw9346_from_light == aw9346_to_light) return;

    if (aw9346_to_light == 0) {
        digitalWrite(LCD_BL_PIN, LOW);
        delay(3);
        aw9346_from_light = 0;
        return;
    }
    if (aw9346_from_light <= 0) {
        digitalWrite(LCD_BL_PIN, HIGH);
        delayMicroseconds(25);
        aw9346_from_light = 16;
    }

    if (aw9346_from_light < aw9346_to_light)
        aw9346_from_light += 16;

    int8_t num = aw9346_from_light - aw9346_to_light;
    for (int8_t i = 0; i < num; i++) {
        digitalWrite(LCD_BL_PIN, LOW);
        delayMicroseconds(1);
        digitalWrite(LCD_BL_PIN, HIGH);
        delayMicroseconds(1);
    }

    aw9346_from_light = aw9346_to_light;
}

} // namespace

void display_knomi_init(void) {
#ifdef CST816S_SUPPORT
    ts_cst816s.begin();
    ts_cst816s.setReportRate(2);
    ts_cst816s.setReportMode(0x60);
    ts_cst816s.setMotionMask(0);
    ts_cst816s.setAutoRst(0);
    ts_cst816s.setLongRst(0);
    ts_cst816s.setDisAutoSleep(1);
#endif

    tft_gc9a01.begin();
    tft_gc9a01.invertDisplay(1);
    tft_gc9a01.fillScreen(TFT_BLACK);
    tft_backlight_init();
    delay(50);
    tft_set_backlight(16);
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
    tft_set_backlight(level);
}

bool display_knomi_read_touch(uint16_t *x, uint16_t *y) {
#ifdef CST816S_SUPPORT
    static touch_event_t event;
    if (ts_cst816s.ready()) {
        ts_cst816s.getTouch(&event);
    }
    if (event.finger) {
        if (x) *x = event.x;
        if (y) *y = event.y;
        return true;
    }
#endif
    if (x) *x = 0;
    if (y) *y = 0;
    return false;
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

#endif // !WAVESHARE28C
